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

#include <boost/bind.hpp>
#include <fctsys.h>
#include <class_drawpanel.h>
#include <class_draw_panel_gal.h>
#include <tool/tool_manager.h>
#include <wxPcbStruct.h>

#include <class_board.h>
#include <class_module.h>

#include <pcbnew.h>
#include <protos.h>
#include <module_editor_frame.h>
#include <board_undo_manager.h>

void FOOTPRINT_EDIT_FRAME::SaveCopyInUndoList( BOARD_ITEM*    aItem,
                                               UNDO_REDO_T    aTypeCommand,
                                               const wxPoint& aTransformPoint )
{
    EDA_ITEM*   item;
    MODULE*     mod = (MODULE *)aItem;

    m_undoManager->Add( aItem, UR_MODEDIT );
    m_undoManager->Commit( wxT( "Edit module with Modedit" ) );
    
    /* Clear current flags (which can be temporary set by a current edit command) */
    for( item = mod->GraphicalItems(); item != NULL; item = item->Next() )
        item->ClearFlags();

    for( D_PAD* pad = mod->Pads();  pad;  pad = pad->Next() )
        pad->ClearFlags();
}

void FOOTPRINT_EDIT_FRAME::SaveCopyInUndoList( const PICKED_ITEMS_LIST& aItemsList,
                                               UNDO_REDO_T        aTypeCommand,
                                               const wxPoint&     aTransformPoint )
{
    assert( aItemsList.GetPickedItem( 0 )->GetParent()->Type() == PCB_MODULE_T );
    MODULE* owner = static_cast<MODULE*>( aItemsList.GetPickedItem( 0 )->GetParent() );

#ifndef NDEBUG
    // All items should have the same parent (MODULE) to make undo/redo entry valid
    for( unsigned int i = 0; i < aItemsList.GetCount(); ++i )
        assert( aItemsList.GetPickedItem( i )->GetParent() == owner );
#endif /* not NDEBUG */

    SaveCopyInUndoList( owner, aTypeCommand, aTransformPoint );
}


void FOOTPRINT_EDIT_FRAME::RestoreCopyFromRedoList( wxCommandEvent& aEvent )
{
    if( m_undoManager->GetRedoCommandCount() <= 0 )
        return;

    // Inform tools that undo command was issued
    TOOL_EVENT event( TC_MESSAGE, TA_UNDO_REDO, AS_GLOBAL );
    m_toolManager->ProcessEvent( event );

    m_undoManager->Redo();

    SetCurItem( NULL );

    OnModify();
    m_canvas->Refresh();
}


void FOOTPRINT_EDIT_FRAME::RestoreCopyFromUndoList( wxCommandEvent& aEvent )
{
     if( m_undoManager->GetUndoCommandCount() <= 0 )
        return;

    // Inform tools that undo command was issued
    TOOL_EVENT event( TC_MESSAGE, TA_UNDO_REDO, AS_GLOBAL );
    m_toolManager->ProcessEvent( event );

    m_undoManager->Undo();

    SetCurItem( NULL );

    OnModify();
    m_canvas->Refresh();
}
