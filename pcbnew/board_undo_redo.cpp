
/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
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
#include <macros.h>

#include <pcbnew.h>
#include <wxPcbStruct.h>

#include <class_board.h>
#include <class_track.h>
#include <class_drawsegment.h>
#include <class_pcb_text.h>
#include <class_mire.h>
#include <class_module.h>
#include <class_dimension.h>
#include <class_zone.h>
#include <class_edge_mod.h>
#include <class_netinfo.h>


#include <ratsnest_data.h>

#include <tools/selection_tool.h>
#include <tool/tool_manager.h>

#include <board_undo_manager.h>

#if 0
/* Functions to undo and redo edit commands.
 *  commands to undo are stored in CurrentScreen->m_UndoList
 *  commands to redo are stored in CurrentScreen->m_RedoList
 *
 *  m_UndoList and m_RedoList handle a std::vector of PICKED_ITEMS_LIST
 *  Each PICKED_ITEMS_LIST handle a std::vector of pickers (class ITEM_PICKER),
 *  that store the list of schematic items that are concerned by the command to undo or redo
 *  and is created for each command to undo (handle also a command to redo).
 *  each picker has a pointer pointing to an item to undo or redo (in fact: deleted, added or
 *  modified),
 * and has a pointer to a copy of this item, when this item has been modified
 * (the old values of parameters are therefore saved)
 *
 *  there are 3 cases:
 *  - delete item(s) command
 *  - change item(s) command
 *  - add item(s) command
 *  and 3 cases for block:
 *  - move list of items
 *  - mirror (Y) list of items
 *  - Flip list of items
 *
 *  Undo command
 *  - delete item(s) command:
 *       =>  deleted items are moved in undo list
 *
 *  - change item(s) command
 *      => A copy of item(s) is made (a DrawPickedStruct list of wrappers)
 *      the .m_Link member of each wrapper points the modified item.
 *      the .m_Item member of each wrapper points the old copy of this item.
 *
 *  - add item(s) command
 *      =>A list of item(s) is made. The .m_Item member of each wrapper points the new item.
 *
 *  Redo command
 *  - delete item(s) old command:
 *      => deleted items are moved in EEDrawList list, and in
 *
 *  - change item(s) command
 *      => the copy of item(s) is moved in Undo list
 *
 *  - add item(s) command
 *      => The list of item(s) is used to create a deleted list in undo list(same as a delete
 *         command)
 *
 *   Some block operations that change items can be undone without memorize items, just the
 *   coordinates of the transform:
 *      move list of items (undo/redo is made by moving with the opposite move vector)
 *      mirror (Y) and flip list of items (undo/redo is made by mirror or flip items)
 *      so they are handled specifically.
 *
 */


/**
 * Function TestForExistingItem
 * test if aItem exists somewhere in lists of items
 * This is a function used by PutDataInPreviousState to be sure an item was not deleted
 * since an undo or redo.
 * This could be possible:
 *   - if a call to SaveCopyInUndoList was forgotten in Pcbnew
 *   - in zones outlines, when a change in one zone merges this zone with an other
 * This function avoids a Pcbnew crash
 * Before using this function to test existence of items,
 * it must be called with aItem = NULL to prepare the list
 * @param board = board to test
 * @param aItem = item to find
 *              = NULL to build the list of existing items
 */
bool BOARD_UNDO_MANAGER::testForExistingItem( BOARD_ITEM* aItem )
{
    
    static std::vector<BOARD_ITEM*> itemsList;
    BOARD *board = getBoard();

    if( aItem == NULL ) // Build list
    {
        // Count items to store in itemsList:
        int icnt = 0;
        BOARD_ITEM* item;

        // Count tracks:
        for( item = aPcb->m_Track; item != NULL; item = item->Next() )
            icnt++;

        // Count modules:
        for( item = board->m_Modules; item != NULL; item = item->Next() )
            icnt++;

        // Count drawings
        for( item = board->m_Drawings; item != NULL; item = item->Next() )
            icnt++;

        // Count zones outlines
        icnt +=  board->GetAreaCount();

        // Count zones segm (now obsolete):
        for( item = board->m_Zone; item != NULL; item = item->Next() )
             icnt++;

        NETINFO_LIST& netInfo = board->GetNetInfo();

        icnt += netInfo.GetNetCount();

        // Build candidate list:
        itemsList.clear();
        itemsList.reserve(icnt);

        // Store items in list:
        // Append tracks:
        for( item = board->m_Track; item != NULL; item = item->Next() )
            itemsList.push_back( item );

        // Append modules:
        for( item = board->m_Modules; item != NULL; item = item->Next() )
            itemsList.push_back( item );

        // Append drawings
        for( item = board->m_Drawings; item != NULL; item = item->Next() )
            itemsList.push_back( item );

        // Append zones outlines
        for( int ii = 0; ii < board->GetAreaCount(); ii++ )
            itemsList.push_back( board->GetArea( ii ) );

        // Append zones segm:
        for( item = board->m_Zone; item != NULL; item = item->Next() )
            itemsList.push_back( item );

        for( NETINFO_LIST::iterator i = netInfo.begin(); i != netInfo.end(); ++i )
            itemsList.push_back ( *i );

        // Sort list
        std::sort( itemsList.begin(), itemsList.end() );
        return false;
    }

    // search in list:
    return std::binary_search( itemsList.begin(), itemsList.end(), aItem );
}

#endif

#if 1
void BOARD_ITEM::SwapData( BOARD_ITEM* aImage )
{
    if( aImage == NULL )
        return;

    // Remark: to create images of edited items to undo, we are using Clone method
    // which can duplication of items foe copy, but does not clone all members
    // mainly pointers in chain and time stamp, which is set to new, unique value.
    // So we have to use the current values of these parameters.

    EDA_ITEM * pnext = Next();
    EDA_ITEM * pback = Back();
    DHEAD* mylist    = m_List;
    time_t timestamp = GetTimeStamp();

    switch( Type() )
    {
    case PCB_MODULE_T:
    {
        MODULE* tmp = (MODULE*) aImage->Clone();
        ( (MODULE*) aImage )->Copy( (MODULE*) this );
        ( (MODULE*) this )->Copy( tmp );

        delete tmp;
    }
        break;

    case PCB_ZONE_AREA_T:
    {
        ZONE_CONTAINER* tmp = (ZONE_CONTAINER*) aImage->Clone();
        ( (ZONE_CONTAINER*) aImage )->Copy( (ZONE_CONTAINER*) this );
        ( (ZONE_CONTAINER*) this )->Copy( tmp );
        delete tmp;
    }
        break;

    case PCB_LINE_T:
        std::swap( *((DRAWSEGMENT*)this), *((DRAWSEGMENT*)aImage) );
        break;

    case PCB_TRACE_T:
    case PCB_VIA_T:
    {
        TRACK* track = (TRACK*) this;
        TRACK* image = (TRACK*) aImage;

        EXCHG(track->m_Layer, image->m_Layer );

        // swap start, end, width and shape for track and image.
        wxPoint exchp = track->GetStart();
        track->SetStart( image->GetStart() );
        image->SetStart( exchp );
        exchp = track->GetEnd();
        track->SetEnd( image->GetEnd() );
        image->SetEnd( exchp );

        int atmp = track->GetWidth();
        track->SetWidth( image->GetWidth() );
        image->SetWidth( atmp );

        if( Type() == PCB_VIA_T )
        {
            VIA *via = static_cast<VIA*>( this );
            VIA *viaimage = static_cast<VIA*>( aImage );

            VIATYPE_T viatmp = via->GetViaType();
            via->SetViaType( viaimage->GetViaType() );
            viaimage->SetViaType( viatmp );

            int drilltmp = via->GetDrillValue();

            if( via->IsDrillDefault() )
                drilltmp = -1;

            int itmp = viaimage->GetDrillValue();

            if( viaimage->IsDrillDefault() )
                itmp = -1;

            EXCHG(itmp, drilltmp );

            if( drilltmp > 0 )
                via->SetDrill( drilltmp );
            else
                via->SetDrillDefault();

            if( itmp > 0 )
                viaimage->SetDrill( itmp );
            else
                viaimage->SetDrillDefault();
        }
    }
        break;

    case PCB_TEXT_T:
        std::swap( *((TEXTE_PCB*)this), *((TEXTE_PCB*)aImage) );
        break;

    case PCB_TARGET_T:
        std::swap( *((PCB_TARGET*)this), *((PCB_TARGET*)aImage) );
        break;

    case PCB_DIMENSION_T:
        std::swap( *((DIMENSION*)this), *((DIMENSION*)aImage) );
        break;

    case PCB_ZONE_T:
    default:
        wxLogMessage( wxT( "SwapData() error: unexpected type %d" ), Type() );
        break;
    }

    // Restore pointers and time stamp, to be sure they are not broken
    Pnext = pnext;
    Pback = pback;
    m_List = mylist;
    SetTimeStamp( timestamp );
}

void PCB_EDIT_FRAME::SaveCopyInUndoList( BOARD_ITEM*    aItem,
                                         UNDO_REDO_T    aCommandType,
                                         const wxPoint& aTransformPoint )
{
    
    m_undoManager->Add ( aItem, aCommandType );
    m_undoManager->SetTransformPoint ( aTransformPoint );
    m_undoManager->Commit ( );
}


void PCB_EDIT_FRAME::SaveCopyInUndoList( const PICKED_ITEMS_LIST& aItemsList,
                                         UNDO_REDO_T        aTypeCommand,
                                         const wxPoint&     aTransformPoint )
{
    m_undoManager->Add ( aItemsList, aTypeCommand );
    m_undoManager->SetTransformPoint ( aTransformPoint );
    m_undoManager->Commit ( );
}

void PCB_EDIT_FRAME::RestoreCopyFromUndoList( wxCommandEvent& aEvent )
{
    if( m_undoManager->GetUndoCommandCount() <= 0 )
        return;

    // Inform tools that undo command was issued
    TOOL_EVENT event( TC_MESSAGE, TA_UNDO_REDO, AS_GLOBAL );
    m_toolManager->ProcessEvent( event );

    m_undoManager->Undo();

    // Rebuild pointers and ratsnest that can be changed.
    if( m_undoManager->IsRatsnestDirty() )
    {
        if( IsGalCanvasActive() )
            GetBoard()->GetRatsnest()->Recalculate();
        else
            Compile_Ratsnest( NULL, true );
    }
    
    OnModify();
    m_canvas->Refresh();
}


void PCB_EDIT_FRAME::RestoreCopyFromRedoList( wxCommandEvent& aEvent )
{
 if( m_undoManager->GetUndoCommandCount() <= 0 )
        return;

    // Inform tools that undo command was issued
    TOOL_EVENT event( TC_MESSAGE, TA_UNDO_REDO, AS_GLOBAL );
    m_toolManager->ProcessEvent( event );

    m_undoManager->Redo();

    // Rebuild pointers and ratsnest that can be changed.
    if( m_undoManager->IsRatsnestDirty() )
    {
        if( IsGalCanvasActive() )
            GetBoard()->GetRatsnest()->Recalculate();
        else
            Compile_Ratsnest( NULL, true );
    }
    
    OnModify();
    m_canvas->Refresh();
}

#endif
