/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012-2014 CERN
 * Copyright (C) 1992-2011 KiCad Developers, see change_log.txt for contributors.
 *
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
 * @author Wayne Stambaugh <stambaughw@verizon.net>
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 * @file base_units.cpp
 * @brief Code to handle objects that require both schematic and board internal units.
 * @note This file is an a less uglier hack to solve the problem of formatting the base units
 *       for either schematics or boards in objects that are include in both domains.
 */

#include <macros.h>
#include <base_struct.h>
#include <class_title_block.h>
#include <common.h>
#include <base_units.h>

PCB_UNITS g_PcbUnits;
SCH_UNITS g_SchUnits;
GERBVIEW_UNITS g_GerbviewUnits;
PL_EDITOR_UNITS g_PLEditorUnits;

// Helper function to print a float number without using scientific notation
// and no trailing 0
// So we cannot always just use the %g or the %f format to print a fp number
// this helper function uses the %f format when needed, or %g when %f is
// not well working and then removes trailing 0

std::string Double2Str( double aValue )
{
    char    buf[50];
    int     len;

    if( aValue != 0.0 && fabs( aValue ) <= 0.0001 )
    {
        // For these small values, %f works fine,
        // and %g gives an exponent
        len = sprintf( buf,  "%.16f", aValue );

        while( --len > 0 && buf[len] == '0' )
            buf[len] = '\0';

        if( buf[len] == '.' )
            buf[len] = '\0';
        else
            ++len;
    }
    else
    {
        // For these values, %g works fine, and sometimes %f
        // gives a bad value (try aValue = 1.222222222222, with %.16f format!)
        len = sprintf( buf, "%.16g", aValue );
    }

    return std::string( buf, len );;
}

/* Remove trailing 0 from a string containing a converted float number.
 * the trailing 0 are removed if the mantissa has more
 * than aTrailingZeroAllowed digits and some trailing 0
 */
void StripTrailingZeros( wxString& aStringValue, unsigned aTrailingZeroAllowed )
{
    struct lconv * lc = localeconv();
    char sep = lc->decimal_point[0];
    unsigned sep_pos = aStringValue.Find( sep );

    if( sep_pos > 0 )
    {
        // We want to keep at least aTrailingZeroAllowed digits after the separator
        unsigned min_len = sep_pos + aTrailingZeroAllowed + 1;

        while( aStringValue.Len() > min_len )
        {
            if( aStringValue.Last() == '0' )
                aStringValue.RemoveLast();
            else
                break;
        }
    }
}

/**
 * Function AngleToStringDegrees
 * is a helper to convert the \a double \a aAngle (in internal unit)
 * to a string in degrees
 */

wxString AngleToStringDegrees( double aAngle )
{
    wxString text;

    text.Printf( wxT( "%.3f" ), aAngle/10.0 );
    StripTrailingZeros( text, 1 );

    return text;
}


double UNITS::FromUser ( double aValue )const
{
    return FromUser ( m_userUnit, aValue );
}

double UNITS::FromUser ( EDA_UNITS_T aUnits, double aValue )const
{
    double value;

    switch ( aUnits )
    {
    case MILLIMETRES:
        value = aValue * m_IUPerMm;
        break;

    case INCHES:
        value = aValue * m_IUPerMils * 1000.0;
        break;

    default:
    case UNSCALED_UNITS:
        value = aValue;
    }

    return value;
}

double UNITS::ToUser ( EDA_UNITS_T aUnits, double aValue )const
{
    switch( aUnits )
    {
        case MILLIMETRES:
            return aValue / m_IUPerMm;

        case INCHES:
            return aValue / m_IUPerMils / 1000.0;

    default:
        return aValue;
    }
}

double UNITS::ToUser ( double aValue )const
{
    return ToUser(m_userUnit, aValue);
}

wxString UNITS::CoordinateToString( int aValue, bool aConvertToMils )const
{
    return LengthToString( (double) aValue, aConvertToMils );
}

wxString UNITS::LengthToString( double aValue, bool aConvertToMils )const
{
    wxString      text;
    const wxChar* format;
    double        value = ToUser ( aValue );

    if( m_userUnit == INCHES )
    {
        if( aConvertToMils )
        {
            if( m_app == APP_EESCHEMA_T )
                format = wxT( "%.0f" );
            else
                format = wxT( "%.1f" );
            value *= 1000;
        }
        else
        {
            if( m_app == APP_EESCHEMA_T )
                format = wxT( "%.3f" );
            else
                format = wxT( "%.4f" );
        }
    }
    else
    {
            if( m_app == APP_EESCHEMA_T )
                format = wxT( "%.2f" );
            else
                format = wxT( "%.3f" );
    }

    text.Printf( format, value );

    if( m_userUnit == INCHES )
        text += ( aConvertToMils ) ? _( " mils" ) : _( " in" );
    else
        text += _( " mm" );

    return text;
}
        
wxString UNITS::StringFromValue( int aValue, bool aAddUnitSymbol )const
{
    double  value_to_print = ToUser( aValue );
    wxString stringValue;
    
    if( m_app == APP_EESCHEMA_T )
    {
        stringValue = wxString::Format( wxT( "%.3f" ), value_to_print );

        // Strip trailing zeros. However, keep at least 3 digits in mantissa
        // For readability
        StripTrailingZeros( stringValue, 3 );
    } else {
        char    buf[50];
        int     len;

        if( value_to_print != 0.0 && fabs( value_to_print ) <= 0.0001 )
        {
            len = sprintf( buf, "%.10f", value_to_print );

            while( --len > 0 && buf[len] == '0' )
                buf[len] = '\0';

            if( buf[len]=='.' || buf[len]==',' )
                buf[len] = '\0';
            else
                ++len;
        }
        else
        {
            len = sprintf( buf, "%.10g", value_to_print );
        }

        stringValue = wxString ( buf, wxConvUTF8 );
    }

    if( aAddUnitSymbol )
    {
        switch( m_userUnit )
        {
        case INCHES:
            stringValue += _( " \"" );
            break;

        case MILLIMETRES:
            stringValue += _( " mm" );
            break;

        case UNSCALED_UNITS:
            break;
        }
    }

    return stringValue;
}

int UNITS::ValueFromString( const wxString& aTextValue ) const
{

    double value;
    double dtmp = 0;

    // Acquire the 'right' decimal point separator
    const struct lconv* lc = localeconv();

    wxChar      decimal_point = lc->decimal_point[0];
    wxString    buf( aTextValue.Strip( wxString::both ) );

    // Convert the period in decimal point
    buf.Replace( wxT( "." ), wxString( decimal_point, 1 ) );

    // An ugly fix needed by WxWidgets 2.9.1 that sometimes
    // back to a point as separator, although the separator is the comma
    // TODO: remove this line if WxWidgets 2.9.2 fixes this issue
    buf.Replace( wxT( "," ), wxString( decimal_point, 1 ) );

    // Find the end of the numeric part
    unsigned brk_point = 0;

    while( brk_point < buf.Len() )
    {
        wxChar ch = buf[brk_point];

        if( !( (ch >= '0' && ch <='9') || (ch == decimal_point) || (ch == '-') || (ch == '+') ) )
        {
            break;
        }

        ++brk_point;
    }

    // Extract the numeric part
    buf.Left( brk_point );

    buf.ToDouble( &dtmp );

    // Check the optional unit designator (2 ch significant)
    wxString unit( buf.Mid( brk_point ).Strip( wxString::leading ).Left( 2 ).Lower() );

    EDA_UNITS_T units = m_userUnit;

    if( unit == wxT( "in" ) || unit == wxT( "\"" ) )
    {
        units = INCHES;
    }
    else if( unit == wxT( "mm" ) )
    {
        units = MILLIMETRES;
    }
    else if( unit == wxT( "mi" ) || unit == wxT( "th" ) ) // Mils or thous
    {
        units = INCHES;
        dtmp /= 1000;
    }

    value = FromUser ( units, dtmp );

    return KiROUND( value );
}

std::string PCB_UNITS::FormatIU( int aValue ) const
{
    char    buf[50];
    int     len;
    double  mm = aValue / m_IUPerMm;

    if( mm != 0.0 && fabs( mm ) <= 0.0001 )
    {
        len = sprintf( buf, "%.10f", mm );

        while( --len > 0 && buf[len] == '0' )
            buf[len] = '\0';

        if( buf[len] == '.' )
            buf[len] = '\0';
        else
            ++len;
    }
    else
    {
        len = sprintf( buf, "%.10g", mm );
    }

    return std::string( buf, len );
}


std::string PCB_UNITS::FormatAngle( double aAngle ) const
{
    char temp[50];

    int len = snprintf( temp, sizeof(temp), "%.10g", aAngle / 10.0 );

    return std::string( temp, len );
}


std::string PCB_UNITS::FormatIU( const wxPoint& aPoint ) const
{
    return FormatIU( aPoint.x ) + " " + FormatIU( aPoint.y );
}


std::string PCB_UNITS::FormatIU( const wxSize& aSize ) const
{
    return FormatIU( aSize.GetWidth() ) + " " + FormatIU ( aSize.GetHeight() );
}

std::string UNITS::FormatIU( int aValue ) const { assert(false); }
std::string UNITS::FormatAngle( double aAngle ) const{ assert(false); }
std::string UNITS::FormatIU( const wxPoint& aPoint ) const{ assert(false); }
std::string UNITS::FormatIU( const wxSize& aSize ) const{ assert(false); }
