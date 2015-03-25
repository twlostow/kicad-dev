/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007-2014 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 1992-2012 KiCad Developers, see AUTHORS.txt for contributors.
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

/**
 * @file legacy_ratsnest.h
 * @brief Legacy ratsnet functions.
 */

#ifndef __LEGACY_RATSNEST_H
#define __LEGACY_RATSNEST_H

#include <gr_basic.h>

class EDA_DRAW_PANEL;
class MODULE;

/*****************************/
/* flags for a RATSNEST_ITEM */
/*****************************/
#define CH_VISIBLE          1        /* Visible */
#define CH_UNROUTABLE       2        /* Don't use autorouter. */
#define CH_ROUTE_REQ        4        /* Must be routed by the autorouter. */
#define CH_ACTIF            8        /* Not routed. */
#define LOCAL_RATSNEST_ITEM 0x8000   /* Line between two pads of a single module. */


/**
 * Class RATSNEST_ITEM
 * describes a ratsnest line: a straight line connecting 2 pads
 */
class LEGACY_RATSNEST_ITEM
{
private:
    int m_NetCode;      // netcode ( = 1.. n ,  0 is the value used for not connected items)

public:
    int    m_Status;    // State: see previous defines (CH_ ...)
    D_PAD* m_PadStart;  // pointer to the starting pad
    D_PAD* m_PadEnd;    // pointer to ending pad
    int    m_Length;    // length of the line (used in some calculations)

    LEGACY_RATSNEST_ITEM();

    /**
     * Function GetNet
     * @return int - the net code.
     */
    int GetNet() const
    {
        return m_NetCode;
    }

    void SetNet( int aNetCode )
    {
        m_NetCode = aNetCode;
    }

    bool IsVisible()
    {
        return (m_Status & CH_VISIBLE) != 0;
    }

    bool IsActive()
    {
        return (m_Status & CH_ACTIF) != 0;
    }

    bool IsLocal()
    {
        return (m_Status & LOCAL_RATSNEST_ITEM) != 0;
    }

    /**
     * Function Draw
     */
    void Draw( EDA_DRAW_PANEL* panel, wxDC* DC, GR_DRAWMODE aDrawMode,
               const wxPoint& offset );
};

typedef std::vector<LEGACY_RATSNEST_ITEM> LEGACY_RATSNEST_ITEMS;

class LEGACY_RATSNEST {
public:

    LEGACY_RATSNEST ( BOARD *aBoard );
    ~LEGACY_RATSNEST ( );

    LEGACY_RATSNEST_ITEM& GetItem ( unsigned int aIndex )
    {
    	return m_FullRatsnest[aIndex];
    }

    void SetVisible ( bool aVisible );
    void Clear();
    
    void BuildRatsnestForModule( MODULE* aModule );
    void BuildBoardRatsnest();
    void TestForActiveLinksInRatsnest( int aNetCode );


	LEGACY_RATSNEST_ITEMS& GetFull()
	{
		return m_FullRatsnest;
	}

	LEGACY_RATSNEST_ITEMS& GetLocal()
	{
		return m_LocalRatsnest;
	}

private: 
 
/// Ratsnest list for the BOARD
    LEGACY_RATSNEST_ITEMS m_FullRatsnest;

/// Ratsnest list relative to a given footprint (used while moving a footprint).
    LEGACY_RATSNEST_ITEMS m_LocalRatsnest;
 
    BOARD* m_board;
 };
 

#endif
