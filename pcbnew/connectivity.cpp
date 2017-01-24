#include "connectivity.h"
#include "connectivity_impl.h"
#include "ratsnest_data.h"

#ifdef USE_OPENMP
#include <omp.h>
#endif /* USE_OPENMP */


CN_CONNECTIVITY_ALGO::CN_CONNECTIVITY_ALGO()
{
    m_pimpl.reset( new CN_CONNECTIVITY_ALGO_IMPL );
}

CN_CONNECTIVITY_ALGO::~CN_CONNECTIVITY_ALGO()
{

}

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

bool CN_CONNECTIVITY_ALGO::Add ( BOARD_ITEM* aItem )
{
#ifdef CONNECTIVITY_DEBUG
    dumpAdd(aItem);
#endif
    return m_pimpl->Add (aItem);
}

bool CN_CONNECTIVITY_ALGO::Remove ( BOARD_ITEM* aItem)
{
#ifdef CONNECTIVITY_DEBUG
    dumpRemove(aItem);
#endif
    return m_pimpl->Remove(aItem);
}

void CN_CONNECTIVITY_ALGO::Update ( BOARD_ITEM* aItem)
{
    //  printf("CN_UPD %p\n", aItem);
#ifdef CONNECTIVITY_DEBUG
    dumpRemove(aItem);
#endif
    m_pimpl->Remove(aItem);
#ifdef CONNECTIVITY_DEBUG
    dumpAdd(aItem);
#endif
    m_pimpl->Add (aItem);
}

bool CN_CONNECTIVITY_ALGO::CheckConnectivity( std::vector<CN_DISJOINT_NET_ENTRY>& aReport )
{
    return m_pimpl->CheckConnectivity( aReport );
}

void CN_CONNECTIVITY_ALGO::Build( BOARD* aBoard )
{
    m_pimpl->Build(aBoard);
}

void CN_CONNECTIVITY_ALGO::Build( const std::vector<BOARD_ITEM *> &aItems )
{
    m_pimpl->Build(aItems);
}


void CN_CONNECTIVITY_ALGO::PropagateNets()
{
    m_pimpl->PropagateNets();
}

int CN_CONNECTIVITY_ALGO::GetUnconnectedCount()
{
    return m_pimpl->GetUnconnectedCount();
}


void CN_CONNECTIVITY_ALGO::FindIsolatedCopperIslands( ZONE_CONTAINER* aZone, std::vector<int>& aIslands )
{
    m_pimpl->FindIsolatedCopperIslands( aZone, aIslands );
}

const CN_CONNECTIVITY_ALGO::CLUSTERS& CN_CONNECTIVITY_ALGO::GetClusters()
{
    return m_pimpl->GetClusters();
}

bool CN_CONNECTIVITY_ALGO::IsNetDirty( int aNet) const
{
    return m_pimpl->IsNetDirty( aNet );
}

void CN_CONNECTIVITY_ALGO::ClearDirtyFlags()
{
    m_pimpl->ClearDirtyFlags();
}

void CN_CONNECTIVITY_ALGO::GetDirtyClusters( CLUSTERS& aClusters )
{
    m_pimpl->GetDirtyClusters( aClusters );
}

int CN_CONNECTIVITY_ALGO::NetCount() const
{
    return m_pimpl->NetCount();
}

CONNECTIVITY_DATA::CONNECTIVITY_DATA( BOARD* aBoard )
{
    m_board = aBoard;

    m_connAlgo = new CN_CONNECTIVITY_ALGO;
    m_ratsnest = new RN_DATA ( m_board );
}


bool CONNECTIVITY_DATA::Add( BOARD_ITEM* aItem )
{
    printf("Cn: add %p\n", aItem);
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
    m_connAlgo->Update( aItem );
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
                            m_ratsnest->AddCluster( c );

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
void CONNECTIVITY_DATA::ProcessBoard()
{
    delete m_connAlgo;//->Clear();

    m_connAlgo = new CN_CONNECTIVITY_ALGO; // fixme: clearnup!
    m_connAlgo->Build( m_board ); // fixme: do we need it here?


    RecalculateRatsnest();
    //m_ratsnest->ProcessBoard();
}

void CONNECTIVITY_DATA::RecalculateRatsnest()
{
    int lastNet = m_connAlgo->NetCount();

    if( lastNet >= (int) m_nets.size() )
    {
        int prevSize = m_nets.size();
        m_nets.resize( lastNet + 1 );

        printf("Creating %d nets\n", m_nets.size() + 1);

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
            printf("net %d: add cluster %p\n", net, c.get());
            m_nets[net]->AddCluster( c );
        }
    }

    m_connAlgo->ClearDirtyFlags();

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
                        m_nets[i]->Update();
                        printf("Unconnected: %d\n", m_nets[i]->GetUnconnected()->size());
                    }
                }
            }  /* end of parallel section */
    #ifdef PROFILE
        prof_end( &totalRealTime );
        wxLogDebug( wxT( "Recalculate all nets: %.1f ms" ), totalRealTime.msecs() );
    #endif /* PROFILE */



    //m_ratsnest->Recalculate();
}
