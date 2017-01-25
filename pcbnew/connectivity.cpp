#define PROFILE

#include "connectivity.h"
#include "connectivity_algo.h"
#include "ratsnest_data.h"


#ifdef USE_OPENMP
#include <omp.h>
#endif /* USE_OPENMP */



static std::vector<BOARD_ITEM*> items;
static FILE *f_log = nullptr;
//static int n_cur = 0;

static void dumpAdd(BOARD_ITEM *aItem)
{
    if(!f_log)
        f_log=fopen("conn_log.txt","wb");
    if(aItem->Type() == PCB_TRACE_T)
    {
        auto t= static_cast<TRACK*>(aItem);
        int n_cur = items.size();
        items.push_back(t);
        fprintf(f_log, "auto t%d = new TRACK(nullptr); t%d->SetStart( wxPoint(%d, %d)); t%d->SetEnd(wxPoint(%d,%d)); t%d->SetLayer((LAYER_ID)%d); t%d->SetWidth(%d); addItem(t%d);\n", n_cur, n_cur, t->GetStart().x, t->GetStart().y, n_cur, t->GetEnd().x, t->GetEnd().y, n_cur, t->GetLayer(), n_cur, t->GetWidth(), n_cur);
        printf("**** auto t%d = new TRACK(nullptr); t%d->SetStart( wxPoint(%d, %d)); t%d->SetEnd(wxPoint(%d,%d)); t%d->SetLayer((LAYER_ID)%d); t%d->SetWidth(%d); addItem(t%d);\n", n_cur, n_cur, t->GetStart().x, t->GetStart().y, n_cur, t->GetEnd().x, t->GetEnd().y, n_cur, t->GetLayer(), n_cur, t->GetWidth(), n_cur);
        fflush(f_log);
    }

}

static void dumpRemove(BOARD_ITEM *aItem)
{
    int n_cur = -1;
//    printf("CN_REM %p\n", aItem);
    for (int i =0 ;i<items.size();i++)
    {
            if( items[i] == aItem )
            {
                n_cur = i; break;
            }
    }

    if(n_cur >= 0)
    {
        fprintf(f_log, "removeItem(t%d);\n", n_cur);
        fflush(f_log);
    }
}


CONNECTIVITY_DATA::CONNECTIVITY_DATA( )
{

    m_connAlgo = new CN_CONNECTIVITY_ALGO;
    //m_ratsnest = new RN_DATA ( m_board );
}


bool CONNECTIVITY_DATA::Add( BOARD_ITEM* aItem )
{
    //printf("Cn: add %p\n", aItem);
    m_connAlgo->Add ( aItem );
    return true;
    //return m_ratsnest->Add ( aItem );
}

bool CONNECTIVITY_DATA::Remove( BOARD_ITEM* aItem )
{
    m_connAlgo->Remove ( aItem );
    return true;
    //return m_ratsnest->Remove ( aItem );
}

/**
 * Function Update()
 * Updates the connectivity data for an item.
 * @param aItem is an item to be updated.
 * @return True if operation succeeded. The item will not be updated if it was not previously
 * added to the ratsnest.
 */
bool CONNECTIVITY_DATA::Update( BOARD_ITEM* aItem )
{
    m_connAlgo->Remove( aItem );
    m_connAlgo->Add( aItem );
#if 0

    printf("Update %p\n", aItem);


    for ( int i = 0; i < m_connAlgo->NetCount(); i++ )
    {
            if ( m_connAlgo->IsNetDirty( i ) )
            {
                    printf("update Dirty net %d\n");

                    auto& clusters = m_connAlgo->GetClusters();

                    m_ratsnest->ClearNet( i );

                    for( auto c : clusters )
                        if ( c->OriginNet() == i )
                            m_ratsnest->ter( c );

                    m_ratsnest->Recalculate( i );

            }

    }

    return true;
    //return m_ratsnest->Update ( aItem );
#endif
}

/**
 * Function ProcessBoard()
 * Prepares data for computing (computes a list of current nodes and connections). It is
 * required to run only once after loading a board.
 */
void CONNECTIVITY_DATA::Build( BOARD *aBoard )
{
    delete m_connAlgo;//->Clear();

    m_connAlgo = new CN_CONNECTIVITY_ALGO; // fixme: clearnup!
    m_connAlgo->Build( aBoard ); // fixme: do we need it here?


    RecalculateRatsnest();
}

void CONNECTIVITY_DATA::Build( const std::vector<BOARD_ITEM *>& aItems )
{
    delete m_connAlgo;//->Clear();

    m_connAlgo = new CN_CONNECTIVITY_ALGO; // fixme: clearnup!
    m_connAlgo->Build( aItems ); // fixme: do we need it here?

    RecalculateRatsnest();
}


//void CONNECTIVITY_DATA::Block()
//{

//}

void CONNECTIVITY_DATA::updateRatsnest()
{
    int lastNet =  m_connAlgo->NetCount();

    #ifdef PROFILE
        prof_counter totalRealTime;
        prof_start( &totalRealTime );
    #endif

    unsigned int i;

    #ifdef USE_OPENMP
            #pragma omp parallel shared(lastNet) private(i)
            {
                #pragma omp for schedule(guided, 1)
    #else /* USE_OPENMP */
            {
    #endif
                // Start with net number 1, as 0 stands for not connected
                for( i = 1; i < lastNet; ++i )
                {
                    if( m_nets[i]->IsDirty() )
                    {
            //            printf("redo %d\n", i);
                        m_nets[i]->Update();
                        //printf("Unconnected: %d\n", m_nets[i]->GetUnconnected()->size());
                    }
                }
            }  /* end of parallel section */
    #ifdef PROFILE
        prof_end( &totalRealTime );
        wxLogDebug( wxT( "Recalculate all nets: %.1f ms" ), totalRealTime.msecs() );
    #endif /* PROFILE */
    //m_ratsnest->Recalculate();
}

void CONNECTIVITY_DATA::addRatsnestCluster( std::shared_ptr<CN_CLUSTER> aCluster )
{
    auto rnNet = m_nets[ aCluster->OriginNet() ];
    RN_NODE_PTR firstNode;

    for ( auto item : *aCluster )
    {
        auto& entry = m_connAlgo->ItemEntry( item->Parent() );

        for (int i = 0; i < item->AnchorCount(); i++)
        {
            auto anchor = item->GetAnchor( i );
            auto node = rnNet->AddNode ( anchor.x, anchor.y );

            if ( firstNode  )
            {
                if ( firstNode != node )
                {
                    rnNet->AddConnection( firstNode, node );
                }
            } else {
                firstNode = node;
            }

            entry.AddRatsnestNode ( node );
        }
    }
#if 0

    auto start = m_links.AddNode( anchors[0].x, anchors[0].y );

    //aCluster->ClearRatsnestNodes();
    //aCluster->AddRatsnestNode( start );

    for(int i = 1; i < anchors.size(); i++ )
    {

        auto end = m_links.AddNode( anchors[i].x, anchors[i].y );

    //    aCluster->AddRatsnestNode( end );

        if(start != end)
            auto conn = m_links.AddConnection( start, end );
    }
    m_dirty = true;
#endif
}


void CONNECTIVITY_DATA::RecalculateRatsnest()
{
    int lastNet = m_connAlgo->NetCount();

    if( lastNet >= (int) m_nets.size() )
    {
        int prevSize = m_nets.size();
        m_nets.resize( lastNet + 1 );

        //printf("Creating %d nets\n", m_nets.size() + 1);

        for ( int i = prevSize; i < m_nets.size(); i++ )
            m_nets[i] = new RN_NET;
    }

    PROF_COUNTER cnt_clusters("build-clusters");
    auto clusters = m_connAlgo->GetClusters();
    cnt_clusters.show();
    printf("Found %d clusters\n", clusters.size());
    int dirtyNets = 0;

    for (int net = 0; net < lastNet; net++)
        if ( m_connAlgo->IsNetDirty(net) )
        {
            m_nets[net]->Clear();
            dirtyNets++;
        }

    for( auto c : clusters )
    {
        int net  = c->OriginNet();
        if ( m_connAlgo->IsNetDirty( net ) )
        {
            //printf("net %d: add cluster %p\n", net, c.get());
            addRatsnestCluster( c );
        }
    }

    m_connAlgo->ClearDirtyFlags();

    updateRatsnest();

}

void CONNECTIVITY_DATA::BlockRatsnestItems( const std::vector<BOARD_ITEM *> &aItems )
{
    std::vector<BOARD_CONNECTED_ITEM*> citems;

    for ( auto item : aItems )
    {
        if ( item->Type() == PCB_MODULE_T )
        {
            for ( auto pad : static_cast<MODULE *>(item) -> PadsIter() )
                citems.push_back(pad);
        } else {
            citems.push_back(static_cast<BOARD_CONNECTED_ITEM*>(item));
        }
    }

    for ( auto item : citems )
    {
        auto& entry = m_connAlgo->ItemEntry ( item );

        for( auto node : entry.RatsnestNodes() )
        {
            node->SetNoLine(true);
        }
    }
}

int CONNECTIVITY_DATA::GetNetCount() const
{
    return m_connAlgo->NetCount();
}

void CONNECTIVITY_DATA::FindIsolatedCopperIslands( ZONE_CONTAINER* aZone, std::vector<int>& aIslands )
{
    m_connAlgo -> FindIsolatedCopperIslands( aZone, aIslands );
}

void CONNECTIVITY_DATA::ComputeDynamicRatsnest( CONNECTIVITY_DATA *aMovedItems )
{

    m_dynamicRatsnest.clear();
    for (int nc = 1; nc < aMovedItems->m_nets.size(); nc ++ )
    {
        auto dynNet = aMovedItems->m_nets[nc];

        if ( dynNet->GetNodeCount() != 0 )
        {
            auto ourNet = m_nets[nc];
            RN_NODE_PTR nodeA, nodeB;

            if ( ourNet->NearestBicoloredPair( *dynNet, nodeA, nodeB ) )
            {
                RN_DYNAMIC_LINE l;
                l.a = VECTOR2I ( nodeA->GetX(), nodeA->GetY() );
                l.b = VECTOR2I ( nodeB->GetX(), nodeB->GetY() );
                l.netCode = nc;

                m_dynamicRatsnest.push_back ( l );
            }

        }
    }

    for ( auto net : aMovedItems->m_nets )
    {
        if ( !net )
            continue;

        const auto edges = net->GetUnconnected();

        if (!edges)
            continue;

        for( const auto& edge : *edges )
        {
            const auto& nodeA = edge->GetSourceNode();
            const auto& nodeB = edge->GetTargetNode();
            RN_DYNAMIC_LINE l;

            l.a = VECTOR2I ( nodeA->GetX(), nodeA->GetY() );
            l.b = VECTOR2I ( nodeB->GetX(), nodeB->GetY() );
            l.netCode = 0;
            m_dynamicRatsnest.push_back ( l );

        }
    }
}

const std::vector<RN_DYNAMIC_LINE> & CONNECTIVITY_DATA::GetDynamicRatsnest() const
{
    return m_dynamicRatsnest;
}

void CONNECTIVITY_DATA::ClearDynamicRatsnest()
{
    m_dynamicRatsnest.clear();
}
