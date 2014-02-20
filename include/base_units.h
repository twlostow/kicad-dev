/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 CERN
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
 * @file base_units.h
 * @brief Implementation of conversion functions that require both schematic and board
 *        internal units.
 */

#ifndef _BASE_UNITS_H_
#define _BASE_UNITS_H_

#include <appl_wxstruct.h>

enum EDA_UNITS_T {
    INCHES = 0,
    MILLIMETRES = 1,
    UNSCALED_UNITS = 2
};

class UNITS {
	public:
		UNITS():
			m_userUnit(MILLIMETRES) {};
	
		
		double FromUser ( EDA_UNITS_T aUnit, double aValue ) const;
		double FromUser ( double aValue ) const;
		double ToUser ( EDA_UNITS_T aUnit, double aValue ) const;
		double ToUser ( double aValue ) const;
	
		wxString CoordinateToString( int aValue, bool aConvertToMils = false) const;
		wxString LengthToString( double aValue, bool aConvertToMils = false) const;
		
		wxString StringFromValue( int aValue, bool aAddUnitSymbol = false) const;
		int ValueFromString( const wxString& aTextValue ) const;
	
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

		double IuToDMils( int aIu ) const
		{
    	    double x = aIu * m_deciMilsPerIU;
		   	return int( x < 0 ? x - 0.5 : x + 0.5 );
		}

		int MilsToIu( int mils ) const
		{
		    double x = mils * m_IUPerMils;
		    return int( x < 0 ? x - 0.5 : x + 0.5 );
		}
		
		wxSize MilsToIu( const wxSize &aSize ) const
		{
		    return wxSize ( MilsToIu (aSize.x), MilsToIu(aSize.y ));
		}
		
		int DMilsToIu( int dmils ) const
		{
		    double x = dmils * m_IUPerDecimils;
		    return int( x < 0 ? x - 0.5 : x + 0.5 );
		}

		int MmToIu( double mm ) const
		{
    		return (int) ( mm < 0 ? mm * m_IUPerMm - 0.5 : mm * m_IUPerMm + 0.5);
		}

		wxString PointToString ( const wxPoint& aPos ) const
		{
			wxString str;
			str << wxT( "@ (" ) << CoordinateToString( aPos.x );
		    str << wxT( "," ) << CoordinateToString( aPos.y );
    		str << wxT( ")" );
    		return str;
		}

		virtual std::string FormatIU( int aValue ) const;
		virtual std::string FormatAngle( double aAngle ) const;
		virtual std::string FormatIU( const wxPoint& aPoint ) const;
		virtual std::string FormatIU( const wxSize& aSize ) const;

		EDA_UNITS_T GetUserUnit() const
		{
			return m_userUnit;
		}

		void SetUserUnit( EDA_UNITS_T aUserUnit )
		{
			m_userUnit = aUserUnit;
		}

		double MmPerIu() const { return m_mmPerIU; }
		double MilsPerIu() const { return m_milsPerIU; }
		double DMilsPerIU() const { return m_deciMilsPerIU; }

		double IuPerMm() const { return m_IUPerMm; }
		double IuPerMils() const { return m_IUPerMils; }
		double IuPerDMils() const { return m_IUPerDecimils; }

	protected:
		
		
		double m_mmPerIU;
		double m_milsPerIU;
		double m_deciMilsPerIU;
	
		double m_IUPerMm;
		double m_IUPerMils;
		double m_IUPerDecimils;
		
		EDA_UNITS_T m_userUnit;
		EDA_APP_T m_app;

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

    	virtual std::string FormatIU( int aValue ) const;
		virtual std::string FormatAngle( double aAngle ) const;
		virtual std::string FormatIU( const wxPoint& aPoint ) const;
		virtual std::string FormatIU( const wxSize& aSize ) const;
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

class IUNIT_HOLDER {
	public:

		IUNIT_HOLDER ( UNITS *aUnits = NULL ) :
			m_appUnits(aUnits) {}

		IUNIT_HOLDER ( const IUNIT_HOLDER& aOther ) 
		{
			m_appUnits = aOther.m_appUnits;
		}

		void SetUnits ( UNITS *aUnits )
		{
			m_appUnits = aUnits;
		}

		const UNITS *Units() const {
			if ( !m_appUnits )
			{
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

extern PCB_UNITS g_PcbUnits;
extern SCH_UNITS g_SchUnits;
extern GERBVIEW_UNITS g_GerbviewUnits;


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

