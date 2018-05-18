/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2011 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file pcbnew/block.cpp
 */


#include <fctsys.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <block_commande.h>
#include <pcb_edit_frame.h>
#include <trigo.h>

#include <class_board.h>
#include <class_track.h>
#include <class_drawsegment.h>
#include <class_pcb_text.h>
#include <class_pcb_target.h>
#include <class_module.h>
#include <class_dimension.h>
#include <class_zone.h>

#include <dialog_block_options.h>

#include <pcbnew.h>
#include <protos.h>

#include <connectivity_data.h>




int PCB_EDIT_FRAME::BlockCommand( EDA_KEY aKey )
{
}


void PCB_EDIT_FRAME::HandleBlockPlace( wxDC* DC )
{
}


bool PCB_EDIT_FRAME::HandleBlockEnd( wxDC* DC )
{
}


void PCB_EDIT_FRAME::Block_SelectItems()
{
}

void PCB_EDIT_FRAME::Block_Delete()
{
}


void PCB_EDIT_FRAME::Block_Rotate()
{
}


void PCB_EDIT_FRAME::Block_Flip()
{
}


void PCB_EDIT_FRAME::Block_Move()
{
}


void PCB_EDIT_FRAME::Block_Duplicate( bool aIncrement )
{
}
