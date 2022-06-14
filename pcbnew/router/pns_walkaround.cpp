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

#include <core/optional.h>

#include <geometry/shape_line_chain.h>

#include "pns_walkaround.h"
#include "pns_optimizer.h"
#include "pns_utils.h"
#include "pns_router.h"
#include "pns_debug_decorator.h"
#include "pns_solid.h"
#include "pns_hull.h"

namespace PNS {

void WALKAROUND::start( const LINE& aInitialPath )
{
    m_iteration = 0;
    m_iterationLimit = 50;
}


NODE::OPT_OBSTACLE WALKAROUND::nearestObstacle( const LINE& aPath )
{
    COLLISION_SEARCH_OPTIONS opts;

    NODE::OPT_OBSTACLE obs = m_world->NearestObstacle(
            &aPath, m_itemMask, m_restrictedSet.empty() ? nullptr : &m_restrictedSet, opts );

    if( m_restrictedSet.empty() )
        return obs;

    else if( obs && m_restrictedSet.find ( obs->m_item ) != m_restrictedSet.end() )
        return obs;

    return NODE::OPT_OBSTACLE();
}


void WALKAROUND::RestrictToSet( bool aEnabled, const std::set<ITEM*>& aSet )
{
    m_restrictedVertices.clear();

    if( aEnabled )
        m_restrictedSet = aSet;
    else
        m_restrictedSet.clear();

    for( auto item : aSet )
    {
        if( auto solid = dyn_cast<SOLID*>( item ) )
        {
            m_restrictedVertices.push_back( solid->Anchor( 0 ) );
        }
    }
}


WALKAROUND::WALKAROUND_STATUS WALKAROUND::singleStep( LINE& aPath, bool aWindingDirection )
{
    OPT<OBSTACLE>& current_obs =
        aWindingDirection ? m_currentObstacle[0] : m_currentObstacle[1];

    if( !current_obs )
        return DONE;

    VECTOR2I initialLast = aPath.CPoint( -1 );

    SHAPE_LINE_CHAIN path_walk;

    HULL hull( *current_obs, false );
    const auto hullShape = hull.Shape();

    bool s_cw = aPath.Walkaround( hullShape, path_walk, aWindingDirection );

    PNS_DBG( Dbg(), BeginGroup, "hull/walk", 1 );
    PNS_DBG( Dbg(), AddShape, &hullShape, RED, 0, wxString::Format( "hull-%s-%d", aWindingDirection ? wxT( "cw" ) : wxT( "ccw" ), m_iteration ) );
    PNS_DBG( Dbg(), AddShape, &aPath.CLine(), GREEN, 0, wxString::Format( "path-%s-%d", aWindingDirection ? wxT( "cw" ) : wxT( "ccw" ), m_iteration ) );
    PNS_DBG( Dbg(), AddShape, &path_walk, BLUE, 0, wxString::Format( "result-%s-%d", aWindingDirection ? wxT( "cw" ) : wxT( "ccw" ), m_iteration ) );
    PNS_DBG( Dbg(), Message, wxString::Format( wxT( "Stat cw %d" ), !!s_cw ) );
    PNS_DBGN( Dbg(), EndGroup );

    path_walk.Simplify();
    aPath.SetShape( path_walk );

    // If the end of the line is inside an obstacle, additional walkaround iterations are not
    // going to help.  Exit now to prevent pegging the iteration limiter and causing lag.
    if( current_obs && hull.Shape().PointInside( initialLast ) &&
        !hull.Shape().PointOnEdge( initialLast ) )
    {
        return ALMOST_DONE;
    }

    current_obs = nearestObstacle( LINE( aPath, path_walk ) );

    return IN_PROGRESS;
}


const WALKAROUND::RESULT WALKAROUND::Route( const LINE& aInitialPath )
{
    LINE path_cw( aInitialPath ), path_ccw( aInitialPath );
    WALKAROUND_STATUS s_cw = IN_PROGRESS, s_ccw = IN_PROGRESS;
    SHAPE_LINE_CHAIN best_path;
    RESULT result;

    // special case for via-in-the-middle-of-track placement
    if( aInitialPath.PointCount() <= 1 )
    {
        if( aInitialPath.EndsWithVia() && m_world->CheckColliding( &aInitialPath.Via(),
                                                                   m_itemMask ) )
            return RESULT( STUCK, STUCK );

        return RESULT( DONE, DONE, aInitialPath, aInitialPath );
    }

    start( aInitialPath );

    m_currentObstacle[0] = m_currentObstacle[1] = nearestObstacle( aInitialPath );

    result.lineCw = aInitialPath;
    result.lineCcw = aInitialPath;

    if( m_forceWinding )
    {
        s_cw = m_forceCw ? IN_PROGRESS : STUCK;
        s_ccw = m_forceCw ? STUCK : IN_PROGRESS;
    }

    // In some situations, there isn't a trivial path (or even a path at all).  Hitting the
    // iteration limit causes lag, so we can exit out early if the walkaround path gets very long
    // compared with the initial path.  If the length exceeds the initial length times this factor,
    // fail out.
    const int maxWalkDistFactor = 10;
    long long lengthLimit       = aInitialPath.CLine().Length() * maxWalkDistFactor;

    while( m_iteration < m_iterationLimit )
    {
        if( s_cw != STUCK && s_cw != ALMOST_DONE )
            s_cw = singleStep( path_cw, true );

        if( s_ccw != STUCK && s_ccw != ALMOST_DONE )
            s_ccw = singleStep( path_ccw, false );

        if( s_cw != IN_PROGRESS )
        {
            result.lineCw = path_cw;
            result.statusCw = s_cw;
        }

        if( s_ccw != IN_PROGRESS )
        {
            result.lineCcw = path_ccw;
            result.statusCcw = s_ccw;
        }

        if( s_cw != IN_PROGRESS && s_ccw != IN_PROGRESS )
            break;

        // Safety valve
        if( path_cw.Line().Length() > lengthLimit && path_ccw.Line().Length() > lengthLimit )
            break;

        m_iteration++;
    }

    if( s_cw == IN_PROGRESS )
    {
        result.lineCw = path_cw;
        result.statusCw = ALMOST_DONE;
    }

    if( s_ccw == IN_PROGRESS )
    {
        result.lineCcw = path_ccw;
        result.statusCcw = ALMOST_DONE;
    }

    if( result.lineCw.SegmentCount() < 1 || result.lineCw.CPoint( 0 ) != aInitialPath.CPoint( 0 ) )
    {
        result.statusCw = STUCK;
    }

    if( result.lineCw.PointCount() > 0 && result.lineCw.CPoint( -1 ) != aInitialPath.CPoint( -1 ) )
    {
        result.statusCw = ALMOST_DONE;
    }

    if( result.lineCcw.SegmentCount() < 1 ||
        result.lineCcw.CPoint( 0 ) != aInitialPath.CPoint( 0 ) )
    {
        result.statusCcw = STUCK;
    }

    if( result.lineCcw.PointCount() > 0 &&
        result.lineCcw.CPoint( -1 ) != aInitialPath.CPoint( -1 ) )
    {
        result.statusCcw = ALMOST_DONE;
    }

    result.lineCw.ClearLinks();
    result.lineCcw.ClearLinks();

    return result;
}


WALKAROUND::WALKAROUND_STATUS WALKAROUND::Route( const LINE& aInitialPath, LINE& aWalkPath,
                                                 bool aOptimize )
{
    LINE path_cw( aInitialPath ), path_ccw( aInitialPath );
    WALKAROUND_STATUS s_cw = IN_PROGRESS, s_ccw = IN_PROGRESS;
    SHAPE_LINE_CHAIN best_path;

    // special case for via-in-the-middle-of-track placement
    if( aInitialPath.PointCount() <= 1 )
    {
        if( aInitialPath.EndsWithVia() && m_world->CheckColliding( &aInitialPath.Via(),
                                                                   m_itemMask ) )
            return STUCK;

        aWalkPath = aInitialPath;
        return DONE;
    }

    start( aInitialPath );

    m_currentObstacle[0] = m_currentObstacle[1] = nearestObstacle( aInitialPath );

    aWalkPath = aInitialPath;

    if( m_forceWinding )
    {
        s_cw = m_forceCw ? IN_PROGRESS : STUCK;
        s_ccw = m_forceCw ? STUCK : IN_PROGRESS;
    }

    while( m_iteration < m_iterationLimit )
    {
        if( path_cw.PointCount() == 0 )
            s_cw = STUCK; // cw path is empty, can't continue

        if( path_ccw.PointCount() == 0 )
            s_ccw = STUCK; // ccw path is empty, can't continue

        if( s_cw != STUCK )
            s_cw = singleStep( path_cw, true );

        if( s_ccw != STUCK )
            s_ccw = singleStep( path_ccw, false );

        if( ( s_cw == DONE && s_ccw == DONE ) || ( s_cw == STUCK && s_ccw == STUCK ) )
        {
            int len_cw  = path_cw.CLine().Length();
            int len_ccw = path_ccw.CLine().Length();

            if( m_forceLongerPath )
                aWalkPath = ( len_cw > len_ccw ? path_cw : path_ccw );
            else
                aWalkPath = ( len_cw < len_ccw ? path_cw : path_ccw );

            break;
        }
        else if( s_cw == DONE && !m_forceLongerPath )
        {
            aWalkPath = path_cw;
            break;
        }
        else if( s_ccw == DONE && !m_forceLongerPath )
        {
            aWalkPath = path_ccw;
            break;
        }

        m_iteration++;
    }

    if( m_iteration == m_iterationLimit )
    {
        int len_cw  = path_cw.CLine().Length();
        int len_ccw = path_ccw.CLine().Length();

        if( m_forceLongerPath )
            aWalkPath = ( len_cw > len_ccw ? path_cw : path_ccw );
        else
            aWalkPath = ( len_cw < len_ccw ? path_cw : path_ccw );
    }

    aWalkPath.Line().Simplify();

    if( aWalkPath.SegmentCount() < 1 )
        return STUCK;

    if( aWalkPath.CPoint( -1 ) != aInitialPath.CPoint( -1 ) )
        return ALMOST_DONE;

    if( aWalkPath.CPoint( 0 ) != aInitialPath.CPoint( 0 ) )
        return STUCK;

    WALKAROUND_STATUS st = s_ccw == DONE || s_cw == DONE ? DONE : STUCK;

    if( st == DONE )
    {
        if( aOptimize )
            OPTIMIZER::Optimize( &aWalkPath, OPTIMIZER::MERGE_OBTUSE, m_world );
    }

    return st;
}
}
