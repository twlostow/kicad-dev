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
#include <wx/msw/enhmeta.h>
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
#ifdef __WXMSW__
    else if( wxEnhMetaFileDC* enhMFDC = dynamic_cast<wxEnhMetaFileDC*>( aDC ) )
        m_gcdc = new wxGCDC( *enhMFDC );

#endif /* __WXMSW__ */
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

    cairo_surface_destroy( m_surface );
    cairo_destroy( m_ctx );
    delete m_gcdc;
}

double CAIRO_PRINT_CTX::GetNativeDPI() const
{
#ifdef __WXGTK__
    return 72.0;
#endif /* __WXGTK__ */
}

bool CAIRO_PRINT_CTX::HasNativeLandscapeRotation() const
{
#ifdef __WXGTK__
    return false;
#endif /* __WXGTK__ */
}


CAIRO_PRINT_GAL::CAIRO_PRINT_GAL( GAL_DISPLAY_OPTIONS& aDisplayOptions,
        cairo_t* aContext, cairo_surface_t* aSurface )
    : CAIRO_GAL_BASE( aDisplayOptions )
{
    cairo_reference( aContext );
    cairo_surface_reference( aSurface );
    context = currentContext = aContext;
    surface = aSurface;
    m_clearColor = COLOR4D( 1.0, 1.0, 1.0, 1.0 );
    resetContext();
}

void CAIRO_PRINT_GAL::ComputeWorldScreenMatrix()
{
    auto s = m_nativePageSize;


//    screenDPI = 72.0;

    //screenDPI * worldUnitLength * zoomFactor;
//  printf("screen dpi %.10f worldLen %.10f zoom %.10f\n", screenDPI, worldUnitLength, zoomFactor );
//  printf("CG native size: %.10f %.10f\n\n", m_nativePageSize.x, m_nativePageSize.y );

    


    //nativePageSize.x -> inches

    worldUnitLength = 1e-9 /* 1 nm */ / 0.0254 /* 1 inch in meters */;

    // worldUnitLength = inch per integer
    worldScale = screenDPI * worldUnitLength * zoomFactor;

    printf( "Native page size %.1f %.1f\n",m_nativePageSize.x  , m_nativePageSize.x );



    if ( m_hasNativeLandscapeRotation )
    {

        MATRIX3x3D translation;
        translation.SetIdentity();
    //    translation.SetTranslation( 0.5 * VECTOR2D( screenSize ) );

        MATRIX3x3D rotate;
        rotate.SetIdentity();
        rotate.SetRotation( rotation );

        MATRIX3x3D scale;
        scale.SetIdentity();
        scale.SetScale( VECTOR2D( worldScale, worldScale ) );

        MATRIX3x3D flip;
        flip.SetIdentity();
        flip.SetScale( VECTOR2D( globalFlipX ? -1.0 : 1.0, globalFlipY ? -1.0 : 1.0 ) );

        MATRIX3x3D lookat;
        lookat.SetIdentity();
        lookat.SetTranslation( -lookAtPoint );

        //worldScreenMatrix = translation * rotate * flip * scale * lookat;
        worldScreenMatrix = translation * flip * scale * rotate * lookat;
    } else {
        auto pageSizeIU = VECTOR2D(m_nativePageSize.y, m_nativePageSize.x) /* inches */ * 0.0254 * 1e9 /* 1 inch in nm */;
        auto pageSizeIUTransposed = VECTOR2D(m_nativePageSize.x, m_nativePageSize.y) /* inches */ * 0.0254 * 1e9 /* 1 inch in nm */;

        MATRIX3x3D scale;
        scale.SetIdentity();
        scale.SetScale( VECTOR2D( worldScale, worldScale ) );

        MATRIX3x3D translation;
        translation.SetIdentity();
        //printf("pageSizeIU %.1f %.1f\n", pageSizeIU.x, pageSizeIU.y );
        translation.SetTranslation( -0.5 * pageSizeIUTransposed );

        MATRIX3x3D rotate;
        rotate.SetIdentity();
        rotate.SetRotation( 90.0 * M_PI/180.0 );

        MATRIX3x3D flip;
        flip.SetIdentity();
        flip.SetScale( VECTOR2D( globalFlipX ? -1.0 : 1.0, globalFlipY ? -1.0 : 1.0 ) );

        MATRIX3x3D translation2;
        translation2.SetIdentity();
        translation2.SetTranslation( 0.5 * pageSizeIU );

        MATRIX3x3D lookat;
        lookat.SetIdentity();
        lookat.SetTranslation( -lookAtPoint );

        worldScreenMatrix =  scale * translation2 * rotate * flip * translation * lookat;
    }
    
    screenWorldMatrix = worldScreenMatrix.Inverse();

    std::cout << "CG world scale " << worldScale << std::endl;
}

void CAIRO_PRINT_GAL::SetNativePageSize( VECTOR2D aSize, bool aHasNativeLandscapeRotation )
{
    m_nativePageSize = aSize;
    m_hasNativeLandscapeRotation = aHasNativeLandscapeRotation;

    ComputeWorldScreenMatrix();
}