/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004-2019 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2007 Dick Hollenbeck, dick@softplc.com
 * Copyright (C) 2019-2020 KiCad Developers, see AUTHORS.txt for contributors.
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


#ifndef DRC_TEST_PROVIDER_CLEARANCE_BASE__H
#define DRC_TEST_PROVIDER_CLEARANCE_BASE__H

#include <class_board.h>

#include <drc_proto/drc_test_provider.h>

namespace test {

class DRC_TEST_PROVIDER_CLEARANCE_BASE : public DRC_TEST_PROVIDER
{
public:
    DRC_TEST_PROVIDER_CLEARANCE_BASE () :
        DRC_TEST_PROVIDER()
        {

        }

    virtual ~DRC_TEST_PROVIDER_CLEARANCE_BASE() 
    {

    }

protected:
    void testCopperDrawItem( BOARD_ITEM* aItem );

    bool doPadToPadsDrc( D_PAD* aRefPad, D_PAD** aStart, D_PAD** aEnd, int x_limit );

    bool checkClearanceSegmToPad( const SEG& refSeg, int refSegWidth, const D_PAD* pad,
                                   int minClearance, int* aActualDist );
    bool checkClearancePadToPad( D_PAD* aRefPad, D_PAD* aPad, int aMinClearance, int* aActual );
    bool poly2segmentDRC( wxPoint* aTref, int aTrefCount, wxPoint aSegStart, wxPoint aSegEnd,
                      int aDist, int* aActual );
    //bool poly2polyDRC( wxPoint* aTref, int aTrefCount, wxPoint* aTtest, int aTtestCount,
      //             int aAllowedDist, int* actualDist );

    wxPoint getLocation( TRACK* aTrack, const SEG& aConflictSeg );
    wxPoint getLocation( TRACK* aTrack, ZONE_CONTAINER* aConflictZone );

    BOARD* m_board;
    int m_largestClearance;
    SHAPE_POLY_SET             m_boardOutline;   // The board outline including cutouts
    bool                       m_boardOutlineValid;
};

};

#endif // DRC_TEST_PROVIDER_CLEARANCE_BASE__H
