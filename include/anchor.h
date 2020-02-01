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

#ifndef __ANCHOR_H
#define __ANCHOR_H

#include <class_board_item.h>
#include <math/vector2d.h>

enum ANCHOR_FLAGS
{
    AF_DIRECTIONAL = 0x1,
    AF_LINEAR   = 0x2,
    AF_POINT    = 0x4,
    AF_CORNER   = 0x8,
    AF_CENTER   = 0x10,
    AF_ORIGIN   = 0x20,
    AF_SNAPPABLE = 0x40,
    AF_OUTLINE = 0x80,
    AF_CONSTRAINABLE = 0x100
};


class ANCHOR
{
private:

    BOARD_ITEM* m_owner;
    VECTOR2I m_pos;
    VECTOR2I m_offset;
    int m_flags;

public:

    ANCHOR( BOARD_ITEM* owner, const VECTOR2I& pos, int flags ) :
        m_owner( owner ),
        m_pos( pos ),
        m_flags( flags )
    {}

    ~ANCHOR()
    {
    }

    BOARD_ITEM* GetOwner() const
    {
        return m_owner;
    }

    int GetFlags() const
    {
        return m_flags;
    }

    const VECTOR2I GetPos() const
    {
        return m_pos;
    }

    void SetOffset( const VECTOR2I& aOffset )
    {
        m_offset = aOffset;
    }

    int Distance( const VECTOR2I& aP ) const
    {
        return ( aP - ( m_pos + m_offset ) ).EuclideanNorm();
    }
};

#endif
