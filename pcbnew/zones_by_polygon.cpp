/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2012 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2017 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file zones_by_polygon.cpp
 */

#include <fctsys.h>
#include <kiface_i.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <pcb_edit_frame.h>
#include <board_commit.h>
#include <view/view.h>

#include <class_board.h>
#include <class_zone.h>

#include <pcbnew.h>
#include <zones.h>
#include <pcbnew_id.h>
#include <protos.h>
#include <zones_functions_for_undo_redo.h>
#include <drc.h>
#include <connectivity_data.h>

#include <widgets/progress_reporter.h>

#include <zone_filler.h>

// Outline creation:
static void Abort_Zone_Create_Outline( EDA_DRAW_PANEL* Panel, wxDC* DC );
static void Show_New_Edge_While_Move_Mouse( EDA_DRAW_PANEL* aPanel, wxDC* aDC,
                                            const wxPoint& aPosition, bool aErase );

// Corner moving
static void Abort_Zone_Move_Corner_Or_Outlines( EDA_DRAW_PANEL* Panel, wxDC* DC );
static void Show_Zone_Corner_Or_Outline_While_Move_Mouse( EDA_DRAW_PANEL* aPanel,
                                                          wxDC*           aDC,
                                                          const wxPoint&  aPosition,
                                                          bool            aErase );

// Local variables
static wxPoint         s_CornerInitialPosition;     // Used to abort a move corner command
static bool            s_CornerIsNew;               // Used to abort a move corner command (if it is a new corner, it must be deleted)
static bool            s_AddCutoutToCurrentZone;    // if true, the next outline will be added to s_CurrentZone
static ZONE_CONTAINER* s_CurrentZone;               // if != NULL, these ZONE_CONTAINER params will be used for the next zone
static wxPoint         s_CursorLastPosition;        // in move zone outline, last cursor position. Used to calculate the move vector
static PICKED_ITEMS_LIST s_PickedList;              // a picked list to save zones for undo/redo command
static PICKED_ITEMS_LIST s_AuxiliaryList;           // a picked list to store zones that are deleted or added when combined


void PCB_EDIT_FRAME::Add_Similar_Zone( wxDC* DC, ZONE_CONTAINER* aZone )
{
}


void PCB_EDIT_FRAME::Add_Zone_Cutout( wxDC* DC, ZONE_CONTAINER* aZone )
{
}


void PCB_EDIT_FRAME::duplicateZone( wxDC* aDC, ZONE_CONTAINER* aZone )
{
}


int PCB_EDIT_FRAME::Delete_LastCreatedCorner( wxDC* DC )
{
}


/**
 * Function Abort_Zone_Create_Outline
 * cancels the Begin_Zone command if at least one EDGE_ZONE was created.
 */
static void Abort_Zone_Create_Outline( EDA_DRAW_PANEL* Panel, wxDC* DC )
{
}


void PCB_EDIT_FRAME::Start_Move_Zone_Corner( wxDC* DC, ZONE_CONTAINER* aZone,
                                             int corner_id, bool IsNewCorner )
{
}


void PCB_EDIT_FRAME::Start_Move_Zone_Drag_Outline_Edge( wxDC*           DC,
                                                        ZONE_CONTAINER* aZone,
                                                        int             corner_id )
{
}


void PCB_EDIT_FRAME::Start_Move_Zone_Outlines( wxDC* DC, ZONE_CONTAINER* aZone )
{
}


void PCB_EDIT_FRAME::End_Move_Zone_Corner_Or_Outlines( wxDC* DC, ZONE_CONTAINER* aZone )
{
}


void PCB_EDIT_FRAME::Remove_Zone_Corner( wxDC* DC, ZONE_CONTAINER* aZone )
{
}


/**
 * Function Abort_Zone_Move_Corner_Or_Outlines
 * cancels the Begin_Zone state if at least one EDGE_ZONE has been created.
 */
void Abort_Zone_Move_Corner_Or_Outlines( EDA_DRAW_PANEL* Panel, wxDC* DC )
{
}


/// Redraws the zone outline when moving a corner according to the cursor position
void Show_Zone_Corner_Or_Outline_While_Move_Mouse( EDA_DRAW_PANEL* aPanel, wxDC* aDC,
                                                   const wxPoint& aPosition, bool aErase )
{
}



int PCB_EDIT_FRAME::Begin_Zone( wxDC* DC )
{
}


bool PCB_EDIT_FRAME::End_Zone( wxDC* DC )
{
}


/* Redraws the zone outlines when moving mouse
 */
static void Show_New_Edge_While_Move_Mouse( EDA_DRAW_PANEL* aPanel, wxDC* aDC,
                                            const wxPoint& aPosition, bool aErase )
{
}

void PCB_EDIT_FRAME::Edit_Zone_Params( wxDC* DC, ZONE_CONTAINER* aZone )
{
}


void PCB_EDIT_FRAME::Delete_Zone_Contour( wxDC* DC, ZONE_CONTAINER* aZone )
{
}
