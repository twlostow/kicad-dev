/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 Jean-Pierre Charras, jean-pierre.charras@ujf-grenoble.fr
 * Copyright (C) 1992-2018 KiCad Developers, see AUTHORS.txt for contributors.
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

/**
 * @file printout_controler.cpp
 * @brief Board print handler implementation file.
 */


// Set this to 1 if you want to test PostScript printing under MSW.
#define wxTEST_POSTSCRIPT_IN_MSW 1

#include <fctsys.h>
#include <pgm_base.h>
#include <gr_basic.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <base_units.h>
#include <pcb_base_frame.h>
#include <class_board.h>
#include <class_zone.h>
#include <class_module.h>
#include <pcbnew.h>
#include <trace_helpers.h>

#include <gal/cairo/cairo_print.h>
#include <pcb_view.h>
#include <pcb_painter.h>

#include <printout_controler.h>


PRINT_PARAMETERS::PRINT_PARAMETERS()
{
    m_PenDefaultSize        = Millimeter2iu( 0.2 ); // A reasonable default value to draw items
                                      // which do not have a specified line width
    m_PrintScale            = 1.0;
    m_XScaleAdjust          = 1.0;
    m_YScaleAdjust          = 1.0;
    m_Print_Sheet_Ref       = false;
    m_PrintMaskLayer.set();
    m_PrintMirror           = false;
    m_Print_Black_and_White = true;
    m_OptionPrintPage       = 1;
    m_PageCount             = 1;
    m_ForceCentered         = false;
    m_Flags                 = 0;
    m_DrillShapeOpt         = PRINT_PARAMETERS::SMALL_DRILL_SHAPE;
    m_PageSetupData         = NULL;
}


BOARD_PRINTOUT_CONTROLLER::BOARD_PRINTOUT_CONTROLLER( const PRINT_PARAMETERS& aParams,
                                                      EDA_DRAW_FRAME*         aParent,
                                                      const wxString&         aTitle ) :
    wxPrintout( aTitle )
{
    m_PrintParams = aParams;   // Make a local copy of the print parameters.
    m_Parent = aParent;
}


bool BOARD_PRINTOUT_CONTROLLER::OnPrintPage( int aPage )
{
    LSET lset = m_PrintParams.m_PrintMaskLayer;
    int pageCount = lset.count();
    wxString layer;
    PCB_LAYER_ID extractLayer;

    // compute layer mask from page number if we want one page per layer
    if( m_PrintParams.m_OptionPrintPage == 0 )  // One page per layer
    {
        // This sequence is TBD, call a different
        // sequencer if needed, such as Seq().  Could not find documentation on
        // page order.
        LSEQ seq = lset.UIOrder();

        // aPage starts at 1, not 0
        if( unsigned( aPage-1 ) < seq.size() )
            m_PrintParams.m_PrintMaskLayer = LSET( seq[aPage-1] );
    }

    if( !m_PrintParams.m_PrintMaskLayer.any() )
        return false;

    extractLayer = m_PrintParams.m_PrintMaskLayer.ExtractLayer();
    if( extractLayer == UNDEFINED_LAYER )
        layer = _( "Multiple Layers" );
    else
        layer = LSET::Name( extractLayer );

    // In Pcbnew we can want the layer EDGE always printed
    if( m_PrintParams.m_Flags == 1 )
        m_PrintParams.m_PrintMaskLayer.set( Edge_Cuts );

    DrawPage( layer, aPage, pageCount );

    m_PrintParams.m_PrintMaskLayer = lset;

    return true;
}


// TODO remove
static void print_cairo_mat(cairo_matrix_t& mat)
{
    std::cout << "matrix: " << std::endl;
    std::cout << "\t" << mat.xx << "\t" << mat.yx << std::endl;
    std::cout << "\t" << mat.xy << "\t" << mat.yy << std::endl;
    std::cout << "\t" << mat.x0 << "\t" << mat.y0 << std::endl;
}


void BOARD_PRINTOUT_CONTROLLER::GetPageInfo( int* minPage, int* maxPage,
                                             int* selPageFrom, int* selPageTo )
{
    *minPage     = 1;
    *selPageFrom = 1;

    int icnt = 1;

    if( m_PrintParams.m_OptionPrintPage == 0 )
        icnt = m_PrintParams.m_PageCount;

    *maxPage   = icnt;
    *selPageTo = icnt;
}


void BOARD_PRINTOUT_CONTROLLER::DrawPage( const wxString& aLayerName, int aPageNum, int aPageCount )
{
    BOARD* brd = ((PCB_BASE_FRAME*) m_Parent)->GetBoard();
    wxDC* dc = GetDC();
    KIGFX::CAIRO_PRINT_CTX printCtx( dc );



    KIGFX::GAL_DISPLAY_OPTIONS options;
    KIGFX::CAIRO_PRINT_GAL gal( options, printCtx.GetContext(), printCtx.GetSurface() );
    KIGFX::PCB_PAINTER painter( &gal );

#if 0
    std::unique_ptr<KIGFX::VIEW> view( static_cast<PCB_BASE_FRAME*>( m_Parent )->GetGalCanvas()->GetView()->DataReference() );


    wxASSERT( dc->GetPPI().x == dc->GetPPI().y );
    gal.SetScreenDPI( dc->GetPPI().x );
    //gal.SetScreenDPI( 72.0 );

    view->SetScaleLimits( 10e9, 0.0001 );
    view->SetGAL( &gal );
    view->SetPainter( &painter );

    for( int i = 0; i < KIGFX::VIEW::VIEW_MAX_LAYERS; i++ )
        view->SetLayerTarget( i, KIGFX::TARGET_NONCACHED );

    gal.ResizeScreen( dc->GetSize().x, dc->GetSize().y );
    wxSize pageSizeIU = m_Parent->GetPageSizeIU();
    BOX2I bBox;

    if( m_PrintParams.m_Print_Sheet_Ref )
    {
        bBox = BOX2I( VECTOR2I( 0, 0 ), VECTOR2I( pageSizeIU ) );
        view->SetLayerVisible( LAYER_WORKSHEET, true );
    }
    else
    {
        // TODO limit the bBox to pageSizeIU if the board is larger than the sheet?
        EDA_RECT boardBbox = brd->ComputeBoundingBox();
        bBox = BOX2I( boardBbox.GetOrigin(), boardBbox.GetSize() );
        view->SetLayerVisible( LAYER_WORKSHEET, false );
    }

    double rotation = 0.0;
    VECTOR2D translation;
    double userscale = m_PrintParams.m_PrintScale;

#endif

    std::cout << "resolution: " << dc->GetResolution() << std::endl;
    std::cout << "ppi: " << dc->GetPPI().x << std::endl;

    if( wxPrinterDC* printerdc = dynamic_cast<wxPrinterDC*>( dc ) )
    {
        std::cout << "paper rect: "
            << printerdc->GetPaperRect().x << " " << printerdc->GetPaperRect().y << " "
            << printerdc->GetPaperRect().width << " " << printerdc->GetPaperRect().height << " " << std::endl;
    }

#if 0
    //if( userscale == 0.0 )  // fit in page
    {
        if( IsPreview() )
        {
            MapScreenSizeToPage();
        }
        else
        {
#ifdef __WXGTK__
            switch( m_PrintParams.m_PageSetupData->GetPrintData().GetOrientation() )
            {
                case wxLANDSCAPE:
                    rotation = -M_PI / 2;
                    translation.x += -bBox.GetWidth();

                    if( m_PrintParams.m_PrintMirror )
                    {
                    }
                    break;

                case wxPORTRAIT:
                    //translation = VECTOR2D( Millimeter2iu( m_PrintParams.m_XScaleAdjust ),
                                            //Millimeter2iu( m_PrintParams.m_YScaleAdjust ) );
                    translation = VECTOR2D( 0, -3.0 * pageSizeIU.GetHeight() / 2.0 );
                    break;

                    // TODO

                default:
                    break;
            }

            //userscale *= 72.0 / dc->GetResolution();
#endif /* __WXGTK__ */
        }
    }

    // TODO "accurate scale" that allows the user to specify custom scale
    // I think it should be renamed to "custom scale", and "approx. scale 1" should be replaced with "Scale 1"
    if( m_PrintParams.m_PrintScale == 1.0 )
    {
        // TODO do not separate X and Y scale adjustments
        userscale *= m_PrintParams.m_XScaleAdjust;
    }

    gal.ComputeWorldScreenMatrix();
    //view->SetViewport( BOX2D( bBox.GetPosition(), bBox.GetSize() ) );
    VECTOR2D ssize = view->ToWorld( gal.GetScreenPixelSize(), false );
    VECTOR2D vsize = bBox.GetSize();
    double zoom  = 1.0 / std::max( fabs( vsize.x / ssize.x ), fabs( vsize.y / ssize.y ) );
    gal.SetLookAtPoint( bBox.Centre() );
    gal.SetZoomFactor( m_PrintParams.m_YScaleAdjust * zoom );
    //gal.SetZoomFactor( gal.GetZoomFactor() * zoom );
    //gal.SetZoomFactor( zoom );
    gal.ComputeWorldScreenMatrix();

    std::cout << "bbox pos: " << bBox.GetPosition() << " size: " << bBox.GetSize() << std::endl;

    gal.SetFlip( m_PrintParams.m_PrintMirror, false );
    //gal.Scale( VECTOR2D( userscale, userscale ) );
    //gal.Rotate( rotation );
    //gal.Translate( translation );

    
    long int dpi = Millimeter2iu( 20 );
    //long int dpi = dc->GetPPI().x;
#endif

    gal.BeginDrawing();
    
    cairo_t* cr = printCtx.GetContext();
    cairo_matrix_t cairoTransformation;
    cairo_get_matrix( cr, &cairoTransformation );

    cairoTransformation.xx = 1.0;
    cairoTransformation.yy = 1.0;

    cairo_set_matrix( cr, &cairoTransformation );
    cairo_get_matrix( cr, &cairoTransformation );


    printf("DoPrint\n");
    printf("Matrix:\n %.10f %.10f %.10f\n", cairoTransformation.xx, cairoTransformation.xy, cairoTransformation.x0 );
    printf("%.10f %.10f %.10f\n", cairoTransformation.yx, cairoTransformation.yy, cairoTransformation.y0 );
    //double dpi = 1.0;
    double ppi = 96.0;

    #define A4_WIDTH_INCH 11.69
    #define A4_HEIGHT_INCH 8.27

    for(int x = 0; x < 20; ++x) {
        for(int y = 0; y < 20; ++y) {

                double xx0 = (double) x / 20.0 * A4_WIDTH_INCH * ppi;
                double yy0 = (double) y / 20.0 * A4_HEIGHT_INCH * ppi;
                double xx1 = (double) (x+1) / 20.0 * A4_WIDTH_INCH * ppi;
                double yy1 = (double) (y+1) / 20.0 * A4_HEIGHT_INCH * ppi;

                //cairo_matrix_init_identity( &cairoTransformation );
                //cairo_set_matrix( cr, &cairoTransformation );

                cairo_set_source_rgb( cr, 0, 0, 0 );
                cairo_set_line_width (cr, 5.0);
                cairo_new_path (cr);
                cairo_move_to (cr, xx0, yy0);
                cairo_line_to (cr, xx1, yy0 );
                cairo_line_to (cr, xx1, yy1 );
                cairo_line_to (cr, xx0, yy1 );
                cairo_line_to (cr, xx0, yy0 );
                cairo_close_path (cr);
                cairo_stroke (cr);
        }
    }

    //view->Redraw();
    gal.EndDrawing();
}


#if 0
void BOARD_PRINTOUT_CONTROLLER::DrawPage( const wxString& aLayerName, int aPageNum, int aPageCount )
{
    wxPoint       offset;
    double        userscale;
    EDA_RECT      boardBoundingBox;
    EDA_RECT      drawRect;
    wxDC*         dc = GetDC();
    BASE_SCREEN*  screen = m_Parent->GetScreen();
    bool          printMirror = m_PrintParams.m_PrintMirror;
    wxSize        pageSizeIU = m_Parent->GetPageSizeIU();
    int           tempScreenNumber;
    int           tempNumberOfScreens;

    wxBusyCursor  dummy;

    BOARD* brd = ((PCB_BASE_FRAME*) m_Parent)->GetBoard();
    boardBoundingBox = brd->ComputeBoundingBox();
    const wxString& titleblockFilename = brd->GetFileName();

    // Use the page size as the drawing area when the board is shown or the user scale
    // is less than 1.
    if( m_PrintParams.PrintBorderAndTitleBlock() )
        boardBoundingBox = EDA_RECT( wxPoint( 0, 0 ), pageSizeIU );

    wxLogTrace( tracePrinting, wxT( "Drawing bounding box:                 x=%d, y=%d, w=%d, h=%d" ),
                boardBoundingBox.GetX(), boardBoundingBox.GetY(),
                boardBoundingBox.GetWidth(), boardBoundingBox.GetHeight() );

    // Compute the PCB size in internal units
    userscale = m_PrintParams.m_PrintScale;

    if( m_PrintParams.m_PrintScale == 0 )   //  fit in page option
    {
        if( boardBoundingBox.GetWidth() && boardBoundingBox.GetHeight() )
        {
            int margin = Millimeter2iu( 10.0 ); // add a margin around the drawings
            double scaleX = (double)(pageSizeIU.x - (2 * margin)) /
                            boardBoundingBox.GetWidth();
            double scaleY = (double)(pageSizeIU.y - (2 * margin)) /
                            boardBoundingBox.GetHeight();
            userscale = (scaleX < scaleY) ? scaleX : scaleY;
        }
        else
            userscale = 1.0;
    }

    wxSize scaledPageSize = pageSizeIU;
    drawRect.SetSize( scaledPageSize );
    scaledPageSize.x = wxRound( scaledPageSize.x / userscale );
    scaledPageSize.y = wxRound( scaledPageSize.y / userscale );


    if( m_PrintParams.m_PageSetupData )
    {
        wxLogTrace( tracePrinting, wxT( "Fit size to page margins:         x=%d, y=%d" ),
                    scaledPageSize.x, scaledPageSize.y );

        // Always scale to the size of the paper.
        FitThisSizeToPageMargins( scaledPageSize, *m_PrintParams.m_PageSetupData );
    }

    // Compute Accurate scale 1
    if( m_PrintParams.m_PrintScale == 1.0 )
    {
        // We want a 1:1 scale, regardless the page setup
        // like page size, margin ...
        MapScreenSizeToPaper(); // set best scale and offset (scale is not used)
        int w, h;
        GetPPIPrinter( &w, &h );
        double accurate_Xscale = (double) w / (IU_PER_MILS*1000);
        double accurate_Yscale = (double) h / (IU_PER_MILS*1000);

        if( IsPreview() )  // Scale must take in account the DC size in Preview
        {
            // Get the size of the DC in pixels
            wxSize       PlotAreaSize;
            dc->GetSize( &PlotAreaSize.x, &PlotAreaSize.y );
            GetPageSizePixels( &w, &h );
            accurate_Xscale *= (double)PlotAreaSize.x / w;
            accurate_Yscale *= (double)PlotAreaSize.y / h;
        }
        // Fine scale adjust
        accurate_Xscale *= m_PrintParams.m_XScaleAdjust;
        accurate_Yscale *= m_PrintParams.m_YScaleAdjust;

        // Set print scale for 1:1 exact scale
        dc->SetUserScale( accurate_Xscale, accurate_Yscale );
    }

    // Get the final size of the DC in pixels
    wxSize       PlotAreaSizeInPixels;
    dc->GetSize( &PlotAreaSizeInPixels.x, &PlotAreaSizeInPixels.y );
    wxLogTrace( tracePrinting, wxT( "Plot area in pixels:              x=%d, y=%d" ),
                PlotAreaSizeInPixels.x, PlotAreaSizeInPixels.y );
    double scalex, scaley;
    dc->GetUserScale( &scalex, &scaley );
    wxLogTrace( tracePrinting, wxT( "DC user scale:                    x=%g, y=%g" ),
                scalex, scaley );

    wxSize PlotAreaSizeInUserUnits;
    PlotAreaSizeInUserUnits.x = KiROUND( PlotAreaSizeInPixels.x / scalex );
    PlotAreaSizeInUserUnits.y = KiROUND( PlotAreaSizeInPixels.y / scaley );
    wxLogTrace( tracePrinting, wxT( "Scaled plot area in user units:   x=%d, y=%d" ),
                PlotAreaSizeInUserUnits.x, PlotAreaSizeInUserUnits.y );

    // In module editor, the module is located at 0,0 but for printing
    // it is moved to pageSizeIU.x/2, pageSizeIU.y/2.
    // So the equivalent board must be moved to the center of the page:
    if( m_Parent->IsType( FRAME_PCB_MODULE_EDITOR ) )
    {
        boardBoundingBox.Move( wxPoint( pageSizeIU.x/2, pageSizeIU.y/2 ) );
    }

    // In some cases the plot origin is the centre of the board outline rather than the center
    // of the selected paper size.
    if( m_PrintParams.CenterOnBoardOutline() )
    {
        // Here we are only drawing the board and it's contents.
        drawRect = boardBoundingBox;
        offset.x += wxRound( (double) -scaledPageSize.x / 2.0 );
        offset.y += wxRound( (double) -scaledPageSize.y / 2.0 );

        wxPoint center = boardBoundingBox.Centre();

        if( printMirror )
        {
            // Calculate the mirrored center of the board.
            center.x = m_Parent->GetPageSizeIU().x - boardBoundingBox.Centre().x;
        }

        offset += center;
    }

    GRResetPenAndBrush( dc );

    EDA_DRAW_PANEL* panel = m_Parent->GetCanvas();
    EDA_RECT        tmp   = *panel->GetClipBox();

    // Set clip box to the max size
    #define MAX_VALUE (INT_MAX/2)   // MAX_VALUE is the max we can use in an integer
                                    // and that allows calculations without overflow
    panel->SetClipBox( EDA_RECT( wxPoint( 0, 0 ), wxSize( MAX_VALUE, MAX_VALUE ) ) );

    screen->m_IsPrinting = true;
    COLOR4D bg_color = m_Parent->GetDrawBgColor();

    // Print frame reference, if requested, before
    if( m_PrintParams.m_Print_Black_and_White )
        GRForceBlackPen( true );

    if( m_PrintParams.PrintBorderAndTitleBlock() )
    {
        tempScreenNumber = screen->m_ScreenNumber;
        tempNumberOfScreens = screen->m_NumberOfScreens;
        screen->m_ScreenNumber = aPageNum;
        screen->m_NumberOfScreens = aPageCount;
        m_Parent->DrawWorkSheet( dc, screen, m_PrintParams.m_PenDefaultSize,
                                 IU_PER_MILS, titleblockFilename, aLayerName );
        screen->m_ScreenNumber = tempScreenNumber;
        screen->m_NumberOfScreens = tempNumberOfScreens;
    }

    if( printMirror )
    {
        // To plot mirror, we reverse the x axis, and modify the plot x origin
        dc->SetAxisOrientation( false, false);

        /* Plot offset x is moved by the x plot area size in order to have
         * the old draw area in the new draw area, because the draw origin has not moved
         * (this is the upper left corner) but the X axis is reversed, therefore the plotting area
         * is the x coordinate values from  - PlotAreaSize.x to 0 */
        int x_dc_offset = PlotAreaSizeInPixels.x;
        x_dc_offset = KiROUND( x_dc_offset  * userscale );
        dc->SetDeviceOrigin( x_dc_offset, 0 );

        wxLogTrace( tracePrinting, wxT( "Device origin:                    x=%d, y=%d" ),
                    x_dc_offset, 0 );

        panel->SetClipBox( EDA_RECT( wxPoint( -MAX_VALUE/2, -MAX_VALUE/2 ),
                                     panel->GetClipBox()->GetSize() ) );
    }

    // screen->m_DrawOrg = offset;
    dc->SetLogicalOrigin( offset.x, offset.y );

    wxLogTrace( tracePrinting, wxT( "Logical origin:                   x=%d, y=%d" ),
                offset.x, offset.y );

#if defined(wxUSE_LOG_TRACE) && defined( DEBUG )
    wxRect paperRect = GetPaperRectPixels();
    wxLogTrace( tracePrinting, wxT( "Paper rectangle:                  left=%d, top=%d, "
                                    "right=%d, bottom=%d" ),
                paperRect.GetLeft(), paperRect.GetTop(), paperRect.GetRight(),
                paperRect.GetBottom() );

    int devLeft = dc->LogicalToDeviceX( drawRect.GetX() );
    int devTop = dc->LogicalToDeviceY( drawRect.GetY() );
    int devRight = dc->LogicalToDeviceX( drawRect.GetRight() );
    int devBottom = dc->LogicalToDeviceY( drawRect.GetBottom() );
    wxLogTrace( tracePrinting, wxT( "Final device rectangle:           left=%d, top=%d, "
                                    "right=%d, bottom=%d\n" ),
                devLeft, devTop, devRight, devBottom );
#endif

    m_Parent->SetDrawBgColor( WHITE );

    /* when printing in color mode, we use the graphic OR mode that gives the same look as
     * the screen but because the background is white when printing, we must use a trick:
     * In order to plot on a white background in OR mode we must:
     * 1 - Plot all items in black, this creates a local black background
     * 2 - Plot in OR mode on black "local" background
     */
    if( !m_PrintParams.m_Print_Black_and_White )
    {
        // Creates a "local" black background
        GRForceBlackPen( true );
        m_Parent->PrintPage( dc, m_PrintParams.m_PrintMaskLayer,
                             printMirror, &m_PrintParams );
        GRForceBlackPen( false );
    }
    else
        GRForceBlackPen( true );


    m_Parent->PrintPage( dc, m_PrintParams.m_PrintMaskLayer, printMirror,
                         &m_PrintParams );

    m_Parent->SetDrawBgColor( bg_color );
    screen->m_IsPrinting = false;
    panel->SetClipBox( tmp );
    GRForceBlackPen( false );
}
#endif
