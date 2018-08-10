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


#ifndef __OUTLINE_SHAPE_BUILDER_H
#define __OUTLINE_SHAPE_BUILDER_H

#include <geometry/shape.h>
#include <geometry/shape_line_chain.h>

class DRAWSEGMENT;

enum OUTLINE_SHAPE_TYPE
    {
        SHT_LINE = 0,
        SHT_CORNER_45,
        SHT_CORNER_90,
        SHT_CORNER_ARC_45,
        SHT_CORNER_ARC_90,
        SHT_LAST
    };


class OUTLINE_SHAPE 
{
    public:
        struct OUTLINE_ELEMENT 
        {
            std::vector<SHAPE*> shapes;
            bool isDiagonal;
            OUTLINE_SHAPE_TYPE type;
        };


        OUTLINE_SHAPE();
        ~OUTLINE_SHAPE();


        void Clear();
        void Simplify();
        void DeleteLastElement();
        void SetInitialPoint( const VECTOR2I& aInitialPoint );
        int GetShapeCount();

        void ConvertToPrimitiveShapes( std::vector<SHAPE*>& aShapes );
        void AppendElement( const OUTLINE_ELEMENT& aElt );

        const std::vector<OUTLINE_ELEMENT>& GetElements() const { return m_outline; }
        const VECTOR2I GetLastPoint() const;

    private:

        boost::optional<VECTOR2I> m_initialPoint;
        std::vector<OUTLINE_ELEMENT> m_outline;
};

/**
 * Class OUTLINE_SHAPE_BUILDER
 *
 * Constructs two-segment trace/outline shapes between two defined points.
 */
class OUTLINE_SHAPE_BUILDER
{
public:

    
    OUTLINE_SHAPE_BUILDER()
    {
        m_allowedShapeTypes = { SHT_LINE, SHT_CORNER_90, SHT_CORNER_45, SHT_CORNER_ARC_90, SHT_CORNER_ARC_45 };
        m_shapeType = SHT_LINE;
    };

    ~OUTLINE_SHAPE_BUILDER() {};

    void SetArcRadius( int aRadius )
    {
        m_arcRadius = aRadius;
    }

    int GetArcRadius() const
    {
        return m_arcRadius;
    }

    void SetShapeType( OUTLINE_SHAPE_TYPE aType )
    {
        m_shapeType = aType;
    }

    OUTLINE_SHAPE_TYPE GetShapeType() const
    {
        return m_shapeType;
    }

    void SetAllowedShapeTypes( std::vector<OUTLINE_SHAPE_TYPE> aAllowedTypes )
    {
        m_allowedShapeTypes = aAllowedTypes;
    }

    bool IsShapeTypeAllowed( OUTLINE_SHAPE_TYPE aType ) const;

    void NextShapeType();

    void SetStart( const VECTOR2I& aStart )
    {
        m_start = aStart;
    }

    void SetEnd( const VECTOR2I& aEnd )
    {
        m_end = aEnd;
    }

    void SetArcApproximationFactor( double aFactor )
    {
        m_arcApproxFactor = aFactor;
    }

    const VECTOR2I& GetStart() const
    {
        return m_start;
    }

    const VECTOR2I& GetEnd() const
    {
        return m_end;
    }

    bool IsDiagonal() const
    {
        return m_diagonal;
    }

    void SetDiagonal( bool aDiagonal )
    {
        m_diagonal = aDiagonal;
    }

    void FlipPosture()
    {
        m_diagonal = !m_diagonal;
    }

    bool    Construct( std::vector<SHAPE*>& aShape );
    bool    Construct( SHAPE_LINE_CHAIN& aShape );
    bool    ConstructAndAppend( OUTLINE_SHAPE& aShape );

private:

    void constructAngledSegs( bool startDiagonal,
            bool is45degree,
            VECTOR2I& a,
            VECTOR2I& b,
            int offset = 0 );

    OUTLINE_SHAPE_TYPE m_shapeType = SHT_LINE;
    int m_arcRadius = 2000000;
    bool m_diagonal = false;
    double m_arcApproxFactor = 0.01;
    VECTOR2I m_start, m_end;
    std::vector<OUTLINE_SHAPE_TYPE> m_allowedShapeTypes;
};

#endif
