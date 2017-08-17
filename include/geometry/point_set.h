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


#ifndef __POINT_SET_H
#define __POINT_SET_H

#include <vector>
#include <algorithm>

#include <math/vector2d.h>

template <class T>
class POINT_SET
{
public:
    template <class T2>
    static const VECTOR2I& PositionGetter( const T2& aItem )
    {
        return aItem->GetPos();
    }

public:

    POINT_SET() {};
    POINT_SET( const std::vector<T>& aPoints ) :
        m_points( aPoints )
    {
    }

    ~POINT_SET() {};

    void Reserve( int size )
    {
        m_points.reserve( size );
    }

    void Add( const T& p )
    {
        m_points.push_back( p );
    }

    void Clear()
    {
        m_points.clear();
    }

    template <class Func>
    void Find( VECTOR2I aPosition, int aDistMax, Func aFunc  )
    {
        /* Search items in m_Candidates that position is <= aDistMax from aPosition
         * (Rectilinear distance)
         * m_Candidates is sorted by X then Y values, so a fast binary search is used
         * to locate the "best" entry point in list
         * The best entry is a pad having its m_Pos.x == (or near) aPosition.x
         * All candidates are near this candidate in list
         * So from this entry point, a linear search is made to find all candidates
         */

        int idxmax = m_points.size() - 1;

        int delta = idxmax + 1;
        int idx = 0;        // Starting index is the beginning of list

        while( delta )
        {
            // Calculate half size of remaining interval to test.
            // Ensure the computed value is not truncated (too small)
            if( ( delta & 1 ) && ( delta > 1 ) )
                delta++;

            delta /= 2;

            const auto& p = PositionGetter( m_points[idx] );

            int dist = p.x - aPosition.x;

            if( std::abs( dist ) <= aDistMax )
            {
                break;                          // A good entry point is found. The list can be scanned from this point.
            }
            else if( p.x < aPosition.x )        // We should search after this point
            {
                idx += delta;

                if( idx > idxmax )
                    idx = idxmax;
            }
            else    // We should search before this p
            {
                idx -= delta;

                if( idx < 0 )
                    idx = 0;
            }
        }

        /* Now explore the candidate list from the "best" entry point found
         * (candidate "near" aPosition.x)
         * We exp the list until abs(candidate->m_Point.x - aPosition.x) > aDistMashar* Currently a linear search is made because the number of candidates
         * having the right X position is usually small
         */
        // search next candidates in list
        VECTOR2I diff;

        for( int ii = idx; ii <= idxmax; ii++ )
        {
            const auto& p = PositionGetter( m_points[ii] );
            diff = p - aPosition;;

            if( std::abs( diff.x ) > aDistMax )
                break; // Exit: the distance is to long, we cannot find other candidates

            if( std::abs( diff.y ) > aDistMax )
                continue; // the y distance is to long, but we can find other candidates

            // We have here a good candidate: add it
            aFunc( m_points[ii] );
        }

        // search previous candidates in list
        for( int ii = idx - 1; ii >=0; ii-- )
        {
            auto& p = PositionGetter( m_points[ii] );
            diff = p - aPosition;

            if( abs( diff.x ) > aDistMax )
                break;

            if( abs( diff.y ) > aDistMax )
                continue;

            // We have here a good candidate:add it
            aFunc( m_points[ii] );
        }
    }

    void Sort()
    {
        auto sortFunctor = [] ( const T& a, const T& b )
                           {
                               const auto   posA    = PositionGetter( a );
                               const auto   posB    = PositionGetter( b );

                               if( posA.x == posB.x )
                                   return posA.y < posB.y;
                               else
                                   return posA.x < posB.x;
                           };

        std::sort( m_points.begin(), m_points.end(), sortFunctor );
    }

private:
    std::vector<T> m_points;
};

#endif
