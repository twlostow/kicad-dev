/**
 * @file class_gbr_screen.cpp
 */

#include <fctsys.h>
#include <common.h>
#include <macros.h>
#include <class_gbr_screen.h>
#include <base_units.h>
#include <gerbview_id.h>

#define GRID_SIZE(x)     wxRealPoint(x, x)

/**
    Default GerbView zoom values.
    Roughly a 1.5 progression.
*/

static const double gbrZoomListDecimils[] =
{
     0.5,
     1.0,
     1.5,
     2.0,
     3.0,
     4.5,
     7.0,
     10.0,
     15.0,
     22.0,
     35.0,
     50.0,
     80.0,
     120.0,
     200.0,
     350.0,
     500.0,
     1000.0,
     2000.0
};

static const GRID_TYPE gbrImperialGridList[] =
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

static const GRID_TYPE gbrMetricGridList[] =
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

GBR_SCREEN::GBR_SCREEN( const wxSize& aPageSizeIU ) :
    BASE_SCREEN( SCREEN_T )
{
    
    SetUnits( &g_GerbviewUnits );

    SetDefaultZoomFactors( gbrZoomListDecimils, DIM (gbrZoomListDecimils) );
    SetDefaultGrids ( gbrMetricGridList, DIM (gbrMetricGridList), true );
    SetDefaultGrids ( gbrImperialGridList, DIM (gbrImperialGridList), false );

    // Set the working grid size to a reasonable value (in 1/10000 inch)
    int g = g_GerbviewUnits.DMilsToIu (500);
    SetGrid(  wxRealPoint (g, g) );

    m_Active_Layer       = LAYER_N_BACK;      // default active layer = bottom layer

    SetZoom(  g_GerbviewUnits.DMilsToIu (350) );            // a default value for zoom

    InitDataPoints( aPageSizeIU );
}


GBR_SCREEN::~GBR_SCREEN()
{
    ClearUndoRedoList();
}


// virtual function
int GBR_SCREEN::MilsToIuScalar()
{
    return (int) g_GerbviewUnits.IuPerMils();
}


/* Virtual function needed by classes derived from BASE_SCREEN
 * this is a virtual pure function in BASE_SCREEN
 * do nothing in GerbView
 * could be removed later
 */
void GBR_SCREEN::ClearUndoORRedoList( UNDO_REDO_CONTAINER&, int )
{
}
