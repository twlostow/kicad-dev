#include <class_board.h>
#include <class_pad.h>
#include <class_module.h>
#include <class_zone.h>

#include <geometry/shape_poly_set.h>

#include <io_mgr.h>
#include <profile.h>

#include <memory>
#include <algorithm>
#include <functional>
#include <vector>
#include <deque>
#include <stdarg.h>

using namespace std;
/**
 * Function SortPadsByXCoord
 * is used by GetSortedPadListByXCoord to Sort a pad list by x coordinate value.
 * This function is used to build ordered pads lists
 */


template <class T>
class IntrusiveList
{
public:
    IntrusiveList<T>()
    {
        m_prev  = nullptr;
        m_next  = nullptr;
        m_root  = (T*) this;
        m_count = 1;
    }

    T* ListRemove()
    {
        // printf("prev %p next %p\n", m_prev, m_next);
        if( m_prev )
            m_prev->m_next = m_next;

        if( m_next )
            m_next->m_prev = m_prev;

        m_root->m_count--;

        T *rv=  nullptr;

        if( m_prev )
            rv = m_prev;
        else if( m_next )
            rv = m_next;

        m_root = nullptr;
        m_prev = nullptr;
        m_next = nullptr;
        return rv;
    }

    int ListSize() const { return m_root ? m_root->m_count : 0; }

    void ListInsert( T* item )
    {
        if( !m_root )
            m_root = item;

        if( m_next )
            m_next->m_prev = item;

        item->m_prev    = (T*) this;
        item->m_next    = m_next;
        item->m_root    = m_root;
        m_root->m_count++;

        m_next = item;
    }

    T* ListNext() const { return m_next; };
    T* ListPrev() const { return m_prev; };

private:
    int m_count;
    T* m_prev, * m_next, * m_root;
    // IntrusiveListBase<T> *m_list;
};

struct CnItem : public IntrusiveList<CnItem>
{
    BOARD_CONNECTED_ITEM* m_item;
    vector<CnItem*> m_connected;

    bool    m_visited;
    bool    m_new;
    bool    m_canChangeNet;

    CnItem()
    {
        m_new = true;
        m_canChangeNet = false;
        m_visited = false;
        m_item = nullptr;
    }

    void Connect( CnItem* aOther )
    {
        m_connected.push_back( aOther );
    }
};

struct CnPoint
{
public:
    CnPoint()
    {
        m_valid = true;
    }

    bool m_valid;
    VECTOR2I    m_pos;
    CnItem*     m_item;
};


static bool sortByXthenYCoord( const CnPoint& ref, const CnPoint& comp )
{
    if( ref.m_pos.x == comp.m_pos.x )
        return ref.m_pos.y < comp.m_pos.y;

    return ref.m_pos.x < comp.m_pos.x;
}


struct CnList
{
    bool m_dirty;
    vector<CnItem*> m_items;
    vector<CnPoint> m_anchors;

    CnList()
    {
        m_dirty = false;
    };    // shared_ptr<BOARD> aBoard ) : m_board(aBoard){};

    void setDirty( bool aDirty = true ) { m_dirty = aDirty; }

    template <class T>
    void FindNearby( VECTOR2I aPosition, int aDistMax, T aFunc );

    template <class T>
    void FindNearby( BOX2I aBBox, T aFunc );

    void ClearConnections()
    {
        for ( auto anchor : m_anchors )
            anchor.m_item->m_connected.clear();
    }


    void addAnchor( VECTOR2I pos, CnItem* item )
    {
        CnPoint p;

        p.m_pos     = pos;
        p.m_item    = item;

        m_anchors.push_back( p );
    }


    void Remove ( CnItem *aItem )
    {
        auto i =  std::find (m_items.begin(), m_items.end(), aItem );
        assert ( i != m_items.end() );
        m_items.erase ( i );
        setDirty();
    }

    void Remove ( BOARD_CONNECTED_ITEM *aItem )
    {
        m_items.erase(
            std::remove_if(
                m_items.begin(), m_items.end(),
                    [aItem](CnItem* item) { return item->m_item == aItem;}),
                m_items.end());

        m_anchors.erase(
                    std::remove_if(
                        m_anchors.begin(), m_anchors.end(),
                            [aItem](CnPoint& p) { return p.m_item->m_item == aItem;}),
                        m_anchors.end());



                        setDirty();
    }


};


// vector<CnPoint*> m_candidates;
// vector<CnCluster> m_clusters;

template <class T>
void CnList::FindNearby( BOX2I aBBox, T aFunc )
{
    for( auto p : m_anchors )
        aFunc( p );
}


template <class T>
void CnList::FindNearby( VECTOR2I aPosition, int aDistMax, T aFunc )
{
    /* Search items in m_Candidates that position is <= aDistMax from aPosition
     * (Rectilinear distance)
     * m_Candidates is sorted by X then Y values, so a fast binary search is used
     * to locate the "best" entry point in list
     * The best entry is a pad having its m_Pos.x == (or near) aPosition.x
     * All candidates are near this candidate in list
     * So from this entry point, a linear search is made to find all candidates
     */

    if( m_dirty )
    {
        sort( m_anchors.begin(), m_anchors.end(), sortByXthenYCoord );
        m_dirty = false;
    }

    int idxmax = m_anchors.size() - 1;

    int delta = m_anchors.size();

    int idx = 0;        // Starting index is the beginning of list

    while( delta )
    {
        // Calculate half size of remaining interval to test.
        // Ensure the computed value is not truncated (too small)
        if( (delta & 1) && ( delta > 1 ) )
            delta++;

        delta /= 2;

        CnPoint& p = m_anchors[idx];

        int dist = p.m_pos.x - aPosition.x;

        if( abs( dist ) <= aDistMax )
        {
            break;                              // A good entry point is found. The list can be scanned from this point.
        }
        else if( p.m_pos.x < aPosition.x )      // We should search after this point
        {
            idx += delta;

            if( idx > idxmax )
                idx = idxmax;
        }
        else    // We should search before this p
        {
            idx -= delta;

            if( idx < 0 )
                idx = 0;
        }
    }

    /* Now explore the candidate list from the "best" entry point found
     * (candidate "near" aPosition.x)
     * We exp the list until abs(candidate->m_Point.x - aPosition.x) > aDistMashar* Currently a linear search is made because the number of candidates
     * having the right X position is usually small
     */
    // search next candidates in list
    VECTOR2I diff;

    for( int ii = idx; ii <= idxmax; ii++ )
    {
        CnPoint& p = m_anchors[ii];
        diff = p.m_pos - aPosition;;

        if( abs( diff.x ) > aDistMax )
            break; // Exit: the distance is to long, we cannot find other candidates

        if( abs( diff.y ) > aDistMax )
            continue; // the y distance is to long, but we can find other candidates

        // We have here a good candidate: add it
        aFunc( p );
    }

    // search previous candidates in list
    for(  int ii = idx - 1; ii >=0; ii-- )
    {
        CnPoint& p = m_anchors[ii];
        diff = p.m_pos - aPosition;

        if( abs( diff.x ) > aDistMax )
            break;

        if( abs( diff.y ) > aDistMax )
            continue;

        // We have here a good candidate:add it
        aFunc( p );
    }
}


using namespace std;

shared_ptr<BOARD> m_board;


struct CnPadList : public CnList
{
    CnItem* Add( D_PAD* pad )
    {
        auto item = new CnItem;

        item->m_item = pad;
        item->m_canChangeNet = false;
        addAnchor( pad->ShapePos(), item );
        m_items.push_back( item );

        setDirty();
        return item;
    };


};

struct CnTrackList : public CnList
{
    CnItem* Add( TRACK* track )
    {
        auto item = new CnItem;

        item->m_item = track;
        item->m_canChangeNet = true;
        m_items.push_back( item );

        addAnchor( track->GetStart(), item );
        addAnchor( track->GetEnd(), item );
        setDirty();
        return item;
    };
};

/*(CnViaList( shared_ptr<BOARD> aBoard )
 *  {
 */
struct CnViaList : public CnList
{
    CnItem* Add( VIA* via )
    {
        auto item = new CnItem;

        item->m_item = via;
        item->m_canChangeNet = true;
        m_items.push_back( item );

        addAnchor( via->GetStart(), item );
        setDirty();
        return item;
    };

};

/*for( int i = 0; i<aBoard->GetAreaCount(); i++ )
 *  {
 *   ZONE_CONTAINER *zone = aBoard->GetArea(i);
 *
 *   zone->ClearFilledPolysList();
 *   zone->UnFill();
 *
 *   // Cannot fill keepout zones:
 *   if( zone->GetIsKeepout() )
 *       continue;
 *
 *   zone->BuildFilledSolidAreasPolygons( aBoard.get() );
 *
 *   const SHAPE_POLY_SET& polys = zone->RawPolysList();
 *
 *   for(int j = 0; j < polys.OutlineCount(); j++)
 *   {
 */

struct CnZone : public CnItem
{
    int m_subpolyIndex;
};

struct CnZoneList : public CnList
{
    CnZoneList()
    {
    }

    const vector<CnItem*> Add( ZONE_CONTAINER* zone )
    {
        const SHAPE_POLY_SET& polys = zone->GetFilledPolysList();

        vector<CnItem*> rv;

        for( int j = 0; j < polys.OutlineCount(); j++ )
        {
            CnZone* zitem = new CnZone;
            zitem->m_subpolyIndex = j;
            zitem->m_item = zone;
            zitem->m_canChangeNet = false;
            m_items.push_back( zitem );
            rv.push_back( zitem );
            setDirty();
        }

        return rv;
    };



};


#if 0

void CnList::Build()
{
    for( auto mod : m_board->Modules() )
        for( auto pad : mod->PadsIter() )
        {
            auto item = new CnItem;
            item->m_item = pad;
            // item->m_net = -1;
            m_items.push_back( item );
        }


}


for( auto track : m_board->Tracks() )
{
    auto item = new CnItem;
    item->m_item = track;
    m_items.push_back( item );
}

for( auto item : m_items )
    for( int i = 0; i < item->anchorCount(); i++ )
    {
        CnPoint* p = new CnPoint;
        p->m_item   = item;
        p->m_pos    = item->anchor( i );
        m_anchors.push_back( p );
    }




sort( m_candidates.begin(), m_candidates.end(), sortByXthenYCoord );

printf( "amnchors : %d\n", m_candidates.size() );
}
* /
#endif

class CnCluster
{
public:
    CnCluster()
    {
        m_items.reserve( 64 );
        m_originPad = nullptr;
        m_originNet = -1;
        m_conflicting = false;
    }

    bool HasValidNet() const
    {
        return m_originNet >= 0;
    }

    int OriginNet() const
    {
        return m_originNet;
    }

    wxString OriginNetName() const
    {
        if( !m_originPad )
            return "<none>";
        else
            return m_originPad->m_item->GetNetname();
    }

    bool Contains ( CnItem *aItem )
    {
        return std::find( m_items.begin(), m_items.end(), aItem ) != m_items.end();
    }

    bool Contains ( BOARD_CONNECTED_ITEM *aItem )
    {
        for ( auto item : m_items )
            if ( item->m_item == aItem )
                return true;
        return false;
    }

    void Dump()
    {
        for( auto item : m_items )
            printf( " - item : %p bitem : %p type : %d inet %s\n", item, item->m_item,
                    item->m_item->Type(), (const char*) item->m_item->GetNetname().c_str() );
    }

    int Size() const
    {
        return m_items.size();
    }

    bool HasNet() const
    {
        return m_originNet >= 0;
    }

    bool IsOrphaned() const
    {
        return m_originPad == nullptr;
    }

    bool IsConflicting() const
    {
        return m_conflicting;
    }

    void Add( CnItem* item )
    {
        m_items.push_back( item );

        // printf("cluster %p add %p\n", this, item );

        if( item->m_item->Type() == PCB_PAD_T )
        {
            if( !m_originPad )
            {
                m_originPad = item;
                m_originNet = item->m_item->GetNetCode();
            }

            if( m_originPad && item->m_item->GetNetCode() != m_originNet )
            {
                m_conflicting = true;
            }
        }
    }

    bool m_conflicting;
    int m_originNet;
    CnItem* m_originPad;

    vector<CnItem*> m_items;
};

class CnConnectivity
{
    vector<CnCluster*> m_clusters;

public:

    CnConnectivity()
    {
        padList = new CnPadList();
        trackList = new CnTrackList();
        viaList     = new CnViaList();
        zoneList    = new CnZoneList();
    }

    void searchConnections( bool aIncludeZones = false )
    {
        auto connect = []( CnItem* a, CnItem* b )
                       {
                           a->m_connected.push_back( b );
                           b->m_connected.push_back( a );
                       };

        padList->ClearConnections();
        viaList->ClearConnections();
        trackList->ClearConnections();
        zoneList->ClearConnections();

        PROF_COUNTER search_cnt( "search-connections" ); search_cnt.start();

        for( auto padItem : padList->m_items )
        {
            auto pad = static_cast<D_PAD*> ( padItem->m_item );
            auto findFunc = [&] ( const CnPoint& point )
                            {
                                if( pad == point.m_item->m_item )
                                    return;

                                if( !( pad->GetLayerSet() &
                                       point.m_item->m_item->GetLayerSet() ).any() )
                                    return;

                                if( pad->HitTest( wxPoint( point.m_pos.x, point.m_pos.y ) ) )
                                    connect( padItem, point.m_item );
                            };

            padList->FindNearby( pad->ShapePos(), pad->GetBoundingRadius(), findFunc );
            trackList->FindNearby( pad->ShapePos(), pad->GetBoundingRadius(), findFunc );
            viaList->FindNearby( pad->ShapePos(), pad->GetBoundingRadius(), findFunc );
        }

        for( auto trackItem : trackList->m_items )
        {
            auto track = static_cast<TRACK*> ( trackItem->m_item );
            int dist_max = track->GetWidth() / 2;

            auto findFunc = [&] ( const CnPoint& point )
                            {
                                if( track == point.m_item->m_item )
                                    return;

                                if( !( track->GetLayerSet() &
                                       point.m_item->m_item->GetLayerSet() ).any() )
                                    return;

                                const VECTOR2I d_start( VECTOR2I( track->GetStart() ) -
                                        point.m_pos );
                                const VECTOR2I d_end( VECTOR2I( track->GetEnd() ) - point.m_pos );

                                if( d_start.EuclideanNorm() < dist_max
                                    || d_end.EuclideanNorm() < dist_max )
                                    connect( trackItem, point.m_item );
                            };

            trackList->FindNearby( track->GetStart(), dist_max, findFunc );
            trackList->FindNearby( track->GetEnd(), dist_max, findFunc );
        }

        for( auto viaItem : viaList->m_items )
        {
            auto via = static_cast<VIA*> ( viaItem->m_item );
            int dist_max = via->GetWidth() / 2;

            auto findFunc = [&] ( const CnPoint& point )
                            {
                                if( via == point.m_item->m_item )
                                    return;

                                if( !( via->GetLayerSet() &
                                       point.m_item->m_item->GetLayerSet() ).any() )
                                    return;

                                if( via->HitTest( wxPoint( point.m_pos.x, point.m_pos.y ) ) )
                                    connect( point.m_item, viaItem );
                            };

            viaList->FindNearby( via->GetStart(), dist_max, findFunc );
            trackList->FindNearby( via->GetStart(), dist_max, findFunc );
        }

        if( aIncludeZones )
        {
            for( auto zi : zoneList->m_items )
            {
                auto zoneItem = static_cast<CnZone*> (zi );
                auto zone = static_cast<ZONE_CONTAINER*> ( zoneItem->m_item );
                auto& polys = zone->GetFilledPolysList();

                auto findFunc = [&] ( const CnPoint& point )
                                {
                                    if( point.m_item->m_item->GetNetCode() != zone->GetNetCode() )
                                        return;

                                    if( !( zone->GetLayerSet() &
                                           point.m_item->m_item->GetLayerSet() ).any() )
                                        return;

                                    if( polys.Contains( point.m_pos, zoneItem->m_subpolyIndex ) )
                                        connect( zoneItem, point.m_item );
                                };

                viaList->FindNearby( BOX2I(), findFunc );
                trackList->FindNearby( BOX2I(), findFunc );
                padList->FindNearby(  BOX2I(), findFunc );
            }
        }

        search_cnt.show();
    }

    void searchClusters( bool aIncludeZones = false )
    {
        PROF_COUNTER cnt( "search-clusters" ); cnt.start();

        CnItem* head = padList->m_items[0];
        int n = 0;

        for( auto item : padList->m_items )
        {
            item->m_visited = false;

            if( item != head )
                head->ListInsert( item );

            n++;
        }

        for( auto item : trackList->m_items )
        {
            item->m_visited = false;

            if( item != head ) head->ListInsert( item );

            n++;
        }

        for( auto item : viaList->m_items )
        {
            item->m_visited = false;

            if( item != head ) head->ListInsert( item );

            n++;
        }

        if (aIncludeZones)
        {
        for( auto item : zoneList->m_items )
        {
            item->m_visited = false;

            if( item != head ) head->ListInsert( item );

            n++;
        }
        }


        m_clusters.clear();
        std::deque<CnItem*> Q;

        while( head )
        {
            auto cluster = new CnCluster();

            Q.clear();
            CnItem* root = head;
            root->m_visited = true;

            head = root->ListRemove();

            Q.push_back( root );

            while( Q.size() )
            {
                CnItem* current = Q.front();

                Q.pop_front();
                cluster->Add( current );

                for( CnItem* n : current->m_connected )
                {
                    if( !n->m_visited )
                    {
                        n->m_visited    = true;
                        Q.push_back( n );
                        head = n->ListRemove();
                    }
                }
            }

            m_clusters.push_back( cluster );
        }

        cnt.show();

        n = 0;

        sort( m_clusters.begin(), m_clusters.end(), []( CnCluster* a, CnCluster* b ) {
            return a->OriginNet() < b->OriginNet();
        } );

        int n_items = 0;

        for( auto cl : m_clusters )
        {
            // printf("cluster %d: net %d [%s], %d items, conflict: %d, orphan: %d \n", n, cl->OriginNet(), (const char *) cl->OriginNetName(), cl->Size(), !!cl->IsConflicting(), !!cl->IsOrphaned()  );
            // cl->Dump();
            n++;
            n_items += cl->Size();
        }

        printf( "all cluster items : %d\n", n_items );
    }

    void SetBoard( shared_ptr<BOARD> aBoard )
    {
        for( int i = 0; i<aBoard->GetAreaCount(); i++ )
        {
            ZONE_CONTAINER* zone = aBoard->GetArea( i );

            zone->ClearFilledPolysList();
            zone->UnFill();

            // Cannot fill keepout zones:
            if( zone->GetIsKeepout() )
                continue;

            zone->BuildFilledSolidAreasPolygons( aBoard.get() );

            Add( zone );
        }

        for( auto tv : aBoard->Tracks() )
            Add( tv );

        for( auto mod : aBoard->Modules() )
            for( auto pad : mod->PadsIter() )
                Add( pad );

        m_board =aBoard;

        printf( "zones : %lu, pads : %lu vias : %lu tracks : %lu\n",
                zoneList->m_items.size(), padList->m_items.size(),
                viaList->m_items.size(), trackList->m_items.size() );
    }

    template <class Container, class BItem>
    void add( Container* c, BItem* brditem )
    {
        auto item = c->Add( brditem );

        m_itemMap.insert( ItemMapPair( brditem, item ) );
    }

    void Remove ( BOARD_CONNECTED_ITEM *aItem )
    {
        switch( aItem->Type() )
        {
        case PCB_PAD_T:

            break;

        case PCB_TRACE_T:

            break;

        case PCB_VIA_T:

            break;


        case PCB_ZONE_AREA_T:
        case PCB_ZONE_T:

            zoneList->Remove ( aItem );

            break;

        default:
            return;
        }
    }

    void Add( BOARD_CONNECTED_ITEM* aItem )
    {
        switch( aItem->Type() )
        {
        case PCB_PAD_T:
            add( padList, static_cast<D_PAD*>(aItem) );
            break;

        case PCB_TRACE_T:
            add( trackList, static_cast<TRACK*>(aItem) );
            break;

        case PCB_VIA_T:
            add( viaList, static_cast<VIA*>(aItem) );
            break;


        case PCB_ZONE_AREA_T:
        case PCB_ZONE_T:

            for( auto zitem : zoneList->Add( static_cast<ZONE_CONTAINER*>(aItem) ) )
            {
                m_itemMap.insert( ItemMapPair( aItem, zitem ) );
            }

            break;

        default:
            return;
        }
    }

    void update();

    void propagateConnections();

    struct CnDisjointNetEntry
    {
        int net;
        wxString netname;
        BOARD_CONNECTED_ITEM *a, *b;
        VECTOR2I anchorA, anchorB;
    };

// PUBLIC API
    void PropagateNets();
    void FindIsolatedCopperIslands( ZONE_CONTAINER *aZone, std::vector<int>& aIslands );
    bool CheckConnectivity( vector<CnDisjointNetEntry>& aReport );


    CnPadList* padList;
    CnTrackList* trackList;
    CnViaList* viaList;
    CnZoneList* zoneList;

    using ItemMapPair = pair <BOARD_ITEM*, CnItem*>;

    unordered_multimap<BOARD_ITEM*, CnItem*> m_itemMap;
};


// for( auto mod : aBoard->Modules() )
// for( auto pad : mod->PadsIter() )

void loadBoard( string name )
{
    PLUGIN::RELEASER pi( IO_MGR::PluginFind( IO_MGR::KICAD ) );

    try
    {
        m_board.reset( pi->Load( name, NULL, NULL ) );
    }
    catch( const IO_ERROR& ioe )
    {
        wxString msg = wxString::Format( _( "Error loading board.\n%s" ),
                ioe.What() );

        fprintf( stderr, "%s\n", (const char*) msg.mb_str() );
        exit( -1 );
    }
}

void saveBoard( shared_ptr<BOARD> board, string name )
{
    PLUGIN::RELEASER pi( IO_MGR::PluginFind( IO_MGR::KICAD ) );

    try
    {
        pi->Save( name, board.get(),  NULL );
    }
    catch( const IO_ERROR& ioe )
    {
        wxString msg = wxString::Format( _( "Error saving board.\n%s" ),
                ioe.What() );

        fprintf( stderr, "%s\n", (const char*) msg.mb_str() );
        exit( -1 );
    }
}


class CONNECTIVITY_DATA : public CnConnectivity
{
    // add , remove, modify
    // PropageteNets()
    // FindIsolatedCopperIslands(ZONE)
    // IsConnected
    // Cluster

    // OrphanCount()
    // UnconnectedCount()
    // ConflictCount()
};

void CnConnectivity::update()
{

}


void CnConnectivity::propagateConnections()
{


    for( auto cluster : m_clusters )
    {
        if( cluster->IsConflicting() )
        {
            printf( "Conflicting nets in cluster %p\n", cluster );
        }
        else if( cluster->IsOrphaned() )
        {
            printf( "Skipping orphaned cluster %p [net: %s]\n", cluster, (const char *)cluster->OriginNetName() );
        }
        else if( cluster->HasValidNet() )
        {
            // normal cluster: just propagate from the pads
            int n_changed = 0;

            for( auto item : cluster->m_items )
            {
                if( item->m_canChangeNet )
                {
                    item->m_item->SetNetCode( cluster->OriginNet() );
                    n_changed++;
                }
            }

            if( n_changed )
                printf( "Cluster %p : net : %d %s\n", cluster,
                        cluster->OriginNet(), (const char*) cluster->OriginNetName() );
            else
                printf( "Cluster %p : nothing to propagate\n", cluster );
        }
        else
        {
            printf( "Cluster %p : connected to unused net\n", cluster );
        }
    }
}


void CnConnectivity::PropagateNets()
{
    searchConnections( false );
    searchClusters( false );
    propagateConnections();
}

void CnConnectivity::FindIsolatedCopperIslands( ZONE_CONTAINER *aZone, std::vector<int>& aIslands )
{
    //auto range = m_itemMap.equal_range( (BOARD_ITEM *)aZone );
    aIslands.clear();
    zoneList->Remove ( aZone );
    Add ( aZone );

    //zoneList->Remove ( aZone );
    searchConnections( true );
    searchClusters( true );

    for ( auto cluster : m_clusters )
        if( cluster->Contains( aZone ) && cluster->IsOrphaned() )
        {
            //printf("cluster %p found orphaned : %d\n", cluster, !!cluster->IsOrphaned());
            for ( auto z: cluster->m_items )
            {
                if ( z -> m_item == aZone )
                {

                    aIslands.push_back ( static_cast<CnZone*>(z) -> m_subpolyIndex);

                }
            }
        }

        printf ("Found isolated islands : ");
        for ( auto c : aIslands)
            printf("%d ", c);
        printf("\n");

}


bool CnConnectivity::CheckConnectivity( vector<CnDisjointNetEntry>& aReport )
{
    searchConnections( true );
    searchClusters( true );

    int maxNetCode = 0;

    for ( auto cluster : m_clusters )
        maxNetCode = std::max(maxNetCode, cluster->OriginNet() );

    for( int net = 1; net <= maxNetCode; net++)
    {
        int count = 0;
        wxString name;

        for ( auto cluster : m_clusters )
            if( cluster->OriginNet() == net )
            {
                name = cluster->OriginNetName();
                count++;
            }

        if ( count > 1 )
            printf("Net %d [%s] is not completely routed (%d disjoint clusters).\n", net, (const char *)name, count );

    }
};


int main( int argc, char* argv[] ) {
    if( argc < 2 )
    {
        printf( "usage : %s board_file\n", argv[0] );
        return 0;
    }

    loadBoard( argv[1] );

    CnConnectivity conns;

    conns.SetBoard( m_board );

    vector<int> islands;
    vector<CnConnectivity::CnDisjointNetEntry> report;

    conns.PropagateNets();

    #if 1
    for( int i = 0; i <m_board->GetAreaCount(); i++)
    {
            ZONE_CONTAINER *zone =  m_board->GetArea(i);
            conns.FindIsolatedCopperIslands( zone, islands );


            std::sort (islands.begin(), islands.end(),  std::greater<int>() );

            for ( auto idx : islands )
            {
            //    printf("Delete poly %d/%d\n", idx, zone->FilledPolysList().OutlineCount());
                zone->FilledPolysList().DeletePolygon( idx );
            }

            conns.Remove ( zone );
            conns.Add ( zone );
    }
    #endif

    conns.CheckConnectivity(report);

    saveBoard( m_board, "tmp.kicad_pcb");
    return 0;
    // conns.propagateNets();

    /*{
     *   PROF_COUNTER cnt ("build-pad-list"); cnt.start();
     *   padList.Build( )
     *   cnt.show();
     *  }
     *  {
     *   PROF_COUNTER cnt ("build-track-via-list"); cnt.start();
     *   cnt.show();
     *  }*/
    // lst.Build();
    // lst.searchConnections();
}
