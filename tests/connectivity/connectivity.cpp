#include "connectivity.h"
#include "connectivity_impl.h"

CN_CONNECTIVITY::CN_CONNECTIVITY()
{
    m_pimpl.reset( new CN_CONNECTIVITY_IMPL );
}

CN_CONNECTIVITY::~CN_CONNECTIVITY()
{

}

void CN_CONNECTIVITY::Add ( BOARD_CONNECTED_ITEM* aItem )
{
    m_pimpl->Add (aItem);
}

void CN_CONNECTIVITY::Remove ( BOARD_CONNECTED_ITEM* aItem)
{
    m_pimpl->Remove(aItem);
}

void CN_CONNECTIVITY::Update ( BOARD_CONNECTED_ITEM* aItem)
{
    m_pimpl->Remove(aItem);
    m_pimpl->Add (aItem);
}

void CN_CONNECTIVITY::SetBoard( BOARD* aBoard )
{
    m_pimpl->SetBoard(aBoard);
}

void CN_CONNECTIVITY::PropagateNets()
{
    m_pimpl->PropagateNets();
}

void CN_CONNECTIVITY::FindIsolatedCopperIslands( ZONE_CONTAINER* aZone, std::vector<int>& aIslands )
{
    m_pimpl->FindIsolatedCopperIslands( aZone, aIslands );
}

const CN_CONNECTIVITY::CLUSTERS& CN_CONNECTIVITY::GetClusters()
{
    return m_pimpl->GetClusters();
}
