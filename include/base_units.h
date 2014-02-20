/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012-2014 CERN
 * Copyright (C) 1992-2011 KiCad Developers, see change_log.txt for contributors.
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
 * @file base_units.h
 * @brief Implementation of conversion functions that require both schematic and board
 *        internal units.
 */

#ifndef _BASE_UNITS_H_
#define _BASE_UNITS_H_

#include <appl_wxstruct.h>

/// User unit types supported by Kicad applications
enum EDA_UNITS_T {
    INCHES = 0,
    MILLIMETRES = 1,
    UNSCALED_UNITS = 2
};

/**
 * Class UNITS
 * - Holds properties of measurement units for a particular Kicad application (pcbnew, eeschema, etc)
 * - Converts between user-selected units (UI) and the Internal Units specific for the application
 * - Converts between millimeters, mils, decimils and inches and IUs
 * - Wraps the g_UserUnit global variable
 * The class serves as a polymorphic base for application-specific unit classes. This is the way to 
 * pass the notion of application-specific unit conversions to common base classes (such as EDA_DRAW_FRAME).
 */
class UNITS {
	public:
		UNITS():
			m_userUnit(INCHES) {};

		/**
		 * Function FromUser
 		 *
 		 * Converts the value \a aValue given in inches or mm (\a aUnit) to Internal Units.
 		 * @param aUnit The user units to convert from.
 		 * @param aValue The value in user units to convert.
 		 */
		double FromUser ( EDA_UNITS_T aUnit, double aValue ) const;
		
		/**
		 * Function FromUser
 		 *
 		 * Converts the value \a aValue given in current user user units to Internal Units.
 		 * @param aValue The value in user units to convert.
 		 */
 		double FromUser ( double aValue ) const;
		
		/**
 		 * Function ToUser
 		 *
 		 * Converts \a aValue in internal units to the appropriate user units defined by \a aUnit.
 		 * @return The converted value, in double
 		 * @param aUnit The units to convert \a aValue to.
 		 * @param aValue The value in internal units to convert.
 		 */
		double ToUser ( EDA_UNITS_T aUnit, double aValue ) const;
		
		/**
 		 * Function ToUser
 		 *
 		 * Converts \a aValue in internal units to the currently set user units (m_userUnit).
 		 * @return The converted value, in double
 		 * @param aValue The value in internal units to convert.
 		 */
		double ToUser ( double aValue ) const;
	
		/**
		 * Function CoordinateToString
		 *
 		 * is a helper to convert the \a integer coordinate \a aValue to a string in inches,
 		 * millimeters, or unscaled units according to the current user units setting.
 		 *
 		 * Should be used only to display a coordinate in status, but not in dialogs,
 		 * because the mantissa of the number displayed has 4 digits max for readability.
 		 * (i.e. the value shows the decimils or the microns )
 		 * However the actual internal value could need up to 8 digits to be printed
 		 *
 		 * @param aValue The integer coordinate to convert.
 		 * @param aConvertToMils Convert inch values to mils if true.  This setting has no effect if
         * the current user unit is millimeters.
 		 * @return The converted string for display in user interface elements.
 		 */
		wxString CoordinateToString( int aValue, bool aConvertToMils = false) const;

		/**
		 * Function LenghtToString
		 * Is a helper to convert the \a double length \a aValue to a string in inches,
		 * millimeters, or unscaled units according to the current user units setting.
		 *
		 * Should be used only to display a coordinate in status, but not in dialogs,
		 * because the mantissa of the number displayed has 4 digits max for readability.
		 * (i.e. the value shows the decimils or the microns )
		 * However the actual internal value could need up to 8 digits to be printed
		 *
		 * @param aValue The double value to convert.
		 * @param aConvertToMils Convert inch values to mils if true.  This setting has no effect if
		 *                       the current user unit is millimeters.
		 * @return The converted string for display in user interface elements.
		 */
		wxString LengthToString( double aValue, bool aConvertToMils = false) const;
		
		/**
		 * Function ReturnStringFromValue
		 * returns the string from \a aValue according to currently set user units (inch, mm) for display.
		 *
		 * For readability, the mantissa has 3 or more digits (max 8 digits),
		 * the trailing 0 are removed if the mantissa has more than 3 digits
		 * and some trailing 0
		 * This function should be used to display values in dialogs because a value
		 * entered in mm (for instance 2.0 mm) could need up to 8 digits mantissa
		 * if displayed in inch to avoid truncation or rounding made just by the printf function.
		 * otherwise the actual value is rounded when read from dialog and converted
		 * in internal units, and therefore modified.
		 *
		 * @param aValue = value in Internal_Unit
		 * @param aAddUnitSymbol = true to add symbol unit to the string value
		 * @return A wxString object containing value and optionally the symbol unit (like 2.000 mm)
		 */
		wxString StringFromValue( int aValue, bool aAddUnitSymbol = false) const;
		
		/**
 		 * Function ValueFromString
 		 * converts \a aTextValue in \a current user units to internal units used by the application.
 		 *
 		 * @param aTextValue A reference to a wxString object containing the string to convert.
 		 * @return The string from Value, according to units (inch, mm ...) for display.
 		 */
		int ValueFromString( const wxString& aTextValue ) const;


		///> Converts from IU to mils
		double IuToMils( int aIu ) const
		{
			switch(m_app)
			{
				case APP_PCBNEW_T:
				case APP_CVPCB_T:
				case APP_GERBVIEW_T:
				{
					double x = aIu * m_milsPerIU;
    				return int( x < 0 ? x - 0.5 : x + 0.5 );
    			}
    			default:
    				return aIu;
    		}
		}

		///> Converts from IU to decimils
		double IuToDMils( int aIu ) const
		{
    	    double x = aIu * m_deciMilsPerIU;
		   	return int( x < 0 ? x - 0.5 : x + 0.5 );
		}

		///> Converts from mils to IU
		int MilsToIu( int mils ) const
		{
		    double x = mils * m_IUPerMils;
		    return int( x < 0 ? x - 0.5 : x + 0.5 );
		}
		
		///> Converts from mils to IU (wrapper for wxSizes)
		wxSize MilsToIu( const wxSize &aSize ) const
		{
		    return wxSize ( MilsToIu (aSize.x), MilsToIu(aSize.y ));
		}
		
		///> Converts from decimils to IU
		int DMilsToIu( int dmils ) const
		{
		    double x = dmils * m_IUPerDecimils;
		    return int( x < 0 ? x - 0.5 : x + 0.5 );
		}

		///> Converts from millimeters to IU
		int MmToIu( double mm ) const
		{
    		return (int) ( mm < 0 ? mm * m_IUPerMm - 0.5 : mm * m_IUPerMm + 0.5);
		}

		/**
		 * Function PointToString()
		 *
		 * Outputs coordinate pair as a string with format "@ (x,y)". Replacement
		 * for the formerly overloaded << operator.
		 * @param aPos  The point to output.
		 * @return wxString& - the output string
		 */
		wxString PointToString ( const wxPoint& aPos ) const
		{
			wxString str;
			str << wxT( "@ (" ) << CoordinateToString( aPos.x );
		    str << wxT( "," ) << CoordinateToString( aPos.y );
    		str << wxT( ")" );
    		return str;
		}

		///> IU Formatting functions for the s-expressions. 
		std::string FormatIU( int aValue ) const;
		std::string FormatAngle( double aAngle ) const;
		std::string FormatIU( const wxPoint& aPoint ) const;
		std::string FormatIU( const wxSize& aSize ) const;

		/**
		 * Function GetUserUnit()
		 *
		 * Returns the current user unit (replacement of g_UserUnit)
		 */
		EDA_UNITS_T GetUserUnit() const
		{
			return m_userUnit;
		}

		/**
		 * Function SetUserUnit()
		 *
		 * Sets user unit (replacement of g_UserUnit)
		 */
		void SetUserUnit( EDA_UNITS_T aUserUnit )
		{
			m_userUnit = aUserUnit;
		}

		///> returns millimeter-to-IU scalefactor
		double MmPerIu() const { return m_mmPerIU; }
		///> returns mils-to-IU scalefactor
		double MilsPerIu() const { return m_milsPerIU; }
		///> returns decimils-to-IU scalefactor
		double DMilsPerIU() const { return m_deciMilsPerIU; }

		///> returns IU-to-millimeter scalefactor
		double IuPerMm() const { return m_IUPerMm; }
		///> returns IU-to-mils scalefactor
		double IuPerMils() const { return m_IUPerMils; }
		///> returns IU-to-decimils scalefactor
		double IuPerDMils() const { return m_IUPerDecimils; }

	protected:
		
		///> scale factors. initialized by the constructor of derived unit class
		double m_mmPerIU;
		double m_milsPerIU;
		double m_deciMilsPerIU;
	
		double m_IUPerMm;
		double m_IUPerMils;
		double m_IUPerDecimils;
		
		EDA_UNITS_T m_userUnit;
		EDA_APP_T m_app; ///> the application ID who owns this units definition

};

/**
 * Class IUNIT_HOLDER
 * 
 * A lightweight interface, providing a notion of application-specific units for 
 * common objects (e.g. EDA_DRAW_FRAME, EDA_ITEM). Simply keeps a pointer to appropriate
 * UNITS derivative.
 */
class IUNIT_HOLDER {
	public:

		IUNIT_HOLDER ( UNITS *aUnits = NULL ) :
			m_appUnits(aUnits) {}

		IUNIT_HOLDER ( const IUNIT_HOLDER& aOther ) 
		{
			m_appUnits = aOther.m_appUnits;
		}

		/**
		 * Function SetUnits()
		 *
		 * Assigns an unit for this object. Usually called from a constructor.
		 */
		void SetUnits ( UNITS *aUnits )
		{
			m_appUnits = aUnits;
		}

		const UNITS *Units() const {
			if ( !m_appUnits )
			{
				///@ todo: fail in a less lame way
				printf("No units assigned for IUNIT_HOLDER-based item!\n\n");
				assert(false);
			}

			return m_appUnits;
		}

		UNITS *ModifiableUnits() 
		{
			if ( !m_appUnits )
			{
				printf("No units assigned for IUNIT_HOLDER-based item!\n\n");
				assert(false);
			}

			return m_appUnits;
		}

	private:
		UNITS *m_appUnits;
};


class PCB_UNITS : public UNITS
{
	public:
		PCB_UNITS ()
		{
		   m_mmPerIU  = 1.0 / 1e6;
    	   m_milsPerIU =  1.0 / 1e6 * 0.0254;
    	   m_deciMilsPerIU = 1.0 / 1e6 * 0.00254;
    	   m_IUPerMm = 1e6;
    	   m_IUPerMils = 1e6 * 0.0254;
    	   m_IUPerDecimils = 1e6 * 0.00254 ;
    	   m_app = APP_PCBNEW_T;
    	}
};

class SCH_UNITS : public UNITS
{
	public:
		SCH_UNITS ()
		{
		   m_milsPerIU =  1.0;
    	   m_deciMilsPerIU = 10.0;
		   m_mmPerIU  = m_milsPerIU / 0.0254;
    	   
    	   m_IUPerMils = 1.0;
    	   m_IUPerMm = m_IUPerMils / 0.0254;
    	   m_IUPerDecimils = 0.1;
    	   m_app = APP_EESCHEMA_T;
    	}   
};

class GERBVIEW_UNITS : public UNITS
{
	public:
		GERBVIEW_UNITS ()
		{
		   m_mmPerIU  = 1.0 / 1e5;
    	   m_milsPerIU =  1.0 / 1e5 * 0.0254;
    	   m_deciMilsPerIU = 1.0 / 1e5 * 0.00254;
    	   m_IUPerMm = 1e5;
    	   m_IUPerMils = 1e5 * 0.0254;
    	   m_IUPerDecimils = 1e5 * 0.00254 ;
    	   m_app = APP_GERBVIEW_T;
    	}   
};

class PL_EDITOR_UNITS : public UNITS
{
	public:
		PL_EDITOR_UNITS ()
		{
		   m_mmPerIU  = 1.0 / 1e3;
    	   m_milsPerIU =  1.0 / 1e3 * 0.0254;
    	   m_deciMilsPerIU = 1.0 / 1e3 * 0.00254;
    	   m_IUPerMm = 1e3;
    	   m_IUPerMils = 1e3 * 0.0254;
    	   m_IUPerDecimils = 1e3 * 0.00254 ;
    	   m_app = APP_GERBVIEW_T;
    	}   
};

///> App-specific units. Declared in a single place as global variables.
extern PCB_UNITS g_PcbUnits;
extern SCH_UNITS g_SchUnits;
extern GERBVIEW_UNITS g_GerbviewUnits;
extern PL_EDITOR_UNITS g_PLEditorUnits;


/** Helper function Double2Str to print a float number without
 * using scientific notation and no trailing 0
 * We want to avoid scientific notation in S-expr files (not easy to read)
 * for floating numbers.
 * So we cannot always just use the %g or the %f format to print a fp number
 * this helper function uses the %f format when needed, or %g when %f is
 * not well working and then removes trailing 0
 */
std::string Double2Str( double aValue );

/**
 * Function StripTrailingZeros
 * Remove trailing 0 from a string containing a converted float number.
 * The trailing 0 are removed if the mantissa has more
 * than aTrailingZeroAllowed digits and some trailing 0
 */
void StripTrailingZeros( wxString& aStringValue, unsigned aTrailingZeroAllowed = 1 );


/**
 * Function AngleToStringDegrees
 * is a helper to convert the \a double \a aAngle (in internal unit)
 * to a string in degrees
 */
wxString AngleToStringDegrees( double aAngle );

#endif   // _BASE_UNITS_H_

