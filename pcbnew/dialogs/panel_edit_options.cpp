/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 1992-2021 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <pgm_base.h>
#include <string_utils.h>
#include <settings/settings_manager.h>
#include <pcbnew_settings.h>
#include <footprint_editor_settings.h>
#include <panel_edit_options.h>


PANEL_EDIT_OPTIONS::PANEL_EDIT_OPTIONS( wxWindow* aParent, bool isFootprintEditor ) :
        PANEL_EDIT_OPTIONS_BASE( aParent ),
        m_isFootprintEditor( isFootprintEditor )
{
    m_magneticPads->Show( m_isFootprintEditor );
    m_magneticGraphics->Show( m_isFootprintEditor );
    m_flipLeftRight->Show( !m_isFootprintEditor );
    m_allowFreePads->Show( !m_isFootprintEditor );

#ifdef __WXOSX_MAC__
    m_mouseCmdsOSX->Show( true );
    m_mouseCmdsWinLin->Show( false );
#else
    m_mouseCmdsWinLin->Show( true );
    m_mouseCmdsOSX->Show( false );
#endif

    m_optionsBook->SetSelection( isFootprintEditor ? 0 : 1 );
}


bool PANEL_EDIT_OPTIONS::TransferDataToWindow()
{
    SETTINGS_MANAGER& mgr = Pgm().GetSettingsManager();

    if( m_isFootprintEditor )
    {
        FOOTPRINT_EDITOR_SETTINGS* cfg = mgr.GetAppSettings<FOOTPRINT_EDITOR_SETTINGS>();

        m_rotationAngle->SetValue( AngleToStringDegrees( (double) cfg->m_RotationAngle ) );
        m_magneticPads->SetValue( cfg->m_MagneticItems.pads == MAGNETIC_OPTIONS::CAPTURE_ALWAYS );
        m_magneticGraphics->SetValue( cfg->m_MagneticItems.graphics );
        m_cbFpGraphic45Mode->SetValue( cfg->m_Use45Limit );
    }
    else
    {
        PCBNEW_SETTINGS* cfg = mgr.GetAppSettings<PCBNEW_SETTINGS>();

        m_rotationAngle->SetValue( AngleToStringDegrees( (double) cfg->m_RotationAngle ) );
        m_magneticPadChoice->SetSelection( static_cast<int>( cfg->m_MagneticItems.pads ) );
        m_magneticTrackChoice->SetSelection( static_cast<int>( cfg->m_MagneticItems.tracks ) );
        m_magneticGraphicsChoice->SetSelection( !cfg->m_MagneticItems.graphics );
        m_flipLeftRight->SetValue( cfg->m_FlipLeftRight );
        m_cbPcbGraphic45Mode->SetValue( cfg->m_Use45DegreeLimit );

        /* Set display options */
        m_OptDisplayCurvedRatsnestLines->SetValue( cfg->m_Display.m_DisplayRatsnestLinesCurved );
        m_showSelectedRatsnest->SetValue( cfg->m_Display.m_ShowModuleRatsnest );

        switch( cfg->m_TrackDragAction )
        {
        case TRACK_DRAG_ACTION::MOVE:            m_rbTrackDragMove->SetValue( true ); break;
        case TRACK_DRAG_ACTION::DRAG:            m_rbTrackDrag45->SetValue( true );   break;
        case TRACK_DRAG_ACTION::DRAG_FREE_ANGLE: m_rbTrackDragFree->SetValue( true ); break;
        }

        m_showPageLimits->SetValue( cfg->m_ShowPageLimits );
        m_autoRefillZones->SetValue( cfg->m_AutoRefillZones );
        m_allowFreePads->SetValue( cfg->m_AllowFreePads );
    }

   return true;
}


bool PANEL_EDIT_OPTIONS::TransferDataFromWindow()
{
    SETTINGS_MANAGER& mgr = Pgm().GetSettingsManager();

    if( m_isFootprintEditor )
    {
        FOOTPRINT_EDITOR_SETTINGS* cfg = mgr.GetAppSettings<FOOTPRINT_EDITOR_SETTINGS>();

        cfg->m_RotationAngle = wxRound( 10.0 * wxAtof( m_rotationAngle->GetValue() ) );

        cfg->m_MagneticItems.pads = m_magneticPads->GetValue() ? MAGNETIC_OPTIONS::CAPTURE_ALWAYS
                                                               : MAGNETIC_OPTIONS::NO_EFFECT;
        cfg->m_MagneticItems.graphics = m_magneticGraphics->GetValue();

        cfg->m_Use45Limit = m_cbFpGraphic45Mode->GetValue();
    }
    else
    {
        PCBNEW_SETTINGS* cfg = mgr.GetAppSettings<PCBNEW_SETTINGS>();

        cfg->m_Display.m_DisplayRatsnestLinesCurved = m_OptDisplayCurvedRatsnestLines->GetValue();
        cfg->m_Display.m_ShowModuleRatsnest = m_showSelectedRatsnest->GetValue();

        cfg->m_RotationAngle = wxRound( 10.0 * wxAtof( m_rotationAngle->GetValue() ) );

        cfg->m_MagneticItems.pads = static_cast<MAGNETIC_OPTIONS>( m_magneticPadChoice->GetSelection() );
        cfg->m_MagneticItems.tracks = static_cast<MAGNETIC_OPTIONS>( m_magneticTrackChoice->GetSelection() );
        cfg->m_MagneticItems.graphics = !m_magneticGraphicsChoice->GetSelection();

        cfg->m_FlipLeftRight = m_flipLeftRight->GetValue();
        cfg->m_AutoRefillZones = m_autoRefillZones->GetValue();
        cfg->m_AllowFreePads = m_allowFreePads->GetValue();
        cfg->m_ShowPageLimits = m_showPageLimits->GetValue();

        if( m_rbTrackDragMove->GetValue() )
            cfg->m_TrackDragAction = TRACK_DRAG_ACTION::MOVE;
        else if( m_rbTrackDrag45->GetValue() )
            cfg->m_TrackDragAction = TRACK_DRAG_ACTION::DRAG;
        else if( m_rbTrackDragFree->GetValue() )
            cfg->m_TrackDragAction = TRACK_DRAG_ACTION::DRAG_FREE_ANGLE;

        cfg->m_Use45DegreeLimit = m_cbPcbGraphic45Mode->GetValue();
    }

    return true;
}


