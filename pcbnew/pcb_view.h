#ifndef __PCB_VIEW_H
#define __PCB_VIEW_H

/*
 *  Simple STEP/IGES File Viewer
 *
 *  GL stuff based on wxWidgets "isosurf" example.
 *
 *  T.W. 2013
 */

#include <gal/graphics_abstraction_layer.h>
#include <view/view_ng.h>
#include <pcb_painter.h>
#include <pad_shapes.h>
#include <profile.h>

#include <class_pad.h>
#include <class_module.h>
#include <class_board.h>
#include <class_track.h>
#include <class_zone.h>

#include <set>

#include <boost/foreach.hpp>
#include <boost/bind.hpp>

namespace KIGFX {

class PCB_VIEW : public VIEW_BASE
{
private:

    enum Layers
    {
        L_PADS = 1,
        L_VIAS,
        L_TRACKS,
        L_GRAPHICS,
        L_ZONES,
        L_MODULES,
        L_RATSNEST
    };

    BOARD* m_board;

    bool m_flipLayerOrder;
    LAYER_ID m_topLayer;
    LSET m_visibleLayers;

public:

    PCB_VIEW();
    ~PCB_VIEW();


    virtual void Add( VIEW_ITEM_NG* aItem, int aLayer = DEFAULT_LAYER );
    virtual void Remove( VIEW_ITEM_NG* aItem );
    virtual void Update( VIEW_ITEM_NG* aItem );

    virtual void updateDisplayLists( const BOX2I& aRect );
    virtual void setupRenderOrder();

    void SetTopLayer ( LAYER_ID aTopLayer );
    void SetVisibleLayers ( LSET aLayers );

    LAYER_ID GetTopLayer( ) const;

    void SetSketchMode( KICAD_T aItemKind, bool aSketchMode );
    void SetZoneRenderMode ( int aMode );

    void SetLayerOpacity ( double aOpacity );
    double GetLayerOpacity ();

    void SetHightlightFactor ( double aHighlight );
    double GetHightlightFactor ( );

    void SetHighlight ( bool aEnable, bool aOnTop );

    void SetBoard( BOARD* aBoard );

    void queryList( const BOX2I& aRect, int layer, VIEW_DISP_LIST& list, int aFlag = 0 );


    void SyncLayersVisibility()
    {
    }

private:

    VIEW_DISP_LIST m_vias, m_padsSMD, m_padsTHT, m_items, m_modules;

    void addModule( MODULE* mod );
    void removeModule( MODULE* mod );

};

}

#endif
