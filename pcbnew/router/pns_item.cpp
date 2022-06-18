/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2013-2014 CERN
 * Copyright (C) 2016-2020 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <zone.h>
#include "pns_node.h"
#include "pns_item.h"
#include "pns_line.h"
#include "pns_router.h"
#include "pns_hull.h"

typedef VECTOR2I::extended_type ecoord;

namespace PNS {

bool ITEM::collideSimple( const ITEM* aOther, const NODE* aNode,  const COLLISION_SEARCH_OPTIONS& aOpts, OBSTACLE *aObsInfo ) const
{
    const ROUTER_IFACE* iface = ROUTER::GetInstance()->GetInterface();
    const SHAPE*        shapeA = Shape();
    const SHAPE*        holeA = Hole();
    int                 lineWidthA = 0;
    const SHAPE*        shapeB = aOther->Shape();
    const SHAPE*        holeB = aOther->Hole();
    int                 lineWidthB = 0;
    const int           clearanceEpsilon = aNode->GetRuleResolver()->ClearanceEpsilon();

    // Sadly collision routines ignore SHAPE_POLY_LINE widths so we have to pass them in as part
    // of the clearance value.
    if( m_kind == LINE_T )
        lineWidthA = static_cast<const LINE*>( this )->Width() / 2;

    if( aOther->m_kind == LINE_T )
        lineWidthB = static_cast<const LINE*>( aOther )->Width() / 2;

    // same nets? no collision!
    if( aOpts.m_differentNetsOnly && m_net == aOther->m_net && m_net >= 0 && aOther->m_net >= 0 )
        return false;

    // check if we are not on completely different layers first
    if( !m_layers.Overlaps( aOther->m_layers ) )
        return false;

    auto checkKeepout =
            []( const ZONE* aKeepout, const BOARD_ITEM* aOther )
            {
                constexpr KICAD_T TRACK_TYPES[] = { PCB_ARC_T, PCB_TRACE_T, EOT };

                if( aKeepout->GetDoNotAllowTracks() && aOther->IsType( TRACK_TYPES ) )
                    return true;

                if( aKeepout->GetDoNotAllowVias() && aOther->Type() == PCB_VIA_T )
                    return true;

                if( aKeepout->GetDoNotAllowPads() && aOther->Type() == PCB_PAD_T )
                    return true;

                // Incomplete test, but better than nothing:
                if( aKeepout->GetDoNotAllowFootprints() && aOther->Type() == PCB_PAD_T )
                {
                    return !aKeepout->GetParentFootprint()
                            || aKeepout->GetParentFootprint() != aOther->GetParentFootprint();
                }

                return false;
            };

    const ZONE* zoneA = dynamic_cast<ZONE*>( Parent() );
    const ZONE* zoneB = dynamic_cast<ZONE*>( aOther->Parent() );

    if( zoneA && aOther->Parent() && !checkKeepout( zoneA, aOther->Parent() ) )
        return false;

    if( zoneB && Parent() && !checkKeepout( zoneB, Parent() ) )
        return false;

    bool thisNotFlashed  = !iface->IsFlashedOnLayer( this, aOther->Layer() );
    bool otherNotFlashed = !iface->IsFlashedOnLayer( aOther, Layer() );

    if( aObsInfo )
    {
        aObsInfo->m_headIsHole = false;
        aObsInfo->m_itemIsHole = false;
    }

    if( ( aNode->GetCollisionQueryScope() == NODE::CQS_ALL_RULES
          || ( thisNotFlashed || otherNotFlashed ) )
        && ( holeA || holeB ) )
    {
        int holeClearance = aNode->GetHoleClearance( this, aOther );

        if( holeA && holeA->Collide( shapeB, holeClearance + lineWidthB - clearanceEpsilon ) )
        {
            if( aObsInfo )
            {
                aObsInfo->m_headIsHole = true;
                aObsInfo->m_clearance = holeClearance;
            }
            return true;
        }

        if( holeB && holeB->Collide( shapeA, holeClearance + lineWidthA - clearanceEpsilon ) )
        {
            if( aObsInfo )
            {
                aObsInfo->m_itemIsHole = true;
                aObsInfo->m_clearance = holeClearance;
            }
            return true;
        }

        if( holeA && holeB )
        {
            int holeToHoleClearance = aNode->GetHoleToHoleClearance( this, aOther );

            if( holeA->Collide( holeB, holeToHoleClearance - clearanceEpsilon ) )
            {
                if( aObsInfo )
                {
                    aObsInfo->m_headIsHole = true;
                    aObsInfo->m_itemIsHole = true;
                    aObsInfo->m_clearance = holeToHoleClearance;
                }
                return true;
            }
        }
    }

    if( !aOther->Layers().IsMultilayer() && thisNotFlashed )
        return false;

    if( !Layers().IsMultilayer() && otherNotFlashed )
        return false;

    int clearance;

    if( aOpts.m_overrideClearance )
    {
        clearance = aOpts.m_overrideClearance;
    }
    else
    {
        clearance = aNode->GetClearance( this, aOther );
    }

    bool isColliding = shapeA->Collide( shapeB, clearance + lineWidthA + lineWidthB - clearanceEpsilon );

    if( isColliding && aObsInfo )
    {
        aObsInfo->m_clearance = clearance;
    }

    return isColliding;
}


bool ITEM::Collide( const ITEM* aOther, const NODE* aNode, const COLLISION_SEARCH_OPTIONS& aOpts, OBSTACLE *aObsInfo ) const
{
    if( collideSimple( aOther, aNode, aOpts, aObsInfo ) )
        return true;

    // Special cases for "head" lines with vias attached at the end.  Note that this does not
    // support head-line-via to head-line-via collisions, but you can't route two independent
    // tracks at once so it shouldn't come up.

    if( m_kind == LINE_T )
    {
        const LINE* line = static_cast<const LINE*>( this );

        if( line->EndsWithVia() && line->Via().collideSimple( aOther, aNode, aOpts, aObsInfo ) )
            return true;
    }

    if( aOther->m_kind == LINE_T )
    {
        const LINE* line = static_cast<const LINE*>( aOther );

        if( line->EndsWithVia() && line->Via().collideSimple( this, aNode, aOpts, aObsInfo ) )
            return true;
    }

    return false;
}


std::string ITEM::KindStr() const
{
    switch( m_kind )
    {
    case ARC_T:       return "arc";
    case LINE_T:      return "line";
    case SEGMENT_T:   return "segment";
    case VIA_T:       return "via";
    case JOINT_T:     return "joint";
    case SOLID_T:     return "solid";
    case DIFF_PAIR_T: return "diff-pair";
    default:          return "unknown";
    }
}


ITEM::~ITEM()
{
}


const SHAPE_LINE_CHAIN HULL::Shape() const
{
    const ITEM* item = m_offendingObstacle.m_item; // item found to be colliding against head
    const ITEM* head = m_offendingObstacle.m_head; // item searched for collisions (either by calling QueryColliding(head) or NearestObstacle(head) )
    NODE *node =  item->Owner();
    int layer = head->Layer();
    const int clearanceEpsilon = node->GetRuleResolver()->ClearanceEpsilon();
    SHAPE_LINE_CHAIN obstacleHull;
    int clearance = 0;

    if ( m_useClearanceEpsilon )
        clearance -= clearanceEpsilon;

    if( ( auto seg = dyn_cast<SEGMENT*>( head ) ) && item->OfKind( ITEM::SEGMENT_T ) ) // segment to segment collision
    {
        clearance += node->GetClearance( item, head );
        obstacleHull = item->Hull( clearance, seg->GetWidth() );
    }


    
    // check for if the head item is a via. if so, we need to generate the hull against
    // the via's antipad or the via's hole depending on 
    if( auto via = dyn_cast<VIA*>( head ) &&  )
    {
        // Don't use via.Drill(); it doesn't include the plating thickness

        int viaHoleRadius = static_cast<const SHAPE_CIRCLE*>( via.Hole() )->GetRadius();
        int viaClearance = node->GetClearance( item, &via );
                               + via.Diameter() / 2;
            int holeClearance = node->GetHoleClearance( item, &via ) + viaHoleRadius;

            if ( m_useClearanceEpsilon )
            {
                viaClearance -= clearanceEpsilon;
                holeClearance -= clearanceEpsilon;
            }

            if( holeClearance > viaClearance )
                viaClearance = holeClearance;

                int width = m_diameter;

                        width = m_hole.GetRadius() * 2;
    }

         

            if node->Get->IsFlashedOnLayer( &via,  ) )
        width = m_hole.GetRadius() * 2;

            auto obstacleHull = item->Hull( viaClearance, 0 );
            //debugDecorator->AddLine( obstacleHull, 3 );

            intersectingPts.clear();
            HullIntersection( obstacleHull, aLine->CLine(), intersectingPts );

            // obstacleHull.Intersect( aLine->CLine(), intersectingPts, true );

            for( const SHAPE_LINE_CHAIN::INTERSECTION& ip : intersectingPts )
                updateNearest( ip, obstacle.m_item, obstacleHull, false );
        }



}


}
