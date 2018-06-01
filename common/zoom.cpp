/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 1992-2018 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file zoom.cpp
 */

/*
 * Manage zoom, grid step, and auto crop.
 */

#include <fctsys.h>
#include <id.h>
#include <class_drawpanel.h>
#include <view/view.h>
#include <base_screen.h>
#include <draw_frame.h>
#include <kicad_device_context.h>
#include <hotkeys_basic.h>
#include <menus_helpers.h>
#include <base_units.h>
#include <tool/tool_manager.h>


void EDA_DRAW_FRAME::RedrawScreen( const wxPoint& aCenterPoint, bool aWarpPointer )
{
    if( IsGalCanvasActive() )
        return;

    AdjustScrollBars( aCenterPoint );

    // Move the mouse cursor to the on grid graphic cursor position
    if( aWarpPointer )
        m_canvas->MoveCursorToCrossHair();

    auto canvas = GetLegacyCanvas();

    canvas->Refresh();
    canvas->Update();
}

void EDA_DRAW_FRAME::RedrawScreen2( const wxPoint& posBefore )
{
    if( IsGalCanvasActive() )
        return;

    auto canvas = GetLegacyCanvas();

    // relative screen position to center before zoom
    wxPoint dPos = posBefore - canvas->GetClientSize() / 2;

    // screen position of crosshair after zoom
    wxPoint newScreenPos = m_canvas->ToDeviceXY( GetCrossHairPosition() );
    wxPoint newCenter = m_canvas->ToLogicalXY( newScreenPos - dPos );

    AdjustScrollBars( newCenter );

    canvas->Refresh();
    canvas->Update();
}


// Factor out the calculation portion of the various BestZoom() implementations.
//
// Note that like it's forerunners this routine has an intentional side-effect: it
// sets the scroll centre position.  While I'm not happy about that, it's probably
// not worth fixing as its days are numbered (GAL canvases use a different method).
double EDA_DRAW_FRAME::bestZoom( double sizeX, double sizeY, double scaleFactor, wxPoint centre )
{
    if( IsGalCanvasActive() )
        return 1.0;

    auto canvas = GetLegacyCanvas();

    double bestzoom = std::max( sizeX * scaleFactor / (double) canvas->GetClientSize().x,
                                sizeY * scaleFactor / (double) canvas->GetClientSize().y );

    // Take scrollbars into account
    DSIZE scrollbarSize = canvas->GetSize() - canvas->GetClientSize();
    centre.x -= int( bestzoom * scrollbarSize.x / 2.0 );
    centre.y -= int( bestzoom * scrollbarSize.y / 2.0 );

    SetScrollCenterPosition( centre );

    return bestzoom;
}


void EDA_DRAW_FRAME::Zoom_Automatique( bool aWarpPointer )
{
    BASE_SCREEN* screen = GetScreen();

    if ( IsGalCanvasActive() )
    {
        m_toolManager->RunAction( "common.Control.zoomFitScreen", true );
        return;
    }

    // Set the best zoom and get center point.

    // BestZoom() can compute an illegal zoom if the client window size
    // is small, say because frame is not maximized.  So use the clamping form
    // of SetZoom():
    double bestzoom = BestZoom();
    screen->SetScalingFactor( bestzoom );

    if( screen->m_FirstRedraw )
        SetCrossHairPosition( GetScrollCenterPosition() );

    RedrawScreen( GetScrollCenterPosition(), aWarpPointer );
}


void EDA_DRAW_FRAME::Window_Zoom( EDA_RECT& Rect )
{
    if ( IsGalCanvasActive() )
        return;

    // Compute the best zoom
    Rect.Normalize();

    auto canvas =  GetLegacyCanvas();

    wxSize size = canvas->GetClientSize();

    // Use ceil to at least show the full rect
    double scalex    = (double) Rect.GetSize().x / size.x;
    double bestscale = (double) Rect.GetSize().y / size.y;

    bestscale = std::max( bestscale, scalex );

    GetScreen()->SetScalingFactor( bestscale );
    RedrawScreen( Rect.Centre(), true );
}


void EDA_DRAW_FRAME::OnZoom( wxCommandEvent& event )
{
    if( m_canvas == NULL )
        return;

    int          id = event.GetId();
    bool         zoom_at_cursor = false;
    BASE_SCREEN* screen = GetScreen();
    wxPoint      center = GetScrollCenterPosition();

    if ( id == ID_KEY_ZOOM_IN )
    {
        id = GetCanvas()->GetEnableZoomNoCenter() ?
             ID_OFFCENTER_ZOOM_IN : ID_POPUP_ZOOM_IN;
    }
    else if ( id == ID_KEY_ZOOM_OUT )
    {
        id = GetCanvas()->GetEnableZoomNoCenter() ?
             ID_OFFCENTER_ZOOM_OUT : ID_POPUP_ZOOM_OUT;
    }

    switch( id )
    {
    case ID_OFFCENTER_ZOOM_IN:
        center = m_canvas->ToDeviceXY( GetCrossHairPosition() );

        if( screen->SetPreviousZoom() )
            RedrawScreen2( center );
        break;

    case ID_POPUP_ZOOM_IN:
        zoom_at_cursor = true;
        center = GetCrossHairPosition();

    // fall thru
    case ID_VIEWER_ZOOM_IN:
    case ID_ZOOM_IN:
        if( screen->SetPreviousZoom() )
            RedrawScreen( center, zoom_at_cursor );
        break;

    case ID_OFFCENTER_ZOOM_OUT:
        center = m_canvas->ToDeviceXY( GetCrossHairPosition() );

        if( screen->SetNextZoom() )
            RedrawScreen2( center );
        break;

    case ID_POPUP_ZOOM_OUT:
        zoom_at_cursor = true;
        center = GetCrossHairPosition();

    // fall thru
    case ID_VIEWER_ZOOM_OUT:
    case ID_ZOOM_OUT:
        if( screen->SetNextZoom() )
            RedrawScreen( center, zoom_at_cursor );
        break;

    case ID_VIEWER_ZOOM_REDRAW:
    case ID_POPUP_ZOOM_REDRAW:
    case ID_ZOOM_REDRAW:
        m_canvas->Refresh();
        break;

    case ID_POPUP_ZOOM_CENTER:
        center = GetCrossHairPosition();
        RedrawScreen( center, true );
        break;

    case ID_POPUP_ZOOM_PAGE:
    case ID_VIEWER_ZOOM_PAGE:
    case ID_ZOOM_PAGE:
        Zoom_Automatique( false );
        break;

    case ID_POPUP_ZOOM_SELECT:
        break;

    case ID_POPUP_CANCEL:
        m_canvas->MoveCursorToCrossHair();
        break;

    default:
        SetPresetZoom( id - ID_POPUP_ZOOM_LEVEL_START );
    }

    UpdateStatusBar();
}


void EDA_DRAW_FRAME::SetNextZoom()
{
    GetScreen()->SetNextZoom();
}


void EDA_DRAW_FRAME::SetPrevZoom()
{
    GetScreen()->SetPreviousZoom();
}


void EDA_DRAW_FRAME::SetPresetZoom( int aIndex )
{
    BASE_SCREEN* screen = GetScreen();

    if( aIndex >= (int) screen->m_ZoomList.size() )
    {
        wxLogDebug( wxT( "%s %d: index %d is outside the bounds of the zoom list." ),
                    __TFILE__, __LINE__, aIndex );
        return;
    }

    if( m_zoomSelectBox )
        m_zoomSelectBox->SetSelection( aIndex );

    if( screen->SetZoom( screen->m_ZoomList[aIndex] ) )
        RedrawScreen( GetScrollCenterPosition(), true );

    UpdateStatusBar();
}


void EDA_DRAW_FRAME::AddMenuZoomAndGrid( wxMenu* MasterMenu )
{
    int         maxZoomIds;
    double      zoom;
    wxString    msg;
    BASE_SCREEN* screen = m_canvas->GetScreen();

    msg = AddHotkeyName( _( "Center" ), m_hotkeysDescrList, HK_ZOOM_CENTER );
    AddMenuItem( MasterMenu, ID_POPUP_ZOOM_CENTER, msg, KiBitmap( zoom_center_on_screen_xpm ) );
    msg = AddHotkeyName( _( "Zoom In" ), m_hotkeysDescrList, HK_ZOOM_IN );
    AddMenuItem( MasterMenu, ID_POPUP_ZOOM_IN, msg, KiBitmap( zoom_in_xpm ) );
    msg = AddHotkeyName( _( "Zoom Out" ), m_hotkeysDescrList, HK_ZOOM_OUT );
    AddMenuItem( MasterMenu, ID_POPUP_ZOOM_OUT, msg, KiBitmap( zoom_out_xpm ) );
    msg = AddHotkeyName( _( "Redraw View" ), m_hotkeysDescrList, HK_ZOOM_REDRAW );
    AddMenuItem( MasterMenu, ID_POPUP_ZOOM_REDRAW, msg, KiBitmap( zoom_redraw_xpm ) );
    msg = AddHotkeyName( _( "Zoom to Fit" ), m_hotkeysDescrList, HK_ZOOM_AUTO );
    AddMenuItem( MasterMenu, ID_POPUP_ZOOM_PAGE, msg, KiBitmap( zoom_fit_in_page_xpm ) );


    wxMenu* zoom_choice = new wxMenu;
    AddMenuItem( MasterMenu, zoom_choice,
                 ID_POPUP_ZOOM_SELECT, _( "Zoom" ),
                 KiBitmap( zoom_selection_xpm ) );

    zoom = screen->GetZoom();
    maxZoomIds = ID_POPUP_ZOOM_LEVEL_END - ID_POPUP_ZOOM_LEVEL_START;
    maxZoomIds = ( (size_t) maxZoomIds < screen->m_ZoomList.size() ) ?
                 maxZoomIds : screen->m_ZoomList.size();

    // Populate zoom submenu.
    for( int i = 0; i < maxZoomIds; i++ )
    {
        msg.Printf( wxT( "%.2f" ), m_zoomLevelCoeff / screen->m_ZoomList[i] );

        zoom_choice->Append( ID_POPUP_ZOOM_LEVEL_START + i, _( "Zoom: " ) + msg,
                             wxEmptyString, wxITEM_CHECK );
        if( zoom == screen->m_ZoomList[i] )
            zoom_choice->Check( ID_POPUP_ZOOM_LEVEL_START + i, true );
    }

    // Create grid submenu as required.
    if( screen->GetGridCount() )
    {
        wxMenu* gridMenu = new wxMenu;
        AddMenuItem( MasterMenu, gridMenu, ID_POPUP_GRID_SELECT,
                     _( "Grid" ), KiBitmap( grid_select_xpm ) );

        wxArrayString gridsList;
        int icurr = screen->BuildGridsChoiceList( gridsList, g_UserUnit != INCHES );

        for( unsigned i = 0; i < gridsList.GetCount(); i++ )
        {
            GRID_TYPE& grid = screen->GetGrid( i );
            gridMenu->Append( grid.m_CmdId, gridsList[i], wxEmptyString, wxITEM_CHECK );

            if( (int)i == icurr )
                gridMenu->Check( grid.m_CmdId, true );
        }
    }

    MasterMenu->AppendSeparator();
    AddMenuItem( MasterMenu, ID_POPUP_CANCEL, _( "Close" ), KiBitmap( cancel_xpm ) );
}
