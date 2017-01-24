#ifndef __CONNECTIVITY_H
#define __CONNECTIVITY_H

#include <wx/string.h>
#include <vector>
#include <memory>

#include <math/vector2d.h>

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

class CN_CLUSTER
{
private:

    bool m_conflicting = false;
    int m_originNet = 0;
    CN_ITEM* m_originPad = nullptr;
    std::vector<CN_ITEM*> m_items;

    CN_RATSNEST_NODES *m_rnNodes = nullptr;

public:
    CN_CLUSTER();
    ~CN_CLUSTER();

    void SetRatsnestNodes( CN_RATSNEST_NODES* aNodes ) { m_rnNodes = aNodes; }
    CN_RATSNEST_NODES* GetRatsnestNodes() const { return m_rnNodes; }

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


class CN_CONNECTIVITY_ALGO
{
public:
    using CLUSTERS = std::vector<CN_CLUSTER_PTR>;

    CN_CONNECTIVITY_ALGO();
    ~CN_CONNECTIVITY_ALGO();

    bool Add ( BOARD_ITEM* aItem );
    bool Remove ( BOARD_ITEM* aItem);
    void Update ( BOARD_ITEM* aItem);

    void Build( BOARD* aBoard );
    void Build( const std::vector<BOARD_ITEM *> &aItems );

// PUBLIC API
    void    PropagateNets();
    void    FindIsolatedCopperIslands( ZONE_CONTAINER* aZone, std::vector<int>& aIslands );
    bool    CheckConnectivity( std::vector<CN_DISJOINT_NET_ENTRY>& aReport );

    void GetDirtyClusters( CLUSTERS& aClusters ) ;
    bool IsNetDirty( int aNet) const;
    void ClearDirtyFlags();
    int NetCount() const;


    const CLUSTERS& GetClusters();
    int GetUnconnectedCount();

private:
    unique_ptr<CN_CONNECTIVITY_ALGO_IMPL> m_pimpl;
};


// a wrapper class encompassing the connectivity computation algorithm and the
class CONNECTIVITY_DATA
{
public:
    CONNECTIVITY_DATA( BOARD* aBoard );

    /**
     * Function Add()
     * Adds an item to the connectivity data.
     * @param aItem is an item to be added.
     * @return True if operation succeeded.
     */
    bool Add( BOARD_ITEM* aItem );

    /**
     * Function Remove()
     * Removes an item from the connectivity data.
     * @param aItem is an item to be updated.
     * @return True if operation succeeded.
     */
    bool Remove( BOARD_ITEM* aItem );

    /**
     * Function Update()
     * Updates the connectivity data for an item.
     * @param aItem is an item to be updated.
     * @return True if operation succeeded. The item will not be updated if it was not previously
     * added to the ratsnest.
     */
    bool Update( BOARD_ITEM* aItem );

    /**
     * Function ProcessBoard()
     * Prepares data for computing (computes a list of current nodes and connections). It is
     * required to run only once after loading a board.
     */
    void ProcessBoard();

    int GetNetCount()
    {
        return m_connAlgo->NetCount();
    }

    RN_DATA *GetRatsnest()
    {
        return m_ratsnest;//.get();
    }

    RN_NET *GetRatsnestForNet ( int aNet )
    {
        return m_nets[aNet];
    }

    CN_CONNECTIVITY_ALGO* GetConnectivityAlgo()
    {
         return m_connAlgo;//.get();
    }


    void RecalculateRatsnest();

    int GetUnconnectedCount()
    {
        return 666;
        //return m_connAlgo->GetUnconnectedCount();
    }



private:

    BOARD *m_board;
    std::vector<RN_NET *> m_nets;

    RN_DATA* m_ratsnest;
    CN_CONNECTIVITY_ALGO* m_connAlgo;
};

#endif
