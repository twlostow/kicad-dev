/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 CERN
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

#include <geometry/shape_poly_set.h>
#include <geometry/shape_line_chain.h>
#include <geometry/shape_file_io.h>

#include <io_mgr.h>
#include <kicad_plugin.h>

#include <class_board.h>
#include <class_zone.h>
#include <class_pad.h>
#include <class_track.h>
#include <class_drawsegment.h>
#include <class_module.h>

#include <pcb_plot_params.h>
#include <pcb_view.h>
#include <printout_controler.h>
#include <pcb_draw_panel_gal.h>

BOARD* loadBoard( const std::string& filename )
{
    PLUGIN::RELEASER pi( new PCB_IO );
    BOARD* brd = nullptr;

    try
    {
        brd = pi->Load( wxString( filename.c_str() ), NULL, NULL );
    }
    catch( const IO_ERROR& ioe )
    {
        wxString msg = wxString::Format( _( "Error loading board.\n%s" ),
                ioe.Problem() );

        printf( "%s\n", (const char*) msg.mb_str() );
        return nullptr;
    }

    return brd;
}

int doPrint(wxWindow* parent, BOARD* board )
{
    PRINT_PARAMETERS  printParams;
    wxPrintData* printData;
    wxPageSetupDialogData* pageSetupData = nullptr;

    printParams.m_PrintMirror = false;
    printParams.m_Print_Sheet_Ref = false;
    printParams.m_Print_Black_and_White = false;

    printParams.m_DrillShapeOpt = PRINT_PARAMETERS::NO_DRILL_SHAPE;
    printParams.m_OptionPrintPage = 0;

    printParams.m_PrintMaskLayer = LSET();

    printParams.m_PrintMaskLayer.set( F_Cu );
    printParams.m_PrintMaskLayer.set( B_Cu );
    printParams.m_PrintMaskLayer.set( Edge_Cuts );
    printParams.m_PrintScale =  1.0;
    printParams.m_XScaleAdjust = 1.0;
    printParams.m_YScaleAdjust = 1.0;
    printParams.m_PenDefaultSize = 1.0;

    const PAGE_INFO& pageInfo = board->GetPageSettings();

    printData = new wxPrintData();

    if( !printData->Ok() )
    {
            printf("Error Init Printer info");
            return -1;
    }

    printData->SetQuality( wxPRINT_QUALITY_HIGH );      // Default resolution = HIGH;

    
    pageSetupData = new wxPageSetupDialogData( *printData );
    printParams.m_PageSetupData = pageSetupData;

    pageSetupData->SetPaperId( pageInfo.GetPaperId() );
    pageSetupData->GetPrintData().SetOrientation( pageInfo.GetWxOrientation() );

    printf("page-is-custom %d orientation %s\n", !!pageInfo.IsCustom(), pageInfo.GetWxOrientation()==wxLANDSCAPE ? "landscape" : "portrait" );


    if( pageInfo.IsCustom() )
    {
        if( pageInfo.IsPortrait() )
            pageSetupData->SetPaperSize( wxSize( Mils2mm( pageInfo.GetWidthMils() ),
                                                   Mils2mm( pageInfo.GetHeightMils() ) ) );
        else
            pageSetupData->SetPaperSize( wxSize( Mils2mm( pageInfo.GetHeightMils() ),
                                                   Mils2mm( pageInfo.GetWidthMils() ) ) );
    }

    *printData = pageSetupData->GetPrintData();


    wxPrintDialogData printDialogData( *printData );
    printDialogData.SetMaxPage( 1 ); //s_Parameters.m_PageCount );
    printDialogData.SetPrintToFile(true);

    wxPrinter printer( &printDialogData );

    auto data = printer.GetPrintDialogData().GetPrintData();

    auto view = new KIGFX::PCB_VIEW();
    view->SetBoard( board );

    printf("view %p\n", view);
    wxString  title = _( "Print" );
    BOARD_PRINTOUT_CONTROLLER printout( printParams, board, view, title );

    if( !printer.Print( parent, &printout, true ) )
    {
        printf("Error printing!\n");
        return -1;
    }
    else
    {
        *printData = printer.GetPrintDialogData().GetPrintData();
    }

    data = printer.GetPrintDialogData().GetPrintData();

    return 0;
}
#if 0
int main( int argc, char *argv[] )
{
    wxInitializer initializer;
    if ( !initializer )
    {
        fprintf(stderr, "Failed to initialize the wxWidgets library, aborting.");
        return -1;
    }

    //auto app = wxApp::GetInstance ();
    //auto top = app->GetTopWindow();

    //printf("top window %p\n", top );

    auto brd = loadBoard(argv[1]);

    doPrint(brd);

    return 0;
}
#endif

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:
    // ctor(s)
    MyFrame(const wxString& title);


private:
    // any class wishing to process wxWidgets events must use this macro
    wxDECLARE_EVENT_TABLE();
};

MyFrame::MyFrame(const wxString& title)
       : wxFrame(NULL, wxID_ANY, title)
{

}

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
wxEND_EVENT_TABLE()


class MyApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
};

IMPLEMENT_APP(MyApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;

    // create the main application window
    MyFrame *frame = new MyFrame("Minimal wxWidgets App");

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    //frame->Show(true);

    auto brd = loadBoard("../../../../tests/dp.kicad_pcb");

    doPrint(frame, brd);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return false;
}
