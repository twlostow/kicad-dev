/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2013-2017 CERN
 * Copyright (C) 2016 KiCad Developers, see AUTHORS.txt for contributors.
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

#include "pns_node.h"
#include "pns_line_placer.h"
#include "pns_walkaround.h"
#include "pns_shove.h"
#include "pns_utils.h"
#include "pns_router.h"
#include "pns_topology.h"
#include "pns_debug_decorator.h"

#include <class_board_item.h>

namespace PNS {

LINE_PLACER::LINE_PLACER( ROUTER* aRouter ) :
    PLACEMENT_ALGO( aRouter )
{
    m_initial_direction = DIRECTION_45::N;
    m_world = NULL;
    m_shove = NULL;
    m_currentNode = NULL;
    m_idle = true;

    // Init temporary variables (do not leave uninitialized members)
    m_lastNode = NULL;
    m_placingVia = false;
    m_currentNet = 0;
    m_currentLayer = 0;
    m_currentMode = RM_MarkObstacles;
    m_startItem = NULL;
    m_chainedPlacement = false;
    m_orthoMode = false;
}


LINE_PLACER::~LINE_PLACER()
{
}


void LINE_PLACER::setWorld( NODE* aWorld )
{
    m_world = aWorld;
}


const VIA LINE_PLACER::makeVia( const VECTOR2I& aP )
{
    const LAYER_RANGE layers( m_sizes.GetLayerTop(), m_sizes.GetLayerBottom() );

    return VIA( aP, layers, m_sizes.ViaDiameter(), m_sizes.ViaDrill(), -1, m_sizes.ViaType() );
}


bool LINE_PLACER::ToggleVia( bool aEnabled )
{
    m_placingVia = aEnabled;

    if( !aEnabled )
        m_head.RemoveVia();

    return true;
}


void LINE_PLACER::setInitialDirection( const DIRECTION_45& aDirection )
{
    m_initial_direction = aDirection;

    if( m_tail.SegmentCount() == 0 )
            m_direction = aDirection;
}



bool LINE_PLACER::checkObtusity( const SEG& aA, const SEG& aB ) const
{
    const DIRECTION_45 dir_a( aA );
    const DIRECTION_45 dir_b( aB );

    return dir_a.IsObtuse( dir_b ) || dir_a == dir_b;
}


bool LINE_PLACER::rhWalkOnly( const VECTOR2I& aP, LINE& aNewHead )
{
    LINE initTrack( m_head );
    LINE walkFull;
    int effort = 0;
    bool rv = true, viaOk;

    viaOk = buildInitialLine( aP, initTrack );

    WALKAROUND walkaround( m_currentNode, Router() );

    walkaround.SetSolidsOnly( false );
    walkaround.SetDebugDecorator( Dbg() );
    walkaround.SetIterationLimit( Settings().WalkaroundIterationLimit() );

    WALKAROUND::WALKAROUND_STATUS wf = walkaround.Route( initTrack, walkFull, false );

    switch( Settings().OptimizerEffort() )
    {
    case OE_LOW:
        effort = 0;
        break;

    case OE_MEDIUM:
    case OE_FULL:
        effort = OPTIMIZER::MERGE_SEGMENTS;
        break;
    }

    if( Settings().SmartPads() )
        effort |= OPTIMIZER::SMART_PADS;

    if( wf == WALKAROUND::STUCK )
    {
        walkFull = walkFull.ClipToNearestObstacle( m_currentNode );
        rv = true;
    }
    else if( m_placingVia && viaOk )
    {
        walkFull.AppendVia( makeVia( walkFull.CPoint( -1 ) ) );
    }

    OPTIMIZER::Optimize( &walkFull, effort, m_currentNode );

    if( m_currentNode->CheckColliding( &walkFull ) )
    {
        aNewHead = m_head;
        return false;
    }

    m_head = walkFull;
    aNewHead = walkFull;

    return rv;
}


bool LINE_PLACER::rhMarkObstacles( const VECTOR2I& aP, LINE& aNewHead )
{
    LINE newHead( m_head ), bestHead( m_head );
    bool hasBest = false;

    buildInitialLine( aP, newHead );

    NODE::OBSTACLES obstacles;

    m_currentNode->QueryColliding( &newHead, obstacles );

    for( auto& obs : obstacles )
    {
        int cl = m_currentNode->GetClearance( obs.m_item, &newHead );
        auto hull = obs.m_item->Hull( cl, newHead.Width() );

        auto nearest = hull.NearestPoint( aP );
        Dbg()->AddLine( hull, 2, 10000 );

        if( ( nearest - aP ).EuclideanNorm() < newHead.Width() + cl )
        {
            buildInitialLine( nearest, newHead );
            if ( newHead.CLine().Length() > bestHead.CLine().Length() )
            {
                bestHead = newHead;
                hasBest = true;
        }
    }
    }

    if( hasBest )
        m_head = bestHead;
    else
        m_head = newHead;

    aNewHead = m_head;

    return static_cast<bool>( m_currentNode->CheckColliding( &m_head ) );
}


const LINE LINE_PLACER::reduceToNearestObstacle( const LINE& aOriginalLine )
{
    const auto& l0 = aOriginalLine.CLine();

    if ( !l0.PointCount() )
        return aOriginalLine;

    int l = l0.Length();
    int step = l / 2;
    VECTOR2I target;

    LINE l_test( aOriginalLine );

    while( step > 0 )
    {
        target = l0.PointAlong( l );
        SHAPE_LINE_CHAIN l_cur( l0 );

        int index = l_cur.Split( target );

        l_test.SetShape( l_cur.Slice( 0, index ) );

        if ( m_currentNode->CheckColliding( &l_test ) )
            l -= step;
        else
            l += step;

        step /= 2;
    }

    l = l_test.CLine().Length();

    while( m_currentNode->CheckColliding( &l_test ) && l > 0 )
    {
        l--;
        target = l0.PointAlong( l );
        SHAPE_LINE_CHAIN l_cur( l0 );

        int index = l_cur.Split( target );

        l_test.SetShape( l_cur.Slice( 0, index ) );
    }

    return l_test;
}


bool LINE_PLACER::rhStopAtNearestObstacle( const VECTOR2I& aP, LINE& aNewHead )
{
    LINE l0;
    l0 = m_head;

    buildInitialLine( aP, l0 );

    LINE l_cur = reduceToNearestObstacle( l0 );

    const auto l_shape = l_cur.CLine();

    if( l_shape.SegmentCount() == 0 )
    {
        return false;
    }

    if( l_shape.SegmentCount() == 1 )
    {
        auto s = l_shape.CSegment( 0 );

        VECTOR2I dL( DIRECTION_45( s ).Left().ToVector() );
        VECTOR2I dR( DIRECTION_45( s ).Right().ToVector() );

        SEG leadL( s.B, s.B + dL );
        SEG leadR( s.B, s.B + dR );

        SEG segL( s.B, leadL.LineProject( aP ) );
        SEG segR( s.B, leadR.LineProject( aP ) );

        LINE finishL( l0, SHAPE_LINE_CHAIN( segL.A, segL.B ) );
        LINE finishR( l0, SHAPE_LINE_CHAIN( segR.A, segR.B ) );

        LINE reducedL = reduceToNearestObstacle( finishL );
        LINE reducedR = reduceToNearestObstacle( finishR );

        int lL = reducedL.CLine().Length();
        int lR = reducedR.CLine().Length();

        if( lL > lR )
            l_cur.Line().Append( reducedL.CLine() );
        else
            l_cur.Line().Append( reducedR.CLine() );

        l_cur.Line().Simplify();
    }

    m_head = l_cur;
    aNewHead = m_head;
    return true;
}


bool LINE_PLACER::rhShoveOnly( const VECTOR2I& aP, LINE& aNewHead )
{
    LINE initTrack( m_head );
    LINE walkSolids, l2;

    bool viaOk = buildInitialLine( aP, initTrack );

    m_currentNode = m_shove->CurrentNode();
    OPTIMIZER optimizer( m_currentNode );

    WALKAROUND walkaround( m_currentNode, Router() );

    walkaround.SetSolidsOnly( true );
    walkaround.SetIterationLimit( 10 );
    walkaround.SetDebugDecorator( Dbg() );
    WALKAROUND::WALKAROUND_STATUS stat_solids = walkaround.Route( initTrack, walkSolids );

    optimizer.SetEffortLevel( OPTIMIZER::MERGE_SEGMENTS );
    optimizer.SetCollisionMask( ITEM::SOLID_T );
    optimizer.Optimize( &walkSolids );

    if( stat_solids == WALKAROUND::DONE )
        l2 = walkSolids;
    else
        l2 = initTrack.ClipToNearestObstacle( m_shove->CurrentNode() );

    LINE l( m_tail );
    l.Line().Append( l2.CLine() );
    l.Line().Simplify();

    if( l.PointCount() == 0 || l2.PointCount() == 0 )
    {
        aNewHead = m_head;
        return false;
    }

    if( m_placingVia && viaOk )
    {
        VIA v1( makeVia( l.CPoint( -1 ) ) );
        VIA v2( makeVia( l2.CPoint( -1 ) ) );

        l.AppendVia( v1 );
        l2.AppendVia( v2 );
    }

    l.Line().Simplify();

    // in certain, uncommon cases there may be loops in the head+tail, In such case, we don't shove to avoid
    // screwing up the database.
    if( l.HasLoops() )
    {
        aNewHead = m_head;
        return false;
    }

    SHOVE::SHOVE_STATUS status = m_shove->ShoveLines( l );

    m_currentNode = m_shove->CurrentNode();

    if( status == SHOVE::SH_OK  || status == SHOVE::SH_HEAD_MODIFIED )
    {
        if( status == SHOVE::SH_HEAD_MODIFIED )
        {
            l2 = m_shove->NewHead();
        }

        optimizer.SetWorld( m_currentNode );
        optimizer.SetEffortLevel( OPTIMIZER::MERGE_OBTUSE | OPTIMIZER::SMART_PADS );
        optimizer.SetCollisionMask( ITEM::ANY_T );
        optimizer.Optimize( &l2 );

        aNewHead = l2;

        return true;
    }
    else
    {
        walkaround.SetWorld( m_currentNode );
        walkaround.SetSolidsOnly( false );
        walkaround.SetIterationLimit( 10 );
        walkaround.SetApproachCursor( true, aP );
        walkaround.Route( initTrack, l2 );
        aNewHead = l2.ClipToNearestObstacle( m_shove->CurrentNode() );

        return false;
    }

    return false;
}


bool LINE_PLACER::routeHead( const VECTOR2I& aP, LINE& aNewHead )
{
    return rhWalkOnly( aP, aNewHead );

    switch( m_currentMode )
    {
    case RM_MarkObstacles:
        return rhMarkObstacles( aP, aNewHead );
    case RM_Walkaround:
        return rhWalkOnly( aP, aNewHead );
    case RM_Shove:
        return rhShoveOnly( aP, aNewHead );
    default:
        break;
    }

    return false;
}


void LINE_PLACER::routeStep( const VECTOR2I& aP )
{

    int i, n_iter = 1;

    LINE new_head(m_head);

    wxLogTrace( "PNS", "INIT-DIR: %s head: %d, tail: %d segs",
            m_initial_direction.Format().c_str(), m_head.SegmentCount(), m_tail.SegmentCount() );

            new_head.Line().Clear();

//    for( i = 0; i < n_iter; i++ )
    //{
        go_back = false;
        m_p_start = m_tail.PointCount() == 0 ? m_currentStart : m_tail.CPoint( -1 );

//        new_head.Line().Append( m_p_start );


        if( !routeHead( aP, new_head ) )
            fail = true;

        //if( !new_head.Is45Degree() )
        //    fail = true;

        //if( !Settings().FollowMouse() )
        //    return;


        //if(fail)
        //    return;

        m_tail.Line().Append( new_head.CLine() );

        /*for(int i = 0 ;i < new_head.PointCount(); i++)
        {
            printf("h %d %d %d\n",i,new_head.CPoint(i).x, new_head.CPoint(i).y );
        }
        printf("CPt %d\n", new_head.PointCount() );
        m_head = new_head;*/
        m_head.Line().Clear();

        printf("tail segs: %d\n", m_tail.CLine().SegmentCount() );

        Dbg()->AddLine( m_tail.CLine(), 4, 10000 );

        OPTIMIZER::Optimize( &m_tail, OPTIMIZER::MERGE_SEGMENTS, m_currentNode );

}


bool LINE_PLACER::route( const VECTOR2I& aP )
{
    routeStep( aP );

    if (!m_head.PointCount() )
        return false;

    return m_head.CPoint(-1) == aP;
}


const LINE LINE_PLACER::Trace() const
{
    LINE tmp( m_head );

    tmp.SetShape( m_tail.CLine() );
    tmp.Line().Append( m_head.CLine() );
    tmp.Line().Simplify();
    return tmp;
}


const ITEM_SET LINE_PLACER::Traces()
{
    m_currentTrace = Trace();
    return ITEM_SET( &m_currentTrace );
}


void LINE_PLACER::FlipPosture()
{
    m_initial_direction = m_initial_direction.Right();
    m_direction = m_direction.Right();
}


NODE* LINE_PLACER::CurrentNode( bool aLoopsRemoved ) const
{
    if( aLoopsRemoved && m_lastNode )
        return m_lastNode;

    return m_currentNode;
}


bool LINE_PLACER::SplitAdjacentSegments( NODE* aNode, ITEM* aSeg, const VECTOR2I& aP )
{
    if( !aSeg )
        return false;

    if( !aSeg->OfKind( ITEM::SEGMENT_T ) )
        return false;

    JOINT* jt = aNode->FindJoint( aP, aSeg );

    if( jt && jt->LinkCount() >= 1 )
        return false;

    SEGMENT* s_old = static_cast<SEGMENT*>( aSeg );

    std::unique_ptr< SEGMENT > s_new[2] = {
        Clone( *s_old ),
        Clone( *s_old )
    };

    s_new[0]->SetEnds( s_old->Seg().A, aP );
    s_new[1]->SetEnds( aP, s_old->Seg().B );

    aNode->Remove( s_old );
    aNode->Add( std::move( s_new[0] ), true );
    aNode->Add( std::move( s_new[1] ), true );

    return true;
}


bool LINE_PLACER::SetLayer( int aLayer )
{
    if( m_idle )
    {
        m_currentLayer = aLayer;
        return true;
    }
    else if( m_chainedPlacement )
    {
        return false;
    }
    else if( !m_startItem || ( m_startItem->OfKind( ITEM::VIA_T ) && m_startItem->Layers().Overlaps( aLayer ) ) )
    {
        m_currentLayer = aLayer;
        initPlacement();
        Move( m_currentEnd, NULL );
        return true;
    }

    return false;
}


bool LINE_PLACER::Start( const VECTOR2I& aP, ITEM* aStartItem )
{
    m_currentStart = VECTOR2I( aP );
    m_currentEnd = VECTOR2I( aP );
    m_currentNet = std::max( 0, aStartItem ? aStartItem->Net() : 0 );
    m_startItem = aStartItem;
    m_placingVia = false;
    m_chainedPlacement = false;

    setInitialDirection( Settings().InitialDirection() );

    initPlacement();
    return true;
}


void LINE_PLACER::initPlacement()
{
    m_idle = false;

    m_head.Line().Clear();
    m_tail.Line().Clear();
    m_head.SetNet( m_currentNet );
    m_tail.SetNet( m_currentNet );
    m_head.SetLayer( m_currentLayer );
    m_tail.SetLayer( m_currentLayer );
    m_head.SetWidth( m_sizes.TrackWidth() );
    m_tail.SetWidth( m_sizes.TrackWidth() );
    m_head.RemoveVia();
    m_tail.RemoveVia();

    m_p_start = m_currentStart;
    m_direction = m_initial_direction;

    NODE* world = Router()->GetWorld();

    world->KillChildren();
    NODE* rootNode = world->Branch();

    SplitAdjacentSegments( rootNode, m_startItem, m_currentStart );

    setWorld( rootNode );

    wxLogTrace( "PNS", "world %p, intitial-direction %s layer %d",
            m_world, m_direction.Format().c_str(), m_currentLayer );

    m_lastNode = NULL;
    m_currentNode = m_world;
    m_currentMode = Settings().Mode();

    m_shove.reset();

    if( m_currentMode == RM_Shove || m_currentMode == RM_Smart )
    {
        m_shove.reset( new SHOVE( m_world->Branch(), Router() ) );
    }
}


bool LINE_PLACER::Move( const VECTOR2I& aP, ITEM* aEndItem )
{
    LINE current;
    VECTOR2I p = aP;
    int eiDepth = -1;

    if( aEndItem && aEndItem->Owner() )
        eiDepth = static_cast<NODE*>( aEndItem->Owner() )->Depth();

    if( m_lastNode )
    {
        delete m_lastNode;
        m_lastNode = NULL;
    }

    bool reachesEnd = route( p );

    current = Trace();

    if( !current.PointCount() )
        m_currentEnd = m_p_start;
    else
        m_currentEnd = current.CLine().CPoint( -1 );

    NODE* latestNode = m_currentNode;
    m_lastNode = latestNode->Branch();

    if( reachesEnd && eiDepth >= 0 && aEndItem && latestNode->Depth() > eiDepth && current.SegmentCount() )
    {
        SplitAdjacentSegments( m_lastNode, aEndItem, current.CPoint( -1 ) );

        if( Settings().RemoveLoops() )
            removeLoops( m_lastNode, current );
    }

    updateLeadingRatLine();
    return true;
}


bool LINE_PLACER::FixRoute( const VECTOR2I& aP, ITEM* aEndItem )
{
    bool realEnd = false;
    int lastV;

    LINE pl = Trace();

    if( m_currentMode == RM_MarkObstacles )
    {
        // Mark Obstacles is sort of a half-manual, half-automated mode in which the
        // user has more responsibility and authority.

        if( aEndItem )
        {
            // The user has indicated a connection should be made.  If either the
            // trace or endItem is netless, then allow the connection by adopting the net of the other.
            if( m_currentNet <= 0 )
            {
                m_currentNet = aEndItem->Net();
                pl.SetNet( m_currentNet );
            }
            else if (aEndItem->Net() <= 0 )
                aEndItem->SetNet( m_currentNet );
        }

        // Collisions still prevent fixing unless "Allow DRC violations" is checked
        if( !Settings().CanViolateDRC() && m_world->CheckColliding( &pl ) )
            return false;
    }

    const SHAPE_LINE_CHAIN& l = pl.CLine();

    if( !l.SegmentCount() )
    {
        if( pl.EndsWithVia() )
        {
            m_lastNode->Add( Clone( pl.Via() ) );
            Router()->CommitRouting( m_lastNode );

            m_lastNode = NULL;
            m_currentNode = NULL;

            m_idle = true;
        }

        return true;
    }

    VECTOR2I p_pre_last = l.CPoint( -1 );
    const VECTOR2I p_last = l.CPoint( -1 );
    DIRECTION_45 d_last( l.CSegment( -1 ) );

    if( l.PointCount() > 2 )
        p_pre_last = l.CPoint( -2 );

    if( aEndItem && m_currentNet >= 0 && m_currentNet == aEndItem->Net() )
        realEnd = true;

    if( aForceFinish )
        realEnd = true;

    if( realEnd || m_placingVia )
        lastV = l.SegmentCount();
    else
        lastV = std::max( 1, l.SegmentCount() - 1 );

    SEGMENT* lastSeg = nullptr;

    for( int i = 0; i < lastV; i++ )
    {
        const SEG& s = pl.CSegment( i );
        lastSeg = new SEGMENT( s, m_currentNet );
        std::unique_ptr< SEGMENT > seg( lastSeg );
        seg->SetWidth( pl.Width() );
        seg->SetLayer( m_currentLayer );
        if( ! m_lastNode->Add( std::move( seg ) ) )
        {
            lastSeg = nullptr;
        }
    }

    if( pl.EndsWithVia() )
        m_lastNode->Add( Clone( pl.Via() ) );

    if( realEnd && lastSeg )
        simplifyNewLine( m_lastNode, lastSeg );

    Router()->CommitRouting( m_lastNode );

    m_lastNode = NULL;
    m_currentNode = NULL;

    if( !realEnd )
    {
        setInitialDirection( d_last );
        m_currentStart = m_placingVia ? p_last : p_pre_last;
        m_startItem = NULL;
        m_placingVia = false;
        m_chainedPlacement = !pl.EndsWithVia();
        initPlacement();
    }
    else
    {
        m_idle = true;
    }

    return realEnd;
}


void LINE_PLACER::removeLoops( NODE* aNode, LINE& aLatest )
{
    if( !aLatest.SegmentCount() )
        return;

    if( aLatest.CLine().CPoint( 0 ) == aLatest.CLine().CPoint( -1 ) )
        return;

    std::set<SEGMENT *> toErase;
    aNode->Add( aLatest, true );

    for( int s = 0; s < aLatest.LinkCount(); s++ )
    {
        SEGMENT* seg = aLatest.GetLink(s);
        LINE ourLine = aNode->AssembleLine( seg );
        JOINT a, b;
        std::vector<LINE> lines;

        aNode->FindLineEnds( ourLine, a, b );

        if( a == b )
        {
            aNode->FindLineEnds( aLatest, a, b );
        }

        aNode->FindLinesBetweenJoints( a, b, lines );

        int removedCount = 0;
        int total = 0;

        for( LINE& line : lines )
        {
            total++;

            if( !( line.ContainsSegment( seg ) ) && line.SegmentCount() )
            {
                for( SEGMENT *ss : line.LinkedSegments() )
                    toErase.insert( ss );

                removedCount++;
            }
        }

        wxLogTrace( "PNS", "total segs removed: %d/%d", removedCount, total );
    }

    for( SEGMENT *s : toErase )
        aNode->Remove( s );

    aNode->Remove( aLatest );
}


void LINE_PLACER::simplifyNewLine( NODE* aNode, SEGMENT* aLatest )
{
    LINE l = aNode->AssembleLine( aLatest );
    SHAPE_LINE_CHAIN simplified( l.CLine() );

    simplified.Simplify();

    if( simplified.PointCount() != l.PointCount() )
    {
        aNode->Remove( l );
        l.SetShape( simplified );
        aNode->Add( l );
    }
}


void LINE_PLACER::UpdateSizes( const SIZES_SETTINGS& aSizes )
{
    m_sizes = aSizes;

    if( !m_idle )
    {
        initPlacement();
    }
}


void LINE_PLACER::updateLeadingRatLine()
{
    LINE current = Trace();
    SHAPE_LINE_CHAIN ratLine;
    TOPOLOGY topo( m_lastNode );

    if( topo.LeadingRatLine( &current, ratLine ) )
        Dbg()->AddLine( ratLine, 5, 10000 );
}


void LINE_PLACER::SetOrthoMode( bool aOrthoMode )
{
    m_orthoMode = aOrthoMode;
}


bool LINE_PLACER::buildInitialLine( const VECTOR2I& aP, LINE& aHead, bool aInvertPosture )
{
    SHAPE_LINE_CHAIN l;

    if( m_p_start == aP )
    {
        l.Clear();
    }
    else
    {
        if( Settings().GetFreeAngleMode() && Settings().Mode() == RM_MarkObstacles )
        {
            l = SHAPE_LINE_CHAIN( m_p_start, aP );
        }
        else
        {
            if ( aInvertPosture )
                l = m_direction.Right().BuildInitialTrace( m_p_start, aP );
            else
                l = m_direction.BuildInitialTrace( m_p_start, aP );
        }

        if( l.SegmentCount() > 1 && m_orthoMode )
        {
            VECTOR2I newLast = l.CSegment( 0 ).LineProject( l.CPoint( -1 ) );

            l.Remove( -1, -1 );
            l.Point( 1 ) = newLast;
        }
    }

    aHead.SetShape( l );

    if( !m_placingVia )
        return true;

    VIA v( makeVia( aP ) );
    v.SetNet( aHead.Net() );

    if( m_currentMode == RM_MarkObstacles )
    {
        aHead.AppendVia( v );
        return true;
    }

    VECTOR2I force;
    VECTOR2I lead = aP - m_p_start;

    bool solidsOnly = ( m_currentMode != RM_Walkaround );

    if( v.PushoutForce( m_currentNode, lead, force, solidsOnly, 40 ) )
    {
        SHAPE_LINE_CHAIN line = m_direction.BuildInitialTrace( m_p_start, aP + force );
        aHead = LINE( aHead, line );

        v.SetPos( v.Pos() + force );
        return true;
    }

    return false; // via placement unsuccessful
}


void LINE_PLACER::GetModifiedNets( std::vector<int>& aNets ) const
{
    aNets.push_back( m_currentNet );
}

LOGGER* LINE_PLACER::Logger()
{
    if( m_shove )
        return m_shove->Logger();

    return NULL;
}

}
