/**
 * @file class_gbr_screen.cpp
 */

#include <fctsys.h>
#include <common.h>
#include <macros.h>
#include <class_gbr_screen.h>
#include <base_units.h>
#include <gerbview_id.h>

/**
    Default GerbView zoom values.
    Roughly a 1.5 progression.
*/
static const double gbrZoomList[] =
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


struct GRID_DEF {
    int id;
    bool metric;
    double value;
};

static const GRID_DEF gbrGridList [] =
{
    // predefined grid list in 0.0001 inches
    { ID_POPUP_GRID_LEVEL_1000,      false, 1000   },
    { ID_POPUP_GRID_LEVEL_500,       false, 500    },
    { ID_POPUP_GRID_LEVEL_250,       false, 250    },
    { ID_POPUP_GRID_LEVEL_200,       false, 200    },
    { ID_POPUP_GRID_LEVEL_100,       false, 100    },
    { ID_POPUP_GRID_LEVEL_50,        false, 50     },
    { ID_POPUP_GRID_LEVEL_25,        false, 25     },
    { ID_POPUP_GRID_LEVEL_20,        false, 20     },
    { ID_POPUP_GRID_LEVEL_10,        false, 10     },
    { ID_POPUP_GRID_LEVEL_5,         false, 5      },
    { ID_POPUP_GRID_LEVEL_2,         false, 2      },
    { ID_POPUP_GRID_LEVEL_1,         false, 1      },
    { ID_POPUP_GRID_LEVEL_5MM,       true, 5.0    },
    { ID_POPUP_GRID_LEVEL_2_5MM,     true, 2.5    },
    { ID_POPUP_GRID_LEVEL_1MM,       true, 1.0    },
    { ID_POPUP_GRID_LEVEL_0_5MM,     true, 0.5    },
    { ID_POPUP_GRID_LEVEL_0_25MM,    true, 0.25   },
    { ID_POPUP_GRID_LEVEL_0_2MM,     true, 0.2    },
    { ID_POPUP_GRID_LEVEL_0_1MM,     true, 0.1    },
    { ID_POPUP_GRID_LEVEL_0_0_5MM,   true, 0.05   },
    { ID_POPUP_GRID_LEVEL_0_0_25MM,  true, 0.025  },
    { ID_POPUP_GRID_LEVEL_0_0_1MM,   true, 0.01   }
};


GBR_SCREEN::GBR_SCREEN( const wxSize& aPageSizeIU ) :
    BASE_SCREEN( SCREEN_T )
{
    for( unsigned i = 0; i < DIM( gbrZoomList );  ++i )
        m_ZoomList.push_back( g_GerbviewUnits.DMilsToIu(gbrZoomList[i] ) );

    for( unsigned i = 0; i < DIM( gbrGridList );  ++i )
    {
        const GRID_DEF *gdef = &gbrGridList[i];
        GRID_TYPE gtype;
        gtype.m_Id = gdef->id;

        int s;
        if( gdef->metric )
            s = g_GerbviewUnits.MmToIu ( gdef->value );
        else
            s = g_GerbviewUnits.DMilsToIu ( gdef->value );

        gtype.m_Size = wxRealPoint ( s, s );

        AddGrid( gtype);
    }

    int g = g_GerbviewUnits.DMilsToIu (500);
    // Set the working grid size to a reasonable value (in 1/10000 inch)
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
