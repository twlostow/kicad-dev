/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2013-2015 CERN
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pns_router.h"
#include "pns_meander.h"
#include "pns_meander_placer_base.h"

PNS_MEANDER_PLACER_BASE::PNS_MEANDER_PLACER_BASE( PNS_ROUTER* aRouter ) :
        PNS_PLACEMENT_ALGO( aRouter )
{

};

PNS_MEANDER_PLACER_BASE::~PNS_MEANDER_PLACER_BASE( ) 
{

};

void PNS_MEANDER_PLACER_BASE::AmplitudeStep( int aSign )
{
    int a = m_settings.m_maxAmplitude + aSign * m_settings.m_step;
    a = std::max( a,  m_settings.m_minAmplitude );
        
    m_settings.m_maxAmplitude = a;
}

void PNS_MEANDER_PLACER_BASE::SpacingStep( int aSign )
{
    int s = m_settings.m_spacing + aSign * m_settings.m_step;
    s = std::max( s, 2 * m_currentWidth );
        
    m_settings.m_spacing = s;
}

void PNS_MEANDER_PLACER_BASE::UpdateSettings( const PNS_MEANDER_SETTINGS& aSettings )
{
    m_settings = aSettings;
}

void PNS_MEANDER_PLACER_BASE::cutTunedLine( const SHAPE_LINE_CHAIN& aOrigin,
                                            const VECTOR2I& aTuneStart,
                                            const VECTOR2I& aCursorPos,
                                            SHAPE_LINE_CHAIN& aPre,
                                            SHAPE_LINE_CHAIN& aTuned,
                                            SHAPE_LINE_CHAIN& aPost )
{
    VECTOR2I n = aOrigin.NearestPoint( aCursorPos );
    VECTOR2I m = aOrigin.NearestPoint( aTuneStart );

    SHAPE_LINE_CHAIN l( aOrigin );
    l.Split( n );
    l.Split( m );

    int i_start = l.Find( m );
    int i_end = l.Find( n );

    if( i_start > i_end )
    {
        l = l.Reverse( );
        i_start = l.Find( m );
        i_end = l.Find( n );
    }

    aPre = l.Slice( 0, i_start );
    aPost = l.Slice( i_end, -1 );    
    aTuned = l.Slice( i_start, i_end );
    aTuned.Simplify( );
}
