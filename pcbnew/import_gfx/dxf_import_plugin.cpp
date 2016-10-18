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

#include "dxf_import_plugin.h"
#include "graphics_importer.h"

#include <libdxfrw.h>
#include <convert_to_biu.h>

#include <class_drawsegment.h>

#include <wx/regex.h>

// minimum bulge value before resorting to a line segment;
// the value 0.0218 is equivalent to about 5 degrees arc,
static const double MIN_BULGE = 0.0218;

DXF_IMPORT_PLUGIN::DXF_IMPORT_PLUGIN()
    : m_DXF2mm( 1.0 )
{
}


bool DXF_IMPORT_PLUGIN::Load( const wxString& aFileName )
{
    wxCHECK( m_importer, false );

    dxfRW dxf( aFileName.ToUTF8() );
    return dxf.read( this, true );
}


// coordinate conversions from dxf to internal units
// @todo remove?
int DXF_IMPORT_PLUGIN::mapX( double aDxfCoordX )
{
    //return Millimeter2iu( m_xOffset + ( aDxfCoordX * m_DXF2mm ) );
    return Millimeter2iu( aDxfCoordX * m_DXF2mm );
}


int DXF_IMPORT_PLUGIN::mapY( double aDxfCoordY )
{
    //return Millimeter2iu( m_yOffset - ( aDxfCoordY * m_DXF2mm ) );
    return Millimeter2iu( -aDxfCoordY * m_DXF2mm );
}


int DXF_IMPORT_PLUGIN::mapDim( double aDxfValue )
{
    return Millimeter2iu( aDxfValue * m_DXF2mm );
}


void DXF_IMPORT_PLUGIN::insertLine( const wxRealPoint& aSegStart, const wxRealPoint& aSegEnd )
{
    wxPoint origin( Millimeter2iu( aSegStart.x ), Millimeter2iu( aSegStart.y ) );
    wxPoint end( Millimeter2iu( aSegEnd.x ), Millimeter2iu( aSegEnd.y ) );
    m_importer->AddLine( origin, end );
}


void DXF_IMPORT_PLUGIN::insertArc( const wxRealPoint& aSegStart, const wxRealPoint& aSegEnd,
        double aBulge )
{
    // ensure aBulge represents an angle from +/- ( 0 .. approx 359.8 deg )
    if( aBulge < -2000.0 )
        aBulge = -2000.0;
    else if( aBulge > 2000.0 )
        aBulge = 2000.0;

    double ang = 4.0 * atan( aBulge );

    // reflect the Y values to put everything in a RHCS
    wxRealPoint sp( aSegStart.x, -aSegStart.y );
    wxRealPoint ep( aSegEnd.x, -aSegEnd.y );
    // angle from end->start
    double offAng = atan2( ep.y - sp.y, ep.x - sp.x );
    // length of subtended segment = 1/2 distance between the 2 points
    double d = 0.5 * sqrt( (sp.x - ep.x) * (sp.x - ep.x) + (sp.y - ep.y) * (sp.y - ep.y) );
    // midpoint of the subtended segment
    double xm   = ( sp.x + ep.x ) * 0.5;
    double ym   = ( sp.y + ep.y ) * 0.5;
    double radius = d / sin( ang * 0.5 );

    if( radius < 0.0 )
        radius = -radius;

    // calculate the height of the triangle with base d and hypotenuse r
    double dh2 = radius * radius - d * d;

    // this should only ever happen due to rounding errors when r == d
    if( dh2 < 0.0 )
        dh2 = 0.0;

    double h = sqrt( dh2 );

    if( ang < 0.0 )
        offAng -= M_PI_2;
    else
        offAng += M_PI_2;

    // for angles greater than 180 deg we need to flip the
    // direction in which the arc center is found relative
    // to the midpoint of the subtended segment.
    if( ang < -M_PI )
        offAng += M_PI;
    else if( ang > M_PI )
        offAng -= M_PI;

    // center point
    double cx = h * cos( offAng ) + xm;
    double cy = h * sin( offAng ) + ym;

    wxPoint center( Millimeter2iu( cx ), Millimeter2iu( -cy ) );
    wxPoint start;
    double angle;

    if( ang < 0.0 )
    {
        start = wxPoint( Millimeter2iu( ep.x ), Millimeter2iu( -ep.y ) );
        angle = RAD2DECIDEG( ang );
    }
    else
    {
        start = wxPoint( Millimeter2iu( sp.x ), Millimeter2iu( -sp.y ) );
        angle = RAD2DECIDEG( -ang );
    }

    m_importer->AddArc( center, start, angle );
}


void DXF_IMPORT_PLUGIN::addHeader( const DRW_Header* data )
{
    std::map<std::string, DRW_Variant*>::const_iterator it;
    m_DXF2mm = 1.0; // assume no scale factor

    for( it = data->vars.begin(); it != data->vars.end(); ++it )
    {
        std::string key = ( (*it).first ).c_str();

        if( key == "$DWGCODEPAGE" )
        {
            //DRW_Variant* var = (*it).second;
            //m_codePage = ( *var->content.s );
        }
        else if( key == "$INSUNITS" )
        {
            DRW_Variant* var = (*it).second;

            switch( var->content.i )
            {
            case 1:     // inches
                m_DXF2mm = 25.4;
                break;

            case 2:     // feet
                m_DXF2mm = 304.8;
                break;

            case 5:     // centimeters
                m_DXF2mm = 10.0;
                break;

            case 6:     // meters
                m_DXF2mm = 1000.0;
                break;

            case 8:     // microinches
                m_DXF2mm = 2.54e-5;
                break;

            case 9:     // mils
                m_DXF2mm = 0.0254;
                break;

            case 10:    // yards
                m_DXF2mm = 914.4;
                break;

            case 11:    // Angstroms
                m_DXF2mm = 1.0e-7;
                break;

            case 12:    // nanometers
                m_DXF2mm = 1.0e-6;
                break;

            case 13:    // micrometers
                m_DXF2mm = 1.0e-3;
                break;

            case 14:    // decimeters
                m_DXF2mm = 100.0;
                break;

            default:
                // use the default of 1.0 for:
                // 0: Unspecified Units
                // 4: mm
                // 3: miles
                // 7: kilometers
                // 15: decameters
                // 16: hectometers
                // 17: gigameters
                // 18: AU
                // 19: lightyears
                // 20: parsecs
                break;
            }
        }
    }
}


void DXF_IMPORT_PLUGIN::addLine( const DRW_Line& aData )
{
    wxPoint start( mapX( aData.basePoint.x ), mapY( aData.basePoint.y ) );
    wxPoint end( mapX( aData.secPoint.x ), mapY( aData.secPoint.y ) );
    m_importer->AddLine( start, end );
}


void DXF_IMPORT_PLUGIN::addCircle( const DRW_Circle& aData )
{
    wxPoint center( mapX( aData.basePoint.x ), mapY( aData.basePoint.y ) );
    m_importer->AddCircle( center, mapDim( aData.radious ) );
}


void DXF_IMPORT_PLUGIN::addArc( const DRW_Arc& data )
{
    // Init arc centre:
    wxPoint center( mapX( data.basePoint.x ), mapY( data.basePoint.y ) );

    // Init arc start point
    double arcStartx = data.radious;
    double arcStarty = 0;
    double startangle = data.staangle;
    double endangle = data.endangle;

    RotatePoint( &arcStartx, &arcStarty, -RAD2DECIDEG( startangle ) );
    wxPoint arcStart( mapX( arcStartx + data.basePoint.x ), mapY( arcStarty + data.basePoint.y ) );

    // calculate arc angle (arcs are CCW, and should be < 0 in Pcbnew)
    double angle = -RAD2DECIDEG( endangle - startangle );

    if( angle > 0.0 )
        angle -= 3600.0;

    m_importer->AddArc( center, arcStart, angle );
}


void DXF_IMPORT_PLUGIN::addLWPolyline(const DRW_LWPolyline& aData )
{
    // Currently, Pcbnew does not know polylines, for boards.
    // So we have to convert a polyline to a set of segments.
    // The import is a simplified import: the width of segment is
    // (obviously constant and is the width of the DRW_LWPolyline.
    // the variable width of each vertex (when exists) is not used.
    wxRealPoint seg_start;
    wxRealPoint poly_start;
    double bulge = 0.0;
    //int lineWidth = mapDim( aData.thickness == 0 ? m_defaultThickness : aData.thickness );

    for( unsigned ii = 0; ii < aData.vertlist.size(); ii++ )
    {
        DRW_Vertex2D* vertex = aData.vertlist[ii];

        if( ii == 0 )
        {
            seg_start.x = /*m_xOffset +*/ vertex->x * m_DXF2mm;
            seg_start.y = /*m_yOffset -*/ vertex->y * m_DXF2mm;
            bulge = vertex->bulge;
            poly_start = seg_start;
            continue;
        }

        wxRealPoint seg_end( /*m_xOffset +*/ vertex->x * m_DXF2mm,
                             /*m_yOffset -*/ vertex->y * m_DXF2mm );

        if( std::abs( bulge ) < MIN_BULGE )
            insertLine( seg_start, seg_end );
        else
            insertArc( seg_start, seg_end, bulge );

        bulge = vertex->bulge;
        seg_start = seg_end;
    }

    // LWPolyline flags bit 0 indicates closed (1) or open (0) polyline
    if( aData.flags & 1 )
    {
        if( std::abs( bulge ) < MIN_BULGE )
            insertLine( seg_start, poly_start );
        else
            insertArc( seg_start, poly_start, bulge );
    }
}


void DXF_IMPORT_PLUGIN::addPolyline(const DRW_Polyline& aData )
{
    // Currently, Pcbnew does not know polylines, for boards.
    // So we have to convert a polyline to a set of segments.
    // Obviously, the z coordinate is ignored

    wxPoint polyline_startpoint;
    wxPoint segment_startpoint;

    for( unsigned ii = 0; ii < aData.vertlist.size(); ii++ )
    {
        DRW_Vertex* vertex = aData.vertlist[ii];

        if( ii == 0 )
        {
            segment_startpoint.x = mapX( vertex->basePoint.x );
            segment_startpoint.y = mapY( vertex->basePoint.y );
            polyline_startpoint  = segment_startpoint;
            continue;
        }

        wxPoint segment_endpoint( mapX( vertex->basePoint.x ), mapY( vertex->basePoint.y ) );
        m_importer->AddLine( segment_startpoint, segment_endpoint );
        segment_startpoint = segment_endpoint;
    }

    // Polyline flags bit 0 indicates closed (1) or open (0) polyline
    if( aData.flags & 1 )
    {
        m_importer->AddLine( segment_startpoint, polyline_startpoint );
    }
}



void DXF_IMPORT_PLUGIN::addText( const DRW_Text& aData )
{
    wxPoint refPoint( mapX( aData.basePoint.x ), mapY( aData.basePoint.y ) );
    wxPoint secPoint( mapX( aData.secPoint.x ), mapY( aData.secPoint.y ) );

    if( aData.alignV != 0 || aData.alignH != 0 || aData.alignH == DRW_Text::HMiddle )
    {
        if( aData.alignH != DRW_Text::HAligned && aData.alignH != DRW_Text::HFit )
        {
            wxPoint tmp = secPoint;
            secPoint = refPoint;
            refPoint = tmp;
        }
    }

    EDA_TEXT_HJUSTIFY_T hJustify = GR_TEXT_HJUSTIFY_LEFT;
    EDA_TEXT_VJUSTIFY_T vJustify = GR_TEXT_VJUSTIFY_BOTTOM;

    switch( aData.alignV )
    {
    case DRW_Text::VBaseLine:
    case DRW_Text::VBottom:
        vJustify = GR_TEXT_VJUSTIFY_BOTTOM;
        break;

    case DRW_Text::VMiddle:
        vJustify = GR_TEXT_VJUSTIFY_CENTER;
        break;

    case DRW_Text::VTop:
        vJustify = GR_TEXT_VJUSTIFY_TOP;
        break;
    }

    switch( aData.alignH )
    {
    case DRW_Text::HLeft:
    case DRW_Text::HAligned:    // no equivalent options in text pcb.
    case DRW_Text::HFit:        // no equivalent options in text pcb.
        hJustify = GR_TEXT_HJUSTIFY_LEFT;
        break;

    case DRW_Text::HCenter:
    case DRW_Text::HMiddle:     // no equivalent options in text pcb.
        hJustify = GR_TEXT_HJUSTIFY_CENTER;
        break;

    case DRW_Text::HRight:
        hJustify = GR_TEXT_HJUSTIFY_RIGHT;
        break;
    }

#if 0
    wxString sty = wxString::FromUTF8( aData.style.c_str() );
    sty = sty.ToLower();

    if( aData.textgen == 2 )
    {
        // Text dir = left to right;
    } else if( aData.textgen == 4 )
    {
        // Text dir = top to bottom;
    } else
    {
    }
#endif

    wxString text = toNativeString( wxString::FromUTF8( aData.text.c_str() ) );

    m_importer->AddText( refPoint, text, mapDim( aData.height ), mapDim( aData.height * 0.8 ),
            aData.angle * 10, hJustify, vJustify );
}


void DXF_IMPORT_PLUGIN::addMText( const DRW_MText& aData )
{
    wxString    text = toNativeString( wxString::FromUTF8( aData.text.c_str() ) );
    wxString    attrib, tmp;

    /* Some texts start by '\' and have formating chars (font name, font option...)
     *  ending with ';'
     *  Here are some mtext formatting codes:
     *  Format code        Purpose
     * \0...\o            Turns overline on and off
     *  \L...\l            Turns underline on and off
     * \~                 Inserts a nonbreaking space
     \\                 Inserts a backslash
     \\\{...\}            Inserts an opening and closing brace
     \\ \File name;        Changes to the specified font file
     \\ \Hvalue;           Changes to the text height specified in drawing units
     \\ \Hvaluex;          Changes the text height to a multiple of the current text height
     \\ \S...^...;         Stacks the subsequent text at the \, #, or ^ symbol
     \\ \Tvalue;           Adjusts the space between characters, from.75 to 4 times
     \\ \Qangle;           Changes obliquing angle
     \\ \Wvalue;           Changes width factor to produce wide text
     \\ \A                 Sets the alignment value; valid values: 0, 1, 2 (bottom, center, top)    while( text.StartsWith( wxT("\\") ) )
     */
    while( text.StartsWith( wxT( "\\" ) ) )
    {
        attrib << text.BeforeFirst( ';' );
        tmp     = text.AfterFirst( ';' );
        text    = tmp;
    }

    wxPoint textpos( mapX( aData.basePoint.x ), mapY( aData.basePoint.y ) );

    // Initialize text justifications:
    EDA_TEXT_HJUSTIFY_T hJustify = GR_TEXT_HJUSTIFY_LEFT;
    EDA_TEXT_VJUSTIFY_T vJustify = GR_TEXT_VJUSTIFY_BOTTOM;

    if( aData.textgen <= 3 )
    {
        vJustify = GR_TEXT_VJUSTIFY_TOP;
    }
    else if( aData.textgen <= 6 )
    {
        vJustify = GR_TEXT_VJUSTIFY_CENTER;
    }
    else
    {
        vJustify = GR_TEXT_VJUSTIFY_BOTTOM;
    }

    if( aData.textgen % 3 == 1 )
    {
        hJustify = GR_TEXT_HJUSTIFY_LEFT;
    }
    else if( aData.textgen % 3 == 2 )
    {
        hJustify = GR_TEXT_HJUSTIFY_CENTER;
    }
    else
    {
        hJustify = GR_TEXT_HJUSTIFY_RIGHT;
    }

#if 0   // These setting have no mening in Pcbnew
    if( data.alignH == 1 )
    {
        // Text is left to right;
    }
    else if( data.alignH == 3 )
    {
        // Text is top to bottom;
    }
    else
    {
        // use ByStyle;
    }

    if( aData.alignV == 1 )
    {
        // use AtLeast;
    }
    else
    {
        // useExact;
    }
#endif

    m_importer->AddText( textpos, text,
            mapDim( aData.height ), mapDim( aData.height * 0.8 ), aData.angle * 10,
            hJustify, vJustify );
}


wxString DXF_IMPORT_PLUGIN::toDxfString( const wxString& aStr )
{
    wxString    res;
    int         j = 0;

    for( unsigned i = 0; i<aStr.length(); ++i )
    {
        int c = aStr[i];

        if( c > 175 || c < 11 )
        {
            res.append( aStr.Mid( j, i - j ) );
            j = i;

            switch( c )
            {
            case 0x0A:
                res += wxT( "\\P" );
                break;

                // diameter:
#ifdef __WINDOWS_
            // windows, as always, is special.
            case 0x00D8:
#else
            case 0x2205:
#endif
                res += wxT( "%%C" );
                break;

            // degree:
            case 0x00B0:
                res += wxT( "%%D" );
                break;

            // plus/minus
            case 0x00B1:
                res += wxT( "%%P" );
                break;

            default:
                j--;
                break;
            }

            j++;
        }
    }

    res.append( aStr.Mid( j ) );
    return res;
}


wxString DXF_IMPORT_PLUGIN::toNativeString( const wxString& aData )
{
    wxString    res;

    // Ignore font tags:
    int         j = 0;

    for( unsigned i = 0; i < aData.length(); ++i )
    {
        if( aData[ i ] == 0x7B )                                     // is '{' ?
        {
            if( aData[ i + 1 ] == 0x5c && aData[ i + 2 ] == 0x66 )    // is "\f" ?
            {
                // found font tag, append parsed part
                res.append( aData.Mid( j, i - j ) );

                // skip to ';'
                for( unsigned k = i + 3; k < aData.length(); ++k )
                {
                    if( aData[ k ] == 0x3B )
                    {
                        i = j = ++k;
                        break;
                    }
                }

                // add to '}'
                for( unsigned k = i; k < aData.length(); ++k )
                {
                    if( aData[ k ] == 0x7D )
                    {
                        res.append( aData.Mid( i, k - i ) );
                        i = j = ++k;
                        break;
                    }
                }
            }
        }
    }

    res.append( aData.Mid( j ) );

#if 1
    wxRegEx regexp;
    // Line feed:
    regexp.Compile( wxT( "\\\\P" ) );
    regexp.Replace( &res, wxT( "\n" ) );

    // Space:
    regexp.Compile( wxT( "\\\\~" ) );
    regexp.Replace( &res, wxT( " " ) );

    // diameter:
    regexp.Compile( wxT( "%%[cC]" ) );
#ifdef __WINDOWS__
    // windows, as always, is special.
    regexp.Replace( &res, wxChar( 0xD8 ) );
#else
    // Empty_set, diameter is 0x2300
    regexp.Replace( &res, wxChar( 0x2205 ) );
#endif

    // degree:
    regexp.Compile( wxT( "%%[dD]" ) );
    regexp.Replace( &res, wxChar( 0x00B0 ) );
    // plus/minus
    regexp.Compile( wxT( "%%[pP]" ) );
    regexp.Replace( &res, wxChar( 0x00B1 ) );
#endif

    return res;
}
