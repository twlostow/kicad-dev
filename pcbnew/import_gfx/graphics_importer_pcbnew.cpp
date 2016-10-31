/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
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

#include "graphics_importer_pcbnew.h"

#include <class_edge_mod.h>
#include <class_pcb_text.h>
#include <class_text_mod.h>
#include <tuple>

using namespace std;

void GRAPHICS_IMPORTER_PCBNEW::AddLine( const wxPoint& aOrigin, const wxPoint& aEnd )
{
    unique_ptr<DRAWSEGMENT> line( createDrawing() );
    line->SetShape( S_SEGMENT );
    line->SetLayer( GetLayer() );
    line->SetWidth( GetLineWidth() );
    line->SetStart( aOrigin * GetScale() );
    line->SetEnd( aEnd * GetScale() );
    addItem( std::move( line ) );
}


void GRAPHICS_IMPORTER_PCBNEW::AddCircle( const wxPoint& aCenter, unsigned int aRadius )
{
    unique_ptr<DRAWSEGMENT> circle( createDrawing() );
    circle->SetShape( S_CIRCLE );
    circle->SetLayer( GetLayer() );
    circle->SetWidth( GetLineWidth() );
    circle->SetCenter( aCenter * GetScale() );
    circle->SetArcStart( wxPoint( aCenter.x + aRadius, aCenter.y ) * GetScale() );
    addItem( std::move( circle ) );
}


void GRAPHICS_IMPORTER_PCBNEW::AddArc( const wxPoint& aCenter, const wxPoint& aStart, double aAngle )
{
    unique_ptr<DRAWSEGMENT> arc( createDrawing() );
    arc->SetShape( S_ARC );
    arc->SetLayer( GetLayer() );
    arc->SetWidth( GetLineWidth() );
    arc->SetCenter( aCenter * GetScale() );
    arc->SetArcStart( aStart * GetScale() );
    arc->SetAngle( aAngle );
    addItem( std::move( arc ) );
}


void GRAPHICS_IMPORTER_PCBNEW::AddPolygon( const std::vector< wxPoint >& aVertices )
{
    unique_ptr<DRAWSEGMENT> polygon( createDrawing() );
    polygon->SetShape( S_POLYGON );
    polygon->SetLayer( GetLayer() );
    polygon->SetPolyPoints( aVertices );
    addItem( std::move( polygon ) );
}


void GRAPHICS_IMPORTER_PCBNEW::AddText( const wxPoint& aOrigin, const wxString& aText,
        unsigned int aHeight, unsigned aWidth, double aOrientation,
        EDA_TEXT_HJUSTIFY_T aHJustify, EDA_TEXT_VJUSTIFY_T aVJustify )
{
    unique_ptr<BOARD_ITEM> boardItem;
    EDA_TEXT* textItem;
    tie( boardItem, textItem ) = createText();
    boardItem->SetLayer( GetLayer() );
    textItem->SetThickness( GetLineWidth() );
    textItem->SetTextPosition( aOrigin * GetScale() );
    textItem->SetOrientation( aOrientation );
    textItem->SetWidth( aWidth * GetScale() );
    textItem->SetHeight( aHeight * GetScale() );
    textItem->SetVertJustify( aVJustify );
    textItem->SetHorizJustify( aHJustify );
    textItem->SetText( aText );
    addItem( std::move( boardItem ) );
}


unique_ptr<DRAWSEGMENT> GRAPHICS_IMPORTER_BOARD::createDrawing() const
{
    return unique_ptr<DRAWSEGMENT>( new DRAWSEGMENT() );
}


pair<unique_ptr<BOARD_ITEM>, EDA_TEXT*> GRAPHICS_IMPORTER_BOARD::createText() const
{
    TEXTE_PCB* text = new TEXTE_PCB( nullptr );
    return make_pair( unique_ptr<BOARD_ITEM>( text ), static_cast<EDA_TEXT*>( text ) );
}


unique_ptr<DRAWSEGMENT> GRAPHICS_IMPORTER_MODULE::createDrawing() const
{
    return unique_ptr<DRAWSEGMENT>( new EDGE_MODULE( nullptr ) );
}


pair<unique_ptr<BOARD_ITEM>, EDA_TEXT*> GRAPHICS_IMPORTER_MODULE::createText() const
{
    TEXTE_MODULE* text = new TEXTE_MODULE( nullptr );
    return make_pair( unique_ptr<BOARD_ITEM>( text ), static_cast<EDA_TEXT*>( text ) );
}
