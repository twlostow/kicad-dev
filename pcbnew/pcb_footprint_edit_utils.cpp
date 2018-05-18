/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2012 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2015 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file modules.cpp
 */

#include <fctsys.h>
#include <gr_basic.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <pcb_edit_frame.h>
#include <trigo.h>
#include <macros.h>

#include <class_board.h>
#include <class_module.h>

#include <pcbnew.h>
#include <drag.h>
#include <dialog_get_footprint_by_name.h>

#include <connectivity_data.h>


MODULE* PCB_BASE_FRAME::GetFootprintFromBoardByReference()
{
}


void PCB_EDIT_FRAME::StartMoveModule( MODULE* aModule, wxDC* aDC,
                                      bool aDragConnectedTracks )
{
}


bool PCB_EDIT_FRAME::Delete_Module( MODULE* aModule, wxDC* aDC )
{
}


void PCB_EDIT_FRAME::Change_Side_Module( MODULE* Module, wxDC* DC )
{
}


void PCB_BASE_FRAME::PlaceModule( MODULE* aModule, wxDC* aDC, bool aRecreateRatsnest )
{
}

void PCB_BASE_FRAME::Rotate_Module( wxDC* DC, MODULE* module, double angle, bool incremental )
{
}
