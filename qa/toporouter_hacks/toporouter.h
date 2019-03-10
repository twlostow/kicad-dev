/*
 *                            COPYRIGHT
 *
 *  Topological Autorouter for
 *  PCB, interactive printed circuit board design
 *  Copyright (C) 2009 Anthony Blake
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Contact addresses for email:
 *  Anthony Blake, tonyb33@gmail.com
 *
 */

#ifndef TOPOROUTER_H
#define TOPOROUTER_H

#include <cstdio>
#include <string>



#if 0
namespace toporouter {
  struct _toporouter_t;
  typedef struct toporouter_layer_t;
  typedef struct _toporouter_t toporouter_t;
};
#endif

#undef DEBUG
#include "toporouter-private.h"

#include <view/view.h>
#include <view/view_item.h>
#include <class_board.h>

class PCB_DRAW_PANEL_GAL;

class RULE_RESOLVER
{
public:
    RULE_RESOLVER( BOARD* aBoard );
    ~RULE_RESOLVER();

    double              GetClearance( const std::string name );
    double              GetLineWidth( const std::string name );
    int                 GetGroupCount();
    std::pair<int, PCB_LAYER_ID> GetLayerGroup( int i );

private:
    BOARD* m_board;
};

class TOPOROUTER_ENGINE;

class TOPOROUTER_PREVIEW : public EDA_ITEM
{
    private:
    TOPOROUTER_ENGINE *m_router;
    struct ROUTE {
        SEG s;
        int layer;
    };
public:
    TOPOROUTER_PREVIEW( TOPOROUTER_ENGINE *engine );
    virtual ~TOPOROUTER_PREVIEW();

    wxString GetClass() const override { return "MyItem"; };

#ifdef DEBUG
    virtual void Show( int nestLevel, std::ostream& os ) const override {};
#endif

    const BOX2I ViewBBox() const override
    {
        BOX2I bb;
        bb.SetMaximum();
        return bb;
    }

    virtual void ViewDraw( int aLayer, KIGFX::VIEW* aView ) const override;

    virtual void ViewGetLayers( int aLayers[], int& aCount ) const override
    {
        aLayers[0] = LAYER_GP_OVERLAY;
        aCount = 1;
    }

    void ClearRouted()
    {
        m_routed.clear();
    }

    void AddRoutedArc( toporouter::toporouter_arc_t *arc, int layer );

    void AddRouted( double x0, double y0, double x1, double y1, int l)
    {
        ROUTE r;
        r.s = SEG(VECTOR2I(x0, y0), VECTOR2I(x1, y1 ));
        r.layer = l;
        //printf("AddRouted: [%.0f %.0f] - [%.0f %.0f] grp %d\n",  x0,y0,x1,y1, l );

        m_routed.push_back(r);
    }


  private:
    std::vector<ROUTE> m_routed;
    void drawSurface( KIGFX::GAL* gal, toporouter::toporouter_t *router, GtsSurface* surf ) const;
    void drawRouted( KIGFX::GAL* gal ) const;
};



class TOPOROUTER_ENGINE
{
public:
    TOPOROUTER_ENGINE( PCB_DRAW_PANEL_GAL *panel );
    ~TOPOROUTER_ENGINE();

    void SetBoard( BOARD* aBoard );
    void ClearWorld();
    void SyncWorld();

    //static TOPOROUTER_ENGINE* GetInstance();

    void Run();

    void   syncBoardOutline( toporouter::toporouter_layer_t* layer, int layerId );
    void syncPads(  toporouter::toporouter_layer_t* layer, int layerId );
    void syncConnectivity( );

    void ImportRoutes();

    RULE_RESOLVER* rules() const
    {
        return m_ruleResolver;
    };
    void log( const std::string fmt, ... );

    TOPOROUTER_PREVIEW *GetPreview() const { return m_preview; };

    toporouter::toporouter_t *GetRouter() const { return m_router; }



private:
    BOARD* board();





private:
    PCB_DRAW_PANEL_GAL *m_panel;
    TOPOROUTER_PREVIEW *m_preview;
    RULE_RESOLVER             *m_ruleResolver;
    BOARD*                    m_board;
    toporouter::toporouter_t* m_router;
};



#endif // TOPOROUTER_H
