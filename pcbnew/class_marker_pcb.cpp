/**
 * @file class_marker_pcb.cpp
 * @brief Functions to handle markers used to show something (usually a drc problem)
 */

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

#include <fctsys.h>
#include <gr_basic.h>
#include <class_drawpanel.h>
#include <wxstruct.h>
#include <trigo.h>
#include <msgpanel.h>

#include <pcbnew.h>
#include <drc_stuff.h>
#include <class_marker_pcb.h>
#include <layers_id_colors_and_visibility.h>


/// Adjust the actual size of markers, when using default shape
#define SCALING_FACTOR_DMILS      30


MARKER_PCB::MARKER_PCB( BOARD_ITEM* aParent ) :
    BOARD_ITEM( aParent, PCB_MARKER_T ),
    MARKER_BASE( )
{
    m_Color = WHITE;
    m_ScalingFactor = g_PcbUnits.DMilsToIu ( SCALING_FACTOR_DMILS );
}


MARKER_PCB::MARKER_PCB( int aErrorCode, const wxPoint& aMarkerPos,
                        const wxString& aText, const wxPoint& aPos,
                        const wxString& bText, const wxPoint& bPos ) :
    BOARD_ITEM( NULL, PCB_MARKER_T ),  // parent set during BOARD::Add()
    MARKER_BASE( aErrorCode, aMarkerPos, aText, aPos, bText, bPos )

{
    m_Color = WHITE;
    m_ScalingFactor = g_PcbUnits.DMilsToIu ( SCALING_FACTOR_DMILS );
}

MARKER_PCB::MARKER_PCB( int aErrorCode, const wxPoint& aMarkerPos,
                        const wxString& aText, const wxPoint& aPos ) :
    BOARD_ITEM( NULL, PCB_MARKER_T ),  // parent set during BOARD::Add()
    MARKER_BASE( aErrorCode, aMarkerPos, aText,  aPos )
{
    m_Color = WHITE;
    m_ScalingFactor = g_PcbUnits.DMilsToIu ( SCALING_FACTOR_DMILS );
}


/* destructor */
MARKER_PCB::~MARKER_PCB()
{
}

/* tests to see if this object is on the given layer.
 * DRC markers are not really on a copper layer, but
 * MARKER_PCB::IsOnCopperLayer return true if aLayer is a cooper layer,
 * because this test is often used to locad a marker
 * param aLayer The layer to test for.
 * return bool - true if on given layer, else false.
 */
bool MARKER_PCB::IsOnLayer( LAYER_NUM aLayer ) const
{
    return IsCopperLayer( aLayer );
}

void MARKER_PCB::GetMsgPanelInfo( std::vector< MSG_PANEL_ITEM >& aList )
{
    const DRC_ITEM& rpt = m_drc;

    aList.push_back( MSG_PANEL_ITEM( _( "Type" ), _( "Marker" ), DARKCYAN ) );

    wxString errorTxt;

    errorTxt.Printf( _( "ErrType (%d)- %s:" ),
            rpt.GetErrorCode(),
            GetChars( getErrorText() ) );

    aList.push_back( MSG_PANEL_ITEM( errorTxt, wxEmptyString, RED ) );

    wxString txtA;
    txtA << g_PcbUnits.PointToString( rpt.GetPointA() ) << wxT( ": " ) << rpt.GetTextA();

    wxString txtB;

    if ( rpt.HasSecondItem() )
        txtB << g_PcbUnits.PointToString( rpt.GetPointB() ) << wxT( ": " ) << rpt.GetTextB();

    aList.push_back( MSG_PANEL_ITEM( txtA, txtB, DARKBROWN ) );
}


void MARKER_PCB::Rotate(const wxPoint& aRotCentre, double aAngle)
{
    RotatePoint( &m_Pos, aRotCentre, aAngle );
}


void MARKER_PCB::Flip(const wxPoint& aCentre )
{
    m_Pos.y  = aCentre.y - (m_Pos.y - aCentre.y);
}


wxString MARKER_PCB::GetSelectMenuText() const
{
    wxString text;
    text.Printf( _( "Marker @(%d,%d)" ), GetPos().x, GetPos().y );

    return text;
}

/**
 * Function ShowHtml
 * translates this object into a fragment of HTML suitable for the
 * wxWidget's wxHtmlListBox class.
 * @return wxString - the html text.
 */

wxString MARKER_PCB::ShowHtml() const
{
    return MARKER_PCB::showHtml( GetReporter() );
}


wxString MARKER_PCB::showHtml(const DRC_ITEM& item) 
{
    wxString ret;
    wxString mainText = item.GetTextA();
    // a wxHtmlWindows does not like < and > in the text to display
    // because these chars have a special meaning in html
    mainText.Replace( wxT("<"), wxT("&lt;") );
    mainText.Replace( wxT(">"), wxT("&gt;") );

    wxString errText = MARKER_PCB::getErrorText(item.GetErrorCode());
    errText.Replace( wxT("<"), wxT("&lt;") );
    errText.Replace( wxT(">"), wxT("&gt;") );


        if( item.ShowNoCoordinate() )
        {
            // omit the coordinate, a NETCLASS has no location
            ret.Printf( _( "ErrType(%d): <b>%s</b><ul><li> %s </li></ul>" ),
                        item.GetErrorCode(),
                        GetChars( errText ),
                        GetChars( mainText ) );
        }
        else if( item.HasSecondItem() )
        {
            wxString auxText = item.GetAuxiliaryText();
            auxText.Replace( wxT("<"), wxT("&lt;") );
            auxText.Replace( wxT(">"), wxT("&gt;") );

            // an html fragment for the entire message in the listbox.  feel free
            // to add color if you want:
            ret.Printf( _( "ErrType(%d): <b>%s</b><ul><li> %s: %s </li><li> %s: %s </li></ul>" ),
                        item.GetErrorCode(),
                        GetChars( errText ),
                        GetChars( g_PcbUnits.PointToString( item.GetPointA() )), GetChars( mainText ),
                        GetChars( g_PcbUnits.PointToString( item.GetPointB() )), GetChars( auxText ) );
        }
        else
        {
            ret.Printf( _( "ErrType(%d): <b>%s</b><ul><li> %s: %s </li></ul>" ),
                        item.GetErrorCode(),
                        GetChars( errText ),
                        GetChars( g_PcbUnits.PointToString( item.GetPointA() ) ), GetChars( mainText ) );
        }

        return ret;
    }

wxString MARKER_PCB::ShowReport( ) const
{
    return MARKER_PCB::showReport( GetReporter() );
}

wxString MARKER_PCB::showReport( const DRC_ITEM& item ) 
{
    
    wxString ret;
    if( item.HasSecondItem() )
    {
        ret.Printf( wxT( "ErrType(%d): %s\n    %s: %s\n    %s: %s\n" ),
                    item.GetErrorCode(),
                    GetChars( MARKER_PCB::getErrorText(item.GetErrorCode()) ),
                    GetChars( g_PcbUnits.PointToString( item.GetPointA() ) ), GetChars( item.GetMainText() ),
                    GetChars( g_PcbUnits.PointToString( item.GetPointB() ) ), GetChars( item.GetAuxiliaryText() ) );
    } else {
        ret.Printf( wxT( "ErrType(%d): %s\n    %s: %s\n" ),
                    item.GetErrorCode(),
                    GetChars( MARKER_PCB::getErrorText(item.GetErrorCode()) ),
                    GetChars( g_PcbUnits.PointToString( item.GetPointA() ) ), GetChars( item.GetMainText() ) );
    }
    return ret;
}

wxString MARKER_PCB::getErrorText() const
{
    return MARKER_PCB::getErrorText ( GetReporter().GetErrorCode() );
}

wxString MARKER_PCB::getErrorText( int aErrorCode ) 
{
    switch( aErrorCode )
    {
    case DRCE_UNCONNECTED_PADS:
        return wxString( _("Unconnected pads") );
    case DRCE_TRACK_NEAR_THROUGH_HOLE:
        return wxString( _("Track near thru-hole") );
    case DRCE_TRACK_NEAR_PAD:
        return wxString( _("Track near pad") );
    case DRCE_TRACK_NEAR_VIA:
        return wxString( _("Track near via") );
    case DRCE_VIA_NEAR_VIA:
        return wxString( _("Via near via") );
    case DRCE_VIA_NEAR_TRACK:
        return wxString( _("Via near track") );
    case DRCE_TRACK_ENDS1:
    case DRCE_TRACK_ENDS2:
    case DRCE_TRACK_ENDS3:
    case DRCE_TRACK_ENDS4:
    case DRCE_ENDS_PROBLEM1:
    case DRCE_ENDS_PROBLEM2:
    case DRCE_ENDS_PROBLEM3:
    case DRCE_ENDS_PROBLEM4:
    case DRCE_ENDS_PROBLEM5:
        return wxString( _("Two track ends too close") );
    case DRCE_TRACK_SEGMENTS_TOO_CLOSE:
        return wxString( _("Two parallel track segments too close") );
    case DRCE_TRACKS_CROSSING:
        return wxString( _("Tracks crossing") );
    case DRCE_PAD_NEAR_PAD1:
        return wxString( _("Pad near pad") );
    case DRCE_VIA_HOLE_BIGGER:
        return wxString( _("Via hole > diameter"));
    case DRCE_MICRO_VIA_INCORRECT_LAYER_PAIR:
        return wxString( _("Micro Via: incorrect layer pairs (not adjacent)"));
    case COPPERAREA_INSIDE_COPPERAREA:
        return wxString( _("Copper area inside copper area"));
    case COPPERAREA_CLOSE_TO_COPPERAREA:
        return wxString( _("Copper areas intersect or are too close"));
    case DRCE_NON_EXISTANT_NET_FOR_ZONE_OUTLINE:
        return wxString( _("Copper area has a nonexistent net name"));
    case DRCE_HOLE_NEAR_PAD:
        return wxString( _("Hole near pad"));
    case DRCE_HOLE_NEAR_TRACK:
        return wxString( _("Hole near track"));
    case DRCE_TOO_SMALL_TRACK_WIDTH:
        return wxString( _("Too small track width"));
    case DRCE_TOO_SMALL_VIA:
        return wxString( _("Too small via size"));
    case DRCE_TOO_SMALL_MICROVIA:
        return wxString( _("Too small micro via size"));

    // use &lt; since this is text ultimately embedded in HTML
    case DRCE_NETCLASS_TRACKWIDTH:
        return wxString( _("NetClass Track Width &lt; global limit"));
    case DRCE_NETCLASS_CLEARANCE:
        return wxString( _("NetClass Clearance &lt; global limit"));
    case DRCE_NETCLASS_VIASIZE:
        return wxString( _("NetClass Via Dia &lt; global limit"));
    case DRCE_NETCLASS_VIADRILLSIZE:
        return wxString( _("NetClass Via Drill &lt; global limit"));
    case DRCE_NETCLASS_uVIASIZE:
        return wxString( _("NetClass uVia Dia &lt; global limit"));
    case DRCE_NETCLASS_uVIADRILLSIZE:
        return wxString( _("NetClass uVia Drill &lt; global limit"));

    case DRCE_VIA_INSIDE_KEEPOUT:
        return wxString( _("Via inside a keepout area"));

    case DRCE_TRACK_INSIDE_KEEPOUT:
        return wxString( _("Track inside a keepout area"));

    case DRCE_PAD_INSIDE_KEEPOUT:
        return wxString( _("Pad inside a keepout area"));

    default:
        {
            wxString msg;
            msg.Printf( wxT( "Unknown DRC error code %d" ), aErrorCode );
            return ( msg );
        }
    }
}
