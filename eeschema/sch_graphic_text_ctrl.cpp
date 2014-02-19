/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
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
 * @file wxwineda.cpp
 */

#include <fctsys.h>
#include <wxstruct.h>
#include <dialog_helpers.h>
#include <base_units.h>
#include <macros.h>
#include <sch_graphic_text_ctrl.h>


/*******************************************************/
/* Class to edit a graphic + text size in INCHES or MM */
/*******************************************************/
SCH_GRAPHIC_TEXT_CTRL::SCH_GRAPHIC_TEXT_CTRL( wxWindow*       parent,
                                              const wxString& Title,
                                              const wxString& TextToEdit,
                                              int             textsize,
                                              EDA_UNITS_T     user_unit,
                                              wxBoxSizer*     BoxSizer,
                                              int             framelen )
{
    m_UserUnit = user_unit;
    m_Title = NULL;
    m_Title = new wxStaticText( parent, -1, Title );

    BoxSizer->Add( m_Title, 0, wxGROW | wxLEFT | wxRIGHT | wxTOP, 5 );

    m_FrameText = new wxTextCtrl( parent, -1, TextToEdit );

    BoxSizer->Add( m_FrameText, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 5 );

    if( !Title.IsEmpty() )
    {
        wxString      msg;
        msg.Printf( _( "Size%s" ), GetChars( ReturnUnitSymbol( m_UserUnit ) ) );
        wxStaticText* text = new wxStaticText( parent, -1, msg );

        BoxSizer->Add( text, 0, wxGROW | wxLEFT | wxRIGHT, 5 );
    }

    wxString value = FormatSize( m_UserUnit, textsize );

    m_FrameSize = new wxTextCtrl( parent, -1, value, wxDefaultPosition, wxSize( 70, -1 ) );

    BoxSizer->Add( m_FrameSize, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
}


SCH_GRAPHIC_TEXT_CTRL::~SCH_GRAPHIC_TEXT_CTRL()
{
    /* no, these are deleted by the BoxSizer
    delete m_FrameText;
    delete m_Title;
    */
}


wxString SCH_GRAPHIC_TEXT_CTRL::FormatSize( EDA_UNITS_T aUnit, int textSize )
{
    // Limiting the size of the text of reasonable values.
    if( textSize < 10 )
        textSize = 10;

    if( textSize > 3000 )
        textSize = 3000;

    SCH_UNITS u;
    u.SetUserUnit (aUnit);

    return u.StringFromValue( textSize );
}


void SCH_GRAPHIC_TEXT_CTRL::SetTitle( const wxString& title )
{
    m_Title->SetLabel( title );
}


void SCH_GRAPHIC_TEXT_CTRL::SetValue( const wxString& value )
{
    m_FrameText->SetValue( value );
}


void SCH_GRAPHIC_TEXT_CTRL::SetValue( int textSize )
{
    wxString value = FormatSize( m_UserUnit, textSize );
    m_FrameSize->SetValue( value );
}


const wxString SCH_GRAPHIC_TEXT_CTRL::GetText() const
{
    wxString text = m_FrameText->GetValue();
    return text;
}


int SCH_GRAPHIC_TEXT_CTRL::ParseSize( const wxString& sizeText, EDA_UNITS_T aUnit )
{
    int    textsize;

    SCH_UNITS u;
    u.SetUserUnit (aUnit);

    textsize = u.ValueFromString( sizeText );

    // Limit to reasonable size
    if( textsize < 10 )
        textsize = 10;

    if( textsize > 3000 )
        textsize = 3000;

    return textsize;
}


int SCH_GRAPHIC_TEXT_CTRL::GetTextSize()
{
    return ParseSize( m_FrameSize->GetValue(), m_UserUnit );
}


void SCH_GRAPHIC_TEXT_CTRL::Enable( bool state )
{
    m_FrameText->Enable( state );
}


