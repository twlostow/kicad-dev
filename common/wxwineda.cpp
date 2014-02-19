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
/********************************************************/
/* Class to display and edit a coordinated INCHES or MM */
/********************************************************/
EDA_POSITION_CTRL::EDA_POSITION_CTRL( wxWindow*       parent,
                                      const wxString& title,
                                      const wxPoint&  pos_to_edit,
                                      EDA_UNITS_T     user_unit,
                                      UNITS*          appUnits,
                                      wxBoxSizer*     BoxSizer )
{
    wxString text;

    m_UserUnit = user_unit;
    SetUnits (appUnits);
    
    if( title.IsEmpty() )
        text = _( "Pos " );
    else
        text = title;

    text   += _( "X" ) + ReturnUnitSymbol( m_UserUnit );
    m_TextX = new wxStaticText( parent, -1, text );

    BoxSizer->Add( m_TextX, 0, wxGROW | wxLEFT | wxRIGHT | wxTOP, 5 );
    m_FramePosX = new wxTextCtrl( parent, -1, wxEmptyString, wxDefaultPosition );

    BoxSizer->Add( m_FramePosX, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 5 );


    if( title.IsEmpty() )
        text = _( "Pos " );
    else
        text = title;
    text   += _( "Y" ) + ReturnUnitSymbol( m_UserUnit );

    m_TextY = new wxStaticText( parent, -1, text );

    BoxSizer->Add( m_TextY, 0, wxGROW | wxLEFT | wxRIGHT | wxTOP, 5 );

    m_FramePosY = new wxTextCtrl( parent, -1, wxEmptyString );

    BoxSizer->Add( m_FramePosY, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 5 );

    SetValue( pos_to_edit.x, pos_to_edit.y );
}


EDA_POSITION_CTRL::~EDA_POSITION_CTRL()
{
    delete m_TextX;
    delete m_TextY;
    delete m_FramePosX;
    delete m_FramePosY;
}


/* Returns (in internal units) to coordinate between (in user units)
 */
wxPoint EDA_POSITION_CTRL::GetValue()
{
    wxPoint coord;

    coord.x = Units()->ValueFromString( m_FramePosX->GetValue() );
    coord.y = Units()->ValueFromString( m_FramePosY->GetValue() );

    return coord;
}


void EDA_POSITION_CTRL::Enable( bool x_win_on, bool y_win_on )
{
    m_FramePosX->Enable( x_win_on );
    m_FramePosY->Enable( y_win_on );
}


void EDA_POSITION_CTRL::SetValue( int x_value, int y_value )
{
    wxString msg;

    m_Pos_To_Edit.x = x_value;
    m_Pos_To_Edit.y = y_value;

    msg = Units()->StringFromValue( m_UserUnit, m_Pos_To_Edit.x );
    m_FramePosX->Clear();
    m_FramePosX->SetValue( msg );

    msg = Units()->StringFromValue( m_UserUnit, m_Pos_To_Edit.y );
    m_FramePosY->Clear();
    m_FramePosY->SetValue( msg );
}


/*******************/
/* EDA_SIZE_CTRL */
/*******************/
EDA_SIZE_CTRL::EDA_SIZE_CTRL( wxWindow* parent, const wxString& title,
                              const wxSize& size_to_edit, EDA_UNITS_T aUnit, UNITS *appUnits,
                              wxBoxSizer* aBoxSizer ) :
    EDA_POSITION_CTRL( parent, title, wxPoint( size_to_edit.x, size_to_edit.y ),
                       aUnit, appUnits, aBoxSizer )
{
}


wxSize EDA_SIZE_CTRL::GetValue()
{
    wxPoint pos = EDA_POSITION_CTRL::GetValue();
    wxSize  size;

    size.x = pos.x;
    size.y = pos.y;
    return size;
}


/**************************************************************/
/* Class to display and edit a dimension INCHES, MM, or other */
/**************************************************************/
EDA_VALUE_CTRL::EDA_VALUE_CTRL( wxWindow* parent, const wxString& title,
                                int value, EDA_UNITS_T user_unit, UNITS *appUnits, wxBoxSizer* BoxSizer )
{
    wxString label = title;

    m_UserUnit = user_unit;
    m_Value = value;
    SetUnits (appUnits);
    
    label  += ReturnUnitSymbol( m_UserUnit );

    m_Text = new wxStaticText( parent, -1, label );

    BoxSizer->Add( m_Text, 0, wxGROW | wxLEFT | wxRIGHT | wxTOP, 5 );

    wxString stringvalue = Units()->StringFromValue( m_UserUnit, m_Value );
    m_ValueCtrl = new wxTextCtrl( parent, -1, stringvalue );

    BoxSizer->Add( m_ValueCtrl,
                   0,
                   wxGROW | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,
                   5 );
}


EDA_VALUE_CTRL::~EDA_VALUE_CTRL()
{
    delete m_ValueCtrl;
    delete m_Text;
}


int EDA_VALUE_CTRL::GetValue()
{
    int      coord;
    wxString txtvalue = m_ValueCtrl->GetValue();

    coord = Units()->ValueFromString( txtvalue );
    return coord;
}


void EDA_VALUE_CTRL::SetValue( int new_value )
{
    wxString buffer;

    m_Value = new_value;

    buffer = Units()->StringFromValue( m_Value );
    m_ValueCtrl->SetValue( buffer );
}


void EDA_VALUE_CTRL::Enable( bool enbl )
{
    m_ValueCtrl->Enable( enbl );
    m_Text->Enable( enbl );
}
