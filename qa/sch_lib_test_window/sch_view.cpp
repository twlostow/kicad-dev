/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013-2017 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
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
 */

#include <memory>
#include <view/view.h>

#include <view/wx_view_controls.h>
#include <worksheet_viewitem.h>
#include <layers_id_colors_and_visibility.h>

#include <class_libentry.h>

#include "sch_view.h"

#include <sch_sheet.h>
#include <sch_screen.h>

namespace KIGFX {

SCH_VIEW::SCH_VIEW( bool aIsDynamic ) :
    VIEW( aIsDynamic )
{
}

SCH_VIEW::~SCH_VIEW()
{
}

void SCH_VIEW::Add( KIGFX::VIEW_ITEM* aItem, int aDrawPriority )
{
    auto ei = static_cast<EDA_ITEM*>(aItem);
    printf("Add %p [%s]\n", aItem, (const char *)ei->GetClass().c_str());
    VIEW::Add( aItem, aDrawPriority );
}


void SCH_VIEW::Remove( KIGFX::VIEW_ITEM* aItem )
{
    VIEW::Remove( aItem );
}


void SCH_VIEW::Update( KIGFX::VIEW_ITEM* aItem, int aUpdateFlags )
{
    VIEW::Update( aItem, aUpdateFlags );
}


void SCH_VIEW::Update( KIGFX::VIEW_ITEM* aItem )
{
    SCH_VIEW::Update( aItem, -1 );
}

void SCH_VIEW::SetWorksheet( KIGFX::WORKSHEET_VIEWITEM* aWorksheet )
{

}

static const LAYER_NUM SCH_LAYER_ORDER[] =
{
    LAYER_GP_OVERLAY,
    LAYER_DRC,
    LAYER_WORKSHEET
};

void SCH_VIEW::DisplaySheet( SCH_SHEET *aSheet )
{
    auto sc = aSheet->GetScreen();

    for( auto item = sc->GetDrawItems(); item; item = item->Next() )
    {
        printf("-- ADD SCHITEM %p\n", item );
        Add(item);
    }
}

void SCH_VIEW::DisplayComponent( LIB_PART *aPart )
{
    for ( auto &item : aPart->GetDrawItems() )
    {
        printf("-- ADD %p\n", &item );
        Add( &item );
    }
}


};
