/*
 *  Simple STEP/IGES File Viewer
 *
 *  GL stuff based on wxWidgets "isosurf" example.
 *
 *  T.W. 2013
 */

#include <profile.h>

#include <string>
#include <cstdio>
#include "pcb_view.h"

#include <view/view_rtree.h>

namespace KIGFX {
PCB_VIEW::PCB_VIEW() :
    KIGFX::VIEW_BASE()
{
    AddTree( L_PADS );
    AddTree( L_VIAS );
    AddTree( L_TRACKS );
    AddTree( L_GRAPHICS );
    AddTree( L_ZONES );
    AddTree( L_MODULES );

    m_needsRecache = true;

    m_topLayer = F_Cu;
    m_visibleLayers = LSET::AllLayersMask();
    m_flipLayerOrder = false;

    printf("Created view trees.\n");
}


PCB_VIEW::~PCB_VIEW()
{
    // delete layers here & r-trees here
}


static int netnameLOD( VIEW_ITEM* item )
{
    BOARD_ITEM* bitem = static_cast<BOARD_ITEM*> (item);

    switch( bitem->Type() )
    {
    case PCB_MODULE_TEXT_T:
        return 0;

    case PCB_PAD_T:
    {
        D_PAD* pad = static_cast<D_PAD*>(bitem);

        if( ( pad->GetSize().x == 0 ) && ( pad->GetSize().y == 0 ) )
            return UINT_MAX;

        return 100000000 / std::max( pad->GetSize().x, pad->GetSize().y );
    }

    case PCB_TRACE_T:
        return 20000000 / ( static_cast<TRACK*>(bitem)->GetWidth() + 1 );

    default:
        return 0;
    }
}


#define THRU_PADS_ONLY 1

struct queryVisitor
{
    queryVisitor( VIEW_DISP_LIST& aCont, int aFlag = 0 ) :
        m_cont( aCont ),
        m_flag( aFlag )
    {
    }

    bool operator()( VIEW_RTREE_ENTRY* aItem )
    {
        EDA_ITEM* item = static_cast<EDA_ITEM*>(aItem->item);
        LAYER_ID layer;

        if( item->Type() == PCB_PAD_T )
        {
            D_PAD* pad = static_cast<D_PAD*>(item);

            if( pad->IsOnLayer( F_Cu ) )
                layer = F_Cu;
            else
                layer = B_Cu;

            switch( pad->GetAttribute() )
            {
            case PAD_ATTRIB_HOLE_NOT_PLATED:
            case PAD_ATTRIB_STANDARD:

                if( !m_flag )
                    return true;

                break;

            default:

                if( m_flag == THRU_PADS_ONLY )
                    return true;

                break;
            }
        } else if (item->Type() == NOT_USED)
        {
            layer = F_Cu;
        } else {
            BOARD_ITEM *bitem = static_cast<BOARD_ITEM*>(item);
            layer = bitem->GetLayer();
        }

        m_cont.Add( layer, aItem );

        return true;
    }

    int m_flag;
    VIEW_DISP_LIST& m_cont;
};


void PCB_VIEW::SetBoard( BOARD* aBoard )
{

    m_board = aBoard;

    printf("SetBoard: %p\n", aBoard );

    PROF_COUNTER pcnt( "build-board-view", true );

    for( MODULE* mod = aBoard->m_Modules; mod; mod = mod->Next() )
        addModule( mod );

    for( TRACK* trk = aBoard->m_Track; trk; trk = trk->Next() )
        if( VIA * via = dyn_cast<VIA*> ( trk ) )
            VIEW_BASE::Add( via, L_VIAS );
        else
            VIEW_BASE::Add( trk, L_TRACKS );




    for( auto drawing : aBoard->Drawings() )
        VIEW_BASE::Add( drawing, L_GRAPHICS );

// for( int i = 0; i < aBoard->GetAreaCount(); ++i )
    // VIEW_BASE::Add( aBoard->GetArea( i ), L_ZONES );


    pcnt.show();
    m_needsRecache = true;

    BOX2I extents = CalculateExtents();

    extents.Inflate( extents.GetWidth() / 10 );

    SetViewport( BOX2D( extents.GetPosition(), extents.GetSize() ) );
}


void PCB_VIEW::queryList( const BOX2I& aRect, int layer, VIEW_DISP_LIST& list, int aFlag )
{
    queryVisitor v( list, aFlag );

    m_trees[layer]->Query( aRect, v );
}

void PCB_VIEW::updateDisplayLists( const BOX2I& aRect )
{
    VIEW_BASE::updateDisplayLists( aRect );

    m_items.Clear();
    m_padsSMD.Clear();
    m_padsTHT.Clear();
    m_vias.Clear();
    m_modules.Clear();

    queryList( aRect, L_TRACKS, m_items );
    queryList( aRect, L_GRAPHICS, m_items );
    queryList( aRect, L_ZONES, m_items );
    queryList( aRect, L_PADS, m_padsSMD );
    queryList( aRect, L_PADS, m_padsTHT, THRU_PADS_ONLY );
    queryList( aRect, L_VIAS, m_vias );
    queryList( aRect, L_MODULES, m_modules );
}


void PCB_VIEW::addModule( MODULE* mod )
{
    for( D_PAD* pad = mod->Pads(); pad; pad = pad->Next() )
        VIEW_BASE::Add( pad, L_PADS );

    for( BOARD_ITEM* item = mod->GraphicalItems(); item; item = item->Next() )
        VIEW_BASE::Add( item, L_GRAPHICS );

    VIEW_BASE::Add( &mod->Reference(), L_MODULES );
    VIEW_BASE::Add( &mod->Value(), L_MODULES );
}

void PCB_VIEW::removeModule( MODULE* mod )
{
    for( D_PAD* pad = mod->Pads(); pad; pad = pad->Next() )
        VIEW_BASE::Remove( pad );

    for( BOARD_ITEM* item = mod->GraphicalItems(); item; item = item->Next() )
        VIEW_BASE::Remove( item );

    VIEW_BASE::Remove( &mod->Reference() );
    VIEW_BASE::Remove( &mod->Value() );
}

template <typename T>
void moveItemToBack(std::vector<T>& v, size_t itemIndex)
{
    typename std::vector<T>::iterator it = v.begin() + itemIndex;
    std::rotate(it, it + 1, v.end());
}

void PCB_VIEW::setupRenderOrder()
{

    m_renderOrder.clear();

    //SetMirror(true, false);

    //addLayer( -1, 0, 0, &m_defaultList, 0 ); // default layer

    addLayer( B_SilkS, B_SilkS, ITEM_GAL_LAYER( MOD_TEXT_BK_VISIBLE ), &m_modules, 0 );
    addLayer( B_SilkS, B_SilkS, B_SilkS, &m_items, 0 );
    addLayer( B_Paste, B_Paste, B_Paste, &m_items, 0 );
    addLayer( B_Adhes, B_Adhes, B_Adhes, &m_items, 0 );
    addLayer( B_Mask, B_Mask, B_Mask, &m_items, 0 );
    addLayer( B_SilkS, B_Cu, B_SilkS, &m_padsSMD, 1 );
    addLayer( B_SilkS, F_Cu, B_SilkS, &m_padsTHT, 1 );
    addLayer( B_Paste, B_Cu, B_Paste, &m_padsSMD, 2 );
    addLayer( B_Paste, F_Cu, B_Paste, &m_padsTHT, 2 );
    addLayer( B_Adhes, B_Cu, B_Adhes, &m_padsSMD, 3 );
    addLayer( B_Adhes, F_Cu, B_Adhes, &m_padsTHT, 3 );
    addLayer( B_Mask, B_Cu, B_Mask, &m_padsSMD, 4 );
    addLayer( B_Mask, F_Cu, B_Mask, &m_padsTHT, 4 );
    addLayer( B_Cu, B_Cu, B_Cu, &m_items, 0 );
    addLayer( B_Cu, B_Cu, NETNAMES_GAL_LAYER( B_Cu ), &m_items, -1, netnameLOD );
    addLayer( B_Cu, B_Cu, ITEM_GAL_LAYER( PAD_BK_VISIBLE ), &m_padsSMD, 0 );
    addLayer( B_Cu, B_Cu, ITEM_GAL_LAYER( PAD_BK_NETNAMES_VISIBLE ), &m_padsSMD, -1, netnameLOD );

    int lc = m_board->GetCopperLayerCount();

    for( int i = lc - 2; i >= In1_Cu; i-- )
    {
        addLayer( (LAYER_ID) i, (LAYER_ID) i, i, &m_items, 0 );
        addLayer( (LAYER_ID) i, (LAYER_ID) i, NETNAMES_GAL_LAYER( i ), &m_items, -1, netnameLOD );
        addLayer( (LAYER_ID) i, (LAYER_ID) i, ITEM_GAL_LAYER( VIAS_VISIBLE ), &m_vias, 0 );
        addLayer( (LAYER_ID) i, (LAYER_ID) i, ITEM_GAL_LAYER( VIAS_HOLES_VISIBLE ), &m_vias, 1 );
    }

    addLayer( F_Adhes, F_Cu, F_Adhes, &m_padsSMD, 1 );
    addLayer( F_Adhes, F_Cu, F_Adhes, &m_padsTHT, 5 );
    addLayer( F_Paste, F_Cu, F_Paste, &m_padsSMD, 2 );
    addLayer( F_Paste, F_Cu, F_Paste, &m_padsTHT, 6 );
    addLayer( F_SilkS, F_Cu, F_SilkS, &m_padsSMD, 3 );
    addLayer( F_SilkS, F_Cu, F_SilkS, &m_padsTHT, 7 );
    addLayer( F_Mask, F_Cu, F_Mask, &m_padsSMD, 4 );
    addLayer( F_Mask, F_Cu, F_Mask, &m_padsTHT, 8 );
    addLayer( F_Cu, F_Cu, F_Cu, &m_items, 0 );
    addLayer( F_Cu, F_Cu, NETNAMES_GAL_LAYER( F_Cu ), &m_items, -1, netnameLOD );
    addLayer( F_Cu, F_Cu, ITEM_GAL_LAYER( PAD_FR_VISIBLE ), &m_padsSMD, 0 );
    addLayer( F_Cu, F_Cu, ITEM_GAL_LAYER( PAD_FR_NETNAMES_VISIBLE ), &m_padsSMD, -1, netnameLOD );
    addLayer( F_Cu, F_Cu, ITEM_GAL_LAYER( PADS_VISIBLE ), &m_padsTHT, 0 );
    addLayer( F_Cu, F_Cu, ITEM_GAL_LAYER( PADS_HOLES_VISIBLE ), &m_padsTHT, 1 );
    addLayer( F_Cu, F_Cu, ITEM_GAL_LAYER( PADS_NETNAMES_VISIBLE ), &m_padsTHT, -2, netnameLOD );
    addLayer( F_Cu, F_Cu, ITEM_GAL_LAYER( VIAS_VISIBLE ), &m_vias, 0 );
    addLayer( F_Cu, F_Cu, ITEM_GAL_LAYER( VIAS_HOLES_VISIBLE ), &m_vias, 1 );

    addLayer( Dwgs_User, Dwgs_User, Dwgs_User, &m_items, 0 );
    addLayer( Cmts_User, Cmts_User, Cmts_User, &m_items, 0 );
    addLayer( Eco1_User, Eco1_User, Eco1_User, &m_items, 0 );
    addLayer( Eco2_User, Eco2_User, Eco2_User, &m_items, 0 );
    addLayer( Edge_Cuts, Edge_Cuts, Edge_Cuts, &m_items, 0 );

    addLayer( F_SilkS, F_SilkS, ITEM_GAL_LAYER( MOD_TEXT_FR_VISIBLE ), &m_modules, 0 );
    addLayer( F_SilkS, F_SilkS, F_SilkS, &m_items, 0 );

    int count = m_renderOrder.size();

    /*for ( std::vector<RENDER_LAYER>::iterator i = m_renderOrder.begin(); i != m_renderOrder.end(); )
    {
        if( i->sortLayer >= 0 && !( m_visibleLayers[i->sortLayer] ) )
            i = m_renderOrder.erase ( i );
        else
            i++;
    }

    for (int i = 0; i < count; i++)
    {
        if ( m_renderOrder[i].sortLayer == m_topLayer )
        {
            moveItemToBack( m_renderOrder, i );
            count--;
            i--;
        }
    }*/


}


void PCB_VIEW::Add( VIEW_ITEM* aItem, int aLayer )
{
    if (BOARD_ITEM *bitem = dynamic_cast <BOARD_ITEM *> (aItem))
    {
        switch( bitem->Type() )
        {
            case PCB_MODULE_T:
                addModule ( static_cast <MODULE *> (bitem ));
                break;

            case PCB_MODULE_TEXT_T:
            case PCB_MODULE_EDGE_T:
                VIEW_BASE::Add( aItem, L_MODULES );
                break;

            case PCB_PAD_T:
                VIEW_BASE::Add( aItem, L_PADS );
                break;

            case PCB_LINE_T:
            case PCB_TEXT_T:
            case PCB_MARKER_T:
            case PCB_DIMENSION_T:
            case PCB_TARGET_T:
                VIEW_BASE::Add( aItem, L_GRAPHICS );
                break;

            case PCB_TRACE_T:
                VIEW_BASE::Add( aItem, L_TRACKS );
                break;

            case PCB_VIA_T:
                VIEW_BASE::Add( aItem, L_VIAS );
                break;

            case PCB_ZONE_T:
            case PCB_ZONE_AREA_T:
                VIEW_BASE::Add( aItem, L_ZONES );
                break;

            default:
                VIEW_BASE::Add( aItem, DEFAULT_LAYER );
                break;

        }
    } else {
        VIEW_BASE::Add( aItem, DEFAULT_LAYER );
    }
}

void PCB_VIEW::Remove( VIEW_ITEM* aItem )
{
    if(MODULE *mod = dynamic_cast <MODULE *> (aItem))
    {
        removeModule ( mod );
        return;
    }

    VIEW_BASE::Remove ( aItem );
}

void PCB_VIEW::Update( VIEW_ITEM* aItem )
{
    VIEW_BASE::Update ( aItem );
}

void PCB_VIEW::SetTopLayer ( LAYER_ID aTopLayer )
{
    m_topLayer = aTopLayer;
    MarkDirty();
}

void PCB_VIEW::SetVisibleLayers ( LSET aLayers )
{
    m_visibleLayers = aLayers;
    MarkDirty();

}


}
