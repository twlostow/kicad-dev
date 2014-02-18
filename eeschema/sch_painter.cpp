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


/// @ todo: this is work in progress. Cleanup will follow.

#include <sch_item_struct.h>
#include <sch_painter.h>

#include <lib_draw_item.h>
#include <lib_rectangle.h>
#include <lib_pin.h>
#include <lib_circle.h>
#include <lib_polyline.h>
#include <lib_arc.h>
#include <lib_field.h>
#include <lib_text.h>
#include <sch_component.h>
#include <sch_field.h>
#include <drawtxt.h>

#include <template_fieldnames.h>
#include <class_libentry.h>
#include <class_library.h>

#include <gal/graphics_abstraction_layer.h>
#include <class_colors_design_settings.h>

#include <boost/foreach.hpp>

namespace KIGFX {

SCH_RENDER_SETTINGS::SCH_RENDER_SETTINGS()
{
  m_backgroundColor = COLOR4D(1.0, 1.0, 1.0, 1.0);
}
    
void SCH_RENDER_SETTINGS::ImportLegacyColors( COLORS_DESIGN_SETTINGS* aSettings )
{

}

const COLOR4D& SCH_RENDER_SETTINGS::GetColor( const VIEW_ITEM* aItem, int aLayer ) const
{
  return COLOR4D();
}

SCH_PAINTER::SCH_PAINTER( GAL* aGal ) :
	KIGFX::PAINTER (aGal)
	{
	    m_settings.reset( new SCH_RENDER_SETTINGS() );
	}

#define HANDLE_ITEM(type_id, type_name) \
  case type_id: \
    draw( (const type_name *) item, aLayer ); \
    break;

bool SCH_PAINTER::Draw( const VIEW_ITEM *aItem, int aLayer )
{
	const EDA_ITEM *item = static_cast<const EDA_ITEM *>(aItem);
	
	switch(item->Type())
	{
		HANDLE_ITEM(LIB_COMPONENT_T, LIB_COMPONENT);
	  HANDLE_ITEM(LIB_RECTANGLE_T, LIB_RECTANGLE);
    HANDLE_ITEM(LIB_POLYLINE_T, LIB_POLYLINE);
    HANDLE_ITEM(LIB_CIRCLE_T, LIB_CIRCLE);
    HANDLE_ITEM(LIB_PIN_T, LIB_PIN);
    HANDLE_ITEM(LIB_ARC_T, LIB_ARC);
    HANDLE_ITEM(LIB_FIELD_T, LIB_FIELD);
  	HANDLE_ITEM(LIB_TEXT_T, LIB_TEXT);
    HANDLE_ITEM(SCH_COMPONENT_T, SCH_COMPONENT);
    HANDLE_ITEM(SCH_FIELD_T, SCH_FIELD);
    	
		default:
			return false;
	}
	return false;
}

void SCH_PAINTER::draw ( const LIB_COMPONENT *aComp, int aLayer )
{
	const LIB_ITEMS& drawItemsList = aComp->GetCDrawItemList();

	for( LIB_ITEMS::const_iterator i = drawItemsList.begin(); i != drawItemsList.end(); ++i)
	{
		Draw (& (*i), aLayer);
	}
}

void SCH_PAINTER::draw ( const SCH_COMPONENT *aComp, int aLayer )
{
  LIB_COMPONENT* entry = CMP_LIBRARY::FindLibraryComponent( aComp->GetLibName() );

  if (!entry)
    return;

  const LIB_ITEMS& drawItemsList = entry->GetCDrawItemList();

  for( LIB_ITEMS::const_iterator i = drawItemsList.begin(); i != drawItemsList.end(); ++i)
  {
    if( i->GetConvert() && i-> GetConvert() != aComp->GetConvert() )
      continue;

    if ( aComp->GetUnit() && i->GetUnit() && ( i -> GetUnit () != aComp->GetUnit() ) ) 
      continue;
    
    if( i->Type() == LIB_FIELD_T )
      continue;
    
      Draw (& (*i), aLayer);  
  }
    
  SCH_FIELD* field = aComp->GetField( REFERENCE );
    
  Draw (field, aLayer);
  for( int ii = VALUE; ii < aComp->GetFieldCount(); ii++ )
  {
    field = aComp->GetField( ii );
    Draw(field, aLayer);
  }
}

void SCH_PAINTER::draw ( const LIB_RECTANGLE *aComp, int aLayer )
{
  
	defaultColors(aComp);
	m_gal->DrawRectangle( aComp->GetPosition(), aComp->GetEnd() );
}

void SCH_PAINTER::triLine ( const VECTOR2D &a, const VECTOR2D &b, const VECTOR2D &c )
{
  m_gal->DrawLine ( a, b );
  m_gal->DrawLine ( b, c );
}


void SCH_PAINTER::defaultColors ( const LIB_ITEM *aItem )
{
  const COLOR4D& fg = m_settings -> TranslateColor ( GetLayerColor ( LAYER_DEVICE ) );
  const COLOR4D& bg = m_settings -> TranslateColor ( GetLayerColor ( LAYER_DEVICE_BACKGROUND ) );
  
  m_gal->SetIsStroke (true);
  m_gal->SetStrokeColor( fg );
  m_gal->SetLineWidth ( aItem->GetPenSize() );
  switch(aItem->GetFillMode())
  {
    case FILLED_WITH_BG_BODYCOLOR:
      m_gal->SetIsFill(true);
      m_gal->SetFillColor ( bg );
      break;

    case FILLED_SHAPE:
      m_gal->SetIsFill(true);
      m_gal->SetFillColor ( fg );
      break;
    default:
      m_gal->SetIsFill(false);
  } 
}

void SCH_PAINTER::draw ( const LIB_CIRCLE *aCircle, int aLayer )
{
  
  defaultColors(aCircle);
  m_gal->DrawCircle( aCircle->GetPosition(), aCircle->GetRadius() );
}

void SCH_PAINTER::draw ( const LIB_ARC *aArc, int aLayer )
{
  defaultColors(aArc);

  int sai = aArc->GetStartAngle();
  int eai = aArc->GetEndAngle();

  if (TRANSFORM().MapAngles( &sai, &eai ))
    std::swap(sai, eai);
  
  double sa = (double) sai * M_PI / 1800.0;
  double ea = (double) eai * M_PI / 1800.0 ;

  VECTOR2D pos = aArc->GetPosition();

  //printf("sai %d eai %d\n", sai, eai);
  pos.y = -pos.y;

  m_gal->DrawArc( pos, aArc->GetRadius(), sa, ea);
  /*m_gal->SetStrokeColor(COLOR4D(1.0,0,0,1.0));
  m_gal->DrawLine ( pos - VECTOR2D(20, 20), pos + VECTOR2D(20, 20));
  m_gal->DrawLine ( pos - VECTOR2D(-20, 20), pos + VECTOR2D(-20, 20));*/
     
}


void SCH_PAINTER::draw ( const LIB_FIELD *aField, int aLayer )
{

    if(!aField->IsVisible())
      return;

    int w;

   if( aField->IsBold() )
      w = GetPenSizeForBold( aField->GetWidth() );
    else
      w = aField->GetPenSize();

    m_gal->SetLineWidth(w);
    m_gal->SetIsFill( false );
    m_gal->SetIsStroke (true);
    m_gal->SetStrokeColor( m_settings->TranslateColor ( aField-> GetDefaultColor () ));
    m_gal->SetGlyphSize ( aField->GetSize() );
    
    m_gal->SetHorizontalJustify( aField->GetHorizJustify( ));
    m_gal->SetVerticalJustify( aField->GetVertJustify( ));

    
    const VECTOR2D pos = aField->GetTextPosition();
    double orient = aField->GetOrientation() * M_PI / 1800.0 + M_PI;


    m_gal->StrokeText( aField->GetText(), pos, orient );
}

void SCH_PAINTER::draw ( const SCH_FIELD *aField, int aLayer )
{

    if(!aField->IsVisible())
      return;

    int w;

   if( aField->IsBold() )
      w = GetPenSizeForBold( aField->GetWidth() );
    else
      w = aField->GetPenSize();

    m_gal->SetLineWidth(w);
    m_gal->SetIsFill( false );
    m_gal->SetIsStroke (true);

    EDA_COLOR_T color;

    switch(aField->GetId())
    {
      case REFERENCE:
        color = GetLayerColor( LAYER_REFERENCEPART );
        break;
      case VALUE:
        color = GetLayerColor( LAYER_VALUEPART );
        break;
      default:
        color = GetLayerColor( LAYER_FIELDS );
        break;
    }
    
    m_gal->SetStrokeColor( m_settings->TranslateColor ( color ));
    m_gal->SetGlyphSize ( aField->GetSize() );
    
    m_gal->SetHorizontalJustify( aField->GetHorizJustify( ));
    m_gal->SetVerticalJustify( aField->GetVertJustify( ));

    
    const VECTOR2D pos = aField->GetTextPosition();
    double orient = aField->GetOrientation() * M_PI / 1800.0 + M_PI;

    m_gal->StrokeText( aField->GetFullyQualifiedText(), pos, orient );
}


void SCH_PAINTER::draw ( const LIB_POLYLINE *aLine, int aLayer )
{
    
  defaultColors(aLine);
  std::deque<VECTOR2D> vtx;

  for( int i = 0; i < aLine->GetCornerCount(); i++)
    vtx.push_back ( VECTOR2D ( aLine->Corner (i)));

  if( aLine->GetFillMode() == FILLED_WITH_BG_BODYCOLOR || aLine->GetFillMode() == FILLED_SHAPE)
    vtx.push_back ( VECTOR2D ( aLine->Corner(0)));

  m_gal->DrawPolygon ( vtx );
}

void SCH_PAINTER::draw ( const LIB_TEXT *aText, int aLayer )
{
  if(!aText->IsVisible())
      return;
    
    int w = aText->GetPenSize();

    m_gal->SetLineWidth(w);
    m_gal->SetIsFill( false );
    m_gal->SetIsStroke (true);
    m_gal->SetStrokeColor( m_settings->TranslateColor ( aText-> GetDefaultColor () ));
    m_gal->SetGlyphSize ( aText->GetSize() );
    
    m_gal->SetHorizontalJustify( GR_TEXT_HJUSTIFY_CENTER );
    m_gal->SetVerticalJustify( GR_TEXT_VJUSTIFY_CENTER );

    EDA_RECT bBox = aText->GetBoundingBox();
    const VECTOR2D pos = bBox.Centre();

    double orient;

    if (aText -> GetOrientation() == TEXT_ORIENT_HORIZ )
      orient = M_PI;
    else
      orient = M_PI / 2;
    
    m_gal->SetBold ( aText->IsBold() );
    m_gal->SetItalic ( aText->IsItalic() );
    m_gal->StrokeText( aText->GetText(), pos, orient );
}


  

void SCH_PAINTER::draw ( const LIB_PIN *aPin, int aLayer )
{
	
  if( !aPin->IsVisible() )
    return;

  const COLOR4D& color = m_settings->TranslateColor ( GetLayerColor( LAYER_PIN ) ); 

	VECTOR2I pos ( aPin->GetPosition() ), p0, dir;
	int len = aPin->GetLength();
	int width = aPin->GetPenSize();
	int shape = aPin->GetShape();
  int orient = aPin->GetOrientation();
	
  switch( orient )
	{
		case PIN_UP:
			p0 = VECTOR2I( pos.x, pos.y + len );
			dir = VECTOR2I(0, 1);
			break;
		case PIN_DOWN:
			p0 = VECTOR2I( pos.x, pos.y - len );
			dir = VECTOR2I(0, -1);
			break;
		case PIN_LEFT:
			p0 = VECTOR2I( pos.x - len, pos.y );
			dir = VECTOR2I(1, 0);
			break;
		case PIN_RIGHT:
			p0 = VECTOR2I( pos.x + len, pos.y );
			dir = VECTOR2I(-1, 0);
			break;

	}

  VECTOR2D pc;
  
  m_gal->SetIsStroke ( true );
  m_gal->SetIsFill ( false );
  m_gal->SetLineWidth ( width );
  m_gal->SetStrokeColor ( color );
  m_gal->SetBold ( false );
  m_gal->SetItalic ( false );

	if(shape & INVERT)
	{

		m_gal->DrawCircle ( p0 + dir * INVERT_PIN_RADIUS, INVERT_PIN_RADIUS );
		m_gal->DrawLine ( p0 + dir * ( 2 * INVERT_PIN_RADIUS ), pos );
	} else if (shape & CLOCK_FALL )
  {
    pc = p0 + dir * CLOCK_PIN_DIM ;

    triLine( p0 + VECTOR2D ( dir.y, -dir.x) * CLOCK_PIN_DIM, 
            pc, 
            p0 + VECTOR2D ( -dir.y, dir.x) * CLOCK_PIN_DIM
            );
    
    m_gal->DrawLine ( pos, pc );
  }
  else {
    m_gal->DrawLine ( p0, pos );
  }

  if(shape & CLOCK)
  {
    if (!dir.y)
    {
      triLine (p0 + VECTOR2D( 0, CLOCK_PIN_DIM ),
                p0 + VECTOR2D( -dir.x * CLOCK_PIN_DIM, 0),
                p0 + VECTOR2D( 0, -CLOCK_PIN_DIM ));
      
    } else {
      triLine ( p0 + VECTOR2D ( CLOCK_PIN_DIM, 0 ),
                p0 + VECTOR2D ( 0, -dir.y * CLOCK_PIN_DIM ),
                p0 + VECTOR2D ( -CLOCK_PIN_DIM, 0 ));
    }
  }
    
  if( shape & LOWLEVEL_IN ) 
  {
    if(!dir.y)
        {
            triLine ( p0 + VECTOR2D(dir.x, 0) * IEEE_SYMBOL_PIN_DIM * 2,
                      p0 + VECTOR2D(dir.x, -1) * IEEE_SYMBOL_PIN_DIM * 2,
                      p0 );
        }
        else    /* MapX1 = 0 */
        {
            triLine ( p0 + VECTOR2D( 0, dir.y) * IEEE_SYMBOL_PIN_DIM * 2,
                      p0 + VECTOR2D(-1, dir.y) * IEEE_SYMBOL_PIN_DIM * 2,
                      p0 );
        }
    }

    if( shape & LOWLEVEL_OUT )    /* IEEE symbol "Active Low Output" */
    {
        if(!dir.y)
          m_gal->DrawLine( p0 - VECTOR2D(0, IEEE_SYMBOL_PIN_DIM), p0 + VECTOR2D(dir.x, 1) * IEEE_SYMBOL_PIN_DIM * 2);
        else
          m_gal->DrawLine (p0 - VECTOR2D(IEEE_SYMBOL_PIN_DIM, 0), p0 + VECTOR2D(0, dir.y) * IEEE_SYMBOL_PIN_DIM * 2);
    }

    if( shape & NONLOGIC ) /* NonLogic pin symbol */
    {
        m_gal->DrawLine ( p0 - VECTOR2D(dir.x + dir.y, dir.y - dir.x) * NONLOGIC_PIN_DIM, 
                          p0 + VECTOR2D(dir.x + dir.y, dir.y - dir.x) * NONLOGIC_PIN_DIM); 
        m_gal->DrawLine ( p0 - VECTOR2D(dir.x - dir.y, dir.x + dir.y) * NONLOGIC_PIN_DIM, 
                          p0 + VECTOR2D(dir.x - dir.y, dir.x + dir.y) * NONLOGIC_PIN_DIM); 
    }

    #define NCSYMB_PIN_DIM TARGET_PIN_RADIUS

    if( aPin->GetType() == PIN_NC )   // Draw a N.C. symbol
    {
        m_gal->DrawLine ( pos + VECTOR2D(-1, -1) * NCSYMB_PIN_DIM,
                          pos + VECTOR2D(1, 1) * NCSYMB_PIN_DIM);
        m_gal->DrawLine ( pos + VECTOR2D(1, -1) * NCSYMB_PIN_DIM,
                          pos + VECTOR2D(-1, 1) * NCSYMB_PIN_DIM);
    }

    m_gal->SetLineWidth ( 0.0 );
    m_gal->DrawCircle( pos, TARGET_PIN_RADIUS );

// Draw the labels


    int labelWidth = std::min ( GetDefaultLineThickness(), width );
    LIB_COMPONENT* libEntry = (const_cast<LIB_PIN *> (aPin)) ->GetParent();
    m_gal->SetLineWidth ( labelWidth );
    wxString    stringPinNum;
                
    /* Get the num and name colors */
    COLOR4D nameColor = m_settings->TranslateColor ( GetLayerColor( LAYER_PINNAM ) );
    COLOR4D numColor  = m_settings->TranslateColor ( GetLayerColor( LAYER_PINNUM ) );

    /* Create the pin num string */
    aPin->ReturnPinStringNum( stringPinNum );
    int textOffset = libEntry->GetPinNameOffset();

    bool showNums = libEntry->ShowPinNumbers();
    bool showNames = libEntry->ShowPinNames();

    m_gal->SetGlyphSize ( VECTOR2D ( aPin->GetNameTextSize(), aPin->GetNameTextSize()) );
    m_gal->SetMirrored ( true ); // don't know why yet...
    m_gal->SetStrokeColor ( nameColor );
    m_gal->SetVerticalJustify( GR_TEXT_VJUSTIFY_CENTER );
        
    if( textOffset )  /* Draw the text inside, but the pin numbers outside. */
    {
      
        if(showNames)      
        {
        switch ( orient )
        {
          case PIN_LEFT:
            m_gal->SetHorizontalJustify( GR_TEXT_HJUSTIFY_RIGHT );
            m_gal->StrokeText( aPin->GetName(), pos + VECTOR2D ( -textOffset - len, 0 ), -M_PI );
            break;
          case PIN_RIGHT:
            m_gal->SetHorizontalJustify( GR_TEXT_HJUSTIFY_LEFT );
            m_gal->StrokeText( aPin->GetName(), pos + VECTOR2D ( textOffset + len, 0 ), -M_PI );
            break;
          case PIN_DOWN:
            m_gal->SetHorizontalJustify ( GR_TEXT_HJUSTIFY_RIGHT );
            m_gal->StrokeText ( aPin->GetName(), pos + VECTOR2D ( 0, -textOffset - len), M_PI / 2);
            break;
          case PIN_UP:
            m_gal->SetHorizontalJustify ( GR_TEXT_HJUSTIFY_LEFT );
            m_gal->StrokeText ( aPin->GetName(), pos + VECTOR2D ( 0, textOffset + len), M_PI / 2);
            break;
        }
      }
       
      if(showNums)
      {
        m_gal->SetStrokeColor( numColor );
        m_gal->SetGlyphSize ( VECTOR2D ( aPin->GetNumberTextSize(), aPin->GetNumberTextSize()) );
        m_gal->SetHorizontalJustify( GR_TEXT_HJUSTIFY_CENTER );
        m_gal->SetVerticalJustify( GR_TEXT_VJUSTIFY_BOTTOM );
        
        switch(orient)
        {
          case PIN_LEFT:
          case PIN_RIGHT:
            m_gal->StrokeText (stringPinNum, VECTOR2D( (p0.x + pos.x) / 2, p0.y - TXTMARGE ), -M_PI );
            break;
          case PIN_DOWN:
          case PIN_UP:
            m_gal->StrokeText (stringPinNum, VECTOR2D ( p0.x - TXTMARGE, (p0.y + pos.y) / 2), M_PI / 2);
           break;
        }
      }
    }
}

        
}; // namespace KIGFX