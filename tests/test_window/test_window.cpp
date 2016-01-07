/*
 *  Simple STEP/IGES File Viewer
 *
 *  GL stuff based on wxWidgets "isosurf" example.
 *
 *  T.W. 2013
 */

#include <wx/wx.h>
#include <wx/app.h>

#include <wx/timer.h>
#include <wx/math.h>
#include <wx/log.h>
#include <wx/popupwin.h>

#include <layers_id_colors_and_visibility.h>

#include <gal/graphics_abstraction_layer.h>
#include <view/view_ng.h>
#include <class_draw_panel_gal.h>
#include <pcb_draw_panel_gal.h>
#include <view/view_overlay.h>
#include <view/wx_view_controls_ng.h>
#include <pcb_painter.h>
#include <pad_shapes.h>
#include <profile.h>

#include <class_pad.h>
#include <class_module.h>
#include <class_board.h>
#include <class_track.h>
#include <class_zone.h>

#include <pcb_view.h>
#include <pcb_painter.h>

#include <io_mgr.h>

#include <set>

#include <boost/foreach.hpp>
#include <boost/bind.hpp>

#include "test_window.h"

IMPLEMENT_APP( MyApp )

using namespace KIGFX;
bool MyApp::OnInit()
{
    if( !wxApp::OnInit() )
        return false;

    // Create the main frame window
    new GAL_TEST_FRAME( NULL, wxT( "board-test" ) );

    return true;
}


const BOX2I VIEW_ITEM_NG::ngViewBBox() const
{
    return BOX2I();
}



void MyApp::OnInitCmdLine( wxCmdLineParser& parser )
{
/*    parser.AddSwitch("", "sb", "Do not use double buffering");
 *   parser.AddSwitch("", "db", "Use double buffering");
 *   parser.AddSwitch("", "va", "Use vertex arrays");
 *
 *   wxApp::OnInitCmdLine(parser);*/
}


bool MyApp::OnCmdLineParsed( wxCmdLineParser& parser )
{
    /*if (parser.Found("sb"))
     *   g_doubleBuffer = GL_FALSE;
     *  else if (parser.Found("db"))
     *   g_doubleBuffer = GL_TRUE;
     *
     *
     *  return wxApp::OnCmdLineParsed(parser);*/
    return true;
}


// ---------------------------------------------------------------------------
// GAL_TEST_FRAME
// ---------------------------------------------------------------------------

// BEGIN_EVENT_TABLE(GAL_TEST_FRAME, wxFrame)
// EVT_MENU(wxID_EXIT, GAL_TEST_FRAME::OnExit)
// EVT_MENU(wxID_OPEN, GAL_TEST_FRAME::OnMenuFileOpen)
///VT_MOTION(GAL_TEST_FRAME::OnMotion)
// END_EVENT_TABLE()


void GAL_TEST_FRAME::OnMenuFileOpen( wxCommandEvent& WXUNUSED( event ) )
{
}


void GAL_TEST_FRAME::OnMotion( wxMouseEvent& event )
{
}



GAL_TEST_FRAME::GAL_TEST_FRAME( wxFrame* frame, const wxString& title, const wxPoint& pos,
        const wxSize& size, long style ) :
    wxFrame( frame, wxID_ANY, title, pos, size, style )
{
    // Make a menubar
    wxMenu* fileMenu = new wxMenu;

    fileMenu->Append( wxID_OPEN, wxT( "&Open..." ) );
    fileMenu->AppendSeparator();
    fileMenu->Append( wxID_EXIT, wxT( "E&xit" ) );
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append( fileMenu, wxT( "&File" ) );
    SetMenuBar( menuBar );

    Show( true );
    Raise();

    m_galPanel = new PCB_DRAW_PANEL_GAL( this, -1, wxPoint( 0,
                    0 ), wxDefaultSize, EDA_DRAW_PANEL_GAL::GAL_TYPE_OPENGL );
    m_galPanel->SetEvtHandlerEnabled( true );
    m_galPanel->SetFocus();
    m_galPanel->Show( true );
    m_galPanel->Raise();

    m_galPanel->StartDrawing();

    BOARD* b = new BOARD();

    PLUGIN::RELEASER pi( IO_MGR::PluginFind( IO_MGR::KICAD ) );

    try
    {
        //b = pi->Load( wxT( "../../demos/video/video.kicad_pcb" ), NULL, NULL );
         b=pi -> Load( wxT("../../../altium-import/wrs.kicad_pcb"), NULL, NULL );
    }
    catch( const IO_ERROR& ioe )
    {
        wxString msg = wxString::Format( _( "Error loading board.\n%s" ),
                ioe.errorText.GetData() );

        printf( "%s\n", (const char*) msg.mb_str() );
    }
    printf( "brd %p\n", b );

    m_galPanel->DisplayBoard( b );

/*    BOARD* brd = new BOARD();
 *   TRACK  t1(brd);
 *
 *   t1.SetStart(wxPoint(0, 0));
 *   t1.SetEnd(wxPoint(1000000, 0));
 *   t1.SetLayer(F_Cu);
 *   t1.SetWidth(100000);*/

    m_ovl = m_galPanel->GetView()->MakeOverlay();

    for( double angle = 0; angle < 360.0; angle += 5.0 )
    {
        VECTOR2D center( 0, 0 );
        VECTOR2D p( center.x + 20000000 * cos( angle * M_PI / 180.0 ), center.y + 20000000 * sin(
                        angle * M_PI / 180.0 ) );

        VECTOR2D p0 = center;
        VECTOR2D p1 = p;

        m_ovl->DrawLine( p0, p1 );

// printf("%.0f %.0f %.0f %.0f\n", p0.x, p0.y, p1.x ,p1.y);


        // printf("DrawL %.1f\n", angle);
    }

    m_ovl->End();

}


GAL_TEST_FRAME::~GAL_TEST_FRAME()
{
    delete m_galPanel;
}


// Intercept menu commands
void GAL_TEST_FRAME::OnExit( wxCommandEvent& WXUNUSED( event ) )
{
    // true is to force the frame to close
    Close( true );
}
