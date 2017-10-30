/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014-2015 CERN
 * Author: Maciej Suminski <maciej.suminski@cern.ch>
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

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/textentry.h>
#include <limits>
#include <base_units.h>
#include <wx/valnum.h>

#include <boost/optional.hpp>

#include "wx_unit_binder.h"

WX_UNIT_BINDER::WX_UNIT_BINDER( wxWindow* aParent, wxTextEntry* aTextInput,
                                wxStaticText* aUnitLabel, wxSpinButton* aSpinButton, EDA_UNITS_T aForceUnits ) :
    m_textEntry( aTextInput ),
    m_unitLabel( aUnitLabel ),
    m_units( g_UserUnit ),
    m_step( 1 ),
    m_min( 0 ),
    m_max( 1 )
{

    m_hasValue = false;

    if (aForceUnits != DEFAULT_UNITS)
        m_units = aForceUnits;
    else
        m_units = g_UserUnit;

    // Use the currently selected units
    m_textEntry->SetValue( wxT( "0" ) );
    m_unitLabel->SetLabel( GetAbbreviatedUnitsLabel( m_units ) );
}


WX_UNIT_BINDER::~WX_UNIT_BINDER()
{
}


void WX_UNIT_BINDER::SetValue( int aValue )
{
    wxString s = StringFromValue( m_units, aValue, false );

    m_textEntry->SetValue( s );

    m_unitLabel->SetLabel( GetAbbreviatedUnitsLabel( m_units ) );
    m_hasValue = true;
}

void WX_UNIT_BINDER::SetValue( double aValue )
{
    wxString s = StringFromValue( m_units, aValue, false );

    m_textEntry->SetValue( s );

    m_unitLabel->SetLabel( GetAbbreviatedUnitsLabel( m_units ) );
    m_hasValue = true;
}

///> Sets wxTextCtrl to the value stored in boost::optional<T> or "<...>" if it is not available.
template<typename T>
    void setCommonVal( const boost::optional<T>& aVal, wxTextEntry* aTxtEntry, WX_UNIT_BINDER& aBinder )
{
    if( aVal )
        aBinder.SetValue( *aVal );
    else
        aTxtEntry->SetValue( "<...>" );
}

void WX_UNIT_BINDER::SetValue( boost::optional<int> aValue )
{
    if ( !aValue )
    {
        m_textEntry->SetValue( "<...>" );
        m_hasValue = false;
    }
    else
    {
        SetValue ( *aValue );
    }
}

void WX_UNIT_BINDER::SetValue( boost::optional<double> aValue )
{
    if ( !aValue )
    {
        m_textEntry->SetValue( "<...>" );
        m_hasValue = false;
    }
    else
    {
        SetValue ( *aValue );
    }
}

bool WX_UNIT_BINDER::HasValue() const
{
    return m_hasValue;
}

int WX_UNIT_BINDER::GetValue() const
{
    wxString s = m_textEntry->GetValue();

    return ValueFromString( m_units, s );
}

double WX_UNIT_BINDER::GetValueDbl() const
{
    wxString s = m_textEntry->GetValue();

    return ValueFromString( m_units, s );
}

bool WX_UNIT_BINDER::Valid() const
{
    double dummy;

    return m_textEntry->GetValue().ToDouble( &dummy );
}

void WX_UNIT_BINDER::Enable( bool aEnable )
{
    wxWindow* wxWin = dynamic_cast<wxWindow*> ( m_textEntry );
    wxWin->Enable( aEnable );
    m_unitLabel->Enable( aEnable );
}

