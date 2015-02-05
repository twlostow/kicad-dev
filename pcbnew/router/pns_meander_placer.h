/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2013-2014 CERN
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

#ifndef __PNS_MEANDER_PLACER_H
#define __PNS_MEANDER_PLACER_H

#include <math/vector2d.h>

#include <geometry/shape.h>
#include <geometry/shape_line_chain.h>

#include "pns_node.h"
#include "pns_via.h"
#include "pns_line.h"
#include "pns_placement_algo.h"
#include "pns_meander.h"
#include "pns_meander_placer_base.h"

class PNS_ROUTER;
class PNS_SHOVE;
class PNS_OPTIMIZER;
class PNS_ROUTER_BASE;

/**
 * Class PNS_MEANDER_PLACER
 *
 * Single track length-matching/meandering tool.
 */

class PNS_MEANDER_PLACER : public PNS_MEANDER_PLACER_BASE
{
public:

    PNS_MEANDER_PLACER( PNS_ROUTER* aRouter );
    ~PNS_MEANDER_PLACER();

    /**
     * Function Start()
     * 
     * Starts routing a single track at point aP, taking item aStartItem as anchor
     * (unless NULL).
     */
    bool Start ( const VECTOR2I& aP, PNS_ITEM* aStartItem );

    /**
     * Function Move()
     * 
     * Moves the end of the currently routed trace to the point aP, taking 
     * aEndItem as anchor (if not NULL).
     * (unless NULL).
     */
    bool Move( const VECTOR2I& aP, PNS_ITEM* aEndItem );

    /**
     * Function FixRoute()
     * 
     * Commits the currently routed track to the parent node, taking
     * aP as the final end point and aEndItem as the final anchor (if provided).
     * @return true, if route has been commited. May return false if the routing
     * result is violating design rules - in such case, the track is only committed
     * if Settings.CanViolateDRC() is on.
     */
    bool FixRoute( const VECTOR2I& aP, PNS_ITEM* aEndItem );
    
    const PNS_LINE Trace() const;

    /**
     * Function CurrentNode()
     *
     * Returns the most recent world state.
     */
    PNS_NODE* CurrentNode( bool aLoopsRemoved = false ) const;
    
    const PNS_ITEMSET Traces();

    const VECTOR2I& CurrentEnd() const;
    
    int CurrentNet() const;
    int CurrentLayer() const;

    int totalLength();

    const wxString TuningInfo() const;
    TUNING_STATUS TuningStatus() const;

    bool checkFit ( PNS_MEANDER_SHAPE* aShape );

private:
    friend class PNS_MEANDER_SHAPE;
    
    void meanderSegment ( const SEG& aBase );

//    TUNING_STATUS tuneLineLength ( SHAPE_LINE_CHAIN & aPre, SHAPE_LINE_CHAIN& aTuned, SHAPE_LINE_CHAIN& aPost );
    TUNING_STATUS tuneLineLength ( SHAPE_LINE_CHAIN& aTuned, int aElongation );



//    void addMeander ( PNS_MEANDER *aM );
//    void addCorner ( const VECTOR2I& aP );

    void splitAdjacentSegments( PNS_NODE* aNode, PNS_ITEM* aSeg, const VECTOR2I& aP );

    void setWorld ( PNS_NODE* aWorld );
    void release();

    int origPathLength () const;

    ///> pointer to world to search colliding items
    PNS_NODE* m_world;

    ///> current routing start point (end of tail, beginning of head)
    VECTOR2I m_currentStart;

    ///> Current world state
    PNS_NODE* m_currentNode;

    PNS_LINE* m_originLine;
    PNS_LINE m_currentTrace;
    PNS_ITEMSET m_tunedPath;

    SHAPE_LINE_CHAIN m_finalShape;
    PNS_MEANDERED_LINE m_result;
    PNS_SEGMENT *m_initialSegment;

    int m_lastLength;
    TUNING_STATUS m_lastStatus;
};

#endif    // __PNS_MEANDER_PLACER_H
