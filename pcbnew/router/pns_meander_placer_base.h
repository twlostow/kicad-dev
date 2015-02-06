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

#ifndef __PNS_MEANDER_PLACER_BASE_H
#define __PNS_MEANDER_PLACER_BASE_H

#include <math/vector2d.h>

#include <geometry/shape.h>
#include <geometry/shape_line_chain.h>

#include "pns_node.h"
#include "pns_via.h"
#include "pns_line.h"
#include "pns_placement_algo.h"
#include "pns_meander.h"

class PNS_ROUTER;
class PNS_SHOVE;
class PNS_OPTIMIZER;
class PNS_ROUTER_BASE;

/**
 * Class PNS_MEANDER_PLACER_BASE
 *
 * Base class for Single trace & Differenial pair meandrators ;)
 */

class PNS_MEANDER_PLACER_BASE : public PNS_PLACEMENT_ALGO
{
public:
    enum TUNING_STATUS {
        TOO_SHORT = 0,
        TOO_LONG,
        TUNED
    };

    PNS_MEANDER_PLACER_BASE ( PNS_ROUTER* aRouter );
    virtual ~PNS_MEANDER_PLACER_BASE ();

    virtual const wxString TuningInfo() const = 0;
    virtual TUNING_STATUS TuningStatus() const = 0;

    virtual void AmplitudeStep ( int aSign );
    virtual void SpacingStep ( int aSign );

    const PNS_MEANDER_SETTINGS& MeanderSettings() const 
    {
        return m_settings;
    }

    virtual void UpdateSettings( const PNS_MEANDER_SETTINGS& aSettings);

    virtual bool checkFit ( PNS_MEANDER_SHAPE* aShape ) { return false; };
    
protected:

    int m_currentWidth;
    PNS_MEANDER_SETTINGS m_settings;    
};

#endif    // __PNS_MEANDER_PLACER_BASE_H
