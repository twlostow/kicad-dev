/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
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
 * @file sch_marker.cpp
 * @brief Class SCH_MARKER implementation
 */

#include <fctsys.h>
#include <wxstruct.h>
#include <class_drawpanel.h>
#include <trigo.h>
#include <msgpanel.h>
#include <html_messagebox.h>

#include <general.h>
#include <sch_marker.h>
#include <erc.h>


/* Marker are mainly used to show an ERC error
 * but they could be used to give a specific info
 */


const wxChar* NameMarqueurType[] =
{
    wxT( "" ),
    wxT( "ERC" ),
    wxT( "PCB" ),
    wxT( "SIMUL" ),
    wxT( "???" )
};


/********************/
/* class SCH_MARKER */
/********************/

SCH_MARKER::SCH_MARKER() : SCH_ITEM( NULL, SCH_MARKER_T ), MARKER_BASE()
{
}


SCH_MARKER::SCH_MARKER( const wxPoint& pos, const wxString& text ) :
    SCH_ITEM( NULL, SCH_MARKER_T ),
    MARKER_BASE( 0, pos, text, pos )
{
}


SCH_MARKER::~SCH_MARKER()
{
}


EDA_ITEM* SCH_MARKER::Clone() const
{
    return new SCH_MARKER( *this );
}


#if defined(DEBUG)

void SCH_MARKER::Show( int nestLevel, std::ostream& os ) const
{
    // for now, make it look like XML:
    NestedSpace( nestLevel, os ) << '<' << GetClass().Lower().mb_str()
                                 << GetPos() << "/>\n";
}

#endif

/**
 * Function Save (do nothing : markers are no more saved in files )
 * writes the data structures for this object out to a FILE in "*.brd" format.
 * @param aFile The FILE to write to.
 * @return bool - true if success writing else false.
 */
bool SCH_MARKER::Save( FILE* aFile ) const
{
    return true;
}


void SCH_MARKER::Draw( EDA_DRAW_PANEL* aPanel, wxDC* aDC,
                       const wxPoint& aOffset, GR_DRAWMODE aDrawMode, EDA_COLOR_T aColor )
{
    EDA_COLOR_T color = m_Color;
    EDA_COLOR_T tmp   = color;

    if( GetMarkerType() == MARK_ERC )
    {
        color = ( GetErrorLevel() == WAR ) ? GetLayerColor( LAYER_ERC_WARN ) :
                                             GetLayerColor( LAYER_ERC_ERR );
    }

    if( aColor < 0 )
        m_Color = color;
    else
        m_Color = aColor;

    DrawMarker( aPanel, aDC, aDrawMode, aOffset );
    m_Color = tmp;
}


bool SCH_MARKER::Matches( wxFindReplaceData& aSearchData, wxPoint * aFindLocation )
{
    if( !SCH_ITEM::Matches( m_drc.GetMainText(), aSearchData ) )
    {
        if( SCH_ITEM::Matches( m_drc.GetAuxiliaryText(), aSearchData ) )
        {
            if( aFindLocation )
                *aFindLocation = m_Pos;
            return true;
        }
        return false;
    }

    if( aFindLocation )
        *aFindLocation = m_Pos;
    return true;
}


/**
 * Function GetBoundingBox
 * returns the orthogonal, bounding box of this object for display purposes.
 * This box should be an enclosing perimeter for visible components of this
 * object, and the units should be in the pcb or schematic coordinate system.
 * It is OK to overestimate the size by a few counts.
 */
const EDA_RECT SCH_MARKER::GetBoundingBox() const
{
    return GetBoundingBoxMarker();
}


void SCH_MARKER::GetMsgPanelInfo( MSG_PANEL_ITEMS& aList )
{
    wxString msg;

    aList.push_back( MSG_PANEL_ITEM( _( "Electrical Rule Check error" ),
                                     getErrorText(), DARKRED ) );
}


void SCH_MARKER::Rotate( wxPoint aPosition )
{
    RotatePoint( &m_Pos, aPosition, 900 );
}


void SCH_MARKER::MirrorX( int aXaxis_position )
{
    m_Pos.y -= aXaxis_position;
    m_Pos.y  = -m_Pos.y;
    m_Pos.y += aXaxis_position;
}


void SCH_MARKER::MirrorY( int aYaxis_position )
{
    m_Pos.x -= aYaxis_position;
    m_Pos.x  = -m_Pos.x;
    m_Pos.x += aYaxis_position;
}


bool SCH_MARKER::IsSelectStateChanged( const wxRect& aRect )
{
    bool previousState = IsSelected();

    if( aRect.Contains( m_Pos ) )
        SetFlags( SELECTED );
    else
        ClearFlags( SELECTED );

    return previousState != IsSelected();
}


bool SCH_MARKER::HitTest( const wxPoint& aPosition, int aAccuracy ) const
{
    return HitTestMarker( aPosition );
}

wxString SCH_MARKER::getErrorText() const
{
    switch( GetReporter().GetErrorCode() )
    {
    case ERCE_UNSPECIFIED:
        return wxString( _("Unspecified ERC error") );
    case ERCE_DUPLICATE_SHEET_NAME:
        return wxString( _("Duplicate sheet names within a given sheet") );
    case ERCE_PIN_NOT_CONNECTED:
        return wxString( _("Pin not connected (and no \"No Connect\" symbol found)") );
    case ERCE_PIN_NOT_DRIVEN:
        return wxString( _("Pin connected to some others pins but no pin to drive it") );
    case ERCE_PIN_TO_PIN_WARNING:
        return wxString( _("Conflicting pin types (severity: warning)") );
    case ERCE_PIN_TO_PIN_ERROR:
        return wxString( _("Conflicting pin types (severity: error") );
    case ERCE_HIERACHICAL_LABEL:
        return wxString( _("Mismatch between hierarchical labels and sheet pins"));
    case ERCE_NOCONNECT_CONNECTED:
        return wxString( _("A No Connect symbol is connected to more than one pin"));

    default:
        return wxString( wxT("Unknown") );
    }
}

/**
 * Function ShowHtml
 * translates this object into a fragment of HTML suitable for the
 * wxWidget's wxHtmlListBox class.
 * @return wxString - the html text.
 */

wxString SCH_MARKER::ShowHtml() const
{
    const DRC_ITEM& item = GetReporter();
    wxString ret;
    wxString mainText = item.GetTextA();
    // a wxHtmlWindows does not like < and > in the text to display
    // because these chars have a special meaning in html
    mainText.Replace( wxT("<"), wxT("&lt;") );
    mainText.Replace( wxT(">"), wxT("&gt;") );

        wxString errText = getErrorText();
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
                        GetChars( g_SchUnits.PointToString( item.GetPointA() )), GetChars( mainText ),
                        GetChars( g_SchUnits.PointToString( item.GetPointB() )), GetChars( auxText ) );
        }
        else
        {
            ret.Printf( _( "ErrType(%d): <b>%s</b><ul><li> %s: %s </li></ul>" ),
                        item.GetErrorCode(),
                        GetChars( errText ),
                        GetChars( g_SchUnits.PointToString( item.GetPointA() ) ), GetChars( mainText ) );
        }

        return ret;
    }

wxString SCH_MARKER::ShowReport( ) const
{
    const DRC_ITEM& item = GetReporter();

    wxString ret;
    if( item.HasSecondItem() )
    {
        ret.Printf( wxT( "ErrType(%d): %s\n    %s: %s\n    %s: %s\n" ),
                    item.GetErrorCode(),
                    GetChars( getErrorText() ),
                    GetChars( g_SchUnits.PointToString( item.GetPointA() ) ), GetChars( item.GetMainText() ),
                    GetChars( g_SchUnits.PointToString( item.GetPointB() ) ), GetChars( item.GetAuxiliaryText() ) );
    } else {
        ret.Printf( wxT( "ErrType(%d): %s\n    %s: %s\n" ),
                    item.GetErrorCode(),
                    GetChars( getErrorText() ),
                    GetChars( g_SchUnits.PointToString( item.GetPointA() ) ), GetChars( item.GetMainText() ) );
    }
    return ret;
}

void SCH_MARKER::DisplayMarkerInfo( EDA_DRAW_FRAME* aFrame )
{
    DIALOG_DISPLAY_HTML_TEXT_BASE infodisplay( (wxWindow*)aFrame, wxID_ANY, _( "Marker Info" ),
                                               wxGetMousePosition(), wxSize( 550, 140 ) );

    infodisplay.m_htmlWindow->SetPage( ShowHtml() );
    infodisplay.ShowModal();
}
