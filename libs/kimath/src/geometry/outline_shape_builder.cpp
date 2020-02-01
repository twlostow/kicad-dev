/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 CERN
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
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


#include <geometry/outline_shape_builder.h>

#include <geometry/shape_segment.h>
#include <geometry/shape_arc.h>

void OUTLINE_SHAPE_BUILDER::constructAngledSegs( bool startDiagonal,
        bool is45degree,
        VECTOR2I& a,
        VECTOR2I& b,
        int offset )
{
    int w   = std::abs( m_end.x - m_start.x );
    int h   = std::abs( m_end.y - m_start.y );
    int sw  = sign( m_end.x - m_start.x );
    int sh  = sign( m_end.y - m_start.y );

    VECTOR2I mp0, mp1;

    // we are more horizontal than vertical?
    if( is45degree )
    {
        if( w > h )
        {
            mp0 = VECTOR2I( ( w - h ) * sw, 0 );        // direction: E
            mp1 = VECTOR2I( h * sw, h * sh );           // direction: NE
        }
        else
        {
            mp0 = VECTOR2I( 0, sh * ( h - w ) );        // direction: N
            mp1 = VECTOR2I( sw * w, sh * w );           // direction: NE
        }
    }
    else                                // 90 degrees regime
    {
        mp0 = VECTOR2I( w * sw, 0 );    // direction: E
        mp1 = VECTOR2I( 0, h * sh );    // direction: S
    }

    auto    mp  = startDiagonal ? mp1 : mp0;
    auto    mid = m_start + mp;

    int l1  = mp.EuclideanNorm();
    int l2  = (mid - m_end).EuclideanNorm();

    int l = std::min( l1, std::min( l2, offset ) );

    a   = m_start + mp - mp.Resize( l );
    b   = m_start + mp - ( mid - m_end ).Resize( l );
}

bool OUTLINE_SHAPE_BUILDER::ConstructAndAppend( OUTLINE_SHAPE& aShape )
{
    OUTLINE_SHAPE::OUTLINE_ELEMENT elt;

    elt.isDiagonal = m_diagonal;
    elt.type = m_shapeType;

    std::vector<SHAPE*> shapes;
    auto rv = Construct( shapes );

    for ( auto& s : shapes ) 
        elt.shapes.push_back( s );
    
    aShape.AppendElement( elt );
    return rv;
}


bool OUTLINE_SHAPE_BUILDER::Construct( std::vector<SHAPE*>& aShape )
{
    VECTOR2I d = m_end - m_start;

    if( d.x == 0 && d.y == 0 )
        return false;


    switch( m_shapeType )
    {
    case SHT_LINE:
        aShape.push_back( new SHAPE_SEGMENT( m_start, m_end ) );
        return true;

    case SHT_CORNER_90:
    {
        if( d.x == 0 || d.y == 0 )
        {
            aShape.push_back( new SHAPE_SEGMENT( m_start, m_end ) );
        }
        else
        {
            VECTOR2I p;

            if( m_diagonal )
                p = VECTOR2I( m_end.x, m_start.y );
            else
                p = VECTOR2I( m_start.x, m_end.y );

            aShape.push_back( new SHAPE_SEGMENT( m_start, p ) );
            aShape.push_back( new SHAPE_SEGMENT( p, m_end ) );
        }

        return true;
    }

    case SHT_CORNER_45:
    {
        VECTOR2I p0, p1;


        if( std::abs( d.x ) == std::abs( d.y ) )
        {
            aShape.push_back( new SHAPE_SEGMENT( m_start, m_end ) );
        }
        else
        {
            constructAngledSegs( m_diagonal, true, p0, p1, 0 );

            aShape.push_back( new SHAPE_SEGMENT( m_start, p0 ) );
            aShape.push_back( new SHAPE_SEGMENT( p1, m_end ) );
        }

        return true;
    }

    case SHT_CORNER_ARC_45:
    case SHT_CORNER_ARC_90:
    {
        VECTOR2I p0, p1, midpoint;
        bool is45 = ( m_shapeType == SHT_CORNER_ARC_45 );


        if( m_shapeType == SHT_CORNER_ARC_45 && std::abs( d.x ) == std::abs( d.y ) )
        {
            aShape.push_back( new SHAPE_SEGMENT( m_start, m_end ) );
        }
        else if( m_shapeType == SHT_CORNER_ARC_90 && ( d.x == 0 || d.y == 0 ) )
        {
            aShape.push_back( new SHAPE_SEGMENT( m_start, m_end ) );
        }
        else
        {
            constructAngledSegs( m_diagonal, is45, p0, p1, m_arcRadius );
            constructAngledSegs( m_diagonal, is45, midpoint, midpoint, 0 );

            aShape.push_back( new SHAPE_SEGMENT( m_start, p0 ) );

            auto arc = new SHAPE_ARC;
            arc->ConstructFromCorners( p0, p1, is45 ? 45.0 : 90.0 );

            SEG chord = arc->GetChord();

            auto    side_a  = chord.Side( midpoint );
            auto    side_b  = chord.Side( arc->GetCenter() );

            if( side_a == side_b )
                arc->ConstructFromCorners( p1, p0, is45 ? 45.0 : 90.0  );

            aShape.push_back( arc );
            aShape.push_back( new SHAPE_SEGMENT( p1, m_end ) );

        }

        return true;
    }

    default:
        return false;
    }

    return false;
}


bool OUTLINE_SHAPE_BUILDER::Construct( SHAPE_LINE_CHAIN& aShape )
{
    std::vector<SHAPE*> shapes;


    if( !Construct( shapes ) )
        return false;

    aShape.Clear();

    for( auto sh : shapes )
    {
        aShape.Append( sh->ConvertToPolyline( ) );
        delete sh;
    }

    aShape.Simplify();

    return true;
}

void OUTLINE_SHAPE_BUILDER::NextShapeType()
{
    for (;;)
    {
        m_shapeType = (OUTLINE_SHAPE_TYPE) ( (int) (m_shapeType) + 1 );

        if( m_shapeType == SHT_LAST )
            m_shapeType = SHT_LINE;

        for ( auto s : m_allowedShapeTypes )
            if ( s == m_shapeType )
                return;
    }
}

bool OUTLINE_SHAPE_BUILDER::IsShapeTypeAllowed( OUTLINE_SHAPE_TYPE aType ) const
{
    for ( auto s : m_allowedShapeTypes )
        if ( s == aType )
            return true;

    return false;
}

OUTLINE_SHAPE::OUTLINE_SHAPE()
{

}


OUTLINE_SHAPE::~OUTLINE_SHAPE()
{

}


void OUTLINE_SHAPE::Simplify()
{
    std::vector<SHAPE*> result;
    SHAPE_LINE_CHAIN lc;
    boost::optional<int> lastWidth;

    for( auto& elt : m_outline )
    {
        for( auto& s : elt.shapes )
        {
            switch(s->Type() )
            {
                case SH_ARC:
                    lc.Simplify();
                   // convertLCToSegments( lc, result );
                    break;
                case SH_SEGMENT:
                {
                 //   if( lastWidth && *lastWidth == s->)
                }

            }
        }
    }

}

void OUTLINE_SHAPE::DeleteLastElement()
{
    if ( m_outline.size() > 0)
        m_outline.erase( m_outline.begin() + m_outline.size() - 1 );
}

void OUTLINE_SHAPE::SetInitialPoint( const VECTOR2I& aInitialPoint )
{
    m_initialPoint = aInitialPoint;
}
    
int OUTLINE_SHAPE::GetShapeCount()
{
    return m_outline.size();
}

void OUTLINE_SHAPE::ConvertToPrimitiveShapes( std::vector<SHAPE*>& aShapes )
{
    for ( auto& elt : m_outline )
    {
        for ( auto& shape : elt.shapes )
        {
            aShapes.push_back( shape );
        }
    }
}
        
void OUTLINE_SHAPE::AppendElement( const OUTLINE_SHAPE::OUTLINE_ELEMENT& aElt )
{
    if ( !m_initialPoint && m_outline.size() > 0 && aElt.shapes.size() > 0 )
    {
        auto& lastShape = aElt.shapes.back();
        
        if ( lastShape->Type() == SH_ARC )
            m_initialPoint = static_cast<SHAPE_ARC*>(lastShape)->GetP0();
        else if ( lastShape->Type() == SH_SEGMENT )
            m_initialPoint = static_cast<SHAPE_SEGMENT*>(lastShape)->GetSeg().A;
    }

    m_outline.push_back( aElt );

    printf("append\n");
    int i = 0;

    for( auto s : m_outline )
    {
        printf("Elt %d shapes: %d\n", i, s.shapes.size() );
        for( auto s2 : s.shapes )
        {
            if ( s2->Type() == SH_SEGMENT )
            {
                auto seg = static_cast<SHAPE_SEGMENT*>(s2);
                printf(" - seg %d %d - %d %d\n", seg->GetSeg().A.x,seg->GetSeg().A.y,seg->GetSeg().B.x,seg->GetSeg().B.y );
            }
            
        i++;
        }
    }

}

void OUTLINE_SHAPE::Clear()
{
    m_outline.clear();
}

const VECTOR2I OUTLINE_SHAPE::GetLastPoint() const
{
    int i = 0;

    
    if ( !m_outline.size() && m_initialPoint )
        return *m_initialPoint;
    else {
        auto i = m_outline.end();
        
        do
        {
            --i;

            if( i->shapes.size() == 0 )
                continue;

            printf("lastShape %d\n", i->shapes.size() );

            auto& lastShape = i->shapes.back();


        
            if ( lastShape->Type() == SH_ARC )
                return static_cast<SHAPE_ARC*>(lastShape)->GetP1();
            else if ( lastShape->Type() == SH_SEGMENT )
                return static_cast<SHAPE_SEGMENT*>(lastShape)->GetSeg().B;
            
        } while ( i != m_outline.begin() );
    }
        
    return VECTOR2I(0, 0);
}