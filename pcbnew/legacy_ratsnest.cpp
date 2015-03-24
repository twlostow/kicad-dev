/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007-2014 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 1992-2012 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

/**
 * @file legacy_ratsnest.cpp
 * @brief Ratsnet functions.
 */

#include <fctsys.h>
#include <gr_basic.h>
#include <common.h>
#include <class_drawpanel.h>
#include <colors_selection.h>
#include <wxBasePcbFrame.h>
#include <macros.h>

#include <class_board.h>
#include <class_module.h>
#include <class_track.h>

#include <pcbnew.h>

#include <minimun_spanning_tree.h>
#include <legacy_ratsnest.h>
#include <pad_index.h>

wxPoint     g_Offset_Module;     // module offset used when moving a footprint

/* Sort function used by  QSORT
 *  Sort pads by net code
 */
static bool sortByNetcode( const D_PAD* const & ref, const D_PAD* const & item )
{
    return ref->GetNetCode() < item->GetNetCode();
}


/**
 * @brief class MIN_SPAN_TREE_PADS (derived from MIN_SPAN_TREE) specializes
 * the base class to calculate a minimum spanning tree from a list of pads,
 * and to add this tree as ratsnest to the main ratsnest list.
 */
class MIN_SPAN_TREE_PADS: public MIN_SPAN_TREE
{
    friend class MIN_SPAN_TREE;
public:
    std::vector <D_PAD*>* m_PadsList;   // list of pads:
    /* these pads are the parents of nodes of the tree.
     * Each node position is the corresponding pad position.
     * This pad list is used to evaluate the weight of an edge in tree.
     * -> edge = link between 2 nodes = links between 2 pads.
     * -> weight of a link = rectilinear distance between the 2 pads
     */

public:
    MIN_SPAN_TREE_PADS(): MIN_SPAN_TREE()
    {
        m_PadsList = NULL;
    }

    void MSP_Init( std::vector <D_PAD*>* aPadsList )
    {
        m_PadsList = aPadsList;
        MIN_SPAN_TREE::MSP_Init( (int) m_PadsList->size() );
    }

    /**
     * Function AddTreeToRatsnest
     * Adds the current minimum spanning tree as ratsnest items
     * to the main ratsnest list
     * @param aRatsnestList = a ratsnest list to add to
     */
    void AddTreeToRatsnest( LEGACY_RATSNEST_ITEMS* aRatsnestList );

    /**
     * Function GetWeight
     * calculates the weight between 2 items
     * NOTE: The weight between a node and itself should be 0
     * @param aItem1 = first item
     * @param aItem2 = other item
     * @return the weight between items ( the rectilinear distance )
     */
    int GetWeight( int aItem1, int aItem2 );
};


void MIN_SPAN_TREE_PADS::AddTreeToRatsnest( LEGACY_RATSNEST_ITEMS* aRatsnestList )
{
    std::vector<D_PAD*>& padsBuffer = *m_PadsList;

    if( padsBuffer.empty() )
        return;

    int netcode = padsBuffer[0]->GetNetCode();

    // Note: to get edges in minimum spanning tree,
    // the index value 0 is not used: it is just
    // the entry point of the minimum spanning tree.
    // The first edge (i.e. rastnest) starts at index 1
    for( int ii = 1; ii < m_Size; ii++ )
    {
        // Create the new ratsnest
        LEGACY_RATSNEST_ITEM net;

        net.SetNet( netcode );
        net.m_Status   = CH_ACTIF | CH_VISIBLE;
        net.m_Length   = GetDist(ii);
        net.m_PadStart = padsBuffer[ii];
        net.m_PadEnd   = padsBuffer[ GetWhoTo(ii) ];

        aRatsnestList->push_back( net );
    }
}

/* Function GetWeight
 * calculates the weight between 2 items
 * Here it calculate the rectilinear distance between 2 pads (2 items)
 * NOTE: The weight between a node and itself should be <=0
 * aItem1 and aItem2 are the 2 items
 * return the rectilinear distance
 */
int MIN_SPAN_TREE_PADS::GetWeight( int aItem1, int aItem2 )
{
    // NOTE: The distance (weight) between a node and itself should be 0
    // so we add 1 to other distances to be sure we never have 0
    // in cases other than a node and itself

    D_PAD* pad1 = (*m_PadsList)[aItem1];
    D_PAD* pad2 = (*m_PadsList)[aItem2];

    if( pad1 == pad2 )
        return 0;

    int weight = abs( pad2->GetPosition().x - pad1->GetPosition().x ) +
                 abs( pad2->GetPosition().y - pad1->GetPosition().y );
    return weight + 1;
}

/* class LEGACY_RATSNEST implementation */

LEGACY_RATSNEST::LEGACY_RATSNEST ( BOARD *aBoard ) :
    m_board( aBoard )
{

}

LEGACY_RATSNEST::~LEGACY_RATSNEST ( )
{

}

void LEGACY_RATSNEST::SetVisible ( bool aIsVisible )
{
    // we must clear or set the CH_VISIBLE flags to hide/show ratsnest
    // because we have a tool to show/hide ratsnest relative to a pad or a module
    // so the hide/show option is a per item selection
    if( m_board->IsElementVisible( RATSNEST_VISIBLE ) )
    {
        for( unsigned ii = 0; ii < m_FullRatsnest.size(); ii++ )
            m_FullRatsnest[ii].m_Status |= CH_VISIBLE;
    }
    else
    {
        for( unsigned ii = 0; ii < m_FullRatsnest.size(); ii++ )
            m_FullRatsnest[ii].m_Status &= ~CH_VISIBLE;
    }
}

void LEGACY_RATSNEST::Clear()
{
    m_FullRatsnest.clear();
    m_LocalRatsnest.clear();
}


void LEGACY_RATSNEST::BuildRatsnestForModule( MODULE* aModule )
{
    // for local ratsnest calculation when moving a footprint:
    // list of pads to use for this local ratsnets:
    // this is the list of connected pads of the current module,
    // and all pads connected to these pads:
    static std::vector <D_PAD*> localPadList;
    static unsigned pads_module_count;  // node count (node = pad with a net
                                        // code) for the footprint being moved
    static unsigned internalRatsCount;  // number of internal links (links
                                        // between pads of the module)
    D_PAD*          pad_ref;
    D_PAD*          pad_externe;
    int             current_net_code;
    int             distance;
    wxPoint         pad_pos;            // True pad position according to the
                                        // current footprint position

    if( (m_board->GetStatus() & LISTE_PAD_OK) == 0 )
    {
        m_board->SetStatus( 0 );
        m_board->BuildListOfNets();
    }

    /* Compute the "local" ratsnest if needed (when this footprint starts move)
     *  and the list of external pads to consider, i.e pads in others
     * footprints which are "connected" to
     *  a pad in the current footprint
     */
    if( (m_board->GetStatus() & RATSNEST_ITEM_LOCAL_OK) == 0 )
    {
        // Compute the "internal" ratsnest, i.e the links between the current
        // footprint pads
        localPadList.clear();
        m_LocalRatsnest.clear();

        // collect active pads of the module:
        for( pad_ref = aModule->Pads();  pad_ref;  pad_ref = pad_ref->Next() )
        {
            if( pad_ref->GetNetCode() == NETINFO_LIST::UNCONNECTED )
                continue;

            localPadList.push_back( pad_ref );
            pad_ref->SetSubRatsnest( 0 );
            pad_ref->SetSubNet( 0 );
        }

        pads_module_count = localPadList.size();

        if( pads_module_count == 0 )
            return;  // no connection!

        sort( localPadList.begin(), localPadList.end(), sortByNetcode );

        // Build the list of pads linked to the current footprint pads
        current_net_code = 0;

        for( unsigned ii = 0; ii < pads_module_count; ii++ )
        {
            pad_ref = localPadList[ii];

            if( pad_ref->GetNetCode() == current_net_code )
                continue;

            // A new net was found, load all pads of others modules members of this net:
            NETINFO_ITEM* net = pad_ref->GetNet();

            if( net == NULL )       //Should not occur
            {
                wxMessageBox( wxT( "BuildRatsnestForModule() error: net not found" ) );
                return;
            }

            PAD_INDEX::PADS& netPads = m_board->GetPadIndex().AllPadsInNet( net );

            for( unsigned jj = 0; jj < netPads.size(); jj++ )
            {
                pad_externe = netPads[jj];

                if( pad_externe->GetParent() == aModule )
                    continue;

                pad_externe->SetSubRatsnest( 0 );
                pad_externe->SetSubNet( 0 );

                localPadList.push_back( pad_externe );
            }
        }

        // Sort the pad list by net_code
        sort( localPadList.begin() + pads_module_count, localPadList.end(),
               sortByNetcode );

        /* Compute the internal rats nest:
         *  this is the same as general ratsnest, but considers only the current
         * footprint pads it is therefore not time consuming, and it is made only
         * once
         */
        current_net_code = localPadList[0]->GetNetCode();

        MIN_SPAN_TREE_PADS  min_spanning_tree;
        std::vector<D_PAD*> padsBuffer;     // contains pads of only one net

        for( unsigned ii = 0; ii < pads_module_count; ii++ )
        {
            // Search the end of pad list relative to the current net
            unsigned jj = ii + 1;

            for( ; jj <= pads_module_count; jj++ )
            {
                if( jj >= pads_module_count )
                    break;

                if( localPadList[jj]->GetNetCode() != current_net_code )
                    break;
            }

            for( unsigned kk = ii;  kk < jj;  kk++ )
                padsBuffer.push_back( localPadList[kk] );

            min_spanning_tree.MSP_Init( &padsBuffer );
            min_spanning_tree.BuildTree();
            min_spanning_tree.AddTreeToRatsnest( &m_LocalRatsnest );
            padsBuffer.clear();

            ii = jj;

            if( ii < localPadList.size() )
                current_net_code = localPadList[ii]->GetNetCode();
        }

        internalRatsCount = m_LocalRatsnest.size();

        // set the flag LOCAL_RATSNEST_ITEM of the ratsnest status:
        for( unsigned ii = 0; ii < m_LocalRatsnest.size(); ii++ )
            m_LocalRatsnest[ii].m_Status = LOCAL_RATSNEST_ITEM;

        m_board->SetStatusBits( RATSNEST_ITEM_LOCAL_OK );
    }   // End of internal ratsnest build

    /* This section computes the "external" ratsnest: it is done when the
     * footprint position changes
     *
     * This section search:
     *  for each current module pad the nearest neighbor external pad (of
     * course for the same net code).
     *  For each current footprint cluster of pad (pads having the same net
     * code),
     *  we search the smaller rats nest.
     *  so, for each net, only one rats nest item is created
     */
    LEGACY_RATSNEST_ITEM local_rats;

    local_rats.m_Length = INT_MAX;
    local_rats.m_Status = 0;
    bool addRats = false;

    // Erase external ratsnest items:
    if( internalRatsCount < m_LocalRatsnest.size() )
        m_LocalRatsnest.erase( m_LocalRatsnest.begin() + internalRatsCount,
                               m_LocalRatsnest.end() );

    current_net_code = localPadList[0]->GetNetCode();

    for( unsigned ii = 0; ii < pads_module_count; ii++ )
    {
        pad_ref = localPadList[ii];

        if( pad_ref->GetNetCode() != current_net_code )
        {
            // if needed, creates a new ratsnest for the old net
            if( addRats )
            {
                m_LocalRatsnest.push_back( local_rats );
            }

            addRats = false;
            current_net_code    = pad_ref->GetNetCode();
            local_rats.m_Length = INT_MAX;
        }

        pad_pos = pad_ref->GetPosition() - g_Offset_Module;

        // Search the nearest external pad of this current pad
        for( unsigned jj = pads_module_count; jj < localPadList.size(); jj++ )
        {
            pad_externe = localPadList[jj];

            // we search pads having the same net code
            if( pad_externe->GetNetCode() < pad_ref->GetNetCode() )
                continue;

            if( pad_externe->GetNetCode() > pad_ref->GetNetCode() ) // pads are sorted by net code
                break;

            distance = abs( pad_externe->GetPosition().x - pad_pos.x ) +
                       abs( pad_externe->GetPosition().y - pad_pos.y );

            if( distance < local_rats.m_Length )
            {
                local_rats.m_PadStart = pad_ref;
                local_rats.m_PadEnd   = pad_externe;
                local_rats.SetNet( pad_ref->GetNetCode() );
                local_rats.m_Length = distance;
                local_rats.m_Status = 0;

                addRats = true;
            }
        }
    }

    if( addRats ) // Ensure the last created rats nest item is stored in buffer
        m_LocalRatsnest.push_back( local_rats );
}


/* class LEGACY_RATSNEST_ITEM implementation */

LEGACY_RATSNEST_ITEM::LEGACY_RATSNEST_ITEM()
{
    m_NetCode  = 0;         // netcode ( = 1.. n ,  0 is the value used for not
                            // connected items)
    m_Status   = 0;         // state
    m_PadStart = NULL;      // pointer to the starting pad
    m_PadEnd   = NULL;      // pointer to ending pad
    m_Length   = 0;         // length of the line (temporary used in some
                            // calculations)
}


/**
 * Function Draw
 * Draws a line (a ratsnest) from the starting pad to the ending pad
 */
void LEGACY_RATSNEST_ITEM::Draw( EDA_DRAW_PANEL* panel,
                          wxDC*           DC,
                          GR_DRAWMODE     aDrawMode,
                          const wxPoint&  aOffset )
{
    GRSetDrawMode( DC, aDrawMode );

    EDA_COLOR_T color = g_ColorsSettings.GetItemColor(RATSNEST_VISIBLE);

    GRLine( panel->GetClipBox(), DC,
            m_PadStart->GetPosition() - aOffset,
            m_PadEnd->GetPosition() - aOffset, 0, color );
}

    


/* Note about the ratsnest computation:
 * Building the general ratsnest:
 * For each net, the ratsnest is the set of lines connecting pads,
 * using the shorter distance
 * Therefore this problem is well known in graph therory, and sloved
 * using the "minimum spanning tree".
 * We use here an algorithm to build the minimum spanning tree known as Prim's algorithm
 */



/**
 * Function to compute the full ratsnest
 * This is the "basic" ratsnest depending only on pads.
 *
 * Create the sorted pad list (if necessary)
 * The active pads (i.e included in a net ) are called nodes
 * This pad list is sorted by net codes
 * A ratsnest can be seen as a logical connection.
 *
 * Update :
 *      nb_nodes = Active pads count for the board
 *      nb_links = link count for the board (logical connection count)
 *      (there are n-1 links in a net which counting n active pads) .
 */
void LEGACY_RATSNEST::BuildBoardRatsnest()
{
    int    noconn;
    
    m_board->SetUnconnectedNetCount( 0 );
    m_FullRatsnest.clear();

    PAD_INDEX& pads = m_board->GetPadIndex();

    if( pads.Size() == 0 )
        return;

    // Created pad list and the net_codes if needed
    if( (m_board->GetStatus( ) & NET_CODES_OK) == 0 )
        m_board->BuildListOfNets();

    for( unsigned ii = 0; ii < pads.Size(); ++ii )
    {
        pads[ii]->SetSubRatsnest( 0 );
    }

    if( m_board->GetNodesCount() == 0 )
        return;                       // No useful connections.

    // Ratsnest computation
    unsigned current_net_code = 1;      // First net code is analyzed.
                                        // (net_code = 0 -> no connect)
    noconn = 0;
    MIN_SPAN_TREE_PADS min_spanning_tree;

    for( ; current_net_code < m_board->GetNetCount(); current_net_code++ )
    {
        NETINFO_ITEM* net = m_board->FindNet( current_net_code );

        if( !net )       // Should not occur
        {
            UTF8 msg = StrPrintf( "%s: error, net %d not found", __func__, current_net_code );
            wxMessageBox( msg );   // BTW, it does happen.
            return;
        }

        net->m_RatsnestStartIdx = m_board->GetRatsnestsCount();
        min_spanning_tree.MSP_Init( &m_board->GetPadIndex().AllPadsInNet( net ) );
        min_spanning_tree.BuildTree();
        min_spanning_tree.AddTreeToRatsnest( &m_FullRatsnest );
        net->m_RatsnestEndIdx = m_board->GetRatsnestsCount();
    }

    m_board->SetUnconnectedNetCount( noconn );
    m_board->SetStatusBits( LISTE_RATSNEST_ITEM_OK );

    // Update the ratsnest display option (visible/invisible) flag
    for( unsigned ii = 0; ii < m_board->GetRatsnestsCount(); ii++ )
    {
        if( !m_board->IsElementVisible( RATSNEST_VISIBLE ) )  // Clear VISIBLE flag
            m_FullRatsnest[ii].m_Status &= ~CH_VISIBLE;
    }
}




/**
 * Function used by TestForActiveLinksInRatsnest
 *  Function testing the ratsnest between 2 blocks ( of the same net )
 *  The search is made between pads in block 1 and the others blocks
 *  The block n ( n > 1 ) is merged with block 1 and linked by the smallest ratsnest
 *  between block 1 and the block n (activate the logical connection)
 *  @param  aRatsnestBuffer = the buffer to store NETINFO_ITEM* items
 *  @param  aNetinfo = the current NETINFO_ITEM for the current net
 *  output: .state member, bit CH_ACTIF of the ratsnest item
 *  @return  last subratsnest id in use
 */
static int testLinksBetweenBlocks( NETINFO_ITEM*          aNetinfo,
                                   LEGACY_RATSNEST_ITEMS& aRatsnestBuffer )
{
    int            subratsnest_id, min_id;
    LEGACY_RATSNEST_ITEM* link, * best_link;

    // Search a link from a block to an other block
    best_link = NULL;

    for( unsigned ii = aNetinfo->m_RatsnestStartIdx; ii < aNetinfo->m_RatsnestEndIdx; ii++ )
    {
        link = &aRatsnestBuffer[ii];

        // If this link joints 2 pads inside the same block, do nothing
        // (these pads are already connected)
        if( link->m_PadStart->GetSubRatsnest() == link->m_PadEnd->GetSubRatsnest() )
            continue;

        // This link joints 2 pads of different blocks: this is a candidate,
        // but we want to select the shorter link, so use it only if it is shorter
        // than the previous candidate:
        if( best_link == NULL )  // no candidate
            best_link = link;
        else if( best_link->m_Length > link->m_Length )  // It is a better candidate.
            best_link = link;
    }

    if( best_link == NULL )
        return 1;

    /* At this point we have found a link between 2 different blocks (subratsnest)
     * we must set its status to ACTIVE and merge the 2 blocks
     */
    best_link->m_Status |= CH_ACTIF;
    subratsnest_id   = best_link->m_PadStart->GetSubRatsnest();
    min_id = best_link->m_PadEnd->GetSubRatsnest();

    if( min_id > subratsnest_id )
        EXCHG( min_id, subratsnest_id );

    PAD_INDEX::PADS& netPads = aNetinfo->GetParent()->GetPadIndex().AllPadsInNet( aNetinfo );

    // Merge the 2 blocks in one sub ratsnest:
    for( unsigned ii = 0; ii < netPads.size(); ii++ )
    {
        if( netPads[ii]->GetSubRatsnest() == subratsnest_id )
        {
            netPads[ii]->SetSubRatsnest( min_id );
        }
    }

    return subratsnest_id;
}


/**
 * Function used by TestForActiveLinksInRatsnest_general
 *  The general ratsnest list must exists because this function explores this ratsnest
 *  Activates (i.e. set the CH_ACTIF flag) the ratsnest links between 2 pads when
 *  at least one pad not already connected (SubRatsnest = 0)
 *  and actives the corresponding link
 *
 * @param   aFirstItem = starting address for the ratsnest list
 * @param   aLastItem   = ending address for the ratsnest list
 * @param   aCurrSubRatsnestId =  last sub ratsnest id in use (computed from the track
 * analysis)
 *
 *      output:
 *          ratsnest list (status member bit CH_ACTIF set)
 *          and pads linked (m_SubRatsnest value set)
 *
 * @return new block number
 */
static void testLinksBetweenPads(   int&            aCurrSubRatsnestId,
                                    LEGACY_RATSNEST_ITEM*  aFirstItem,
                                    LEGACY_RATSNEST_ITEM*  aLastItem )
{
    for( LEGACY_RATSNEST_ITEM* item = aFirstItem; item < aLastItem; item++ )
    {
        D_PAD* pad_start = item->m_PadStart;
        D_PAD* pad_end   = item->m_PadEnd;

        /* Update the current SubRatsnest if the 2 pads are not connected :
         * a new cluster is created and the link activated
         */
        if( (pad_start->GetSubRatsnest() == 0) && (pad_end->GetSubRatsnest() == 0) )
        {
            aCurrSubRatsnestId++;
            pad_start->SetSubRatsnest( aCurrSubRatsnestId );
            pad_end->SetSubRatsnest( aCurrSubRatsnestId );
            item->m_Status |= CH_ACTIF;
        }

        /* If a pad is already connected to a subratsnest: activate the link
         * the pad other is merged in the existing subratsnest
         */
        else if( pad_start->GetSubRatsnest() == 0 )
        {
            pad_start->SetSubRatsnest( pad_end->GetSubRatsnest() );
            item->m_Status |= CH_ACTIF;
        }
        else if( pad_end->GetSubRatsnest() == 0 )
        {
            pad_end->SetSubRatsnest( pad_start->GetSubRatsnest() );
            item->m_Status |= CH_ACTIF;
        }
    }
}

/* function TestForActiveLinksInRatsnest
 * determine the active links inside the full ratsnest
 *
 * I used an algorithm inspired by the "Lee algorithm".
 * The idea is all pads must be connected by a physical track or a logical track
 * a physical track is the existing track on copper layers.
 * a logical track is the link that must be activated (visible) if
 * no track found between 2 pads.
 * The algorithm explore the existing full ratnest
 * This is a 2 steps algorithm (executed for each net).
 * - First:
 *   Initialise for each pad the subratsnest id to its subnet value
 *   explore the full ratnest (relative to the net) and active a link each time at least one pad of
 *   the given link is not connected to an other pad by a track ( subratsnest = 0)
 *   If the 2 pads linked have both the subratsnest id = 0, a new subratsnest value is created
 * -  Second:
 *   explore the full ratnest (relative to the net) and find a link that links
 *   2 pads having different subratsnest values
 *   Active the link and merge the 2 subratsnest value.
 *
 * This is usually fast because the ratsnest is not built here: it is just explored
 * to see what link must be activated
 */
void LEGACY_RATSNEST::TestForActiveLinksInRatsnest( int aNetCode )
{
    LEGACY_RATSNEST_ITEM* rats;
    D_PAD*         pad;
    NETINFO_ITEM*  net;

    if( m_board->GetPadIndex().Size() == 0 )
        return;

    if( (m_board->GetStatus() & LISTE_RATSNEST_ITEM_OK) == 0 )
        BuildBoardRatsnest();

    for( int net_code = 1; net_code < (int) m_board->GetNetCount(); net_code++ )
    {
        net = m_board->FindNet( net_code );

        wxCHECK_RET( net != NULL,
                     wxString::Format( wxT( "Net code %d not found!" ), net_code ) );

        if( aNetCode && (net_code != aNetCode) )
            continue;

        // Create subratsnests id from subnets created by existing tracks:
        int subratsnest = 0;
        
        PAD_INDEX::PADS& netPads = m_board->GetPadIndex().AllPadsInNet( net );

        for( unsigned ip = 0; ip < netPads.size(); ip++ )
        {
            pad = netPads[ip];
            int subnet = pad->GetSubNet();
            pad->SetSubRatsnest( subnet );
            subratsnest = std::max( subratsnest, subnet );
        }

        for( unsigned ii = net->m_RatsnestStartIdx; ii < net->m_RatsnestEndIdx; ii++ )
        {
            m_FullRatsnest[ii].m_Status &= ~CH_ACTIF;
        }

        // First pass - activate links for not connected pads
        rats = &m_FullRatsnest[0];
        testLinksBetweenPads( subratsnest,
                              rats + net->m_RatsnestStartIdx,
                              rats + net->m_RatsnestEndIdx );

        // Second pass activate links between blocks (Iteration)
        while( subratsnest > 1 )
        {
            subratsnest = testLinksBetweenBlocks( net, m_FullRatsnest );
        }
    }

    m_board->SetUnconnectedNetCount( 0 );

    unsigned cnt = 0;

    for( unsigned ii = 0; ii < m_board->GetRatsnestsCount(); ii++ )
    {
        if( m_FullRatsnest[ii].IsActive() )
            cnt++;
    }

    m_board->SetUnconnectedNetCount( cnt );
}

/**
 *  function DrawGeneralRatsnest
 *  Only ratsnest items with the status bit CH_VISIBLE set are displayed
 * @param aDC = the current device context (can be NULL)
 * @param aNetcode: if > 0, Display only the ratsnest relative to the
 * corresponding net_code
 */
void PCB_BASE_FRAME::DrawGeneralRatsnest( wxDC* aDC, int aNetcode )
{
    if( ( m_Pcb->GetStatus() & LISTE_RATSNEST_ITEM_OK ) == 0 )
        return;

    if( ( m_Pcb->GetStatus() & DO_NOT_SHOW_GENERAL_RASTNEST ) )
        return;

    if( aDC == NULL )
        return;

    const int state = CH_VISIBLE | CH_ACTIF;

    LEGACY_RATSNEST_ITEMS& rats = m_Pcb->GetLegacyRatsnest()->GetFull();

    for( unsigned ii = 0; ii < rats.size(); ii++ )
    {
        LEGACY_RATSNEST_ITEM& item = rats[ii];

        if( ( item.m_Status & state ) != state )
            continue;

        if( ( aNetcode <= 0 ) || ( aNetcode == item.GetNet() ) )
        {
            item.Draw( m_canvas, aDC, GR_XOR, wxPoint( 0, 0 ) );
        }
    }
}

void PCB_BASE_FRAME::TraceModuleRatsNest( wxDC* DC )
{
    if( DC == NULL )
        return;

    if( ( m_Pcb->GetStatus() & RATSNEST_ITEM_LOCAL_OK ) == 0 )
        return;

    EDA_COLOR_T tmpcolor = g_ColorsSettings.GetItemColor(RATSNEST_VISIBLE);

    LEGACY_RATSNEST_ITEMS& localRats = m_Pcb->GetLegacyRatsnest()->GetLocal();

    for( unsigned ii = 0; ii < localRats.size(); ii++ )
    {
        LEGACY_RATSNEST_ITEM* rats = &localRats[ii];

        if( rats->m_Status & LOCAL_RATSNEST_ITEM )
        {
            g_ColorsSettings.SetItemColor(RATSNEST_VISIBLE, YELLOW);
            rats->Draw( m_canvas, DC, GR_XOR, g_Offset_Module );
        }
        else
        {
            g_ColorsSettings.SetItemColor(RATSNEST_VISIBLE, tmpcolor);

            wxPoint tmp = rats->m_PadStart->GetPosition();

            rats->m_PadStart->SetPosition( tmp - g_Offset_Module );
            rats->Draw( m_canvas, DC, GR_XOR, wxPoint( 0, 0 ) );

            rats->m_PadStart->SetPosition( tmp );
        }
    }

    g_ColorsSettings.SetItemColor( RATSNEST_VISIBLE, tmpcolor );
}


/*
 * PCB_BASE_FRAME::BuildAirWiresTargetsList and
 * PCB_BASE_FRAME::TraceAirWiresToTargets
 * are 2 function to show the near connecting points when
 * a new track is created, by displaying g_MaxLinksShowed airwires
 * between the on grid mouse cursor and these connecting points
 * during the creation of a track
 */

/* Buffer to store pads coordinates when creating a track.
 *  these pads are members of the net
 *  and when the mouse is moved, the g_MaxLinksShowed links to neighbors are
 * drawn
 */
static std::vector <wxPoint> s_TargetsLocations;
static wxPoint s_CursorPos; // Coordinate of the moving point (mouse cursor and
                            // end of current track segment)

/* Used by BuildAirWiresTargetsList(): sort function by link length
 * (rectilinear distance between s_CursorPos and item pos)
 */
static bool sort_by_distance( const wxPoint& ref, const wxPoint& compare )
{
    wxPoint deltaref = ref - s_CursorPos;       // relative coordinate of ref
    wxPoint deltacmp = compare - s_CursorPos;   // relative coordinate of compare

    // rectilinear distance between ref and s_CursorPos:
    int     lengthref = abs( deltaref.x ) + abs( deltaref.y );

    // rectilinear distance between compare and s_CursorPos:
    int     lengthcmp = abs( deltacmp.x ) + abs( deltacmp.y );

    return lengthref < lengthcmp;
}

static bool sort_by_point( const wxPoint& ref, const wxPoint& compare )
{
    if( ref.x == compare.x )
        return ref.y < compare.y;

    return ref.x < compare.x;
}

/* Function BuildAirWiresTargetsList
 * Build a list of candidates that can be a coonection point
 * when a track is started.
 * This functions prepares data to show airwires to nearest connecting points (pads)
 * from the current new track to candidates during track creation
 */
void PCB_BASE_FRAME::BuildAirWiresTargetsList( BOARD_CONNECTED_ITEM* aItemRef,
                                               const wxPoint& aPosition, bool aInit )
{
    if( ( ( m_Pcb->GetStatus() & LISTE_RATSNEST_ITEM_OK ) == 0 )
       || ( ( m_Pcb->GetStatus() & LISTE_PAD_OK ) == 0 )
       || ( ( m_Pcb->GetStatus() & NET_CODES_OK ) == 0 ) )
    {
        s_TargetsLocations.clear();
        return;
    }

    s_CursorPos = aPosition;    // needed for sort_by_distance

    if( aInit )
    {
        s_TargetsLocations.clear();

        if( aItemRef == NULL )
            return;

        int net_code = aItemRef->GetNetCode();
        int subnet = aItemRef->GetSubNet();

        if( net_code <= 0 )
            return;

        NETINFO_ITEM* net = m_Pcb->FindNet( net_code );

        if( net == NULL )        // Should not occur
        {
            wxMessageBox( wxT( "BuildAirWiresTargetsList() error: net not found" ) );
            return;
        }

        // Create a list of pads candidates ( pads not already connected to the
        // current track ):
        PAD_INDEX::PADS& netPads = m_Pcb->GetPadIndex().AllPadsInNet( net );

        for( unsigned ii = 0; ii < netPads.size(); ii++ )
        {
            D_PAD* pad = netPads[ii];

            if( pad == aItemRef )
                continue;

            if( !pad->GetSubNet() || (pad->GetSubNet() != subnet) )
                s_TargetsLocations.push_back( pad->GetPosition() );
        }

        // Create a list of tracks ends candidates, not already connected to the
        // current track:
        for( TRACK* track = m_Pcb->m_Track; track; track = track->Next() )
        {
            if( track->GetNetCode() < net_code )
                continue;
            if( track->GetNetCode() > net_code )
                break;

            if( !track->GetSubNet() || (track->GetSubNet() != subnet) )
            {
                if( aPosition != track->GetStart() )
                    s_TargetsLocations.push_back( track->GetStart() );
                if( aPosition != track->GetEnd() && track->GetStart() != track->GetEnd() )
                    s_TargetsLocations.push_back( track->GetEnd() );
            }
        }

        // Remove duplicate targets, using the C++ unique algorithm
        sort( s_TargetsLocations.begin(), s_TargetsLocations.end(), sort_by_point );
        std::vector< wxPoint >::iterator it = unique( s_TargetsLocations.begin(), s_TargetsLocations.end() );

        // Using the C++ unique algorithm only moves the duplicate entries to the end of
        // of the array.  This removes the duplicate entries from the array.
        s_TargetsLocations.resize( it - s_TargetsLocations.begin() );
    }   // end if Init

    // in all cases, sort by distances:
    sort( s_TargetsLocations.begin(), s_TargetsLocations.end(), sort_by_distance );
}


void PCB_BASE_FRAME::TraceAirWiresToTargets( wxDC* aDC )
{
    if( aDC == NULL )
        return;

    if( s_TargetsLocations.size() == 0 )
        return;

    GRSetDrawMode( aDC, GR_XOR );
    DISPLAY_OPTIONS* displ_opts = (DISPLAY_OPTIONS*)GetDisplayOptions();

    for( int ii = 0; ii < (int) s_TargetsLocations.size(); ii++ )
    {
        if( ii >= displ_opts->m_MaxLinksShowed )
            break;

        GRLine( m_canvas->GetClipBox(), aDC, s_CursorPos, s_TargetsLocations[ii], 0, YELLOW );
    }
}
