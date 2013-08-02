/**
 * @file pagelayout_editor/onrightclick.cpp
  * @brief functions called on rigth click mouse event
*/

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2013 CERN
 * Copyright (C) 1992-2013 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <fctsys.h>
#include <class_drawpanel.h>
#include <pl_editor_id.h>

#include <pl_editor_frame.h>
#include <design_tree_frame.h>
#include <properties_frame.h>
#include <menus_helpers.h>
#include <worksheet_shape_builder.h>
#include <class_worksheet_dataitem.h>
#include <hotkeys.h>


/* Prepare the right-click pullup menu.
 * The menu already has a list of zoom commands.
 */
bool PL_EDITOR_FRAME::OnRightClick( const wxPoint& aPosition, wxMenu* aPopMenu )
{
    bool busy = GetScreen()->GetCurItem() != NULL;
    wxString msg;

    if( ! busy )     // No item currently edited
    {
        WORKSHEET_DATAITEM* old_item = m_treePagelayout->GetPageLayoutSelectedItem();
        WORKSHEET_DATAITEM* item = Locate( aPosition );

        if( item && old_item != item )
        {
            m_treePagelayout->SelectCell( item );
            m_canvas->Refresh();
        }

        // Add menus to edit and delete the item
        if( item )
        {
            if( (item->GetFlags() & LOCATE_STARTPOINT) )
            {
                msg = AddHotkeyName( _( "Move Start Point" ), s_PlEditor_Hokeys_Descr,
                                     HK_MOVE_START_POINT );
                AddMenuItem( aPopMenu, ID_POPUP_ITEM_MOVE_START_POINT, msg,
                             KiBitmap( move_xpm ) );
            }

            if( (item->GetFlags() & LOCATE_ENDPOINT ) )
            {
                msg = AddHotkeyName(  _( "Move End Point" ), s_PlEditor_Hokeys_Descr,
                                     HK_MOVE_END_POINT );
                AddMenuItem( aPopMenu, ID_POPUP_ITEM_MOVE_END_POINT, msg,
                             KiBitmap( move_xpm ) );
            }

            msg = AddHotkeyName(  _( "Move Item" ), s_PlEditor_Hokeys_Descr,
                                 HK_MOVE_ITEM );
            AddMenuItem( aPopMenu, ID_POPUP_ITEM_MOVE, msg,
                         KiBitmap( move_xpm ) );
            aPopMenu->AppendSeparator();

            msg = AddHotkeyName(  _( "Delete" ), s_PlEditor_Hokeys_Descr,
                                 HK_DELETE_ITEM );
            AddMenuItem( aPopMenu, ID_POPUP_ITEM_DELETE, msg, KiBitmap( delete_xpm ) );
            aPopMenu->AppendSeparator();
        }
    }
    else     // An item is currently in edit
    {
        AddMenuItem( aPopMenu, ID_POPUP_ITEM_PLACE, _( "Place" ),
                     KiBitmap( move_xpm ) );
        AddMenuItem( aPopMenu, ID_POPUP_ITEM_PLACE_CANCEL, _( "Cancel" ),
                     KiBitmap( cancel_xpm ) );
        aPopMenu->AppendSeparator();
    }

    if( ! busy )
    {
        AddMenuItem( aPopMenu, ID_POPUP_ITEM_ADD_LINE, _( "Add line" ),
                     KiBitmap( add_dashed_line_xpm ) );
        AddMenuItem( aPopMenu, ID_POPUP_ITEM_ADD_RECT, _( "Add rect" ),
                     KiBitmap( add_rectangle_xpm ) );
        AddMenuItem( aPopMenu, ID_POPUP_ITEM_ADD_TEXT, _( "Add text" ),
                     KiBitmap( add_text_xpm ) );
        AddMenuItem( aPopMenu, ID_POPUP_ITEM_ADD_POLY, _( "Import poly descr file" ),
                     KiBitmap( add_polygon_xpm ) );

        aPopMenu->AppendSeparator();
    }

    return true;
}
