/**
 * @file classpcb.cpp
 * @brief Member functions of classes used in Pcbnew (see pcbstruct.h)
 *        except for tracks (see class_track.cpp).
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2012 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2012 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <fctsys.h>
#include <common.h>
#include <macros.h>
#include <trigo.h>
#include <class_pcb_screen.h>
#include <eda_text.h>                // FILLED
#include <base_units.h>

#include <pcbnew.h>
#include <class_board_design_settings.h>
#include <layers_id_colors_and_visibility.h>

#include <pcbnew_id.h>



/**
    Default Pcbnew zoom values.
    Limited to 19 values to keep a decent size to menus.
    Roughly a 1.5 progression.
    The last 2 values are handy when somebody uses a library import of a module
    (or foreign data) which has a bad coordinate.
    Also useful in GerbView for this reason.
    Zoom 5 and 10 can create artefacts when drawing (integer overflow in low level graphic
    functions )
*/
static const double pcbZoomListDecimils[] =
{
     0.1 ,
     0.2 ,
     0.3 ,
     0.5 ,
     1.0 ,
     1.5 ,
     2.0 ,
     3.0 ,
     4.5 ,
     7.0 ,
     10.0 ,
     15.0 ,
     22.0 ,
     35.0 ,
     50.0 ,
     80.0 ,
     120.0 ,
     200.0 ,
     300.0 ,

/*
    The largest distance that wx can support is INT_MAX, since it represents
    distance often in a wxCoord or wxSize. As a scalar, a distance is always
    positive. On most machines which run KiCad, int is 32 bits and INT_MAX is
    2147483647. The most difficult distance for a virtual (world) cartesian
    space is the hypotenuse, or diagonal measurement at a 45 degree angle. This
    puts the most stress on the distance magnitude within the bounded virtual
    space. So if we allow this distance to be our constraint of <= INT_MAX, this
    constraint then propagates to the maximum distance in X and in Y that can be
    supported on each axis. Remember that the hypotenuse of a 1x1 square is
    sqrt( 1x1 + 1x1 ) = sqrt(2) = 1.41421356.

    hypotenuse of any square = sqrt(2) * deltaX;

    Let maximum supported hypotenuse be INT_MAX, then:

    MAX_AXIS = INT_MAX / sqrt(2) = 2147483647 / 1.41421356 = 1518500251

    This maximum distance is imposed by wxWidgets, not by KiCad. The imposition
    comes in the form of the data structures used in the graphics API at the
    wxDC level. Obviously when we are not interacting with wx we can use double
    to compute distances larger than this. For example the computation of the
    total length of a net, can and should be done in double, since it might
    actually be longer than a single diagonal line.

    The next choice is what to use for internal units (IU), sometimes called
    world units.  If nanometers, then the virtual space must be limited to
    about 1.5 x 1.5 meters square.  This is 1518500251 divided by 1e9 nm/meter.

    The maximum zoom factor then depends on the client window size.  If we ask
    wx to handle something outside INT_MIN to INT_MAX, there are unreported
    problems in the non-Debug build because wxRound() goes silent.

    Let:
    const double MAX_AXIS = 1518500251;

    Then a maximum zoom factor for a screen of 1920 pixels wide is
        1518500251 / 1920 = 790885.

    The largest ZOOM_FACTOR in above table is ZOOM_FACTOR( 300 ), which computes
    out to 762000 just below 790885.
*/
};

#define GRID_SIZE(x)     wxRealPoint(x, x)

// Default grid sizes for PCB editor screens.
static const GRID_TYPE pcbImperialGridList[] =
{
    // predefined grid list in 0.0001 inches
    { ID_POPUP_GRID_LEVEL_1000,     GRID_SIZE( 1000 )  },
    { ID_POPUP_GRID_LEVEL_500,      GRID_SIZE( 500 )   },
    { ID_POPUP_GRID_LEVEL_250,      GRID_SIZE( 250 )   },
    { ID_POPUP_GRID_LEVEL_200,      GRID_SIZE( 200 )   },
    { ID_POPUP_GRID_LEVEL_100,      GRID_SIZE( 100 )   },
    { ID_POPUP_GRID_LEVEL_50,       GRID_SIZE( 50 )    },
    { ID_POPUP_GRID_LEVEL_25,       GRID_SIZE( 25 )    },
    { ID_POPUP_GRID_LEVEL_20,       GRID_SIZE( 20 )    },
    { ID_POPUP_GRID_LEVEL_10,       GRID_SIZE( 10 )    },
    { ID_POPUP_GRID_LEVEL_5,        GRID_SIZE( 5 )     },
    { ID_POPUP_GRID_LEVEL_2,        GRID_SIZE( 2 )     },
    { ID_POPUP_GRID_LEVEL_1,        GRID_SIZE( 1 )     }
};

static const GRID_TYPE pcbMetricGridList[] =
{
    // predefined grid list in mm
    { ID_POPUP_GRID_LEVEL_5MM,      GRID_SIZE( 5.0 )     },
    { ID_POPUP_GRID_LEVEL_2_5MM,    GRID_SIZE( 2.5 )     },
    { ID_POPUP_GRID_LEVEL_1MM,      GRID_SIZE( 1.0 )     },
    { ID_POPUP_GRID_LEVEL_0_5MM,    GRID_SIZE( 0.5 )     },
    { ID_POPUP_GRID_LEVEL_0_25MM,   GRID_SIZE( 0.25 )    },
    { ID_POPUP_GRID_LEVEL_0_2MM,    GRID_SIZE( 0.2 )     },
    { ID_POPUP_GRID_LEVEL_0_1MM,    GRID_SIZE( 0.1 )     },
    { ID_POPUP_GRID_LEVEL_0_0_5MM,  GRID_SIZE( 0.05 )    },
    { ID_POPUP_GRID_LEVEL_0_0_25MM, GRID_SIZE( 0.025 )   },
    { ID_POPUP_GRID_LEVEL_0_0_1MM,  GRID_SIZE( 0.01 )    }
};


PCB_SCREEN::PCB_SCREEN( const wxSize& aPageSizeIU ) :
    BASE_SCREEN( SCREEN_T )
{
    // D(wxSize displayz = wxGetDisplaySize();)
    // D(printf( "displayz x:%d y:%d lastZoomFactor: %.16g\n", displayz.x, displayz.y, pcbZoomList[DIM(pcbZoomList)-1] );)

    
    SetUnits( &g_PcbUnits );

    SetDefaultZoomFactors( pcbZoomListDecimils, DIM (pcbZoomListDecimils) );
    SetDefaultGrids ( pcbMetricGridList, DIM (pcbMetricGridList), true );
    SetDefaultGrids ( pcbImperialGridList, DIM (pcbImperialGridList), false );

    // Set the working grid size to a reasonable value (in 1/10000 inch)
    SetGrid( ID_POPUP_GRID_LEVEL_100 );

    m_Active_Layer       = LAYER_N_BACK;      // default active layer = bottom layer
    m_Route_Layer_TOP    = LAYER_N_FRONT;     // default layers pair for vias (bottom to top)
    m_Route_Layer_BOTTOM = LAYER_N_BACK;

    SetZoom( Units() -> DMilsToIu( 120 ) );             // a default value for zoom

    InitDataPoints( aPageSizeIU );
}


PCB_SCREEN::~PCB_SCREEN()
{
    ClearUndoRedoList();
}


int PCB_SCREEN::MilsToIuScalar()
{
    return (int)Units()->IuPerMils();
}


DISPLAY_OPTIONS::DISPLAY_OPTIONS()
{
    DisplayPadFill          = FILLED;
    DisplayViaFill          = FILLED;
    DisplayPadNum           = true;
    DisplayPadIsol          = true;

    DisplayModEdge          = true;
    DisplayModText          = true;
    DisplayPcbTrackFill     = true;  // false = sketch , true = filled
    ShowTrackClearanceMode  = SHOW_CLEARANCE_NEW_TRACKS_AND_VIA_AREAS;
    m_DisplayViaMode        = VIA_HOLE_NOT_SHOW;

    DisplayPolarCood        = false; /* false = display absolute coordinates,
                                      * true = display polar cordinates */
    DisplayZonesMode        = 0;     /* 0 = Show filled areas outlines in zones,
                                      * 1 = do not show filled areas outlines
                                      * 2 = show outlines of filled areas */
    DisplayNetNamesMode     = 3;     /* 0 do not show netnames,
                                      * 1 show netnames on pads
                                      * 2 show netnames on tracks
                                      * 3 show netnames on tracks and pads */
    DisplayDrawItems        = true;
    ContrastModeDisplay     = false;
}
