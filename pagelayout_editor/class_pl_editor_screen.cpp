/**
 * @file class_pl_editor_screen.cpp
 */
/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
 * @author Jean-Pierre Charras, jp.charras at wanadoo.fr
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
#include <common.h>
#include <macros.h>
#include <class_pl_editor_screen.h>
#include <base_units.h>
#include <pl_editor_id.h>


#define GRID_SIZE(x)     wxRealPoint(x, x)

/**
    Default zoom values.
    Roughly a 1.5 progression.
*/
static const double pl_editorZoomList[] =
{
    5,
    7.0,
    10.0,
    15.0,
    22.0,
    35.0,
    50.0,
    80.0,
    120.0,
    200.0,
    350.0,
    500.0,
    750.0,
    1000.0,
    1500.0,
    2000.0,
    3000.0,
};


// Default grid sizes for PCB editor screens.
static GRID_TYPE pl_editorGridList[] =
{
    // predefined grid list in mm
    { ID_POPUP_GRID_LEVEL_1MM,      GRID_SIZE( 1.0 )     },
    { ID_POPUP_GRID_LEVEL_0_5MM,    GRID_SIZE( 0.5 )     },
    { ID_POPUP_GRID_LEVEL_0_25MM,   GRID_SIZE( 0.25 )    },
    { ID_POPUP_GRID_LEVEL_0_2MM,    GRID_SIZE( 0.2 )     },
    { ID_POPUP_GRID_LEVEL_0_1MM,    GRID_SIZE( 0.1 )     },
};


PL_EDITOR_SCREEN::PL_EDITOR_SCREEN( const wxSize& aPageSizeIU ) :
    BASE_SCREEN( SCREEN_T )
{
    SetUnits( &g_GerbviewUnits );

    for( unsigned i = 0; i < DIM( pl_editorZoomList );  ++i )
        m_ZoomList.push_back( g_PLEditorUnits.MmToIu ( pl_editorZoomList[i] ) / 1000.0 );

    SetDefaultGrids ( pl_editorGridList, DIM (pl_editorGridList), true );

    // Set the working grid size to a reasonable value
    SetGrid( ID_POPUP_GRID_LEVEL_0_5MM );

    SetZoom( g_PLEditorUnits.DMilsToIu( 350 ) );            // a default value for zoom

    InitDataPoints( aPageSizeIU );
    m_NumberOfScreens = 2;
}


PL_EDITOR_SCREEN::~PL_EDITOR_SCREEN()
{
    ClearUndoRedoList();
}


// virtual function
int PL_EDITOR_SCREEN::MilsToIuScalar()
{
    return (int)g_PLEditorUnits.IuPerMils();
}


/* Virtual function needed by classes derived from BASE_SCREEN
 * this is a virtual pure function in BASE_SCREEN
 */
void PL_EDITOR_SCREEN::ClearUndoORRedoList( UNDO_REDO_CONTAINER& aList,
                                            int aItemCount )
{
    if( aItemCount == 0 )
        return;

    unsigned icnt = aList.m_CommandsList.size();

    if( aItemCount > 0 )
        icnt = aItemCount;

    for( unsigned ii = 0; ii < icnt; ii++ )
    {
        if( aList.m_CommandsList.size() == 0 )
            break;

        PICKED_ITEMS_LIST* curr_cmd = aList.m_CommandsList[0];
        aList.m_CommandsList.erase( aList.m_CommandsList.begin() );

        curr_cmd->ClearListAndDeleteItems();
        delete curr_cmd;    // Delete command
    }
}
