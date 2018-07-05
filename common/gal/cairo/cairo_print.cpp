/*
 * Copyright (C) 2018 CERN
 * Author: Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gal/cairo/cairo_print.h>

#include <stdexcept>
#include <wx/dcclient.h>
#include <wx/dcgraph.h>
#include <wx/dcmemory.h>
#include <wx/dcprint.h>

#ifdef __WXMSW__
#include <windows.h>
#include <gdiplus.h>
#include <cairo-win32.h>
#endif /* __WXMSW__ */

#ifdef __WXMAC__
#include <ApplicationServices/ApplicationServices.h>
#include <cairo-quartz.h>
#endif /* __WXMAC__ */

using namespace KIGFX;

CAIRO_PRINT_CTX::CAIRO_PRINT_CTX( wxDC* aDC )
    : m_gcdc( nullptr ),m_ctx( nullptr ), m_surface( nullptr )
{
    if( wxPrinterDC* printerDC = dynamic_cast<wxPrinterDC*>( aDC ) )
        m_gcdc = new wxGCDC( *printerDC );
    else if( wxMemoryDC* memoryDC = dynamic_cast<wxMemoryDC*>( aDC ) )
        m_gcdc = new wxGCDC( *memoryDC );
    else if( wxWindowDC* windowDC = dynamic_cast<wxWindowDC*>( aDC ) )
        m_gcdc = new wxGCDC( *windowDC );
    else
        throw std::runtime_error( "Unhandled wxDC type" );

    wxGraphicsContext* gctx = m_gcdc->GetGraphicsContext();

    if( !gctx )
        throw std::runtime_error( "Could not get the Graphics Context" );

#ifdef __WXGTK__
    m_ctx = static_cast<cairo_t*>( gctx->GetNativeContext() );
    m_surface = cairo_get_target( m_ctx );
#endif /* __WXGTK__ */

#ifdef __WXMSW__
    Gdiplus::Graphics* g = static_cast<Gdiplus::Graphics*>( gctx->GetNativeContext() );
    m_hdc = g->GetHDC();
    m_surface = cairo_win32_printing_surface_create( static_cast<HDC>( m_hdc ) );
    m_ctx = cairo_create( m_surface );
#endif /* __WXMSW__ */

#ifdef __WXMAC__
    wxSize size = m_gcdc->GetSize();
    CGContextRef cg = (CGContextRef) gctx->GetNativeContext();
    m_surface = cairo_quartz_surface_create_for_cg_context( cg, size.x, size.y );
    m_ctx = cairo_create( m_surface );
#endif /* __WXMAC__ */

    if( cairo_status( m_ctx ) != CAIRO_STATUS_SUCCESS )
        throw std::runtime_error( "Could not create Cairo context" );

    if( cairo_surface_status( m_surface ) != CAIRO_STATUS_SUCCESS )
        throw std::runtime_error( "Could not create Cairo surface" );

    cairo_reference( m_ctx );
    cairo_surface_reference( m_surface );
}


CAIRO_PRINT_CTX::~CAIRO_PRINT_CTX()
{
#ifdef __WXMSW__
    cairo_surface_show_page( m_surface );
    wxGraphicsContext* gctx = m_gcdc->GetGraphicsContext();
    Gdiplus::Graphics* g = static_cast<Gdiplus::Graphics*>( gctx->GetNativeContext() );
    g->ReleaseHDC( static_cast<HDC>( m_hdc ) );
#endif /* __WXMSW__ */

    //cairo_surface_destroy( m_surface );
    //cairo_destroy( m_ctx );
    //delete m_gcdc;
}


CAIRO_PRINT_GAL::CAIRO_PRINT_GAL( GAL_DISPLAY_OPTIONS& aDisplayOptions,
        cairo_t* aContext, cairo_surface_t* aSurface )
    : CAIRO_GAL_BASE( aDisplayOptions )
{
    context = currentContext = aContext;
    surface = aSurface;
    m_clearColor = COLOR4D( 1.0, 1.0, 1.0, 1.0 );
    resetContext();
}
