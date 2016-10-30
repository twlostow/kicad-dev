#include "connectivity.h"
#include "connectivity_impl.h"
#include "ratsnest_data.h"


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

void CN_CONNECTIVITY_ALGO::SetBoard( BOARD* aBoard )
{
    m_pimpl->SetBoard(aBoard);
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

CONNECTIVITY_DATA::CONNECTIVITY_DATA( BOARD* aBoard )
{
    m_board = aBoard;

    m_connAlgo = new CN_CONNECTIVITY_ALGO;
    m_ratsnest = new RN_DATA ( m_board );
}


bool CONNECTIVITY_DATA::Add( BOARD_ITEM* aItem )
{
    m_connAlgo->Add ( aItem );
    return m_ratsnest->Add ( aItem );
}

bool CONNECTIVITY_DATA::Remove( BOARD_ITEM* aItem )
{
    m_connAlgo->Remove ( aItem );
    return m_ratsnest->Remove ( aItem );
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
    return m_ratsnest->Update ( aItem );
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
    m_connAlgo->SetBoard( m_board ); // fixme: do we need it here?
    m_ratsnest->ProcessBoard();
}

void CONNECTIVITY_DATA::RecalculateRatsnest()
{
    m_ratsnest->Recalculate();
}
