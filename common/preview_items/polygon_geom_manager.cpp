/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2017 Kicad Developers, see AUTHORS.txt for contributors.
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

#include <preview_items/polygon_geom_manager.h>

#include <geometry/outline_shape_builder.h>

POLYGON_GEOM_MANAGER::POLYGON_GEOM_MANAGER( CLIENT& aClient ):
    m_client( aClient )
{
    m_shapeBuilder.reset ( new OUTLINE_SHAPE_BUILDER );
    m_shapeBuilder->SetShapeType ( OUTLINE_SHAPE_BUILDER::SHT_CORNER_45 );
}

void POLYGON_GEOM_MANAGER::AddPoint( const VECTOR2I& aPt )
{
    // if this is the first point, make sure the client is happy
    // for us to continue
    if( !IsPolygonInProgress() && !m_client.OnFirstPoint() )
        return;

    if ( m_lockedPoints.PointCount() == 0 )
    {
        m_lockedPoints.Append ( aPt );
    }
    else
    {

        updateLeaderPoints( aPt );

        if( m_leaderPts.PointCount() > 1 )
        {
            // there are enough leader points - the next
            // locked-in point is the end of the first leader
            // segment
            m_lockedPoints.Append( m_leaderPts );
        }
    }

    m_client.OnGeometryChange( *this );
}


void POLYGON_GEOM_MANAGER::SetFinished()
{
    m_client.OnComplete( *this );
}


void POLYGON_GEOM_MANAGER::SetLeaderMode( LEADER_MODE aMode )
{

}


void POLYGON_GEOM_MANAGER::SetCursorPosition( const VECTOR2I& aPos )
{
    updateLeaderPoints( aPos );
}


bool POLYGON_GEOM_MANAGER::IsPolygonInProgress() const
{
    return m_lockedPoints.PointCount() > 0;
}


bool POLYGON_GEOM_MANAGER::NewPointClosesOutline( const VECTOR2I& aPt ) const
{
    return m_lockedPoints.PointCount() && m_lockedPoints.CPoint( 0 ) == aPt;
}


void POLYGON_GEOM_MANAGER::DeleteLastCorner()
{
    if( m_lockedPoints.PointCount() > 0 )
    {
        m_lockedPoints.Remove( -1 );
    }

    // update the new last segment (was previously
    // locked in), reusing last constraints
    if( m_lockedPoints.PointCount() > 0 )
    {
        updateLeaderPoints( m_leaderPts.CPoint( -1 ) );
    }

    m_client.OnGeometryChange( *this );
}


void POLYGON_GEOM_MANAGER::Reset()
{
    m_lockedPoints.Clear();
    m_leaderPts.Clear();

    m_client.OnGeometryChange( *this );
}


void POLYGON_GEOM_MANAGER::updateLeaderPoints( const VECTOR2I& aEndPoint )
{
    SHAPE_LINE_CHAIN newChain;

    if ( m_lockedPoints.PointCount() > 0 )
    {
        m_shapeBuilder->SetStart( m_lockedPoints.CPoint( -1 ) );
        m_shapeBuilder->SetEnd( aEndPoint );
        m_shapeBuilder->Construct( m_leaderPts );
    }
    else
    {
        m_leaderPts.Append ( aEndPoint );
    }

    m_client.OnGeometryChange( *this );
}


const SHAPE_LINE_CHAIN& POLYGON_GEOM_MANAGER::GetLockedInPoints() const
{
    return m_lockedPoints;
}


const SHAPE_LINE_CHAIN& POLYGON_GEOM_MANAGER::GetLeaderLinePoints() const
{
    return m_leaderPts;
}

OUTLINE_SHAPE_BUILDER* POLYGON_GEOM_MANAGER::GetOutlineBuilder() const
{
    return m_shapeBuilder.get();
}
