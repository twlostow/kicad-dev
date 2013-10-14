/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
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

#ifndef __SHAPE_RECT_H
#define __SHAPE_RECT_H

#include <geometry/shape.h>
#include <geometry/shape_line_chain.h>
#include <geometry/shape_circle.h>
#include <geometry/seg.h>

class SHAPE_RECT : public SHAPE {
    public:
        /**
         * Constructor
         * Creates an empty (0-sized) rectangle
         */
        SHAPE_RECT() :
            SHAPE( SH_RECT ), m_w( 0 ), m_h( 0 ) {};

        /**
         * Constructor
         * Creates a rectangle defined by top-left corner (aX0, aY0), width aW and height aH.
         */
        SHAPE_RECT( int aX0, int aY0, int aW, int aH ) :
            SHAPE( SH_RECT ), m_p0( aX0, aY0 ), m_w( aW ), m_h( aH ) {};

        /**
         * Constructor
         * Creates a rectangle defined by top-left corner aP0, width aW and height aH.
         */
         SHAPE_RECT( const VECTOR2I &aP0, int aW, int aH ) :
            SHAPE( SH_RECT ), m_p0( aP0 ), m_w( aW ), m_h( aH ) {};

        /// @copydoc SHAPE::BBox()
        const BOX2I BBox(int aClearance = 0) const
        {
            BOX2I bbox( VECTOR2I( m_p0.x - aClearance, m_p0.y - aClearance ),
                          VECTOR2I( m_w + 2 * aClearance, m_h + 2 * aClearance ) );
            //printf("bb : %s\n",bbox.Format().c_str());
            return bbox;
        }

        /**
         * Function Diagonal()
         *
         * Returns length of the diagonal of the rectangle
         * @return diagonal length
         */
        int Diagonal() const
        {
            return VECTOR2I( m_w, m_h ).EuclideanNorm();
        }

        /// @copydoc SHAPE::Collide()
        bool Collide( const SEG& aSeg, int aClearance = 0 ) const
        {
            //VECTOR2I pmin = VECTOR2I(std::min(aSeg.a.x, aSeg.b.x), std::min(aSeg.a.y, aSeg.b.y));
            //VECTOR2I pmax = VECTOR2I(std::max(aSeg.a.x, aSeg.b.x), std::max(aSeg.a.y, aSeg.b.y));
            //BOX2I r(pmin, VECTOR2I(pmax.x - pmin.x, pmax.y - pmin.y));

            //if (BBox(0).SquaredDistance(r) > aClearance * aClearance)
            //    return false;

            if( BBox( 0 ).Contains( aSeg.a ) || BBox( 0 ).Contains( aSeg.b ) )
                return true;

             VECTOR2I vts[] = {     VECTOR2I( m_p0.x, m_p0.y ),
                                VECTOR2I( m_p0.x, m_p0.y + m_h ),
                                VECTOR2I( m_p0.x + m_w, m_p0.y + m_h ),
                                VECTOR2I( m_p0.x + m_w, m_p0.y ),
                                VECTOR2I( m_p0.x, m_p0.y ) };

            for( int i = 0; i < 4; i++ )
            {
                SEG s( vts[i], vts[i + 1], i );
                if( s.Distance( aSeg ) <= aClearance )
                    return true;
            }

            return false;
        };

        /**
         * Function GetPosition()
         *
         * @return top-left corner of the rectangle
         */
        const VECTOR2I& GetPosition() const { return m_p0; }

        /**
         * Function GetSize()
         *
         * @return size of the rectangle
         */
        const VECTOR2I GetSize() const { return VECTOR2I( m_w, m_h ); }

        /**
         * Function GetWidth()
         *
         * @return width of the rectangle
         */
         const int GetWidth() const { return m_w; }

        /**
         * Function GetHeight()
         *
         * @return height of the rectangle
         */
        const int GetHeight() const { return m_h; }

    private:
        ///> Top-left corner
        VECTOR2I m_p0;

        ///> Width
        int m_w;

        ///> Height
        int m_h;
    };

#endif // __SHAPE_RECT_H
