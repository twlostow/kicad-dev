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

#include <lib_draw_item.h>
#include <lib_rectangle.h>
#include <lib_pin.h>
#include <lib_circle.h>
#include <lib_polyline.h>
#include <lib_arc.h>
#include <lib_field.h>
#include <lib_text.h>
#include <sch_line.h>
#include <sch_component.h>
#include <sch_field.h>
#include <sch_junction.h>
#include <sch_text.h>
#include <draw_graphic_text.h>

#include <template_fieldnames.h>
#include <class_libentry.h>
#include <class_library.h>

#include <gal/graphics_abstraction_layer.h>
#include <colors_design_settings.h>

#include "sch_painter.h"

#include <draw_graphic_text.h>

namespace KIGFX {
    struct COLOR_DEF
    {
        std::string configName;
        SCH_LAYER_ID layer;
        COLOR4D color;

        COLOR_DEF( std::string name, SCH_LAYER_ID layer_, COLOR4D color_ )
        {
            configName = name;
            layer = layer_;
            color = color_;
        }
    };

    const std::vector<COLOR_DEF> defaultColors =
    {
            COLOR_DEF( "Color4DWireEx",             LAYER_WIRE,                 COLOR4D( GREEN ) ),
            COLOR_DEF( "Color4DBusEx",              LAYER_BUS,                  COLOR4D( BLUE ) ),
            COLOR_DEF( "Color4DConnEx",             LAYER_JUNCTION,             COLOR4D( GREEN ) ),
            COLOR_DEF( "Color4DLLabelEx",           LAYER_LOCLABEL,             COLOR4D( BLACK ) ),
            COLOR_DEF( "Color4DHLabelEx",           LAYER_HIERLABEL,            COLOR4D( BROWN ) ),
            COLOR_DEF( "Color4DGLabelEx",           LAYER_GLOBLABEL,            COLOR4D( RED ) ),
            COLOR_DEF( "Color4DPinNumEx",           LAYER_PINNUM,               COLOR4D( RED ) ),
            COLOR_DEF( "Color4DPinNameEx",          LAYER_PINNAM,               COLOR4D( CYAN ) ),
            COLOR_DEF( "Color4DFieldEx",            LAYER_FIELDS,               COLOR4D( MAGENTA ) ),
            COLOR_DEF( "Color4DReferenceEx",        LAYER_REFERENCEPART,        COLOR4D( CYAN ) ),
            COLOR_DEF( "Color4DValueEx",            LAYER_VALUEPART,            COLOR4D( CYAN ) ),
            COLOR_DEF( "Color4DNoteEx",             LAYER_NOTES,                COLOR4D( LIGHTBLUE ) ),
            COLOR_DEF( "Color4DBodyEx",             LAYER_DEVICE,               COLOR4D( RED ) ),
            COLOR_DEF( "Color4DBodyBgEx",           LAYER_DEVICE_BACKGROUND,    COLOR4D( LIGHTYELLOW ) ),
            COLOR_DEF( "Color4DNetNameEx",          LAYER_NETNAM,               COLOR4D( DARKGRAY ) ),
            COLOR_DEF( "Color4DPinEx",              LAYER_PIN,                  COLOR4D( RED ) ),
            COLOR_DEF( "Color4DSheetEx",            LAYER_SHEET,                COLOR4D( MAGENTA ) ),
            COLOR_DEF( "Color4DSheetFileNameEx",    LAYER_SHEETFILENAME,        COLOR4D( BROWN ) ),
            COLOR_DEF( "Color4DSheetNameEx",        LAYER_SHEETNAME,            COLOR4D( CYAN ) ),
            COLOR_DEF( "Color4DSheetLabelEx",       LAYER_SHEETLABEL,           COLOR4D( BROWN ) ),
            COLOR_DEF( "Color4DNoConnectEx",        LAYER_NOCONNECT,            COLOR4D( BLUE ) ),
            COLOR_DEF( "Color4DErcWEx",             LAYER_ERC_WARN,             COLOR4D( GREEN ) ),
            COLOR_DEF( "Color4DErcEEx",             LAYER_ERC_ERR,              COLOR4D( RED ) ),
            COLOR_DEF( "Color4DGridEx",             LAYER_SCHEMATIC_GRID,       COLOR4D( DARKGRAY ) ),
            COLOR_DEF( "Color4DBgCanvasEx",         LAYER_SCHEMATIC_BACKGROUND, COLOR4D( WHITE ) ),
            COLOR_DEF( "Color4DBrighenedEx",        LAYER_BRIGHTENED,           COLOR4D( PUREMAGENTA ) )
        };

SCH_RENDER_SETTINGS::SCH_RENDER_SETTINGS()
{
  m_backgroundColor = COLOR4D(1.0, 1.0, 1.0, 1.0);



  for ( const auto& l : defaultColors )
  {
      auto c = l.color;
      printf("%.2f %.2f %.2f %.2f\n", c.r, c.g, c.b, c.a );
        m_layerColors[l.layer] = c;
  }

}

void SCH_RENDER_SETTINGS::ImportLegacyColors( const COLORS_DESIGN_SETTINGS* aSettings )
{

}

const COLOR4D& SCH_RENDER_SETTINGS::GetColor( const VIEW_ITEM* aItem, int aLayer ) const
{
  static COLOR4D col(0.0, 0.0, 0.0, 1.0);
  return col;
}

SCH_PAINTER::SCH_PAINTER( GAL* aGal ) :
	KIGFX::PAINTER (aGal)
	{

	}

#define HANDLE_ITEM(type_id, type_name) \
  case type_id: \
    draw( (type_name *) item, aLayer ); \
    break;

bool SCH_PAINTER::Draw( const VIEW_ITEM *aItem, int aLayer )
{
	auto item2 = static_cast<const EDA_ITEM *>(aItem);
    auto item = const_cast<EDA_ITEM*>(item2);

    printf("Draw %p\n", aItem );

	switch(item->Type())
	{
		HANDLE_ITEM(LIB_PART_T, LIB_PART);
	    HANDLE_ITEM(LIB_RECTANGLE_T, LIB_RECTANGLE);
    HANDLE_ITEM(LIB_POLYLINE_T, LIB_POLYLINE);
    HANDLE_ITEM(LIB_CIRCLE_T, LIB_CIRCLE);
    HANDLE_ITEM(LIB_PIN_T, LIB_PIN);
    HANDLE_ITEM(LIB_ARC_T, LIB_ARC);
    HANDLE_ITEM(LIB_FIELD_T, LIB_FIELD);
  	HANDLE_ITEM(LIB_TEXT_T, LIB_TEXT);
    HANDLE_ITEM(SCH_COMPONENT_T, SCH_COMPONENT);
    HANDLE_ITEM(SCH_JUNCTION_T, SCH_JUNCTION);
    HANDLE_ITEM(SCH_LINE_T, SCH_LINE);
    HANDLE_ITEM(SCH_TEXT_T, SCH_TEXT);
    //HANDLE_ITEM(SCH_FIELD_T, SCH_FIELD);

		default:
			return false;
	}
	return false;
}

void SCH_PAINTER::draw ( LIB_PART *aComp, int aLayer )
{
    auto comp = const_cast<LIB_PART*>(aComp);
    for ( auto& item : comp->GetDrawItems() )
		Draw ( &item, aLayer );

}

void SCH_PAINTER::draw ( SCH_COMPONENT *aComp, int aLayer )
{
#if 0
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
#endif
}

void SCH_PAINTER::draw ( LIB_RECTANGLE *aComp, int aLayer )
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
  const COLOR4D& fg = m_schSettings.GetLayerColor ( LAYER_DEVICE );
  const COLOR4D& bg = m_schSettings.GetLayerColor ( LAYER_DEVICE_BACKGROUND );

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

void SCH_PAINTER::draw ( LIB_CIRCLE *aCircle, int aLayer )
{
  defaultColors(aCircle);
  m_gal->DrawCircle( aCircle->GetPosition(), aCircle->GetRadius() );
}

void SCH_PAINTER::draw ( LIB_ARC *aArc, int aLayer )
{
  defaultColors(aArc);

  int sai = aArc->GetFirstRadiusAngle();
  int eai = aArc->GetSecondRadiusAngle();

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


void SCH_PAINTER::draw ( LIB_FIELD *aField, int aLayer )
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
    m_gal->SetStrokeColor( aField->GetDefaultColor () );
    m_gal->SetGlyphSize ( aField->GetTextSize() );

    m_gal->SetHorizontalJustify( aField->GetHorizJustify( ));
    m_gal->SetVerticalJustify( aField->GetVertJustify( ));


    const VECTOR2D pos = aField->GetPosition();
    double orient = aField->GetTextAngleRadians() + M_PI;


    m_gal->StrokeText( aField->GetText(), pos, orient );
}

#if 0
void SCH_PAINTER::draw ( SCH_FIELD *aField, int aLayer )
{

    if(!aField->IsVisible())
      return;

    int w;

   if( aField->IsBold() )
      w =  aField->GetPenSize() * 1.5; //GetPenSizeForBold( aField->GetWidth() );
    else
      w = aField->GetPenSize();

    m_gal->SetLineWidth(w);
    m_gal->SetIsFill( false );
    m_gal->SetIsStroke (true);

    COLOR4D color;

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

    m_gal->SetStrokeColor( color );
    m_gal->SetGlyphSize ( aField->GetTextSize() );

    m_gal->SetHorizontalJustify( aField->GetHorizJustify( ));
    m_gal->SetVerticalJustify( aField->GetVertJustify( ));


    const VECTOR2D pos = aField->GetPosition();
    double orient = aField->GetTextAngleRadians() + M_PI;

    m_gal->StrokeText( aField->GetFullyQualifiedText(), pos, orient );
}
#endif


void SCH_PAINTER::draw ( LIB_POLYLINE *aLine, int aLayer )
{

  defaultColors(aLine);
  std::deque<VECTOR2D> vtx;

  for ( auto p : aLine->GetPolyPoints() )
    vtx.push_back ( VECTOR2D ( p ) );

  if( aLine->GetFillMode() == FILLED_WITH_BG_BODYCOLOR || aLine->GetFillMode() == FILLED_SHAPE)
    vtx.push_back ( vtx[0] );

  m_gal->DrawPolygon ( vtx );
}

void SCH_PAINTER::draw ( LIB_TEXT *aText, int aLayer )
{
  if(!aText->IsVisible())
      return;

    int w = aText->GetPenSize();

    m_gal->SetLineWidth(w);
    m_gal->SetIsFill( false );
    m_gal->SetIsStroke (true);
    m_gal->SetStrokeColor( aText-> GetDefaultColor () );
    m_gal->SetGlyphSize ( aText->GetTextSize() );

    m_gal->SetHorizontalJustify( GR_TEXT_HJUSTIFY_CENTER );
    m_gal->SetVerticalJustify( GR_TEXT_VJUSTIFY_CENTER );

    EDA_RECT bBox = aText->GetBoundingBox();
    const VECTOR2D pos = bBox.Centre();

    double orient = aText->GetTextAngleRadians() + M_PI;

    m_gal->SetFontBold ( aText->IsBold() );
    m_gal->SetFontItalic ( aText->IsItalic() );
    m_gal->StrokeText( aText->GetText(), pos, orient );
}


static int InternalPinDecoSize( const LIB_PIN &aPin )
{

    return aPin.GetNameTextSize() != 0 ? aPin.GetNameTextSize() / 2 : aPin.GetNumberTextSize() / 2;
}

/// Utility for getting the size of the 'external' pin decorators (as a radius)
// i.e. the negation circle, the polarity 'slopes' and the nonlogic
// marker
static int ExternalPinDecoSize( const LIB_PIN &aPin )
{
    return aPin.GetNumberTextSize() / 2;
}


void SCH_PAINTER::draw ( LIB_PIN *aPin, int aLayer )
{

  if( !aPin->IsVisible() )
    return;

  const COLOR4D& color = m_schSettings.GetLayerColor( LAYER_PIN );

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
  m_gal->SetFontBold ( false );
  m_gal->SetFontItalic ( false );

  const int radius = ExternalPinDecoSize( *aPin );
  const int clock_size = InternalPinDecoSize( *aPin );

	if(shape == PINSHAPE_INVERTED)
	{

		m_gal->DrawCircle ( p0 + dir * radius, radius );
		m_gal->DrawLine ( p0 + dir * ( 2 * radius ), pos );
	} else if (shape == PINSHAPE_FALLING_EDGE_CLOCK )
  {

    pc = p0 + dir * clock_size ;

    triLine( p0 + VECTOR2D ( dir.y, -dir.x) * clock_size,
            pc,
            p0 + VECTOR2D ( -dir.y, dir.x) * clock_size
            );

    m_gal->DrawLine ( pos, pc );
  }
  else {
    m_gal->DrawLine ( p0, pos );
  }

  if(shape == PINSHAPE_CLOCK)
  {
    if (!dir.y)
    {
      triLine (p0 + VECTOR2D( 0, clock_size ),
                p0 + VECTOR2D( -dir.x * clock_size, 0),
                p0 + VECTOR2D( 0, -clock_size ));

    } else {
      triLine ( p0 + VECTOR2D ( clock_size, 0 ),
                p0 + VECTOR2D ( 0, -dir.y * clock_size ),
                p0 + VECTOR2D ( -clock_size, 0 ));
    }
  }

  if( shape == PINSHAPE_INPUT_LOW )
  {
    if(!dir.y)
        {
            triLine ( p0 + VECTOR2D(dir.x, 0) * radius * 2,
                      p0 + VECTOR2D(dir.x, -1) * radius * 2,
                      p0 );
        }
        else    /* MapX1 = 0 */
        {
            triLine ( p0 + VECTOR2D( 0, dir.y) * radius * 2,
                      p0 + VECTOR2D(-1, dir.y) * radius * 2,
                      p0 );
        }
    }

    if( shape == PINSHAPE_OUTPUT_LOW )    /* IEEE symbol "Active Low Output" */
    {
        if(!dir.y)
          m_gal->DrawLine( p0 - VECTOR2D(0, radius), p0 + VECTOR2D(dir.x, 1) * radius * 2);
        else
          m_gal->DrawLine (p0 - VECTOR2D(radius, 0), p0 + VECTOR2D(0, dir.y) * radius * 2);
    }

    if( shape == PINSHAPE_NONLOGIC ) /* NonLogic pin symbol */
    {
        m_gal->DrawLine ( p0 - VECTOR2D(dir.x + dir.y, dir.y - dir.x) * radius,
                          p0 + VECTOR2D(dir.x + dir.y, dir.y - dir.x) * radius);
        m_gal->DrawLine ( p0 - VECTOR2D(dir.x - dir.y, dir.x + dir.y) * radius,
                          p0 + VECTOR2D(dir.x - dir.y, dir.x + dir.y) * radius);
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
    LIB_PART* libEntry = (const_cast<LIB_PIN *> (aPin)) ->GetParent();
    m_gal->SetLineWidth ( labelWidth );
    wxString    stringPinNum;

    /* Get the num and name colors */
    COLOR4D nameColor = m_schSettings.GetLayerColor( LAYER_PINNAM );
    COLOR4D numColor  = m_schSettings.GetLayerColor( LAYER_PINNUM );

    /* Create the pin num string */
    stringPinNum = aPin->GetNumber();
    int textOffset = libEntry->GetPinNameOffset();

    bool showNums = libEntry->ShowPinNumbers();
    bool showNames = libEntry->ShowPinNames();

    m_gal->SetGlyphSize ( VECTOR2D ( aPin->GetNameTextSize(), aPin->GetNameTextSize()) );
    m_gal->SetTextMirrored ( true ); // don't know why yet...
    m_gal->SetStrokeColor ( nameColor );
    m_gal->SetVerticalJustify( GR_TEXT_VJUSTIFY_CENTER );

    int         nameLineWidth = aPin->GetPenSize();
    nameLineWidth = Clamp_Text_PenSize( nameLineWidth, aPin->GetNameTextSize(), false );
    int         numLineWidth = aPin->GetPenSize();
    numLineWidth = Clamp_Text_PenSize( numLineWidth, aPin->GetNumberTextSize(), false );

    #define PIN_TEXT_MARGIN 4


    int         name_offset = PIN_TEXT_MARGIN +
                              ( nameLineWidth + GetDefaultLineThickness() ) / 2;
    int         num_offset = - PIN_TEXT_MARGIN -
                             ( numLineWidth + GetDefaultLineThickness() ) / 2;

    printf("numoffs %d w %d s %d\n", num_offset, numLineWidth,aPin->GetNumberTextSize() );

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

    #define TXTMARGE 10

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
            m_gal->StrokeText (stringPinNum, VECTOR2D( (p0.x + pos.x) / 2, p0.y - num_offset ), -M_PI );
            break;
          case PIN_DOWN:
          case PIN_UP:
            m_gal->StrokeText (stringPinNum, VECTOR2D ( p0.x - num_offset, (p0.y + pos.y) / 2), M_PI / 2);
           break;
        }
      }
    }
}

void SCH_PAINTER::draw ( SCH_JUNCTION *aJct, int aLayer )
{
    const COLOR4D& color = m_schSettings.GetLayerColor( LAYER_JUNCTION );


    m_gal->SetIsStroke(true);
    m_gal->SetIsFill(true);
    m_gal->SetStrokeColor( color );
    m_gal->SetFillColor( color );
    m_gal->DrawCircle( aJct->GetPosition(),  aJct->GetSymbolSize() / 2 );
}

void SCH_PAINTER::draw ( SCH_LINE *aLine, int aLayer )
{
    COLOR4D color = GetLayerColor( LAYER_WIRE );
    int width = aLine->GetPenSize();

    /*if( Color != COLOR4D::UNSPECIFIED )
        color = Color;
    else if( m_color != COLOR4D::UNSPECIFIED )
        color = m_color;
    else
        color = GetLayerColor( GetState( BRIGHTENED ) ? LAYER_BRIGHTENED : m_Layer );

    GRSetDrawMode( DC, DrawMode );

    wxPoint start = m_start;
    wxPoint end = m_end;

    if( ( m_Flags & STARTPOINT ) == 0 )
        start += offset;

    if( ( m_Flags & ENDPOINT ) == 0 )
        end += offset;*/

    m_gal->SetIsStroke(true);
    m_gal->SetStrokeColor(color);
    m_gal->SetLineWidth( width );
    m_gal->DrawLine( aLine->GetStartPoint(), aLine->GetEndPoint() );

    //GRLine( panel->GetClipBox(), DC, start.x, start.y, end.x, end.y, width, color,
    //        getwxPenStyle( (PlotDashType) GetLineStyle() ) );

    /*if( m_startIsDangling )
        DrawDanglingSymbol( panel, DC, start, color );

    if( m_endIsDangling )
        DrawDanglingSymbol( panel, DC, end, color );*/
}

void SCH_PAINTER::draw ( SCH_TEXT *aText, int aLayer )
{
    COLOR4D     color;
    int         linewidth = aText->GetThickness() == 0 ? GetDefaultLineThickness() : aText->GetThickness();

    linewidth = Clamp_Text_PenSize( linewidth, aText->GetTextSize(), aText->IsBold() );

    wxPoint text_offset = aText->GetTextPos() + aText->GetSchematicTextOffset();

    int savedWidth = aText->GetThickness();

    //if( m_isDangling && panel)
        //DrawDanglingSymbol( panel, DC, GetTextPos() + aOffset, color );

        wxString shownText( aText->GetShownText() );
        if( shownText.Length() == 0 )
            return;


        m_gal->SetStrokeColor( color );
        m_gal->SetIsFill( false );
        m_gal->SetIsStroke( true );
        m_gal->SetTextAttributes( aText );
        m_gal->SetLineWidth( linewidth );
        m_gal->StrokeText( shownText, text_offset, aText->GetTextAngleRadians() );


}


}; // namespace KIGFX
