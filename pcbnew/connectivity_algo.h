//#define CONNECTIVITY_DEBUG

#ifndef __CONNECTIVITY_ALGO_H
#define __CONNECTIVITY_ALGO_H

// TODO: RN_NET could operate on CN_CLUSTERs

#include <class_board.h>
#include <class_pad.h>
#include <class_module.h>
#include <class_zone.h>

#include <geometry/shape_poly_set.h>
#include <geometry/poly_grid_partition.h>

#include <io_mgr.h>
#include <profile.h>

#include <memory>
#include <algorithm>
#include <functional>
#include <vector>
#include <deque>
#include <stdarg.h>

#include <ratsnest_data.h>
//#include "connectivity.h"


using std::shared_ptr;
using std::unique_ptr;

class CN_ITEM;
class CN_CONNECTIVITY_ALGO_IMPL;
class CN_RATSNEST_NODES;
class BOARD;
class BOARD_CONNECTED_ITEM;
class BOARD_ITEM;
class ZONE_CONTAINER;
class RN_DATA;
class RN_NET;

struct CN_DISJOINT_NET_ENTRY
{
    int net;
    wxString netname;
    BOARD_CONNECTED_ITEM* a, * b;
    VECTOR2I anchorA, anchorB;
};


class CN_ANCHOR
{
public:
    CN_ANCHOR()
    {
        m_item = nullptr;
    }

    CN_ANCHOR( const VECTOR2I& aPos, CN_ITEM* aItem )
    {
        m_pos   = aPos;
        m_item  = aItem;
        assert(m_item);
    }

    bool Valid() const;

    bool operator<( const CN_ANCHOR& aOther ) const
    {
        assert(m_item);
        assert(aOther.m_item);

        if( m_pos.x == aOther.m_pos.x )
            return m_pos.y < aOther.m_pos.y;
        else
            return m_pos.x < aOther.m_pos.x;
    }

    CN_ITEM* Item() const
    {
        return m_item;
    }

    BOARD_CONNECTED_ITEM *Parent() const;

    const VECTOR2I& Pos() const
    {
        return m_pos;
    }

private:
    VECTOR2I m_pos;
    CN_ITEM* m_item;
};

class CN_CLUSTER
{
private:

    bool m_conflicting = false;
    int m_originNet = 0;
    CN_ITEM* m_originPad = nullptr;
    std::vector<CN_ITEM*> m_items;

    //CN_RATSNEST_NODES *m_rnNodes = nullptr;

public:
    CN_CLUSTER();
    ~CN_CLUSTER();

    const std::vector<VECTOR2I> GetAnchors();

    bool HasValidNet() const
    {
        return m_originNet >= 0;
    }

    int OriginNet() const
    {
        return m_originNet;
    }

    wxString OriginNetName() const;

    bool Contains( CN_ITEM* aItem );
    bool Contains( BOARD_CONNECTED_ITEM* aItem );
    void Dump();

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

    void Add( CN_ITEM* item );

    using ITER = decltype(m_items)::iterator;

    ITER begin() { return m_items.begin(); };
    ITER end() { return m_items.end(); };
};

typedef shared_ptr<CN_CLUSTER> CN_CLUSTER_PTR;

// a lightweight intrusive list container
template <class T>
class INTRUSIVE_LIST
{
public:
    INTRUSIVE_LIST<T>()
    {
        ListClear();
    }

    void ListClear()
    {
        m_prev  = nullptr;
        m_next  = nullptr;
        m_root  = (T*) this;
        m_count = 1;
    }

    T* ListRemove()
    {
        if( m_prev )
            m_prev->m_next = m_next;

        if( m_next )
            m_next->m_prev = m_prev;

        m_root->m_count--;

        T* rv = nullptr;

        if( m_prev )
            rv = m_prev;
        else if( m_next )
            rv = m_next;

        m_root  = nullptr;
        m_prev  = nullptr;
        m_next  = nullptr;
        return rv;
    }

    int ListSize() const
    {
        return m_root ? m_root->m_count : 0;
    }

    void ListInsert( T* item )
    {
        if( !m_root )
            m_root = item;

        if( m_next )
            m_next->m_prev = item;

        item->m_prev = (T*) this;
        item->m_next = m_next;
        item->m_root = m_root;
        m_root->m_count++;

        m_next = item;
    }

    T* ListNext() const { return m_next; };
    T* ListPrev() const { return m_prev; };

private:
    int m_count;
    T* m_prev;
    T* m_next;
    T* m_root;
};

// basic connectivity item
class CN_ITEM : public INTRUSIVE_LIST<CN_ITEM>
{
private:
    BOARD_CONNECTED_ITEM* m_parent;

// list of items physically connected (touching)
    std::vector<CN_ITEM*> m_connected;

// visited flag for the BFS scan
    bool m_visited;

// can the net propagator modify the netcode?
    bool m_canChangeNet;

// valid flag, used to identify garbage items (we use lazy removal)
    bool m_valid;

// dirty flag, used to identify recently added item not yet scanned into the connectivity search
    bool m_dirty;

public:
    void Dump();

    CN_ITEM( BOARD_CONNECTED_ITEM *aParent, bool aCanChangeNet )
    {
        m_parent = aParent;
        m_canChangeNet = aCanChangeNet;
        m_visited = false;
        m_valid = true;
        m_dirty = true;
    }

    void SetValid ( bool aValid )
    {
        m_valid = aValid;
    }

    bool Valid() const
    {
        return m_valid;
    }

    void SetDirty ( bool aDirty )
    {
        m_dirty = aDirty;
    }

    bool Dirty() const
    {
        return m_dirty;
    }

    BOARD_CONNECTED_ITEM *Parent() const
    {
        return m_parent;
    }

    void Connect( CN_ITEM* aOther )
    {
        m_connected.push_back( aOther );
    }

    const std::vector<CN_ITEM*>& ConnectedItems()  const
    {
        return m_connected;
    }

    void ClearConnections()
    {
        m_connected.clear();
    }

    void SetVisited ( bool aVisited )
    {
        m_visited = aVisited;
    }

    bool Visited() const
    {
        return m_visited;
    }

    bool CanChangeNet() const
    {
            return m_canChangeNet;
    }

    static void Connect( CN_ITEM* a, CN_ITEM* b )
    {
       a->m_connected.push_back( b );
       b->m_connected.push_back( a );
    }

    void RemoveInvalidRefs();

    virtual int AnchorCount() const;
    virtual const VECTOR2I GetAnchor( int n ) const;

    int Net() const;
};


class CN_LIST
{
private:
    bool m_dirty;
    int m_count;
    std::vector<CN_ANCHOR> m_anchors;

protected:

    std::vector<CN_ITEM*> m_items;


    void addAnchor( VECTOR2I pos, CN_ITEM* item )
    {
        m_anchors.push_back( CN_ANCHOR( pos, item ) );
    }

private:

    void sort()
    {
        if( m_dirty )
        {
            std::sort( m_anchors.begin(), m_anchors.end() );

            m_dirty = false;
        }
    }


public:
    CN_LIST()
    {
        m_dirty = false;
    };

    using ITER = decltype(m_items)::iterator;

    ITER begin() { return m_items.begin(); };
    ITER end() { return m_items.end(); };

    template <class T>
    void FindNearby( VECTOR2I aPosition, int aDistMax, T aFunc );

    template <class T>
    void FindNearby( BOX2I aBBox, T aFunc );

    void SetDirty( bool aDirty = true )
    {
        m_dirty = aDirty;
    }

    bool IsDirty() const
    {
        return m_dirty;
    }

    void ClearConnections()
    {
        for( auto& anchor : m_anchors )
            anchor.Item()->ClearConnections();
    }

    void RemoveInvalidItems()
    {
        auto lastAnchor = std::remove_if(m_anchors.begin(), m_anchors.end(), [] ( const CN_ANCHOR& anchor) {
            return !anchor.Valid();

        } );

        m_anchors.resize( lastAnchor - m_anchors.begin() );

        auto lastItem = std::remove_if(m_items.begin(), m_items.end(), [] ( CN_ITEM * item) {
            if ( !item->Valid() )
            {
        //        delete item; // fixme: leak
                return true;
            }
            return false;
        } );

        m_items.resize( lastItem - m_items.begin() );

        for ( auto item : m_items )
            item->RemoveInvalidRefs();
    }

    int Size() const
    {
        return m_items.size();
    }
};


class CN_PAD_LIST : public CN_LIST
{
public:

    CN_ITEM* Add( D_PAD* pad )
    {
        auto item = new CN_ITEM( pad, false );

        addAnchor( pad->ShapePos(), item );
        m_items.push_back( item );

        SetDirty();
        return item;
    };
};

class CN_TRACK_LIST : public CN_LIST
{
public:
    CN_ITEM* Add( TRACK* track )
    {
        auto item = new CN_ITEM( track, true );
        m_items.push_back( item );

        addAnchor( track->GetStart(), item );
        addAnchor( track->GetEnd(), item );
        SetDirty();

        return item;
    };
};

class CN_VIA_LIST : public CN_LIST
{
public:
    CN_ITEM* Add( VIA* via )
    {
        auto item = new CN_ITEM( via, true );
        m_items.push_back( item );
        addAnchor( via->GetStart(), item );
        SetDirty();
        return item;
    };
};

class CN_ZONE : public CN_ITEM
{
public:
    CN_ZONE ( ZONE_CONTAINER *aParent, bool aCanChangeNet, int aSubpolyIndex ) :
        CN_ITEM (aParent, aCanChangeNet ),
        m_subpolyIndex ( aSubpolyIndex )
    {
        SHAPE_LINE_CHAIN outline = aParent->GetFilledPolysList().COutline( aSubpolyIndex );
        outline.SetClosed(true);
        outline.Simplify();

        m_cachedPoly.reset( new POLY_GRID_PARTITION( outline, 16 ) );
    }

    int SubpolyIndex() const
    {
        return m_subpolyIndex;
    }

    bool ContainsAnchor ( const CN_ANCHOR& anchor ) const
    {
        return m_cachedPoly->ContainsPoint ( anchor.Pos () );
    }

    bool ContainsPoint ( const VECTOR2I p ) const
    {
        return m_cachedPoly->ContainsPoint ( p );
    }

    const BOX2I& BBox() const
    {
        return m_cachedPoly->BBox();
    }

    virtual int AnchorCount() const;
    virtual const VECTOR2I GetAnchor( int n ) const;

private:
    unique_ptr<POLY_GRID_PARTITION> m_cachedPoly;
    int m_subpolyIndex;
};

class CN_ZONE_LIST : public CN_LIST
{
public:
    CN_ZONE_LIST() { }

    const std::vector<CN_ITEM*> Add( ZONE_CONTAINER* zone )
    {
        const auto& polys = zone->GetFilledPolysList();
        std::vector<CN_ITEM*> rv;

        //printf("add zone %p\n", zone);
        for( int j = 0; j < polys.OutlineCount(); j++ )
        {
            CN_ZONE* zitem = new CN_ZONE( zone, false, j );
            const auto& outline = zone->GetFilledPolysList().COutline( j );

            for( int k = 0; k < outline.PointCount(); k++)
                addAnchor   ( outline.CPoint( k ), zitem );

            //printf("added %d anchors\n", outline.PointCount());

            m_items.push_back( zitem );
            rv.push_back( zitem );
            SetDirty();
        }

        return rv;
    };
};


template <class T>
void CN_LIST::FindNearby( BOX2I aBBox, T aFunc )
{
    for( auto &p : m_anchors )
    {
            if ( p.Valid() && aBBox.Contains( p.Pos() ) )
                aFunc( p );
    }
}

template <class T>
void CN_LIST::FindNearby( VECTOR2I aPosition, int aDistMax, T aFunc )
{
    /* Search items in m_Candidates that position is <= aDistMax from aPosition
     * (Rectilinear distance)
     * m_Candidates is sorted by X then Y values, so a fast binary search is used
     * to locate the "best" entry point in list
     * The best entry is a pad having its m_Pos.x == (or near) aPosition.x
     * All candidates are near this candidate in list
     * So from this entry point, a linear search is made to find all candidates
     */

    sort();

    int idxmax = m_anchors.size() - 1;

    int delta = idxmax + 1;
    int idx = 0;        // Starting index is the beginning of list

    while( delta )
    {
        // Calculate half size of remaining interval to test.
        // Ensure the computed value is not truncated (too small)
        if( (delta & 1) && ( delta > 1 ) )
            delta++;

        delta /= 2;

        auto p = m_anchors[idx];

        int dist = p.Pos().x - aPosition.x;

        if( std::abs( dist ) <= aDistMax )
        {
            break;                              // A good entry point is found. The list can be scanned from this point.
        }
        else if( p.Pos().x < aPosition.x )      // We should search after this point
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
        auto &p = m_anchors[ii];
        diff = p.Pos() - aPosition;;

        if( std::abs( diff.x ) > aDistMax )
            break; // Exit: the distance is to long, we cannot find other candidates

        if( std::abs( diff.y ) > aDistMax )
            continue; // the y distance is to long, but we can find other candidates

        // We have here a good candidate: add it
        if(p.Valid())
            aFunc( p );
    }

    // search previous candidates in list
    for(  int ii = idx - 1; ii >=0; ii-- )
    {
        auto &p = m_anchors[ii];
        diff = p.Pos() - aPosition;

        if( abs( diff.x ) > aDistMax )
            break;

        if( abs( diff.y ) > aDistMax )
            continue;

        // We have here a good candidate:add it
        if(p.Valid())
            aFunc( p );
    }
}

class CN_CONNECTIVITY_ALGO
{
public:
    using CLUSTERS = std::vector<CN_CLUSTER_PTR>;
private:

    class ITEM_MAP_ENTRY
    {
    public:
        ITEM_MAP_ENTRY( CN_ITEM *aItem = nullptr )
        {
            if (aItem)
                m_items.push_back(aItem);
        }

        void MarkItemsAsInvalid ()
        {

            for ( auto item : m_items )
            {
                item -> SetValid ( false );
                //for ( auto anchor : item->Anchors() )
                //{
                //    anchor->SetValid(false);
                //}
            }
        }

        void Link( CN_ITEM *aItem )
        {
            m_items.push_back( aItem );
        }

        void AddRatsnestNode( RN_NODE_PTR aNode )
        {
            m_rnNodes.push_back( aNode );
        }

        void ClearRatsnestNodes( )
        {
            m_rnNodes.clear();
        }

        std::vector<RN_NODE_PTR>& RatsnestNodes() { return m_rnNodes; };
        std::vector<RN_NODE_PTR> m_rnNodes;
        std::list<CN_ITEM *> m_items;
    };

    enum CLUSTER_SEARCH_MODE
    {
        CSM_PROPAGATE,
        CSM_CONNECTIVITY_CHECK,
        CSM_RATSNEST
    };

    CN_PAD_LIST m_padList;
    CN_TRACK_LIST m_trackList;
    CN_VIA_LIST m_viaList;
    CN_ZONE_LIST m_zoneList;

    using ITEM_MAP_PAIR = std::pair <const BOARD_CONNECTED_ITEM*, ITEM_MAP_ENTRY>;

    std::unordered_map<const BOARD_CONNECTED_ITEM*, ITEM_MAP_ENTRY> m_itemMap;

    CLUSTERS m_connClusters;
    CLUSTERS m_ratsnestClusters;
    std::vector<bool> m_dirtyNets;

    void searchConnections( bool aIncludeZones = false );
    const CLUSTERS searchClusters( CLUSTER_SEARCH_MODE aMode );

    void update();
    void propagateConnections();

    template <class Container, class BItem>
    void add( Container& c, BItem brditem )
    {
        auto item = c.Add( brditem );
        m_itemMap[ brditem ] = ITEM_MAP_ENTRY ( item );
    }

    bool isDirty() const;

    void markNetAsDirty ( int aNet );
    void markItemNetAsDirty( const BOARD_ITEM *aItem );



public:

    CN_CONNECTIVITY_ALGO();
    ~CN_CONNECTIVITY_ALGO();

    ITEM_MAP_ENTRY& ItemEntry ( const BOARD_CONNECTED_ITEM* aItem )
    {
        return m_itemMap[ aItem ];
    }


    bool IsNetDirty( int aNet) const
    {
        return m_dirtyNets[ aNet ];
    }

    void ClearDirtyFlags()
    {
        for ( auto i = m_dirtyNets.begin(); i != m_dirtyNets.end(); ++i )
            *i = false;
    }

    void GetDirtyClusters( CLUSTERS& aClusters )
    {
        for ( auto cl : m_ratsnestClusters )
        {
            int net = cl->OriginNet();
            if ( net >= 0 && m_dirtyNets[net] )
                aClusters.push_back( cl );
        }
    }

    int NetCount() const
    {
        return m_dirtyNets.size();
    }

    void Build( BOARD* aBoard );
    void Build( const std::vector<BOARD_ITEM *> &aItems );

    bool Remove( BOARD_ITEM* aItem );
    bool Add( BOARD_ITEM* aItem );

    //void AddRatsnestNode ( BOARD_CONNECTED_ITEM* aItem, RN_NODE_PTR aNode );
    //std::vector<RN_NODE_PTR>& GetRatsnestNodes() const;


// PUBLIC API
    void    PropagateNets();
    void    FindIsolatedCopperIslands( ZONE_CONTAINER* aZone, std::vector<int>& aIslands );
    bool    CheckConnectivity( std::vector<CN_DISJOINT_NET_ENTRY>& aReport );

    const CLUSTERS& GetClusters();
    int GetUnconnectedCount();
};

#endif
