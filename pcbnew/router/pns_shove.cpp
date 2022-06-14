/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2013-2014 CERN
 * Copyright (C) 2016-2021 KiCad Developers, see AUTHORS.txt for contributors.
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <deque>
#include <cassert>
#include <math/box2.h>

#include <geometry/shape_compound.h>

#include <wx/log.h>

#include "pns_arc.h"
#include "pns_line.h"
#include "pns_node.h"
#include "pns_debug_decorator.h"
#include "pns_walkaround.h"
#include "pns_shove.h"
#include "pns_solid.h"
#include "pns_optimizer.h"
#include "pns_via.h"
#include "pns_utils.h"
#include "pns_router.h"
#include "pns_topology.h"
#include "pns_hull.h"

#include "time_limit.h"

// fixme - move all logger calls to debug decorator

typedef VECTOR2I::extended_type ecoord;

namespace PNS {

void SHOVE::replaceItems( ITEM* aOld, std::unique_ptr< ITEM > aNew )
{
    OPT_BOX2I changed_area = ChangedArea( aOld, aNew.get() );

    if( changed_area )
        m_affectedArea = m_affectedArea ? m_affectedArea->Merge( *changed_area ) : *changed_area;

    m_currentNode->Replace( aOld, std::move( aNew ) );
}


void SHOVE::replaceLine( LINE& aOld, LINE& aNew, bool aIncludeInChangedArea, NODE* aNode )
{
    if( aIncludeInChangedArea )
    {
        OPT_BOX2I changed_area = ChangedArea( aOld, aNew );

        if( changed_area )
        {
            SHAPE_RECT r( *changed_area );
            PNS_DBG(Dbg(), AddShape, &r, BLUE, 0, wxT( "shove-changed-area" ) );

            m_affectedArea =
                    m_affectedArea ? m_affectedArea->Merge( *changed_area ) : *changed_area;
        }
    }

    bool  foundPredecessor = false;
    LINE* rootLine = nullptr;

    // Keep track of the 'root lines', i.e. the unmodified (pre-shove) versions
    // of the affected tracks in a map. The optimizer can then query the pre-shove shape
    // for each shoved line and perform additional constraint checks (i.e. prevent overzealous
    // optimizations)

    // Check if the shoved line already has an ancestor (e.g. line from a previous shove
    // iteration/cursor movement)
    for( auto link : aOld.Links() )
    {
        auto oldLineIter = m_rootLineHistory.find( link );

        if( oldLineIter != m_rootLineHistory.end() )
        {
            rootLine = oldLineIter->second;
            foundPredecessor = true;
            break;
        }
    }

    // If found, use it, otherwise, create new entry in the map (we have a genuine new 'root' line)
    if( !foundPredecessor )
    {
        for( auto link : aOld.Links() )
        {
            if( ! rootLine )
            {
                rootLine = aOld.Clone();
            }

            m_rootLineHistory[link] = rootLine;
        }
    }

    // Now update the NODE (calling Replace invalidates the Links() in a LINE)
    if( aNode )
    {
        aNode->Replace( aOld, aNew );
    }
    else
    {
        m_currentNode->Replace( aOld, aNew );
    }

    // point the Links() of the new line to its oldest ancestor
    for( auto link : aNew.Links() )
    {
        m_rootLineHistory[ link ] = rootLine;
    }
}




void SHOVE::sanityCheck( LINE* aOld, LINE* aNew )
{
    assert( aOld->CPoint( 0 ) == aNew->CPoint( 0 ) );
    assert( aOld->CPoint( -1 ) == aNew->CPoint( -1 ) );
}


SHOVE::SHOVE( NODE* aWorld, ROUTER* aRouter ) :
    ALGO_BASE( aRouter )
{
    m_optFlagDisableMask = 0;
    m_root = aWorld;
    m_currentNode = aWorld;
    SetDebugDecorator( aRouter->GetInterface()->GetDebugDecorator() );

    // Initialize other temporary variables:
    m_draggedVia = nullptr;
    m_iter = 0;
    m_multiLineMode = false;
    m_restrictSpringbackTagId = 0;
    m_springbackDoNotTouchNode = nullptr;
}


SHOVE::~SHOVE()
{
    std::unordered_set<LINE*> alreadyDeleted;

    for( auto it : m_rootLineHistory )
    {
        auto it2 = alreadyDeleted.find( it.second );

        if( it2 == alreadyDeleted.end() )
        {
            alreadyDeleted.insert( it.second );
            delete it.second;
        }
    }
}


LINE SHOVE::assembleLine( const LINKED_ITEM* aSeg, int* aIndex )
{
    return m_currentNode->AssembleLine( const_cast<LINKED_ITEM*>( aSeg ), aIndex, true );
}


// A dumb function that checks if the shoved line is shoved the right way, e.g. visually
// "outwards" of the line/via applying pressure on it. Unfortunately there's no mathematical
// concept of orientation of an open curve, so we use some primitive heuristics: if the shoved
// line wraps around the start of the "pusher", it's likely shoved in wrong direction.

// Update: there's no concept of an orientation of an open curve, but nonetheless Tom's dumb
// as.... (censored)
// Two open curves put together make a closed polygon... Tom should learn high school geometry!
bool SHOVE::checkShoveDirection( const LINE& aCurLine, const LINE& aObstacleLine,
                                 const LINE& aShovedLine ) const
{
    SHAPE_LINE_CHAIN::POINT_INSIDE_TRACKER checker( aCurLine.CPoint( 0) );
    checker.AddPolyline( aObstacleLine.CLine() );
    checker.AddPolyline( aShovedLine.CLine().Reverse() );

    bool inside = checker.IsInside();

    return !inside;
}


/*
 * Push aObstacleLine away from aCurLine's via by the clearance distance, returning the result
 * in aResultLine.
 *
 * Must be called only when aCurLine itself is on another layer (or has no segments) so that it
 * can be ignored.
 */
SHOVE::SHOVE_STATUS SHOVE::shoveLineFromLoneVia( const LINE& aCurLine, const LINE& aObstacleLine,
                                                 LINE& aResultLine, OBSTACLE& aObstacleInfo )
{
    // Build a hull for aCurLine's via and re-walk aObstacleLine around it.
    int obstacleLineWidth = aObstacleLine.Width();
    int clearance = aObstacleInfo.m_clearance;

    SHAPE_LINE_CHAIN hull = aCurLine.Via().Hull( clearance, obstacleLineWidth );
    SHAPE_LINE_CHAIN path_cw;
    SHAPE_LINE_CHAIN path_ccw;

    if( ! aObstacleLine.Walkaround( hull, path_cw, true ) )
        return SH_INCOMPLETE;

    if( ! aObstacleLine.Walkaround( hull, path_ccw, false ) )
        return SH_INCOMPLETE;

    const SHAPE_LINE_CHAIN& shortest = path_ccw.Length() < path_cw.Length() ? path_ccw : path_cw;

    if( shortest.PointCount() < 2 )
        return SH_INCOMPLETE;

    if( aObstacleLine.CPoint( -1 ) != shortest.CPoint( -1 ) )
        return SH_INCOMPLETE;

    if( aObstacleLine.CPoint( 0 ) != shortest.CPoint( 0 ) )
        return SH_INCOMPLETE;

    aResultLine.SetShape( shortest );

    if( aResultLine.Collide( &aCurLine, m_currentNode ) )
        return SH_INCOMPLETE;

    return SH_OK;
}


/*
 * Re-walk aObstacleLine around the given set of hulls, returning the result in aResultLine.
 */
SHOVE::SHOVE_STATUS SHOVE::shoveLineToHullSet( const LINE& aCurLine, const LINE& aObstacleLine,
                                               LINE& aResultLine, const HULL_SET& aHulls )
{
    const SHAPE_LINE_CHAIN& obs = aObstacleLine.CLine();

    int attempt;

    PNS_DBG( Dbg(), BeginGroup, "shove-details", 1 );

    for( attempt = 0; attempt < 4; attempt++ )
    {
        bool invertTraversal = ( attempt >= 2 );
        bool clockwise = attempt % 2;
        int vFirst = -1, vLast = -1;

        LINE l( aObstacleLine );
        SHAPE_LINE_CHAIN path( l.CLine() );

        for( int i = 0; i < (int) aHulls.size(); i++ )
        {
            const SHAPE_LINE_CHAIN& hull = aHulls[invertTraversal ? aHulls.size() - 1 - i : i];

            PNS_DBG( Dbg(), AddShape, &hull, YELLOW, 10000, wxString::Format( "hull[%d]", i ) );
            PNS_DBG( Dbg(), AddShape, &path, WHITE, l.Width(), wxString::Format( "path[%d]", i ) );
            PNS_DBG( Dbg(), AddShape, &obs, LIGHTGRAY, aObstacleLine.Width(),  wxString::Format( "obs[%d]", i ) );

            if( !l.Walkaround( hull, path, clockwise ) )
            {
                PNS_DBG( Dbg(), Message, wxString::Format( wxT( "Fail-Walk %s %s %d\n" ),
                                                           hull.Format().c_str(),
                                                           l.CLine().Format().c_str(),
                                                           clockwise? 1 : 0) );

                PNS_DBGN( Dbg(), EndGroup );
                return SH_INCOMPLETE;
            }

            PNS_DBG( Dbg(), AddShape, &path, WHITE, l.Width(), wxString::Format( "path-presimp[%d]", i ) );

            path.Simplify();

            PNS_DBG( Dbg(), AddShape, &path, WHITE, l.Width(), wxString::Format( "path-postsimp[%d]", i ) );

            l.SetShape( path );
        }

        for( int i = 0; i < std::min( path.PointCount(), obs.PointCount() ); i++ )
        {
            if( path.CPoint( i ) != obs.CPoint( i ) )
            {
                vFirst = i;
                break;
            }
        }

        int k = obs.PointCount() - 1;

        for( int i = path.PointCount() - 1; i >= 0 && k >= 0; i--, k-- )
        {
            if( path.CPoint( i ) != obs.CPoint( k ) )
            {
                vLast = i;
                break;
            }
        }

        if( ( vFirst < 0 || vLast < 0 ) && !path.CompareGeometry( aObstacleLine.CLine() ) )
        {
            PNS_DBG( Dbg(), Message, wxString::Format( wxT( "attempt %d fail vfirst-last" ),
                                                       attempt ) );
            continue;
        }

        if( path.CPoint( -1 ) != obs.CPoint( -1 ) || path.CPoint( 0 ) != obs.CPoint( 0 ) )
        {
            PNS_DBG( Dbg(), Message, wxString::Format( wxT( "attempt %d fail vend-start\n" ),
                                                       attempt ) );
            continue;
        }

        if( !checkShoveDirection( aCurLine, aObstacleLine, l ) )
        {
            PNS_DBG( Dbg(), Message, wxString::Format( wxT( "attempt %d fail direction-check" ),
                                                       attempt ) );
            aResultLine.SetShape( l.CLine() );
            continue;
        }

        if( path.SelfIntersecting() )
        {
            PNS_DBG( Dbg(), Message, wxString::Format( wxT( "attempt %d fail self-intersect" ),
                                                       attempt ) );
            continue;
        }

        bool colliding = l.Collide( &aCurLine, m_currentNode );

        if(( aCurLine.Marker() & MK_HEAD ) && !colliding )
        {
            JOINT* jtStart = m_currentNode->FindJoint( aCurLine.CPoint( 0 ), &aCurLine );

            for( ITEM* item : jtStart->LinkList() )
            {
                if( item->Collide( &l, m_currentNode ) )
                    colliding = true;
            }
        }

        if( colliding )
        {
            PNS_DBG( Dbg(), Message, wxString::Format( wxT( "attempt %d fail coll-check" ),
                                                       attempt ) );
            continue;
        }

        aResultLine.SetShape( l.CLine() );

        PNS_DBGN( Dbg(), EndGroup );

        return SH_OK;
    }

    PNS_DBGN( Dbg(), EndGroup );

    return SH_INCOMPLETE;
}


/*
 * Push aObstacleLine line away from aCurLine by the clearance distance, and return the result in
 * aResultLine.
 */
SHOVE::SHOVE_STATUS SHOVE::ShoveObstacleLine( const LINE& aCurLine, const LINE& aObstacleLine,
                                              LINE& aResultLine, OBSTACLE& aObstacleInfo )
{
    aResultLine.ClearLinks();

    bool obstacleIsHead = false;

    for( LINKED_ITEM* s : aObstacleLine.Links() )
    {
        if( s->Marker() & MK_HEAD )
        {
            obstacleIsHead = true;
            break;
        }
    }

    SHOVE_STATUS rv;
    bool viaOnEnd = aCurLine.EndsWithVia();

    if( viaOnEnd && ( !aCurLine.Via().LayersOverlap( &aObstacleLine ) || aCurLine.SegmentCount() == 0 ) )
    {
        // Shove aObstacleLine to the hull of aCurLine's via.

        rv = shoveLineFromLoneVia( aCurLine, aObstacleLine, aResultLine, aObstacleInfo );
    }
    else
    {
        // Build a set of hulls around the segments of aCurLine.  Hulls are at the clearance
        // distance + aObstacleLine's linewidth so that when re-walking aObstacleLine along the
        // hull it will be at the appropriate clearance.

        int      obstacleLineWidth = aObstacleLine.Width();
        int      clearance = aObstacleInfo.m_clearance;
        int      currentLineSegmentCount = aCurLine.SegmentCount();

        HULL_SET hulls;
        hulls.reserve( currentLineSegmentCount + 1 );

        PNS_DBG( Dbg(), Message, wxString::Format( wxT( "shove process-single: cur net %d obs %d cl %d" ),
                                          aCurLine.Net(), aObstacleLine.Net(), clearance ) );

        for( int i = 0; i < currentLineSegmentCount; i++ )
        {
            SEGMENT seg( aCurLine, aCurLine.CSegment( i ) );
            int     extra = 0;

            // Arcs need additional clearance to ensure the hulls are always bigger than the arc
            if( aCurLine.CLine().IsArcSegment( i ) )
                extra = SHAPE_ARC::DefaultAccuracyForPCB();

            if( extra > 0 )
	    {
                PNS_DBG( Dbg(), Message, wxString::Format( wxT( "shove add-extra-clearance %d" ),
                                          extra ) );
            }

            SHAPE_LINE_CHAIN hull =
                    seg.Hull( clearance + extra, obstacleLineWidth );

            hulls.push_back( hull );
        }

        if( viaOnEnd )
        {
            const VIA& via = aCurLine.Via();
            int viaClearance = aObstacleInfo.m_clearance; // getClearance( &via, &aObstacleLine );
            /*int holeClearance = getHoleClearance( &via, &aObstacleLine );

            if( holeClearance + via.Drill() / 2 > viaClearance + via.Diameter() / 2 )
                viaClearance = holeClearance + via.Drill() / 2 - via.Diameter() / 2;*/

            hulls.push_back( aCurLine.Via().Hull( viaClearance, obstacleLineWidth ) );
        }

        rv = shoveLineToHullSet( aCurLine, aObstacleLine, aResultLine, hulls );
    }

    if( obstacleIsHead )
        aResultLine.Mark( aResultLine.Marker() | MK_HEAD );

    return rv;
}


/*
 * TODO describe....
 */
SHOVE::SHOVE_STATUS SHOVE::onCollidingSegment( LINE& aCurrent, SEGMENT* aObstacleSeg, OBSTACLE& aObstacleInfo )
{
    int segIndex;
    LINE obstacleLine = assembleLine( aObstacleSeg, &segIndex );
    LINE shovedLine( obstacleLine );
    SEGMENT tmp( *aObstacleSeg );

    if( obstacleLine.HasLockedSegments() )
    {
        PNS_DBG(Dbg(), Message, "try walk (locked segments)");
        return SH_TRY_WALK;
    }

    SHOVE_STATUS rv = ShoveObstacleLine( aCurrent, obstacleLine, shovedLine, aObstacleInfo );

    const double extensionWalkThreshold = 1.0;

    double obsLen = obstacleLine.CLine().Length();
    double shovedLen = shovedLine.CLine().Length();
    double extensionFactor = 0.0;

    if( obsLen != 0.0f )
        extensionFactor = shovedLen / obsLen - 1.0;

    if( extensionFactor > extensionWalkThreshold )
        return SH_TRY_WALK;

    assert( obstacleLine.LayersOverlap( &shovedLine ) );

    if( Dbg() )
    {
        PNS_DBG( Dbg(), AddItem, aObstacleSeg, BLUE, 0, wxT( "shove-changed-area" ) );
        PNS_DBG( Dbg(), AddItem, &aCurrent, RED, 10000, wxT( "current-line" ) );
        PNS_DBG( Dbg(), AddItem, &obstacleLine, GREEN, 10000, wxT( "obstacle-line" ) );
        PNS_DBG( Dbg(), AddItem, &shovedLine, BLUE, 10000, wxT( "shoved-line" ) );
    }

    if( rv == SH_OK )
    {
        if( shovedLine.Marker() & MK_HEAD )
        {
            if( m_multiLineMode )
                return SH_INCOMPLETE;

            m_newHead = shovedLine;
        }

        int rank = aCurrent.Rank();
        shovedLine.SetRank( rank - 1 );

        sanityCheck( &obstacleLine, &shovedLine );
        replaceLine( obstacleLine, shovedLine );

        if( !pushLineStack( shovedLine ) )
            rv = SH_INCOMPLETE;
    }

    return rv;
}


/*
 * TODO describe....
 */
SHOVE::SHOVE_STATUS SHOVE::onCollidingArc( LINE& aCurrent, ARC* aObstacleArc, OBSTACLE& aObstacleInfo )
{
    int segIndex;
    LINE obstacleLine = assembleLine( aObstacleArc, &segIndex );
    LINE shovedLine( obstacleLine );
    ARC tmp( *aObstacleArc );

    if( obstacleLine.HasLockedSegments() )
        return SH_TRY_WALK;

    SHOVE_STATUS rv = ShoveObstacleLine( aCurrent, obstacleLine, shovedLine, aObstacleInfo );

    const double extensionWalkThreshold = 1.0;

    double obsLen = obstacleLine.CLine().Length();
    double shovedLen = shovedLine.CLine().Length();
    double extensionFactor = 0.0;

    if( obsLen != 0.0f )
        extensionFactor = shovedLen / obsLen - 1.0;

    if( extensionFactor > extensionWalkThreshold )
        return SH_TRY_WALK;

    assert( obstacleLine.LayersOverlap( &shovedLine ) );

    PNS_DBG( Dbg(), AddItem, &tmp, WHITE, 10000, wxT( "obstacle-arc" ) );
    PNS_DBG( Dbg(), AddItem, &aCurrent, RED, 10000, wxT( "current-line" ) );
    PNS_DBG( Dbg(), AddItem, &obstacleLine, GREEN, 10000, wxT( "obstacle-line" ) );
    PNS_DBG( Dbg(), AddItem, &shovedLine, BLUE, 10000, wxT( "shoved-line" ) );

    if( rv == SH_OK )
    {
        if( shovedLine.Marker() & MK_HEAD )
        {
            if( m_multiLineMode )
                return SH_INCOMPLETE;

            m_newHead = shovedLine;
        }

        int rank = aCurrent.Rank();
        shovedLine.SetRank( rank - 1 );

        sanityCheck( &obstacleLine, &shovedLine );
        replaceLine( obstacleLine, shovedLine );

        if( !pushLineStack( shovedLine ) )
            rv = SH_INCOMPLETE;
    }

    return rv;
}


/*
 * TODO describe....
 */
SHOVE::SHOVE_STATUS SHOVE::onCollidingLine( LINE& aCurrent, LINE& aObstacle, OBSTACLE& aObstacleInfo )
{
    LINE shovedLine( aObstacle );

    SHOVE_STATUS rv = ShoveObstacleLine( aCurrent, aObstacle, shovedLine, aObstacleInfo );

    PNS_DBG( Dbg(), AddItem, &aObstacle, RED, 100000, wxT( "obstacle-line" ) );
    PNS_DBG( Dbg(), AddItem, &aCurrent, GREEN, 150000, wxT( "current-line" ) );
    PNS_DBG( Dbg(), AddItem, &shovedLine, BLUE, 200000, wxT( "shoved-line" ) );

    if( rv == SH_OK )
    {
        if( shovedLine.Marker() & MK_HEAD )
        {
            if( m_multiLineMode )
                return SH_INCOMPLETE;

            m_newHead = shovedLine;
        }

        sanityCheck( &aObstacle, &shovedLine );
        replaceLine( aObstacle, shovedLine );

        int rank = aObstacle.Rank();
        shovedLine.SetRank( rank - 1 );


        if( !pushLineStack( shovedLine ) )
        {
            rv = SH_INCOMPLETE;
        }
    }

    return rv;
}


/*
 * TODO describe....
 */
SHOVE::SHOVE_STATUS SHOVE::onCollidingSolid( LINE& aCurrent, ITEM* aObstacle, OBSTACLE& aObstacleInfo )
{
    WALKAROUND walkaround( m_currentNode, Router() );
    LINE walkaroundLine( aCurrent );

    if( aCurrent.EndsWithVia() )
    {
        VIA vh = aCurrent.Via();
        VIA* via = nullptr;
        JOINT* jtStart = m_currentNode->FindJoint( vh.Pos(), &aCurrent );

        if( !jtStart )
            return SH_INCOMPLETE;

        for( ITEM* item : jtStart->LinkList() )
        {
            if( item->OfKind( ITEM::VIA_T ) )
            {
                via = (VIA*) item;
                break;
            }
        }

        if( via && via->Collide( aObstacle, m_currentNode ) )
            return onCollidingVia( aObstacle, via, aObstacleInfo );
    }

    TOPOLOGY topo( m_currentNode );

    std::set<ITEM*> cluster = topo.AssembleCluster( aObstacle, aCurrent.Layers().Start() );



    PNS_DBG( Dbg(), BeginGroup, "walk-cluster", 1 );

    for( auto item : cluster )
    {
        PNS_DBG( Dbg(), AddItem, item, RED, 10000, wxT( "cl-item" ) );
    }

    PNS_DBGN( Dbg(), EndGroup );


    walkaround.SetDebugDecorator( Dbg() );
    walkaround.SetSolidsOnly( false );
    walkaround.RestrictToSet( true, cluster );
    walkaround.SetIterationLimit( 16 ); // fixme: make configurable

    int currentRank = aCurrent.Rank();
    int nextRank;

    bool success = false;

    for( int attempt = 0; attempt < 2; attempt++ )
    {
        if( attempt == 1 || Settings().JumpOverObstacles() )
        {

            nextRank = currentRank - 1;
        }
        else
        {
            nextRank = currentRank + 10000;
        }


    	WALKAROUND::WALKAROUND_STATUS status = walkaround.Route( aCurrent, walkaroundLine, false );

        if( status != WALKAROUND::DONE )
            continue;

        walkaroundLine.ClearLinks();
        walkaroundLine.Unmark();
    	walkaroundLine.Line().Simplify();

    	if( walkaroundLine.HasLoops() )
            continue;

    	if( aCurrent.Marker() & MK_HEAD )
    	{
            walkaroundLine.Mark( MK_HEAD );

            if( m_multiLineMode )
                continue;

            m_newHead = walkaroundLine;
        }

    	sanityCheck( &aCurrent, &walkaroundLine );

        if( !m_lineStack.empty() )
        {
            LINE lastLine = m_lineStack.front();
            OBSTACLE newobs;

            if( lastLine.Collide( &walkaroundLine, m_currentNode, COLLISION_SEARCH_OPTIONS(), &newobs ) )
            {
                LINE dummy( lastLine );

                if( ShoveObstacleLine( walkaroundLine, lastLine, dummy, newobs ) == SH_OK )
                {
                    success = true;
                    break;
                }
            }
            else
            {
                success = true;
                break;
            }
        }
    }

    if(!success)
        return SH_INCOMPLETE;

    replaceLine( aCurrent, walkaroundLine );
    walkaroundLine.SetRank( nextRank );

    PNS_DBG( Dbg(), AddItem, &aCurrent, RED, 10000, wxT( "current-line" ) );
    PNS_DBG( Dbg(), AddItem, &walkaroundLine, BLUE, 10000, wxT( "walk-line" ) );

    popLineStack();

    if( !pushLineStack( walkaroundLine ) )
        return SH_INCOMPLETE;

    return SH_OK;
};


/*
 * Pops NODE stackframes which no longer collide with aHeadSet.  Optionally sets aDraggedVia
 * to the dragged via of the last unpopped state.
 */
NODE* SHOVE::reduceSpringback( const ITEM_SET& aHeadSet, VIA_HANDLE& aDraggedVia )
{
    PNS_DBG( Dbg(), BeginGroup, "reduce-springback", 1 );

int depth = 0;

    while( !m_nodeStack.empty() )
    {
        SPRINGBACK_TAG& spTag = m_nodeStack.back();

        // Prevent the springback algo from erasing NODEs that might contain items used by the ROUTER_TOOL/LINE_PLACER.
        // I noticed this can happen for the endItem provided to LINE_PLACER::Move() and cause a nasty crash.
        if( spTag.m_node == m_springbackDoNotTouchNode )
            break;

        OPT<OBSTACLE> obs = spTag.m_node->CheckColliding( aHeadSet );

        if( obs )
        {
            PNS_DBG( Dbg(), AddItem, aHeadSet[0], RED, 10000, wxString::Format("head depth %d", depth ));
            PNS_DBG( Dbg(), AddItem, obs->m_item, GREEN, 10000, wxString::Format("obstacle [%s]", obs->m_item->KindStr().c_str() ) );
        }



        depth--;
        if( !obs && !spTag.m_locked )
        {
            aDraggedVia = spTag.m_draggedVia;
            aDraggedVia.valid = true;

            delete spTag.m_node;
            m_nodeStack.pop_back();
        }
        else
        {
           break;
        }
    }


    PNS_DBGN( Dbg(), EndGroup );
    return m_nodeStack.empty() ? m_root : m_nodeStack.back().m_node;
}


/*
 * Push the current NODE on to the stack.  aDraggedVia is the dragged via *before* the push
 * (which will be restored in the event the stackframe is popped).
 */
bool SHOVE::pushSpringback( NODE* aNode, const OPT_BOX2I& aAffectedArea, VIA* aDraggedVia )
{
    SPRINGBACK_TAG st;
    OPT_BOX2I prev_area;

    if( !m_nodeStack.empty() )
        prev_area = m_nodeStack.back().m_affectedArea;

    if( aDraggedVia )
    {
        st.m_draggedVia = aDraggedVia->MakeHandle();
    }

    st.m_node = aNode;

    if( aAffectedArea )
    {
        if( prev_area )
            st.m_affectedArea = prev_area->Merge( *aAffectedArea );
        else
            st.m_affectedArea = aAffectedArea;
    }
    else
    {
        st.m_affectedArea = prev_area;
    }

    st.m_seq = ( m_nodeStack.empty() ? 1 : m_nodeStack.back().m_seq + 1 );
    st.m_locked = false;

    m_nodeStack.push_back( st );

    return true;
}


/*
 * Push or shove a via by at least aForce.  (The via might be pushed or shoved slightly further
 * to keep it from landing on an existing joint.)
 */
SHOVE::SHOVE_STATUS SHOVE::pushOrShoveVia( VIA* aVia, const VECTOR2I& aForce, int aCurrentRank )
{
    LINE_PAIR_VEC draggedLines;
    VECTOR2I p0( aVia->Pos() );
    JOINT* jt = m_currentNode->FindJoint( p0, aVia );
    VECTOR2I p0_pushed( p0 + aForce );

    PNS_DBG( Dbg(), Message, wxString::Format( wxT( "via force [%d %d]\n" ), aForce.x, aForce.y ) );

    // nothing to do...
    if ( aForce.x == 0 && aForce.y == 0 )
        return SH_OK;

    if( !jt )
    {
        PNS_DBG( Dbg(), Message,
                 wxString::Format( wxT( "weird, can't find the center-of-via joint\n" ) ) );
        return SH_INCOMPLETE;
    }

    if( aVia->IsLocked() )
        return SH_TRY_WALK;

    if( jt->IsLocked() )
        return SH_INCOMPLETE;

    // make sure pushed via does not overlap with any existing joint
    while( true )
    {
        JOINT* jt_next = m_currentNode->FindJoint( p0_pushed, aVia );

        if( !jt_next )
            break;

        p0_pushed += aForce.Resize( 2 );
    }

    std::unique_ptr<VIA> pushedVia = Clone( *aVia );
    pushedVia->SetPos( p0_pushed );
    pushedVia->Mark( aVia->Marker() );

    for( ITEM* item : jt->LinkList() )
    {
        if( item->OfKind( ITEM::SEGMENT_T | ITEM::ARC_T ) )
        {
            LINKED_ITEM* li = static_cast<LINKED_ITEM*>( item );
            LINE_PAIR lp;
            int segIndex;

            lp.first = assembleLine( li, &segIndex );

            if( lp.first.HasLockedSegments() )
                return SH_TRY_WALK;

            assert( segIndex == 0 || ( segIndex == ( lp.first.SegmentCount() - 1 ) ) );

            if( segIndex == 0 )
                lp.first.Reverse();

            lp.second = lp.first;
            lp.second.ClearLinks();
            lp.second.DragCorner( p0_pushed, lp.second.CLine().Find( p0 ) );
            lp.second.AppendVia( *pushedVia );
            draggedLines.push_back( lp );
        }
    }

    pushedVia->SetRank( aCurrentRank - 1 );
    PNS_DBG( Dbg(), Message, wxString::Format("PushViaRank %d\n", pushedVia->Rank() ) );

    if( aVia->Marker() & MK_HEAD )      // push
    {
        m_draggedVia = pushedVia.get();
    }
    else
    {                                   // shove
        if( jt->IsStitchingVia() )
            pushLineStack( LINE( *pushedVia ) );
    }

    PNS_DBG( Dbg(), AddPoint, aVia->Pos(), LIGHTGREEN, 100000, "via-pre" );
    PNS_DBG( Dbg(), AddPoint, pushedVia->Pos(), LIGHTRED, 100000, "via-post" );

    replaceItems( aVia, std::move( pushedVia ) );

    for( LINE_PAIR lp : draggedLines )
    {
        if( lp.first.Marker() & MK_HEAD )
        {
            lp.second.Mark( MK_HEAD );

            if( m_multiLineMode )
                return SH_INCOMPLETE;

            m_newHead = lp.second;
        }

        unwindLineStack( &lp.first );

        if( lp.second.SegmentCount() )
        {
            replaceLine( lp.first, lp.second );
            lp.second.SetRank( aCurrentRank - 1 );


            PNS_DBG( Dbg(), Message, wxString::Format("PushViaF %p %d\n", &lp.second, lp.second.SegmentCount() ) );

            if( !pushLineStack( lp.second, true ) )
                return SH_INCOMPLETE;
        }
        else
        {
            m_currentNode->Remove( lp.first );
        }

        PNS_DBG( Dbg(), AddItem, &lp.first, LIGHTGREEN, 10000, wxT( "fan-pre" ) );
        PNS_DBG( Dbg(), AddItem, &lp.second, LIGHTRED, 10000, wxT( "fan-post" ) );
    }

    return SH_OK;
}


/*
 * Calculate the minimum translation vector required to resolve a collision with a via and
 * shove the via by that distance.
 */
SHOVE::SHOVE_STATUS SHOVE::onCollidingVia( ITEM* aCurrent, VIA* aObstacleVia, OBSTACLE& aObstacleInfo )
{
    assert( aObstacleVia );

    int clearance = aObstacleInfo.m_clearance; // getClearance( aCurrent, aObstacleVia );
    VECTOR2I mtv;
    int rank = -1;

    bool lineCollision = false;
    bool viaCollision = false;
    VECTOR2I mtvLine, mtvVia;

    PNS_DBG( Dbg(), BeginGroup, "push-via-by-line", 1 );

    if( aCurrent->OfKind( ITEM::LINE_T ) )
    {
        VIA vtmp ( *aObstacleVia );

        if( aObstacleInfo.m_maxFanoutWidth > 0 && aObstacleInfo.m_maxFanoutWidth > aObstacleVia->Diameter() )
        {
            vtmp.SetDiameter( aObstacleInfo.m_maxFanoutWidth );
        }

        LINE* currentLine = (LINE*) aCurrent;

        PNS_DBG( Dbg(), AddItem, currentLine, LIGHTRED, 10000, wxT( "current-line" ) );
        
        if( currentLine->EndsWithVia() )
        {
            PNS_DBG( Dbg(), AddItem, &currentLine->Via(), LIGHTRED, 10000, wxT( "current-line-via" ) );
        }
        
        PNS_DBG( Dbg(), AddItem, &vtmp, LIGHTRED, 100000, wxT( "orig-via" ) );

        lineCollision = vtmp.Shape()->Collide( currentLine->Shape(),
                                                        clearance + currentLine->Width() / 2,
                                                        &mtvLine );

        // Check the via if present. Via takes priority.
        if( currentLine->EndsWithVia() )
        {
            const VIA& currentVia = currentLine->Via();
            int        viaClearance = aObstacleInfo.m_clearance; // getClearance( &currentVia, &vtmp );

            viaCollision = currentVia.Shape()->Collide( vtmp.Shape(), viaClearance, &mtvVia );
        }
    }
    else if( aCurrent->OfKind( ITEM::SOLID_T ) )
    {
       // TODO: is this possible at all? We don't shove solids.
       return SH_INCOMPLETE;
    }

    // fixme: we may have a sign issue in Collide(CIRCLE, LINE_CHAIN)
    if( viaCollision )
        mtv = -mtvVia;
    else if ( lineCollision )
        mtv = -mtvLine;
    else
        mtv = VECTOR2I(0, 0);

    SHOVE::SHOVE_STATUS st = pushOrShoveVia( aObstacleVia, -mtv, aCurrent->Rank() );
    PNS_DBG(Dbg(), Message, wxString::Format("push-or-shove-via st %d", st ) );

    PNS_DBGN( Dbg(), EndGroup );

    return st;
}


/*
 * TODO describe....
 */
SHOVE::SHOVE_STATUS SHOVE::onReverseCollidingVia( LINE& aCurrent, VIA* aObstacleVia, OBSTACLE& aObstacleInfo )
{
    int n = 0;
    LINE cur( aCurrent );
    cur.ClearLinks();

    JOINT* jt = m_currentNode->FindJoint( aObstacleVia->Pos(), aObstacleVia );
    LINE shoved( aCurrent );
    shoved.ClearLinks();

    cur.RemoveVia();
    unwindLineStack( &aCurrent );

    for( ITEM* item : jt->LinkList() )
    {
        if( item->OfKind( ITEM::SEGMENT_T | ITEM::ARC_T ) && item->LayersOverlap( &aCurrent ) )
        {
            LINKED_ITEM* li = static_cast<LINKED_ITEM*>( item );
            LINE head = assembleLine( li );

            head.AppendVia( *aObstacleVia );

            SHOVE_STATUS st = ShoveObstacleLine( head, cur, shoved, aObstacleInfo );

            if( st != SH_OK )
            {
#if 0
                m_logger.NewGroup( "on-reverse-via-fail-shove", m_iter );
                m_logger.Log( aObstacleVia, 0, "the-via" );
                m_logger.Log( &aCurrent, 1, "current-line" );
                m_logger.Log( &shoved, 3, "shoved-line" );
#endif

                return st;
            }

            cur.SetShape( shoved.CLine() );
            n++;
        }
    }

    if( !n )
    {
#if 0
        m_logger.NewGroup( "on-reverse-via-fail-lonevia", m_iter );
        m_logger.Log( aObstacleVia, 0, "the-via" );
        m_logger.Log( &aCurrent, 1, "current-line" );
#endif

        LINE head( aCurrent );
        head.Line().Clear();
        head.AppendVia( *aObstacleVia );
        head.ClearLinks();

        SHOVE_STATUS st = ShoveObstacleLine( head, aCurrent, shoved, aObstacleInfo );

        if( st != SH_OK )
            return st;

        cur.SetShape( shoved.CLine() );
    }

    if( aCurrent.EndsWithVia() )
        shoved.AppendVia( aCurrent.Via() );

#if 0
    m_logger.NewGroup( "on-reverse-via", m_iter );
    m_logger.Log( aObstacleVia, 0, "the-via" );
    m_logger.Log( &aCurrent, 1, "current-line" );
    m_logger.Log( &shoved, 3, "shoved-line" );
#endif

    PNS_DBG( Dbg(), AddItem, aObstacleVia, YELLOW, 0, wxT( "rr-the-via" ) );
    PNS_DBG( Dbg(), AddItem, &aCurrent, BLUE, 0, wxT( "rr-current-line" ) );
    PNS_DBG( Dbg(), AddItem, &shoved, GREEN, 0, wxT( "rr-shoved-line" ) );

    int currentRank = aCurrent.Rank();
    replaceLine( aCurrent, shoved );

    if( !pushLineStack( shoved ) )
        return SH_INCOMPLETE;

    shoved.SetRank( currentRank );

    return SH_OK;
}


void SHOVE::unwindLineStack( LINKED_ITEM* aSeg )
{
    int d = 0;

    for( std::vector<LINE>::iterator i = m_lineStack.begin(); i != m_lineStack.end() ; )
    {
        if( i->ContainsLink( aSeg ) )
        {
            PNS_DBG(Dbg(), Message, wxString::Format("Unwind lc %d (depth %d/%d)", i->SegmentCount(), d, (int)m_lineStack.size() ) );
            i = m_lineStack.erase( i );
        }
        else
            i++;

        d++;
    }

    for( std::vector<LINE>::iterator i = m_optimizerQueue.begin(); i != m_optimizerQueue.end() ; )
    {
        if( i->ContainsLink( aSeg ) )
            i = m_optimizerQueue.erase( i );
        else
            i++;
    }
}


void SHOVE::unwindLineStack( ITEM* aItem )
{
    if( aItem->OfKind( ITEM::SEGMENT_T  | ITEM::ARC_T ) )
        unwindLineStack( static_cast<LINKED_ITEM*>( aItem ) );
    else if( aItem->OfKind( ITEM::LINE_T ) )
    {
        LINE* l = static_cast<LINE*>( aItem );

        for( LINKED_ITEM* seg : l->Links() )
            unwindLineStack( seg );
    }
}


bool SHOVE::pushLineStack( const LINE& aL, bool aKeepCurrentOnTop )
{
    if( !aL.IsLinkedChecked() && aL.SegmentCount() != 0 )
    {
        PNS_DBG( Dbg(), AddItem, &aL, BLUE, 10000, wxT( "push line stack failed" ) );

        return false;
    }

    if( aKeepCurrentOnTop && m_lineStack.size() > 0)
    {
        m_lineStack.insert( m_lineStack.begin() + m_lineStack.size() - 1, aL );
    }
    else
    {
        m_lineStack.push_back( aL );
    }

    m_optimizerQueue.push_back( aL );

    return true;
}


void SHOVE::popLineStack( )
{
    LINE& l = m_lineStack.back();

    for( std::vector<LINE>::iterator i = m_optimizerQueue.begin(); i != m_optimizerQueue.end(); )
    {
        bool found = false;

        for( LINKED_ITEM* s : l.Links() )
        {
            if( i->ContainsLink( s ) )
            {
                i = m_optimizerQueue.erase( i );
                found = true;
                break;
            }
        }

        if( !found )
            i++;
    }

    m_lineStack.pop_back();
}


bool SHOVE::fixupViaCollisions( const LINE* aCurrent, OBSTACLE& obs )
{
    // if the current obstacle is a via, consider also the lines connected to it
    // if their widths are larger or equal than the via diameter, the shove algorithm
    // will very likely fail in the subsequent iterations (as our base assumption is track
    // ends can never move on their own, only dragged by force-propagated vias

    // our colliding item is a via: just find the max width of the traces connected to it
    if( obs.m_item->OfKind( ITEM::VIA_T ) )
    {
        VIA*   v = static_cast<VIA*>( obs.m_item );
        int    maxw = 0;
        JOINT* jv = m_currentNode->FindJoint( v->Pos(), v );

        for( auto link : jv->Links() )
        {
            if( link.item->OfKind( ITEM::SEGMENT_T ) ) // consider segments ...
            {
                auto seg = static_cast<SEGMENT*>( link.item );
                maxw = std::max( seg->Width(), maxw );
            }
            else if( link.item->OfKind( ITEM::ARC_T ) ) // ... or arcs
            {
                auto arc = static_cast<ARC*>( link.item );
                maxw = std::max( arc->Width(), maxw );
            }
        }

        obs.m_maxFanoutWidth = 0;

        if( maxw > 0 && maxw >= v->Diameter() )
        {
            obs.m_maxFanoutWidth = maxw + 1;
            PNS_DBG( Dbg(), Message,
                     wxString::Format( "Fixup via: new-w %d via-w %d", maxw, v->Diameter() ) );

            return true;
        }
        return false;
    }


    // our colliding item is a segment. check if it has a via on either of the ends.
    if( !obs.m_item->OfKind( ITEM::SEGMENT_T ) )
        return false;

    auto s = static_cast<SEGMENT*>( obs.m_item );

    JOINT* ja = m_currentNode->FindJoint( s->Seg().A, s );
    JOINT* jb = m_currentNode->FindJoint( s->Seg().B, s );

    VIA* vias[] = { ja->Via(), jb->Via() };

    for( int i = 0; i < 2; i++ )
    {
        VIA* v = vias[i];

        // via diameter is larger than the segment width - cool, the force propagation algo
        // will be able to deal with it, no need to intervene
        if( !v || v->Diameter() > s->Width() )
            continue;

        VIA vtest( *v );
        vtest.SetDiameter( s->Width() );

        // enlarge the via to the width of the segment
        if( vtest.Collide( aCurrent, m_currentNode ) )
        {
            // if colliding, drop the segment in the shove iteration loop and force-propagate the via instead
            obs.m_item = v;
            obs.m_maxFanoutWidth = s->Width() + 1;
            return true;
        }
    }
    return false;
}

/*
 * Resolve the next collision.
 */
SHOVE::SHOVE_STATUS SHOVE::shoveIteration( int aIter )
{
    LINE currentLine = m_lineStack.back();
    NODE::OPT_OBSTACLE nearest;
    SHOVE_STATUS st = SH_NULL;

    if( Dbg() )
    {
    Dbg()->SetIteration( aIter );
    }

    PNS_DBG( Dbg(), AddItem, &currentLine, RED, currentLine.Width(), wxString::Format( "current-coll-chk rank %d", currentLine.Rank() ) );

    for( ITEM::PnsKind search_order : { ITEM::SOLID_T, ITEM::VIA_T, ITEM::SEGMENT_T } )
    {
         nearest = m_currentNode->NearestObstacle( &currentLine, search_order );

         if( nearest )
         {
             PNS_DBG( Dbg(), Message,
                      wxString::Format( wxT( "nearest %p %s rank %d" ), nearest->m_item,
                                        nearest->m_item->KindStr(), nearest->m_item->Rank() ) );

            PNS_DBG( Dbg(), AddShape, nearest->m_item->Shape(), YELLOW, 10000, wxT("nearest") );
         }   

         if( nearest )
            break;
    }

    if( !nearest )
    {
        m_lineStack.pop_back();
        PNS_DBG( Dbg(), Message, "no-nearest-item ");
        return SH_OK;
    }

    bool viaFixup = fixupViaCollisions( &currentLine, nearest.get() );

    PNS_DBG( Dbg(), Message, wxString::Format( "iter %d: VF %d", aIter, viaFixup?1:0 ) );


    ITEM* ni = nearest->m_item;

    unwindLineStack( ni );

    if( !ni->OfKind( ITEM::SOLID_T ) && ni->Rank() >= 0 && ni->Rank() > currentLine.Rank() )
    {
        // Collision with a higher-ranking object (ie: one that we've already shoved)
        //
        switch( ni->Kind() )
        {
        case ITEM::VIA_T:
        {
            PNS_DBG( Dbg(), BeginGroup, wxString::Format( "iter %d: reverse-collide-via", aIter ).ToStdString(), 0 );

            if( currentLine.EndsWithVia() )
            {
                st = SH_INCOMPLETE;
            }
            else
            {
                st = onReverseCollidingVia( currentLine, (VIA*) ni, nearest.get() );
            }

            PNS_DBGN( Dbg(), EndGroup );

            break;
        }

        case ITEM::SEGMENT_T:
        {
            PNS_DBG( Dbg(), BeginGroup, wxString::Format( "iter %d: reverse-collide-segment ",
                                                       aIter ).ToStdString(), 0 );
            LINE revLine = assembleLine( static_cast<SEGMENT*>( ni ) );

            popLineStack();
            st = onCollidingLine( revLine, currentLine, nearest.get() );


            if( !pushLineStack( revLine ) )
            {
                return SH_INCOMPLETE;
            }


            PNS_DBGN( Dbg(), EndGroup );

            break;
        }

        case ITEM::ARC_T:
        {
            //TODO(snh): Handle Arc shove separate from track
            PNS_DBG( Dbg(), BeginGroup, wxString::Format( "iter %d: reverse-collide-arc ", aIter ).ToStdString(), 0 );
            LINE revLine = assembleLine( static_cast<ARC*>( ni ) );

            popLineStack();
            st = onCollidingLine( revLine, currentLine, nearest.get() );

            PNS_DBGN( Dbg(), EndGroup );

            if( !pushLineStack( revLine ) )
                return SH_INCOMPLETE;

            break;
        }

        default:
            assert( false );
        }
    }
    else
    {
        // Collision with a lower-ranking object or a solid
        //
        switch( ni->Kind() )
        {
        case ITEM::SEGMENT_T:
            PNS_DBG( Dbg(), BeginGroup, wxString::Format( "iter %d: collide-segment ", aIter ), 0 );

            st = onCollidingSegment( currentLine, (SEGMENT*) ni, nearest.get() );

            if( st == SH_TRY_WALK )
                st = onCollidingSolid( currentLine, ni, nearest.get() );

            PNS_DBGN( Dbg(), EndGroup );

            break;

            //TODO(snh): Customize Arc collide
        case ITEM::ARC_T:
            PNS_DBG( Dbg(), BeginGroup, wxString::Format( "iter %d: collide-arc ", aIter ), 0 );

            st = onCollidingArc( currentLine, static_cast<ARC*>( ni ), nearest.get() );

            if( st == SH_TRY_WALK )
                st = onCollidingSolid( currentLine, ni, nearest.get() );

            PNS_DBGN( Dbg(), EndGroup );

            break;

        case ITEM::VIA_T:
            PNS_DBG( Dbg(), BeginGroup, wxString::Format( "iter %d: collide-via (fixup: %d)", aIter, 0 ), 0 );            
            st = onCollidingVia( &currentLine, (VIA*) ni, nearest.get() );

            if( st == SH_TRY_WALK )
                st = onCollidingSolid( currentLine, ni, nearest.get() );

            PNS_DBGN( Dbg(), EndGroup );

            break;

        case ITEM::SOLID_T:
            PNS_DBG( Dbg(), BeginGroup, wxString::Format( "iter %d: walk-solid ", aIter ), 0);
            st = onCollidingSolid( currentLine, (SOLID*) ni, nearest.get() );

            PNS_DBGN( Dbg(), EndGroup );

            break;

        default:
            break;
        }
    }

    return st;
}


/*
 * Resolve collisions.
 * Each iteration pushes the next colliding object out of the way.  Iterations are continued as
 * long as they propagate further collisions, or until the iteration timeout or max iteration
 * count is reached.
 */
SHOVE::SHOVE_STATUS SHOVE::shoveMainLoop()
{
    SHOVE_STATUS st = SH_OK;

    m_affectedArea = OPT_BOX2I();

    PNS_DBG( Dbg(), Message, wxString::Format( "ShoveStart [root: %d jts, current: %d jts]",
                                               m_root->JointCount(),
                                               m_currentNode->JointCount() ) );

    int iterLimit = Settings().ShoveIterationLimit();
    TIME_LIMIT timeLimit = Settings().ShoveTimeLimit();

    m_iter = 0;

    timeLimit.Restart();

    if( m_lineStack.empty() && m_draggedVia )
    {
        // If we're shoving a free via then push a proxy LINE (with the via on the end) onto
        // the stack.
        pushLineStack( LINE( *m_draggedVia ) );
    }

    while( !m_lineStack.empty() )
    {
        PNS_DBG( Dbg(), Message, wxString::Format( "iter %d: node %p stack %d ", m_iter,
                                                   m_currentNode, (int) m_lineStack.size() ) );

        st = shoveIteration( m_iter );

        m_iter++;

        if( st == SH_INCOMPLETE || timeLimit.Expired() || m_iter >= iterLimit )
        {
            PNS_DBG( Dbg(), Message, wxString::Format( "Fail [time limit expired: %d iter %d iter limit %d",
                                               timeLimit.Expired()?1:0, m_iter, iterLimit ) );
            st = SH_INCOMPLETE;
            break;
        }
    }

    return st;
}


OPT_BOX2I SHOVE::totalAffectedArea() const
{
    OPT_BOX2I area;

    if( !m_nodeStack.empty() )
        area = m_nodeStack.back().m_affectedArea;

    if( area  && m_affectedArea)
        area->Merge( *m_affectedArea );
    else if( !area )
        area = m_affectedArea;

    return area;
}


SHOVE::SHOVE_STATUS SHOVE::ShoveLines( const LINE& aCurrentHead )
{
    SHOVE_STATUS st = SH_OK;

    m_multiLineMode = false;

    PNS_DBG( Dbg(), Message,
             wxString::Format( "Shove start, lc = %d", aCurrentHead.SegmentCount() ) )

    // empty head? nothing to shove...
    if( !aCurrentHead.SegmentCount() && !aCurrentHead.EndsWithVia() )
        return SH_INCOMPLETE;

    LINE head( aCurrentHead );
    head.ClearLinks();

    m_lineStack.clear();
    m_optimizerQueue.clear();
    m_newHead = OPT_LINE();

#if 0
    m_logger.Clear();
#endif

    // Pop NODEs containing previous shoves which are no longer necessary
    //
    ITEM_SET headSet;
    headSet.Add( aCurrentHead );

    VIA_HANDLE dummyVia;

    NODE* parent = reduceSpringback( headSet, dummyVia );

    // Create a new NODE to store this version of the world
    m_currentNode = parent->Branch();
    m_currentNode->ClearRanks();
    m_currentNode->Add( head );

    m_currentNode->LockJoint( head.CPoint(0), &head, true );

    if( !head.EndsWithVia() )
        m_currentNode->LockJoint( head.CPoint( -1 ), &head, true );

    head.Mark( MK_HEAD );
    head.SetRank( 100000 );

    PNS_DBG( Dbg(), AddItem, &head, CYAN, 0, wxT( "head, after shove" ) );

    if( head.EndsWithVia() )
    {
        std::unique_ptr< VIA >headVia = Clone( head.Via() );
        headVia->Mark( MK_HEAD );
        headVia->SetRank( 100000 );
        m_currentNode->Add( std::move( headVia ) );
    }

    if( !pushLineStack( head ) )
    {
        delete m_currentNode;
        m_currentNode = parent;

        return SH_INCOMPLETE;
    }

    st = shoveMainLoop();

    if( st == SH_OK )
    {
        runOptimizer( m_currentNode );

        if( m_newHead )
            st = m_currentNode->CheckColliding( &( *m_newHead ) ) ? SH_INCOMPLETE : SH_HEAD_MODIFIED;
        else
            st = m_currentNode->CheckColliding( &head ) ? SH_INCOMPLETE : SH_OK;
    }

    m_currentNode->RemoveByMarker( MK_HEAD );

    PNS_DBG( Dbg(), Message, wxString::Format( "Shove status : %s after %d iterations",
           ( ( st == SH_OK || st == SH_HEAD_MODIFIED ) ? "OK" : "FAILURE"), m_iter ) );

    if( st == SH_OK || st == SH_HEAD_MODIFIED )
    {
        pushSpringback( m_currentNode, m_affectedArea, nullptr );
    }
    else
    {
        delete m_currentNode;

        m_currentNode = parent;
        m_newHead = OPT_LINE();
    }

    if(m_newHead)
        m_newHead->Unmark();

    if( m_newHead && head.EndsWithVia() )
    {
        VIA v = head.Via();
        v.SetPos( m_newHead->CPoint( -1 ) );
        m_newHead->AppendVia(v);
    }

    return st;
}


SHOVE::SHOVE_STATUS SHOVE::ShoveMultiLines( const ITEM_SET& aHeadSet )
{
    SHOVE_STATUS st = SH_OK;

    m_multiLineMode = true;

    ITEM_SET headSet;

    for( const ITEM* item : aHeadSet.CItems() )
    {
        const LINE* headOrig = static_cast<const LINE*>( item );

        // empty head? nothing to shove...
        if( !headOrig->SegmentCount() )
            return SH_INCOMPLETE;

        headSet.Add( *headOrig );
    }

    m_lineStack.clear();
    m_optimizerQueue.clear();

#if 0
    m_logger.Clear();
#endif

    VIA_HANDLE dummyVia;

    NODE* parent = reduceSpringback( headSet, dummyVia );

    m_currentNode = parent->Branch();
    m_currentNode->ClearRanks();
    int n = 0;

    for( const ITEM* item : aHeadSet.CItems() )
    {
        const LINE* headOrig = static_cast<const LINE*>( item );
        LINE head( *headOrig );
        head.ClearLinks();

        m_currentNode->Add( head );

        head.Mark( MK_HEAD );
        head.SetRank( 100000 );
        n++;

        if( !pushLineStack( head ) )
            return SH_INCOMPLETE;

        if( head.EndsWithVia() )
        {
            std::unique_ptr< VIA > headVia = Clone( head.Via() );
            headVia->Mark( MK_HEAD );
            headVia->SetRank( 100000 );
            m_currentNode->Add( std::move( headVia ) );
        }
    }

    st = shoveMainLoop();

    if( st == SH_OK )
        runOptimizer( m_currentNode );

    m_currentNode->RemoveByMarker( MK_HEAD );

    PNS_DBG( Dbg(), Message, wxString::Format( "Shove status : %s after %d iterations",
           ( st == SH_OK ? "OK" : "FAILURE"), m_iter ) );

    if( st == SH_OK )
    {
        pushSpringback( m_currentNode, m_affectedArea, nullptr );
    }
    else
    {
        delete m_currentNode;
        m_currentNode = parent;
    }

    return st;
}


static VIA* findViaByHandle ( NODE *aNode, const VIA_HANDLE& handle )
{
    JOINT* jt = aNode->FindJoint( handle.pos, handle.layers.Start(), handle.net );

    if( !jt )
        return nullptr;

    for( ITEM* item : jt->LinkList() )
    {
        if( item->OfKind( ITEM::VIA_T ) )
        {
            if( item->Net() == handle.net && item->Layers().Overlaps(handle.layers) )
                return static_cast<VIA*>( item );
        }
    }

    return nullptr;
}


SHOVE::SHOVE_STATUS SHOVE::ShoveDraggingVia( const VIA_HANDLE aOldVia, const VECTOR2I& aWhere,
                                             VIA_HANDLE& aNewVia )
{
    SHOVE_STATUS st = SH_OK;

    m_lineStack.clear();
    m_optimizerQueue.clear();
    m_newHead = OPT_LINE();
    m_draggedVia = nullptr;

    VIA* viaToDrag = findViaByHandle( m_currentNode, aOldVia );

    if( !viaToDrag )
        return SH_INCOMPLETE;

    // Pop NODEs containing previous shoves which are no longer necessary
    ITEM_SET headSet;

    VIA headVia ( *viaToDrag );
    headVia.SetPos( aWhere );
    headSet.Add( headVia );
    VIA_HANDLE prevViaHandle;
    NODE* parent = reduceSpringback( headSet, prevViaHandle );

    if( prevViaHandle.valid )
    {
        aNewVia = prevViaHandle;
        viaToDrag = findViaByHandle( parent, prevViaHandle );
    }

    // Create a new NODE to store this version of the world
    m_currentNode = parent->Branch();
    m_currentNode->ClearRanks();

    viaToDrag->Mark( MK_HEAD );
    viaToDrag->SetRank( 100000 );

    // Push the via to its new location
    st = pushOrShoveVia( viaToDrag, ( aWhere - viaToDrag->Pos() ), 0 );

    // Shove any colliding objects out of the way
    if( st == SH_OK )
        st = shoveMainLoop();

    if( st == SH_OK )
        runOptimizer( m_currentNode );

    if( st == SH_OK || st == SH_HEAD_MODIFIED )
    {
        wxLogTrace( "PNS","setNewV %p", m_draggedVia );

        if (!m_draggedVia)
            m_draggedVia = viaToDrag;

        aNewVia = m_draggedVia->MakeHandle();

        pushSpringback( m_currentNode, m_affectedArea, viaToDrag );
    }
    else
    {
        delete m_currentNode;
        m_currentNode = parent;
    }

    return st;
}


LINE* SHOVE::findRootLine( LINE *aLine )
{
    for( auto link : aLine->Links() )
    {
        if( auto seg = dyn_cast<SEGMENT*>( link ) )
        {
            auto it = m_rootLineHistory.find( seg );

            if( it != m_rootLineHistory.end() )
                return it->second;
        }
    }

    return nullptr;
}


void SHOVE::runOptimizer( NODE* aNode )
{
    OPTIMIZER optimizer( aNode );
    int optFlags = 0;
    int n_passes = 0;

    PNS_OPTIMIZATION_EFFORT effort = Settings().OptimizerEffort();

    OPT_BOX2I area = totalAffectedArea();

    int maxWidth = 0;

    for( LINE& line : m_optimizerQueue )
        maxWidth = std::max( line.Width(), maxWidth );

    if( area )
    {
        area->Inflate( maxWidth );
        area = area->Intersect( VisibleViewArea() );
    }

    switch( effort )
    {
    case OE_LOW:
        optFlags |= OPTIMIZER::MERGE_OBTUSE;
        n_passes = 1;
        break;

    case OE_MEDIUM:
        optFlags |= OPTIMIZER::MERGE_SEGMENTS;

        n_passes = 2;
        break;

    case OE_FULL:
        optFlags = OPTIMIZER::MERGE_SEGMENTS;
        n_passes = 2;
        break;

    default:
        break;
    }

    optFlags |= OPTIMIZER::LIMIT_CORNER_COUNT;

    if( area )
    {
        SHAPE_RECT r( *area );

        PNS_DBG( Dbg(), AddShape, &r, BLUE, 0, wxT( "opt-area" ) );

        optFlags |= OPTIMIZER::RESTRICT_AREA;
        optimizer.SetRestrictArea( *area, false );
    }

    if( Settings().SmartPads() )
        optFlags |= OPTIMIZER::SMART_PADS;


    optimizer.SetEffortLevel( optFlags & ~m_optFlagDisableMask );
    optimizer.SetCollisionMask( ITEM::ANY_T );

    for( int pass = 0; pass < n_passes; pass++ )
    {
        std::reverse( m_optimizerQueue.begin(), m_optimizerQueue.end() );

        for( LINE& line : m_optimizerQueue)
        {
            if( !( line.Marker() & MK_HEAD ) )
            {
                LINE optimized;
                LINE* root = findRootLine( &line );

                if( optimizer.Optimize( &line, &optimized, root ) )
                {
                    replaceLine( line, optimized, false, aNode );
                    line = optimized; // keep links in the lines in the queue up to date
                }
            }
        }
    }
}


NODE* SHOVE::CurrentNode()
{
    return m_nodeStack.empty() ? m_root : m_nodeStack.back().m_node;
}


const LINE SHOVE::NewHead() const
{
    assert( m_newHead );

    return *m_newHead;
}


void SHOVE::SetInitialLine( LINE& aInitial )
{
    m_root = m_root->Branch();
    m_root->Remove( aInitial );
}


bool SHOVE::AddLockedSpringbackNode( NODE* aNode )
{
    SPRINGBACK_TAG sp;
    sp.m_node = aNode;
    sp.m_locked = true;

    m_nodeStack.push_back(sp);
    return true;
}


bool SHOVE::RewindSpringbackTo( NODE* aNode )
{
    bool found = false;

    auto iter = m_nodeStack.begin();

    while( iter != m_nodeStack.end() )
    {
        if ( iter->m_node == aNode )
        {
            found = true;
            break;
        }

        iter++;
    }

    if( !found )
        return false;

    auto start = iter;

    aNode->KillChildren();
    m_nodeStack.erase( start, m_nodeStack.end() );

    return true;
}


bool SHOVE::RewindToLastLockedNode()
{
    if( m_nodeStack.empty() )
        return false;

    while( !m_nodeStack.back().m_locked && m_nodeStack.size() > 1 )
        m_nodeStack.pop_back();

    return m_nodeStack.back().m_locked;
}


void SHOVE::UnlockSpringbackNode( NODE* aNode )
{
    auto iter = m_nodeStack.begin();

    while( iter != m_nodeStack.end() )
    {
        if ( iter->m_node == aNode )
        {
            iter->m_locked = false;
            break;
        }

        iter++;
    }
}


void SHOVE::DisablePostShoveOptimizations( int aMask )
{
    m_optFlagDisableMask = aMask;
}


void SHOVE::SetSpringbackDoNotTouchNode( NODE *aNode )
{
    m_springbackDoNotTouchNode = aNode;
}

}

