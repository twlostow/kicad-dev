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
        m_prev = nullptr;
        m_next = nullptr;
        m_root = (T*) this;
        m_count = 1;
    }

    T* ListRemove()
    {

        //printf("prev %p next %p\n", m_prev, m_next);
        if(m_prev)
            m_prev->m_next = m_next;
        if(m_next)
            m_next->m_prev = m_prev;

        m_root->m_count--;

        if(m_prev)
            return m_prev;
        else if (m_next)
            return m_next;

        m_root = nullptr;
        return nullptr;
    }

    int ListSize() const { return m_root ? m_root->m_count : 0; }

    void ListInsert (T* item )
    {

        if(!m_root)
            m_root = item;

        if(m_next)
            m_next->m_prev = item;

        item->m_prev = (T*)this;
        item->m_next = m_next;
        item->m_root = m_root;
        m_root->m_count++;

        m_next = item;
    }

    T* ListNext() const { return m_next; };
    T* ListPrev() const { return m_prev; };

private:
    int m_count;
    T *m_prev, *m_next, *m_root;
    //IntrusiveListBase<T> *m_list;
};

struct CnItem : public IntrusiveList<CnItem>
{
    BOARD_CONNECTED_ITEM* m_item;
    vector<CnItem*> m_connected;

    CnItem *m_parent;
    bool m_visited;
    bool m_new;

    CnItem()
    {
        m_parent = nullptr;
        m_new = true;
        m_visited = false;
        m_item = nullptr;
    }

    void Connect ( CnItem *aOther )
    {
        m_connected.push_back(aOther);
    }
};

struct CnPoint
{
public:
    CnPoint()
    {
        m_dirty = true;
    }

    VECTOR2I m_pos;
    CnItem* m_item;
    bool m_dirty;
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

    CnList( ) {
        m_dirty = false;
    };    // shared_ptr<BOARD> aBoard ) : m_board(aBoard){};

    void setDirty ( bool aDirty = true ) { m_dirty = aDirty; }

    template <class T>
    void FindNearby( VECTOR2I aPosition, int aDistMax, T aFunc );
    template <class T>
    void FindNearby( BOX2I aBBox, T aFunc );

    void addAnchor( VECTOR2I pos, CnItem* item )
    {
        CnPoint p;

        p.m_pos = pos;
        p.m_item = item;

        m_anchors.push_back( p );
    }

    vector<CnPoint> m_anchors;
};


// vector<CnPoint*> m_candidates;
// vector<CnCluster> m_clusters;

template <class T>
void CnList::FindNearby( BOX2I aBBox, T aFunc )
{
    for ( auto p : m_anchors )
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

     if ( m_dirty )
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
        else if( p.m_pos.x < aPosition.x )   // We should search after this point
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

    CnItem *Add( D_PAD *pad )
    {
        auto item = new CnItem;
        item->m_item = pad;
        addAnchor( pad->GetPosition(), item );
        m_pads.push_back( item );
        setDirty();
        return item;
    };

    vector<CnItem*> m_pads;
};

struct CnTrackList : public CnList
{

    CnItem *Add ( TRACK *track )
    {
        auto item = new CnItem;
        item->m_item = track;
        m_tracks.push_back( item );

        addAnchor( track->GetStart(), item );
        addAnchor( track->GetEnd(), item );
        setDirty();
        return item;
    };

    vector<CnItem*> m_tracks;
};

/*(CnViaList( shared_ptr<BOARD> aBoard )
{
*/
struct CnViaList : public CnList
{

    CnItem *Add ( VIA *via )
    {
        auto item = new CnItem;
        item->m_item = via;
        m_vias.push_back( item );

        addAnchor( via->GetStart(), item );
        setDirty();
        return item;
    };

    vector<CnItem*> m_vias;
};

/*for( int i = 0; i<aBoard->GetAreaCount(); i++ )
{
    ZONE_CONTAINER *zone = aBoard->GetArea(i);

    zone->ClearFilledPolysList();
    zone->UnFill();

    // Cannot fill keepout zones:
    if( zone->GetIsKeepout() )
        continue;

    zone->BuildFilledSolidAreasPolygons( aBoard.get() );

    const SHAPE_POLY_SET& polys = zone->RawPolysList();

    for(int j = 0; j < polys.OutlineCount(); j++)
    {
        */

struct CnZone : public CnItem
{
        int m_subpolyIndex;
};

struct CnZoneList : public CnList
{
    vector<CnZone*> m_zones;

    CnZoneList( )
    {
    }

    const vector<CnItem*> Add ( ZONE_CONTAINER *zone )
    {
        const SHAPE_POLY_SET& polys = zone->RawPolysList();
        vector<CnItem*> rv;

        for(int j = 0; j < polys.OutlineCount(); j++)
        {
            CnZone *zitem = new CnZone;
            zitem->m_subpolyIndex = j;
            zitem->m_item = zone;
            m_zones.push_back( zitem );
            rv.push_back(zitem);
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
        p->m_item = item;
        p->m_pos = item->anchor( i );
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
            m_items.reserve(64);
            m_originPad = nullptr;
            m_originNet = -1;
            m_conflicting = false;
        }

        int OriginNet() const
        {
                return m_originNet;
        }

        wxString OriginNetName() const
        {
                if(!m_originPad)
                    return "<none>";
                else
                    return m_originPad->m_item->GetNetname();
        }

        void Dump()
        {
            for ( auto item : m_items )
                printf(" - item : %p bitem : %p type : %d\n", item,  item->m_item, item->m_item->Type());
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

        void Add( CnItem *item )
        {
            m_items.push_back(item);

            //printf("cluster %p add %p\n", this, item );

            if(item->m_item->Type() == PCB_PAD_T )
            {
                if (!m_originPad)
                {
                    m_originPad = item;
                    m_originNet = item->m_item->GetNetCode();
                }

                if(m_originPad && item->m_item->GetNetCode() != m_originNet )
                {
                    m_conflicting = true;
                }

            }
        }

        bool m_conflicting;
        int m_originNet;
        CnItem *m_originPad;

        vector<CnItem*> m_items;
};

class CnConnectivity
{
public:

    CnConnectivity( )
    {
        padList = new CnPadList( );
        trackList = new CnTrackList( );
        viaList = new CnViaList( );
        zoneList = new CnZoneList ( );
    }

    void searchConnections()
    {


        auto connect = []( CnItem *a, CnItem *b )
        {
            a->m_connected.push_back(b);
            b->m_connected.push_back(a);
        };


        PROF_COUNTER search_cnt ("search-connections"); search_cnt.start();
        for( auto padItem : padList->m_pads )
        {
            auto pad = static_cast<D_PAD*> ( padItem->m_item );
            auto findFunc = [&] ( const CnPoint& point )
                            {
                                if ( pad == point.m_item->m_item )
                                    return;

                                if( !( pad->GetLayerSet() &
                                       point.m_item->m_item->GetLayerSet() ).any() )
                                    return;

                                if( pad->HitTest( wxPoint( point.m_pos.x, point.m_pos.y ) ) )
                                {
                                    //printf( "- found %p type %d\n", point.m_item->m_item,
                                    //        point.m_item->m_item->Type() );

                                    connect(padItem, point.m_item );
                                }
                            };


        //    printf( "pad %p\n", pad );
            padList->FindNearby( pad->GetPosition(), pad->GetBoundingRadius(), findFunc );
            trackList->FindNearby ( pad->GetPosition(), pad->GetBoundingRadius(), findFunc );
            viaList->FindNearby ( pad->GetPosition(), pad->GetBoundingRadius(), findFunc );
        }

        for( auto trackItem : trackList->m_tracks )
        {
            auto track = static_cast<TRACK*> ( trackItem->m_item );
            int dist_max = track->GetWidth() / 2;

            auto findFunc = [&] ( const CnPoint& point )
                            {
                                if ( track == point.m_item->m_item )
                                    return;

                                if( !( track->GetLayerSet() &
                                       point.m_item->m_item->GetLayerSet() ).any() )
                                    return;

                                const VECTOR2I d_start ( VECTOR2I(track->GetStart()) - point.m_pos );
                                const VECTOR2I d_end ( VECTOR2I(track->GetEnd()) - point.m_pos );

                                if (d_start.EuclideanNorm() < dist_max || d_end.EuclideanNorm() < dist_max)
                                {
                                    connect( trackItem, point.m_item );

                                //printf( "- found %p type %d\n", point.m_item->m_item,
                                        //point.m_item->m_item->Type() );
                                }

                            };
                        //    printf( "track %p\n", track );

        trackList->FindNearby ( track->GetStart(), dist_max, findFunc );
        trackList->FindNearby ( track->GetEnd(), dist_max, findFunc );

        }

        for( auto viaItem : viaList->m_vias)
        {
            auto via = static_cast<VIA*> ( viaItem->m_item );
            int dist_max = via->GetWidth() / 2;

            auto findFunc = [&] ( const CnPoint& point )
                            {
                                if ( via == point.m_item->m_item )
                                    return;

                                if( !( via->GetLayerSet() &
                                       point.m_item->m_item->GetLayerSet() ).any() )
                                    return;

                                if( via->HitTest( wxPoint( point.m_pos.x, point.m_pos.y ) ) )
                                {
                            //        printf( "- found %p type %d\n", point.m_item->m_item,
                            //                point.m_item->m_item->Type() );

                                    connect( point.m_item, viaItem );
                                }
                            };
                            //printf( "via %p\n", via );

            viaList->FindNearby ( via->GetStart(), dist_max, findFunc );
            trackList->FindNearby ( via->GetStart(), dist_max, findFunc );
        }

        for( auto zoneItem : zoneList->m_zones )
        {
            auto zone = static_cast<ZONE_CONTAINER*> ( zoneItem->m_item );
            auto& polys = zone->RawPolysList();

            //printf("zone %p\n", zone);

            auto findFunc = [&] ( const CnPoint& point )
            {
                if( point.m_item->m_item->GetNetCode() != zone->GetNetCode() )
                    return;


                if( !( zone->GetLayerSet() &
                       point.m_item->m_item->GetLayerSet() ).any() )
                    return;

                //printf("do %p %p %d\n", point.m_item, zoneItem, zoneItem->m_subpolyIndex);


                if ( polys.Contains( point.m_pos, zoneItem->m_subpolyIndex ) )
                {
                //            printf( "- found %p type %d\n", point.m_item->m_item,
                //                    point.m_item->m_item->Type() );

                    connect(zoneItem, point.m_item);
                }
            };

            viaList->FindNearby ( BOX2I(), findFunc );
            trackList->FindNearby ( BOX2I(), findFunc );
            padList->FindNearby (  BOX2I(), findFunc );

        }

        search_cnt.show();
    }

    void searchClusters()
    {

        PROF_COUNTER cnt ( "search-clusters" ); cnt.start();

        CnItem *head = padList->m_pads[0];
        int n =0;
        for ( auto item : padList->m_pads ) {
            item->m_parent = nullptr;
            item->m_visited = false;
            if (item != head)
            head->ListInsert(item);
            n++;
        }

        for ( auto item : trackList->m_tracks ) { item->m_parent = nullptr; item->m_visited = false; if (item != head) head->ListInsert(item);
        n++; }
        for ( auto item : viaList->m_vias ) { item->m_parent = nullptr; item->m_visited = false; if (item != head) head->ListInsert(item);
        n++;
        }
        for ( auto item : zoneList->m_zones ) { item->m_parent = nullptr; item->m_visited = false; if (item != head) head->ListInsert(item);
        n++;
        }

        CnItem *t = head;

        map<BOARD_CONNECTED_ITEM*, int> counts;

        for ( auto item : trackList->m_tracks )
        {
            //printf("T %p %p\n", item, item->m_item);
        }
        for ( auto item : viaList->m_vias )
        {
            //printf("V %p %p\n", item, item->m_item);
        }
        for ( auto item : padList->m_pads )
        {
            //printf("P %p %p\n", item, item->m_item);
        }

        while(t)
        {
            printf("Litem %p %p\n", t, t->m_item);
            counts[t->m_item]++;
            t = t->ListNext();
        }

        //for (auto i : counts)
            //printf("%p : %d t %d\n",i.first, i.second, i.first->Type());

        vector<CnCluster*> clusters;
        std::deque<CnItem*> Q;
        while( head )
        {
            auto cluster = new CnCluster();
            //printf("remaining : %d\n",  );
            Q.clear();
            CnItem *root = head;
            //printf("root %p\n", root);
            root->m_visited = true;

            head = root->ListRemove();
            n--;

//            if(!root->m_connected.size())
//                continue;

            Q.push_back(root);
            // printf("root %p\n",root);
            while (Q.size())
            {
                CnItem *current = Q.front();

                Q.pop_front();
        //        printf("Add %p\n", current);
                cluster->Add(current);
            //    printf("scan %p cn %d\n", current, current->m_connected.size() );


            //    printf("scan %d\n", current->m_connected.size() );
                for( CnItem *n : current->m_connected )
                {
                    if ( !n->m_visited )
                    {
                        n->m_visited = true;
                        n->m_parent = current;

        //                printf("->next %p\n", n);

                        Q.push_back(n);
                        head = n->ListRemove();
                        n--;
                        //printf("found %p head %p\n", n, head);
                    }
                }
            }
            clusters.push_back(cluster);
        }

        cnt.show();

        n = 0;

        sort ( clusters.begin(), clusters.end(), [](CnCluster *a, CnCluster *b) { return a->OriginNet() < b->OriginNet(); } );

        int n_items = 0;

        for(auto cl : clusters)
        {
            printf("cluster %d: net %d [%s], %d items, conflict: %d, orphan: %d\n", n, cl->OriginNet(), (const char *) cl->OriginNetName(), cl->Size(), !!cl->IsConflicting(), !!cl->IsOrphaned() );
            cl->Dump();
            n++;
            n_items += cl->Size();
        }

        printf("all cluster items : %d\n", n_items);
    }

    void SetBoard ( shared_ptr<BOARD> aBoard )
    {
        for( int i = 0; i<aBoard->GetAreaCount(); i++ )
        {
            ZONE_CONTAINER *zone = aBoard->GetArea(i);

            zone->ClearFilledPolysList();
            zone->UnFill();

            // Cannot fill keepout zones:
            if( zone->GetIsKeepout() )
                continue;

            zone->BuildFilledSolidAreasPolygons( aBoard.get() );

            Add ( zone );
        }

        for( auto tv : aBoard->Tracks() )
            Add ( tv );

        for( auto mod : aBoard->Modules() )
            for ( auto pad : mod->PadsIter() )
                Add ( pad );

        printf("zones : %d, pads : %d vias : %d tracks : %d\n", zoneList->m_zones.size(),padList->m_pads.size(), viaList->m_vias.size(), trackList->m_tracks.size() );

    }

    void Add ( BOARD_ITEM *aItem )
    {
        switch (aItem->Type())
        {
            case PCB_PAD_T:
            {
                auto item = padList->Add( static_cast<D_PAD*>(aItem) );
                break;
            }
            case PCB_TRACE_T:
            {
                auto item = trackList->Add( static_cast<TRACK*>(aItem) );
                break;
            }
            case PCB_VIA_T:
            {
                auto item = viaList->Add( static_cast<VIA*>(aItem) );
                break;
            }

            case PCB_ZONE_AREA_T:
            case PCB_ZONE_T:
            {
                for ( auto zitem :  zoneList->Add( static_cast<ZONE_CONTAINER*>(aItem) ) )
                    {}
            }

                break;

            default:
                return;
        }
    }

    CnPadList* padList;
    CnTrackList* trackList;
    CnViaList* viaList;
    CnZoneList *zoneList;

    unordered_multimap<BOARD_ITEM*, CnItem*> m_itemMap;

};


//for( auto mod : aBoard->Modules() )
//    for( auto pad : mod->PadsIter() )

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

main(int argc, char *argv[]) {

    if(argc < 2)
    {
        printf("usage : %s board_file\n", argv[0]);
        return 0;
    }

    loadBoard( argv[1] );

    CnConnectivity conns;

    conns.SetBoard( m_board );
    conns.searchConnections();
    conns.searchClusters();
    
    //conns.propagateNets();

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
