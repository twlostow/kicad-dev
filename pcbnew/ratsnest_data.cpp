/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2013-2015 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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
 * @file ratsnest_data.cpp
 * @brief Class that computes missing connections on a PCB.
 */

#ifdef USE_OPENMP
#include <omp.h>
#endif /* USE_OPENMP */

#include <ratsnest_data.h>

#include <class_board.h>
#include <class_module.h>
#include <class_pad.h>
#include <class_track.h>
#include <class_zone.h>

#include <functional>
using namespace std::placeholders;

#include <geometry/shape_poly_set.h>

#include <cassert>
#include <algorithm>
#include <limits>

#ifdef PROFILE
#include <profile.h>
#endif

#include "connectivity_algo.h"

static uint64_t getDistance( const RN_NODE_PTR& aNode1, const RN_NODE_PTR& aNode2 )
{
    // Drop the least significant bits to avoid overflow
    int64_t x = ( aNode1->GetX() - aNode2->GetX() ) >> 16;
    int64_t y = ( aNode1->GetY() - aNode2->GetY() ) >> 16;

    // We do not need sqrt() here, as the distance is computed only for comparison
    return ( x * x + y * y );
}


static bool sortDistance( const RN_NODE_PTR& aOrigin, const RN_NODE_PTR& aNode1,
                   const RN_NODE_PTR& aNode2 )
{
    return getDistance( aOrigin, aNode1 ) < getDistance( aOrigin, aNode2 );
}


static bool sortWeight( const RN_EDGE_PTR& aEdge1, const RN_EDGE_PTR& aEdge2 )
{
    return aEdge1->GetWeight() < aEdge2->GetWeight();
}


bool operator==( const RN_NODE_PTR& aFirst, const RN_NODE_PTR& aSecond )
{
    return aFirst->GetX() == aSecond->GetX() && aFirst->GetY() == aSecond->GetY();
}


bool operator!=( const RN_NODE_PTR& aFirst, const RN_NODE_PTR& aSecond )
{
    return aFirst->GetX() != aSecond->GetX() || aFirst->GetY() != aSecond->GetY();
}


RN_NODE_AND_FILTER operator&&( const RN_NODE_FILTER& aFilter1, const RN_NODE_FILTER& aFilter2 )
{
    return RN_NODE_AND_FILTER( aFilter1, aFilter2 );
}


RN_NODE_OR_FILTER operator||( const RN_NODE_FILTER& aFilter1, const RN_NODE_FILTER& aFilter2 )
{
    return RN_NODE_OR_FILTER( aFilter1, aFilter2 );
}


static bool isEdgeConnectingNode( const RN_EDGE_PTR& aEdge, const RN_NODE_PTR& aNode )
{
    return aEdge->GetSourceNode() == aNode || aEdge->GetTargetNode() == aNode;
}


static std::vector<RN_EDGE_MST_PTR>* kruskalMST( RN_LINKS::RN_EDGE_LIST& aEdges,
                                                 std::vector<RN_NODE_PTR>& aNodes )
{
    unsigned int nodeNumber = aNodes.size();
    unsigned int mstExpectedSize = nodeNumber - 1;
    unsigned int mstSize = 0;
    bool ratsnestLines = false;

    // The output
    std::vector<RN_EDGE_MST_PTR>* mst = new std::vector<RN_EDGE_MST_PTR>;
    mst->reserve( mstExpectedSize );

    // Set tags for marking cycles
    std::unordered_map<RN_NODE_PTR, int> tags;
    unsigned int tag = 0;

    for( RN_NODE_PTR& node : aNodes )
    {
        node->SetTag( tag );
        tags[node] = tag++;
    }

    // Lists of nodes connected together (subtrees) to detect cycles in the graph
    std::vector<std::list<int> > cycles( nodeNumber );
    for( unsigned int i = 0; i < nodeNumber; ++i )
        cycles[i].push_back( i );

    // Kruskal algorithm requires edges to be sorted by their weight
    aEdges.sort( sortWeight );

    while( mstSize < mstExpectedSize && !aEdges.empty() )
    {
        RN_EDGE_PTR& dt = aEdges.front();

        int srcTag = tags[dt->GetSourceNode()];
        int trgTag = tags[dt->GetTargetNode()];

        // Check if by adding this edge we are going to join two different forests
        if( srcTag != trgTag )
        {
            // Because edges are sorted by their weight, first we always process connected
            // items (weight == 0). Once we stumble upon an edge with non-zero weight,
            // it means that the rest of the lines are ratsnest.
            if( !ratsnestLines && dt->GetWeight() != 0 )
                ratsnestLines = true;

            // Update tags
            if( ratsnestLines )
            {
                for( auto it = cycles[trgTag].begin(); it != cycles[trgTag].end(); ++it )
                {
                    tags[aNodes[*it]] = srcTag;
                }
                // Do a copy of edge, but make it RN_EDGE_MST. In contrary to RN_EDGE,
                // RN_EDGE_MST saves both source and target node and does not require any other
                // edges to exist for getting source/target nodes
                RN_EDGE_MST_PTR newEdge = std::make_shared<RN_EDGE_MST>( dt->GetSourceNode(),
                                                                         dt->GetTargetNode(),
                                                                         dt->GetWeight() );

                assert( newEdge->GetSourceNode()->GetTag() != newEdge->GetTargetNode()->GetTag() );
                assert( newEdge->GetWeight() > 0 );

                mst->push_back( newEdge );
                ++mstSize;
            }
            else
            {
                //for( it = cycles[trgTag].begin(), itEnd = cycles[trgTag].end(); it != itEnd; ++it )
                //for( auto it : cycles[trgTag] )
                for( auto it = cycles[trgTag].begin(); it != cycles[trgTag].end(); ++it )
                {
                    tags[aNodes[*it]] = srcTag;
                    aNodes[*it]->SetTag( srcTag );
                }

                // Processing a connection, decrease the expected size of the ratsnest MST
                --mstExpectedSize;
            }

            // Move nodes that were marked with old tag to the list marked with the new tag
            cycles[srcTag].splice( cycles[srcTag].end(), cycles[trgTag] );
        }

        // Remove the edge that was just processed
        aEdges.erase( aEdges.begin() );
    }

    // Probably we have discarded some of edges, so reduce the size
    mst->resize( mstSize );

    return mst;
}


void RN_NET::validateEdge( RN_EDGE_MST_PTR& aEdge )
{
    RN_NODE_PTR source = aEdge->GetSourceNode();
    RN_NODE_PTR target = aEdge->GetTargetNode();
    bool update = false, changed = false;

    // If any of nodes belonging to the edge has the flag set,
    // change it to the closest node that has flag cleared
    // note: finding the right nodes can be done iteratively to get the best results,
    // but it is not likely to be worth the time cost
    do
    {
        if( changed || source->GetNoLine() )
        {
            changed = false;
            std::list<RN_NODE_PTR> closest = GetClosestNodes( target,
                    LINE_TARGET_SAME_TAG( source->GetTag() ) );

            if( !closest.empty() )
            {
                RN_NODE_PTR& node = closest.front();

                if( node != source )
                {
                    changed = true;
                    update = true;
                    source = node;
                }
            }
        }

        if( changed || target->GetNoLine() )
        {
            changed = false;
            std::list<RN_NODE_PTR> closest = GetClosestNodes( source,
                    LINE_TARGET_SAME_TAG( target->GetTag() ) );

            if( !closest.empty() )
            {
                RN_NODE_PTR& node = closest.front();

                if( node != target )
                {
                    changed = true;
                    update = true;
                    target = node;
                }
            }
        }
    }
    while( changed );

    assert( source->GetTag() >= 0 && target->GetTag() >= 0 );
    assert( source->GetTag() != target->GetTag() );
    assert( source != target );

    // Replace an invalid edge with new, valid one
    if( update )
        aEdge.reset( new RN_EDGE_MST( source, target ) );
}


/*void RN_NET::removeNode( RN_NODE_PTR& aNode, const BOARD_CONNECTED_ITEM* aParent )
{
    aNode->RemoveParent( aParent );

    if( m_links.RemoveNode( aNode ) )
    {
        clearNode( aNode );
        m_dirty = true;
    }
}


void RN_NET::removeEdge( RN_EDGE_MST_PTR& aEdge, const BOARD_CONNECTED_ITEM* aParent )
{
    // Save nodes, so they can be cleared later
    RN_NODE_PTR start = aEdge->GetSourceNode();
    RN_NODE_PTR end = aEdge->GetTargetNode();

    start->RemoveParent( aParent );
    end->RemoveParent( aParent );

    // Connection has to be removed before running RemoveNode(),
    // as RN_NODE influences the reference counter
    m_links.RemoveConnection( aEdge );

    // Remove nodes associated with the edge. It is done in a safe way, there is a check
    // if nodes are not used by other edges.
    if( m_links.RemoveNode( start ) )
        clearNode( start );

    if( m_links.RemoveNode( end ) )
        clearNode( end );

    m_dirty = true;
}*/


const RN_NODE_PTR& RN_LINKS::AddNode( int aX, int aY )
{
    RN_NODE_SET::iterator node;
    bool wasNewElement;

    std::tie( node, wasNewElement ) = m_nodes.emplace( std::make_shared<RN_NODE>( aX, aY ) );

    return *node;
}


/*bool RN_LINKS::RemoveNode( const RN_NODE_PTR& aNode )
{
    if( aNode->GetRefCount() == 0 )
    {
        m_nodes.erase( aNode );

        return true;
    }

    return false;
}*/


RN_EDGE_MST_PTR RN_LINKS::AddConnection( const RN_NODE_PTR& aNode1, const RN_NODE_PTR& aNode2,
                                         unsigned int aDistance )
{
    assert( aNode1 != aNode2 );
    RN_EDGE_MST_PTR edge = std::make_shared<RN_EDGE_MST>( aNode1, aNode2, aDistance );
    m_edges.push_back( edge );

    return edge;
}


void RN_NET::compute()
{
    const RN_LINKS::RN_NODE_SET& boardNodes = m_links.GetNodes();
    const RN_LINKS::RN_EDGE_LIST& boardEdges = m_links.GetConnections();

    // Special cases do not need complicated algorithms (actually, it does not work well with
    // the Delaunay triangulator)
    if( boardNodes.size() <= 2 )
    {
        m_rnEdges.reset( new std::vector<RN_EDGE_MST_PTR>( 0 ) );

        // Check if the only possible connection exists
        if( boardEdges.size() == 0 && boardNodes.size() == 2 )
        {
            RN_LINKS::RN_NODE_SET::const_iterator last = ++boardNodes.begin();

            // There can be only one possible connection, but it is missing
            RN_EDGE_MST_PTR edge = std::make_shared<RN_EDGE_MST>( *boardNodes.begin(), *last );
            edge->GetSourceNode()->SetTag( 0 );
            edge->GetTargetNode()->SetTag( 1 );
            m_rnEdges->push_back( edge );
        }
        else
        {
            // Set tags to nodes as connected
            for( RN_NODE_PTR node : boardNodes )
                node->SetTag( 0 );
        }


        return;
    }

    // Move and sort (sorting speeds up) all nodes to a vector for the Delaunay triangulation
    std::vector<RN_NODE_PTR> nodes( boardNodes.size() );
    std::partial_sort_copy( boardNodes.begin(), boardNodes.end(), nodes.begin(), nodes.end() );

    TRIANGULATOR triangulator;
    triangulator.CreateDelaunay( nodes.begin(), nodes.end() );
    std::unique_ptr<RN_LINKS::RN_EDGE_LIST> triangEdges( triangulator.GetEdges() );

    // Compute weight/distance for edges resulting from triangulation
    RN_LINKS::RN_EDGE_LIST::iterator eit, eitEnd;
    for( eit = (*triangEdges).begin(), eitEnd = (*triangEdges).end(); eit != eitEnd; ++eit )
        (*eit)->SetWeight( getDistance( (*eit)->GetSourceNode(), (*eit)->GetTargetNode() ) );

    // Add the currently existing connections list to the results of triangulation
    std::copy( boardEdges.begin(), boardEdges.end(), std::front_inserter( *triangEdges ) );

    // Get the minimal spanning tree
    m_rnEdges.reset( kruskalMST( *triangEdges, nodes ) );
}


void RN_NET::clearNode( const RN_NODE_PTR& aNode )
{
    if( !m_rnEdges )
        return;

    std::vector<RN_EDGE_MST_PTR>::iterator newEnd;

    // Remove all ratsnest edges for associated with the node
    newEnd = std::remove_if( m_rnEdges->begin(), m_rnEdges->end(),
                             std::bind( isEdgeConnectingNode, _1, std::cref( aNode ) ) );

    m_rnEdges->resize( std::distance( m_rnEdges->begin(), newEnd ) );
}

void RN_NET::Update()
{

    compute();

    for( RN_EDGE_MST_PTR& edge : *m_rnEdges )
        validateEdge( edge );

    m_dirty = false;
}

void RN_NET::Clear()
{
    m_links.Clear();
    if ( m_rnEdges )
        m_rnEdges->clear();
    m_blockedNodes.clear();
    m_simpleNodes.clear();
    m_dirty = true;
}


const RN_NODE_PTR RN_NET::GetClosestNode( const RN_NODE_PTR& aNode ) const
{
    const RN_LINKS::RN_NODE_SET& nodes = m_links.GetNodes();
    RN_LINKS::RN_NODE_SET::const_iterator it, itEnd;

    unsigned int minDistance = std::numeric_limits<unsigned int>::max();
    RN_NODE_PTR closest;

    for( it = nodes.begin(), itEnd = nodes.end(); it != itEnd; ++it )
    {
        RN_NODE_PTR node = *it;

        // Obviously the distance between node and itself is the shortest,
        // that's why we have to skip it
        if( node != aNode )
        {
            unsigned int distance = getDistance( node, aNode );
            if( distance < minDistance )
            {
                minDistance = distance;
                closest = node;
            }
        }
    }

    return closest;
}


const RN_NODE_PTR RN_NET::GetClosestNode( const RN_NODE_PTR& aNode,
                                          const RN_NODE_FILTER& aFilter ) const
{
    const RN_LINKS::RN_NODE_SET& nodes = m_links.GetNodes();
    RN_LINKS::RN_NODE_SET::const_iterator it, itEnd;

    unsigned int minDistance = std::numeric_limits<unsigned int>::max();
    RN_NODE_PTR closest;

    for( it = nodes.begin(), itEnd = nodes.end(); it != itEnd; ++it )
    {
        RN_NODE_PTR node = *it;

        // Obviously the distance between node and itself is the shortest,
        // that's why we have to skip it
        if( node != aNode && aFilter( node ) )
        {
            unsigned int distance = getDistance( node, aNode );

            if( distance < minDistance )
            {
                minDistance = distance;
                closest = node;
            }
        }
    }

    return closest;
}


std::list<RN_NODE_PTR> RN_NET::GetClosestNodes( const RN_NODE_PTR& aNode, int aNumber ) const
{
    std::list<RN_NODE_PTR> closest;
    const RN_LINKS::RN_NODE_SET& nodes = m_links.GetNodes();

    // Copy nodes
    std::copy( nodes.begin(), nodes.end(), std::back_inserter( closest ) );

    // Sort by the distance from aNode
    closest.sort( std::bind( sortDistance, std::cref( aNode ), _1, _2 ) );

    // aNode should not be returned in the results
    closest.remove( aNode );

    // Trim the result to the asked size
    if( aNumber > 0 )
        closest.resize( std::min( (size_t)aNumber, nodes.size() ) );
    return closest;
}


std::list<RN_NODE_PTR> RN_NET::GetClosestNodes( const RN_NODE_PTR& aNode,
                                                const RN_NODE_FILTER& aFilter, int aNumber ) const
{
    std::list<RN_NODE_PTR> closest;
    const RN_LINKS::RN_NODE_SET& nodes = m_links.GetNodes();

    // Copy filtered nodes
    std::copy_if( nodes.begin(), nodes.end(), std::back_inserter( closest ), std::cref( aFilter ) );

    // Sort by the distance from aNode
    closest.sort( std::bind( sortDistance, std::cref( aNode ), _1, _2 ) );

    // aNode should not be returned in the results
    closest.remove( aNode );

    // Trim the result to the asked size
    if( aNumber > 0 )
        closest.resize( std::min( static_cast<size_t>( aNumber ), nodes.size() ) );
    return closest;
}


std::list<RN_NODE_PTR> RN_NET::GetNodes( const BOARD_CONNECTED_ITEM* aItem ) const
{
    /*std::list<RN_NODE_PTR> nodes;

    switch( aItem->Type() )
    {
    case PCB_PAD_T:
    {
        PAD_NODE_MAP::const_iterator it = m_pads.find( static_cast<const D_PAD*>( aItem ) );

        if( it != m_pads.end() )
            nodes.push_back( it->second.m_Node );
    }
    break;

    case PCB_VIA_T:
    {
        VIA_NODE_MAP::const_iterator it = m_vias.find( static_cast<const VIA*>( aItem ) );

        if( it != m_vias.end() )
            nodes.push_back( it->second );
    }
    break;

    case PCB_TRACE_T:
    {
        TRACK_EDGE_MAP::const_iterator it = m_tracks.find( static_cast<const TRACK*>( aItem ) );

        if( it != m_tracks.end() )
        {
            nodes.push_back( it->second->GetSourceNode() );
            nodes.push_back( it->second->GetTargetNode() );
        }
    }
    break;

    case PCB_ZONE_AREA_T:
    {
        ZONE_DATA_MAP::const_iterator itz = m_zones.find( static_cast<const ZONE_CONTAINER*>( aItem ) );

        if( itz != m_zones.end() )
        {
            const std::deque<RN_POLY>& polys = itz->second.m_Polygons;

            for( std::deque<RN_POLY>::const_iterator it = polys.begin(); it != polys.end(); ++it )
                nodes.push_back( it->GetNode() );
        }
    }
    break;

    default:
        break;
    }

    return nodes;*/
}


void RN_NET::GetAllItems( std::list<BOARD_CONNECTED_ITEM*>& aOutput, const KICAD_T aTypes[] ) const
{
/*    if( aType & RN_PADS )
    {
        for( auto it : m_pads )
            aOutput.push_back( const_cast<D_PAD*>( it.first ) );
    }

    if( aType & RN_VIAS )
    {
        for( auto it : m_vias )
            aOutput.push_back( const_cast<VIA*>( it.first ) );
    }

    if( aType & RN_TRACKS )
    {
        for( auto it : m_tracks )
            aOutput.push_back( const_cast<TRACK*>( it.first ) );
    }

    if( aType & RN_ZONES )
    {
        for( auto it : m_zones )
            aOutput.push_back( const_cast<ZONE_CONTAINER*>( it.first ) );
    }*/
}

void RN_NET::GetConnectedItems( const BOARD_CONNECTED_ITEM* aItem,
                                std::list<BOARD_CONNECTED_ITEM*>& aOutput,
                                const KICAD_T aTypes[] ) const
{
/*    std::list<RN_NODE_PTR> nodes = GetNodes( aItem );
    assert( !nodes.empty() );

    int tag = nodes.front()->GetTag();
    assert( tag >= 0 );

    if( aTypes & RN_PADS )
    {
        for( PAD_NODE_MAP::const_iterator it = m_pads.begin(); it != m_pads.end(); ++it )
        {
            if( it->second.m_Node->GetTag() == tag )
                aOutput.push_back( const_cast<D_PAD*>( it->first ) );
        }
    }

    if( aTypes & RN_VIAS )
    {
        for( VIA_NODE_MAP::const_iterator it = m_vias.begin(); it != m_vias.end(); ++it )
        {
            if( it->second->GetTag() == tag )
                aOutput.push_back( const_cast<VIA*>( it->first ) );
        }
    }

    if( aTypes & RN_TRACKS )
    {
        for( TRACK_EDGE_MAP::const_iterator it = m_tracks.begin(); it != m_tracks.end(); ++it )
        {
            if( it->second->GetTag() == tag )
                aOutput.push_back( const_cast<TRACK*>( it->first ) );
        }
    }

    if( aTypes & RN_ZONES )
    {
        for( ZONE_DATA_MAP::const_iterator it = m_zones.begin(); it != m_zones.end(); ++it )
        {
            for( const RN_EDGE_MST_PTR& edge : it->second.m_Edges )
            {
                if( edge->GetTag() == tag )
                {
                    aOutput.push_back( const_cast<ZONE_CONTAINER*>( it->first ) );
                    break;
                }
            }
        }
    }*/
}

const RN_NODE_PTR& RN_NET::AddNode( int aX, int aY )
{
    return m_links.AddNode( aX, aY );
}

void RN_NET::AddConnection( const RN_NODE_PTR& aNode1, const RN_NODE_PTR& aNode2,
                    unsigned int aDistance  )
{
    m_links.AddConnection( aNode1, aNode2, aDistance );
}

bool RN_NET::NearestBicoloredPair( const RN_NET& aOtherNet, RN_NODE_PTR& aNode1, RN_NODE_PTR& aNode2 ) const
{
    bool rv = false;
    VECTOR2I::extended_type distMax = VECTOR2I::ECOORD_MAX;

    for ( auto nodeA : m_links.GetNodes() )
    {
        for ( auto nodeB : aOtherNet.m_links.GetNodes() )
        {
            if ( !nodeA->GetNoLine() )
            {
                const VECTOR2I posA ( nodeA->GetX(), nodeA->GetY() );
                const VECTOR2I posB ( nodeB->GetX(), nodeB->GetY() );

                auto squaredDist = (posA - posB).SquaredEuclideanNorm();

                if ( squaredDist < distMax )
                {
                    rv = true;
                    distMax = squaredDist;
                    aNode1 = nodeA;
                    aNode2 = nodeB;
                }
            }
        }
    }

    return rv;
}

unsigned int RN_NET::GetNodeCount() const
{
    return m_links.GetNodes().size();
}
