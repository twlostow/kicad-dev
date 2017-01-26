#ifndef __CONNECTIVITY_H
#define __CONNECTIVITY_H

#include <core/typeinfo.h>

#include <wx/string.h>
#include <vector>
#include <list>
#include <memory>

#include <math/vector2d.h>

using std::shared_ptr;
using std::unique_ptr;

class CN_ITEM;
class CN_CLUSTER;
class CN_CONNECTIVITY_ALGO;
class BOARD;
class BOARD_CONNECTED_ITEM;
class BOARD_ITEM;
class ZONE_CONTAINER;
class RN_DATA;
class RN_NET;

struct RN_DYNAMIC_LINE
{
    int netCode;
    VECTOR2I a, b;
};

// a wrapper class encompassing the connectivity computation algorithm and the
class CONNECTIVITY_DATA
{
public:
    CONNECTIVITY_DATA();

    void Build( BOARD* aBoard );
    void Build( const std::vector<BOARD_ITEM *> &aItems );


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

     int GetNetCount() const;


    RN_NET *GetRatsnestForNet ( int aNet )
    {
        return m_nets[aNet];
    }

    void GetConnectedItems( const BOARD_CONNECTED_ITEM* aItem,
                            std::list<BOARD_CONNECTED_ITEM*>& aOutput,
                            const KICAD_T aTypes[] ) const {};

    void GetNetItems( const BOARD_CONNECTED_ITEM* aItem,
                            std::list<BOARD_CONNECTED_ITEM*>& aOutput,
                            const KICAD_T aTypes[] ) const {};


    void PropagateNets();

    void FindIsolatedCopperIslands( ZONE_CONTAINER* aZone, std::vector<int>& aIslands );

    void RecalculateRatsnest();

    int GetUnconnectedCount()
    {
        return 666;
        //return m_connAlgo->GetUnconnectedCount();
    }

    void ComputeDynamicRatsnest( const std::vector<BOARD_ITEM *> &aItems  );
    void ClearDynamicRatsnest();

    /**
     * Function GetConnectedItems()
     * Adds items that are connected together to a list.
     * @param aItem is the reference item to find other connected items.
     * @param aOutput is the list that will contain found items.
     * @param aTypes allows to filter by item types.
     */
    //void GetConnectedItems( const BOARD_CONNECTED_ITEM* aItem,
    //                        std::list<BOARD_CONNECTED_ITEM*>& aOutput,
    //                        RN_ITEM_TYPE aTypes = RN_ALL ) const;

    /**
     * Function GetNetItems()
     * Adds all items that belong to a certain net to a list.
     * @param aNetCode is the net code.
     * @param aOutput is the list that will have items added.
     * @param aTypes allows to filter by item types.
     */
    //void GetNetItems( int aNetCode, std::list<BOARD_CONNECTED_ITEM*>& aOutput,
    //                        RN_ITEM_TYPE aTypes = RN_ALL ) const;

    /**
     * Function AreConnected()
     * Checks if two items are connected with copper.
     * @param aThis is the first item.
     * @param aOther is the second item.
     * @return True if they are connected, false otherwise.
     */
    //bool AreConnected( const BOARD_CONNECTED_ITEM* aItem, const BOARD_CONNECTED_ITEM* aOther );



    const std::vector<RN_DYNAMIC_LINE> & GetDynamicRatsnest() const;

private:

    void updateRatsnest();
    void addRatsnestCluster( std::shared_ptr<CN_CLUSTER> aCluster );
    void blockRatsnestItems( const std::vector<BOARD_ITEM *> &aItems );


    unique_ptr<CONNECTIVITY_DATA> m_dynamicConnectivity;
    shared_ptr<CN_CONNECTIVITY_ALGO> m_connAlgo;

    std::vector<RN_DYNAMIC_LINE> m_dynamicRatsnest;
    std::vector<RN_NET *> m_nets;
};

#endif
