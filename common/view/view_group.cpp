/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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
 *
 */

/**
 * @file view_group.cpp
 * @brief VIEW_GROUP extends VIEW_ITEM by possibility of grouping items into a single object.
 * VIEW_GROUP does not take over ownership of the held items. The main purpose of this class is
 * to group items and draw them on a single layer (in particular the overlay).
 */

#include <set>
#include <algorithm>
#include <view/view_item_ng.h>
#include <view/view_group.h>
#include <view/view_ng.h>
#include <painter.h>
#include <gal/graphics_abstraction_layer.h>
#include <boost/foreach.hpp>
#include <layers_id_colors_and_visibility.h>

using namespace KIGFX;

VIEW_GROUP::VIEW_GROUP( VIEW_BASE* aView ) :
    m_layer( ITEM_GAL_LAYER( GP_OVERLAY ) )
{
    m_view = aView;
}


VIEW_GROUP::~VIEW_GROUP()
{
}


void VIEW_GROUP::Add( VIEW_ITEM_NG* aItem )
{
    m_items.insert( aItem );
}


void VIEW_GROUP::Remove( VIEW_ITEM_NG* aItem )
{
    m_items.erase( aItem );
}


void VIEW_GROUP::Clear()
{
    m_items.clear();
}


unsigned int VIEW_GROUP::GetSize() const
{
    return m_items.size();
}


const BOX2I VIEW_GROUP::ngViewBBox() const
{
    BOX2I maxBox;

    maxBox.SetMaximum();
    return maxBox;
}

void VIEW_GROUP::ngViewGetLayers( int aLayers[], int& aCount ) const
{
    aLayers[0] = VIEW_BASE::DEFAULT_OVERLAY;
    aCount = 1;
}

void VIEW_GROUP::ngViewDraw( int aLayer, VIEW_BASE* aView ) const
{
    PAINTER* painter = aView->GetPainter();
    GAL *gal = aView->GetGAL();

    // Draw all items immediately (without caching)
    BOOST_FOREACH( VIEW_ITEM_NG* item, m_items )
    {
        gal->PushDepth();

        int layers[16], layers_count;
        item->ngViewGetLayers( layers, layers_count );

        for( int i = 0; i < layers_count; i++ )
        {
            gal->AdvanceDepth();

            if( !painter->Draw( item, layers[i] ) )
                item->ngViewDraw( layers[i], aView ); // Alternative drawing method
        }

        gal->PopDepth();
    }
}

#if 0

#endif

void VIEW_GROUP::FreeItems()
{
    BOOST_FOREACH( VIEW_ITEM_NG* item, m_items )
    {
        delete item;
    }
    m_items.clear();
}

#if 0

void VIEW_GROUP::ItemsSetVisibility( bool aVisible )
{
/*    std::set<VIEW_ITEM*>::const_iterator it, it_end;

    for( it = m_items.begin(), it_end = m_items.end(); it != it_end; ++it )
        (*it)->ViewSetVisible( aVisible );*/
}


void VIEW_GROUP::ItemsViewUpdate( VIEW_ITEM::VIEW_UPDATE_FLAGS aFlags )
{
/*    std::set<VIEW_ITEM*>::const_iterator it, it_end;

    for( it = m_items.begin(), it_end = m_items.end(); it != it_end; ++it )
        (*it)->ViewUpdate( aFlags );*/
}
#endif


void VIEW_GROUP::updateBbox()
{
    // Save the used VIEW, as it used nulled during Remove()
    VIEW_BASE* view = m_view;

    // Reinsert the group, so the bounding box can be updated
    view->Remove( this );
    view->Add( this );
}
