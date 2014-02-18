/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
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

#include <sch_item_struct.h>
#include <sch_component_preview_panel.h>
#include <sch_painter.h>
#include <sch_component.h>

#include <gal/graphics_abstraction_layer.h>
#include <gal/color4d.h>

#include <class_library.h>

SCH_COMPONENT_PREVIEW_PANEL::SCH_COMPONENT_PREVIEW_PANEL( wxWindow *aParent ) :
	m_currentComponent (NULL),
	EDA_DRAW_PANEL_GAL (aParent, wxID_ANY, wxDefaultPosition, wxDefaultSize, EDA_DRAW_PANEL_GAL::GAL_TYPE_CAIRO )
{


	SetPainter ( new KIGFX::SCH_PAINTER ( GetGAL() ));

	m_gal->SetFlip( false, true  );
	m_gal->SetGridSize( VECTOR2D( 50, 50 ) );
	m_gal->SetGridDrawThreshold ( 5 );
	m_gal->SetGridColor ( KIGFX::COLOR4D( 0.75, 0.75, 0.75, 1.0 ));
    
	m_view->SetLayerVisible( 0, true );
	m_view->SetLayerVisible( 1, true );
	
	m_view->SetLayerTarget( 0, KIGFX::TARGET_NONCACHED );
	m_view->SetLayerTarget( 1, KIGFX::TARGET_NONCACHED );

	m_view->SetPanBoundary ( BOX2I( VECTOR2I ( -1000000, -1000000 ), VECTOR2I(2000000, 2000000 )));
}

SCH_COMPONENT_PREVIEW_PANEL::~SCH_COMPONENT_PREVIEW_PANEL()
{
	if(m_currentComponent)
	{
		m_view->Remove (m_currentComponent);
		delete m_currentComponent;
	}
}

void SCH_COMPONENT_PREVIEW_PANEL::PreviewComponent( LIB_COMPONENT *aComponent, int aUnit )
{
	if(m_currentComponent)
	{
		m_view->Remove(m_currentComponent);
		delete m_currentComponent;
	}

	m_currentComponent = NULL;
	
	if(!aComponent)
		return;

	m_currentComponent = new SCH_COMPONENT( *aComponent, NULL, 1 );
	m_currentComponent->SetConvert(1);
	m_currentComponent->SetUnit ( aUnit );

	if(m_currentComponent)
	{
		m_currentComponent->ViewSetVisible( true );
		m_view->Add(m_currentComponent);

		BOX2I ext = m_view->CalculateExtents();
		ext.Inflate( std::max (ext.GetWidth(), ext.GetHeight()) / 20 );
		m_view->SetViewport( BOX2D( ext.GetPosition(), ext.GetSize() ) );
	}

	Refresh();
}