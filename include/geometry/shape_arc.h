/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
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

#ifndef __SHAPE_ARC_H
#define __SHAPE_ARC_H

#include <geometry/shape.h>
#include <geometry/seg.h>

const SHAPE_TYPE SH_ARC = (SHAPE_TYPE) 100;

class SHAPE_ARC : public SHAPE {

public:
    SHAPE_ARC():
        SHAPE( SH_ARC ), m_width( 0 ) {};

    SHAPE_ARC( const VECTOR2I& pa, const VECTOR2I& pb, const VECTOR2I& pCenter, int aWidth = 0 ):
        SHAPE( SH_ARC ), m_p0( pa ), m_p1( pb ), m_pc ( pCenter ), m_width( aWidth ) {};

        SHAPE_ARC( const SHAPE_ARC& aOther )
            : SHAPE( SH_ARC )
        {
            m_p0 = aOther.m_p0;
            m_p1 = aOther.m_p1;
            m_pc = aOther.m_pc;
        }

    ~SHAPE_ARC() {};

    SHAPE* Clone() const override
    {
        return new SHAPE_ARC( *this );
    }

    const BOX2I BBox( int aClearance = 0 ) const override
    {
        //return BOX2I( m_seg.A, m_seg.B - m_seg.A ).Inflate( aClearance + ( m_width + 1 ) / 2 );
    }

    bool Collide( const SEG& aSeg, int aClearance = 0 ) const override;
    bool Collide( const VECTOR2I& aP, int aClearance = 0 ) const override;

    void SetWidth( int aWidth )
    {
        m_width = aWidth;
    }

    int GetWidth() const
    {
        return m_width;
    }

    bool IsSolid() const override
    {
        return true;
    }

    void Move( const VECTOR2I& aVector ) override
    {
        m_p0 += aVector;
        m_p1 += aVector;
        m_pc += aVector;
    }

    bool ConstructFromCorners( VECTOR2I aP0, VECTOR2I aP1, double aCenterAngle );


    bool ConstructFromCornerAndAngles( VECTOR2I aP0,
            double aStartAngle,
            double aCenterAngle,
            double aRadius );

private:
    VECTOR2I m_p0, m_p1, m_pc;
    int m_width;
};

#endif
