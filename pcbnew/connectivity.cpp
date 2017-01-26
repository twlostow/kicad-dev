#define PROFILE

#include "connectivity.h"
#include "connectivity_algo.h"
#include "ratsnest_data.h"


#ifdef USE_OPENMP
#include <omp.h>
#endif /* USE_OPENMP */

CONNECTIVITY_DATA::CONNECTIVITY_DATA( )
{
    m_connAlgo.reset( new CN_CONNECTIVITY_ALGO );
}


bool CONNECTIVITY_DATA::Add( BOARD_ITEM* aItem )
{
    m_connAlgo->Add ( aItem );
    return true;
}

bool CONNECTIVITY_DATA::Remove( BOARD_ITEM* aItem )
{
    m_connAlgo->Remove ( aItem );
    return true;
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
    return true;
}

/**
 * Function ProcessBoard()
 * Prepares data for computing (computes a list of current nodes and connections). It is
 * required to run only once after loading a board.
 */
void CONNECTIVITY_DATA::Build( BOARD *aBoard )
{
    m_connAlgo.reset( new CN_CONNECTIVITY_ALGO ); // fixme: clearnup!
    m_connAlgo->Build( aBoard ); // fixme: do we need it here?

    RecalculateRatsnest();
}

void CONNECTIVITY_DATA::Build( const std::vector<BOARD_ITEM *>& aItems )
{

    m_connAlgo.reset( new CN_CONNECTIVITY_ALGO ); // fixme: clearnup!
    m_connAlgo->Build( aItems ); // fixme: do we need it here?

    RecalculateRatsnest();
}

void CONNECTIVITY_DATA::updateRatsnest()
{
    int lastNet =  m_connAlgo->NetCount();

    #ifdef PROFILE
        PROF_COUNTER rnUpdate ( "update-ratsnest" );
    #endif

    int i;

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
        rnUpdate.Show();
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
}

void CONNECTIVITY_DATA::RecalculateRatsnest()
{
    int lastNet = m_connAlgo->NetCount();

    if( lastNet >= (int) m_nets.size() )
    {
        unsigned int prevSize = m_nets.size();
        m_nets.resize( lastNet + 1 );

        for ( unsigned int i = prevSize; i < m_nets.size(); i++ )
            m_nets[i] = new RN_NET;
    }

    PROF_COUNTER cnt_clusters("build-clusters");
    auto clusters = m_connAlgo->GetClusters();
    cnt_clusters.Show();

    printf("Found %d clusters\n", clusters.size());

    int dirtyNets = 0;

    for (int net = 0; net < lastNet; net++)
        if ( m_connAlgo->IsNetDirty(net) )
        {
            m_nets[net]->Clear();
            dirtyNets++;
        }

    printf("Dirty nets after cl: %d\n", dirtyNets);

    for( auto c : clusters )
    {
        int net  = c->OriginNet();
        if ( m_connAlgo->IsNetDirty( net ) )
        {
            addRatsnestCluster( c );
        }
    }

    m_connAlgo->ClearDirtyFlags();

    updateRatsnest();
}

void CONNECTIVITY_DATA::blockRatsnestItems( const std::vector<BOARD_ITEM *> &aItems )
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

void CONNECTIVITY_DATA::ComputeDynamicRatsnest( const std::vector<BOARD_ITEM *> &aItems )
{
    m_dynamicConnectivity.reset( new CONNECTIVITY_DATA );
    m_dynamicConnectivity->Build ( aItems );

    m_dynamicRatsnest.clear();

    blockRatsnestItems ( aItems );

    for (unsigned int nc = 1; nc < m_dynamicConnectivity->m_nets.size(); nc ++ )
    {
        auto dynNet = m_dynamicConnectivity->m_nets[nc];

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

    for ( auto net : m_dynamicConnectivity->m_nets )
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
    m_dynamicConnectivity.reset();
    m_dynamicRatsnest.clear();
}

void CONNECTIVITY_DATA::PropagateNets()
{
    m_connAlgo->PropagateNets();
}
