/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 CERN
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

#ifndef __GRID_HELPER_H
#define __GRID_HELPER_H

#include <vector>

#include <math/vector2d.h>
#include <core/optional.h>
#include <origin_viewitem.h>
#include <anchor.h>

#include <layers_id_colors_and_visibility.h>

#include <geometry/seg.h>

class PCB_BASE_FRAME;


class GRID_HELPER {
private:

    class PREVIEW;

public:



    enum SNAP_MODE
    {
        SM_DRAW = 1,
        SM_CONSTRAIN = 2,
        SM_CURRENT_LAYER = 4
    };

    GRID_HELPER( PCB_BASE_FRAME* aFrame );
    ~GRID_HELPER();

    void SetGrid( int aSize );
    void SetOrigin( const VECTOR2I& aOrigin );

    VECTOR2I GetGrid() const;
    VECTOR2I GetOrigin() const;

    void SetAuxAxes( bool aEnable, const VECTOR2I& aOrigin = VECTOR2I( 0, 0 ), bool aEnableDiagonal = false );

    VECTOR2I Align( const VECTOR2I& aPoint ) const;
    VECTOR2I AlignToSegment ( const VECTOR2I& aPoint, const SEG& aSeg );
    VECTOR2I BestDragOrigin( const VECTOR2I& aMousePos, BOARD_ITEM* aItem );
    VECTOR2I BestSnapAnchor( const VECTOR2I& aOrigin, BOARD_ITEM* aDraggedItem );


    void SetSnapMode( int aMode = SM_DRAW );
    void Update( );
    const VECTOR2I Snap() const;

    void AddAuxItems( const std::vector<BOARD_ITEM*>& auxItems );
    void ClearAuxItems();
    
private:

    std::vector<ANCHOR*> m_anchors;

    std::set<BOARD_ITEM*> queryVisible( const BOX2I& aArea ) const;

    ANCHOR* nearestAnchor( const VECTOR2I& aPos, int aFlags, LSET aMatchLayers );

    void computeAnchors( BOARD_ITEM* aItem, const VECTOR2I& aRefPos );

    void clearAnchors()
    {
        for ( auto a : m_anchors )
            delete a;

        m_anchors.clear();
    }


    OPT<ANCHOR*> m_currentAnchor;
    PREVIEW *m_preview;
    PCB_BASE_FRAME* m_frame;
    OPT<VECTOR2I> m_auxAxis;
    bool m_diagonalAuxAxesEnable;
    KIGFX::ORIGIN_VIEWITEM m_viewSnapPoint, m_viewAxis;
    int m_snapMode;
    std::vector<ANCHOR*> m_currentAnchorSet;
    std::vector<BOARD_ITEM*> m_auxItems;
};

#endif
