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
 * @file ratsnest.cpp
 * @brief Ratsnets functions.
 */

#include <fctsys.h>
#include <gr_basic.h>
#include <common.h>
#include <class_drawpanel.h>
#include <pcb_edit_frame.h>
#include <macros.h>

#include <class_board.h>
#include <class_module.h>
#include <class_track.h>

#include <pcbnew.h>

#include <connectivity_data.h>
#include <ratsnest_data.h>

/**
 * Function Compile_Ratsnest
 *  Create the entire board ratsnest.
 *  Must be called after a board change (changes for
 *  pads, footprints or a read netlist ).
 * @param aDC = the current device context (can be NULL)
 * @param aDisplayStatus : if true, display the computation results
 */
void PCB_BASE_FRAME::Compile_Ratsnest( wxDC* aDC, bool aDisplayStatus )
{
}


/**
 *  function DrawGeneralRatsnest
 *  Only ratsnest items with the status bit CH_VISIBLE set are displayed
 * @param aDC = the current device context (can be NULL)
 * @param aNetcode: if > 0, Display only the ratsnest relative to the
 * corresponding net_code
 */
void PCB_BASE_FRAME::DrawGeneralRatsnest( wxDC* aDC, int aNetcode )
{
}


void PCB_BASE_FRAME::TraceModuleRatsNest( wxDC* DC )
{
}


/*
 * PCB_BASE_FRAME::BuildAirWiresTargetsList and
 * PCB_BASE_FRAME::TraceAirWiresToTargets
 * are 2 function to show the near connecting points when
 * a new track is created, by displaying g_MaxLinksShowed airwires
 * between the on grid mouse cursor and these connecting points
 * during the creation of a track
 */

/* Buffer to store pads coordinates when creating a track.
 *  these pads are members of the net
 *  and when the mouse is moved, the g_MaxLinksShowed links to neighbors are
 * drawn
 */

static wxPoint s_CursorPos;     // Coordinate of the moving point (mouse cursor and
                                // end of current track segment)

/* Function BuildAirWiresTargetsList
 * Build a list of candidates that can be a coonection point
 * when a track is started.
 * This functions prepares data to show airwires to nearest connecting points (pads)
 * from the current new track to candidates during track creation
 */

static BOARD_CONNECTED_ITEM* s_ref = nullptr;
static int s_refNet = -1;

void PCB_BASE_FRAME::BuildAirWiresTargetsList( BOARD_CONNECTED_ITEM* aItemRef,
        const wxPoint& aPosition, int aNet )
{
}


static MODULE movedModule( nullptr );

void PCB_BASE_FRAME::build_ratsnest_module( MODULE* aModule, wxPoint aMoveVector )
{

}


void PCB_BASE_FRAME::TraceAirWiresToTargets( wxDC* aDC )
{
}


// Redraw in XOR mode the outlines of the module.
void MODULE::DrawOutlinesWhenMoving( EDA_DRAW_PANEL* panel, wxDC* DC,
        const wxPoint& aMoveVector )
{

}


void PCB_EDIT_FRAME::Show_1_Ratsnest( EDA_ITEM* item, wxDC* DC )
{

}
