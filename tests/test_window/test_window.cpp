/* 
   Simple STEP/IGES File Viewer

   GL stuff based on wxWidgets "isosurf" example.
   
   T.W. 2013
*/

#include <wx/wx.h>
#include <wx/app.h>

#include <wx/timer.h>
#include <wx/math.h>
#include <wx/log.h>
#include <wx/popupwin.h>

#include <gal/graphics_abstraction_layer.h>
#include <class_draw_panel_gal.h>
#include <view/view.h>
#include <view/view_overlay.h>
#include <view/wx_view_controls.h>

#include "test_window.h"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    // Create the main frame window
    new GAL_TEST_FRAME(NULL, wxT("board-test"));

    return true;
}

void MyApp::OnInitCmdLine(wxCmdLineParser& parser)
{
/*    parser.AddSwitch("", "sb", "Do not use double buffering");
    parser.AddSwitch("", "db", "Use double buffering");
    parser.AddSwitch("", "va", "Use vertex arrays");

    wxApp::OnInitCmdLine(parser);*/
}

bool MyApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    /*if (parser.Found("sb"))
        g_doubleBuffer = GL_FALSE;
    else if (parser.Found("db"))
        g_doubleBuffer = GL_TRUE;

    
    return wxApp::OnCmdLineParsed(parser);*/
    return true;
}

//---------------------------------------------------------------------------
// GAL_TEST_FRAME
//---------------------------------------------------------------------------

//BEGIN_EVENT_TABLE(GAL_TEST_FRAME, wxFrame)
  //  EVT_MENU(wxID_EXIT, GAL_TEST_FRAME::OnExit)
    //EVT_MENU(wxID_OPEN, GAL_TEST_FRAME::OnMenuFileOpen)
    ///VT_MOTION(GAL_TEST_FRAME::OnMotion)
//END_EVENT_TABLE()


void GAL_TEST_FRAME::OnMenuFileOpen( wxCommandEvent& WXUNUSED(event) )
{
}




void GAL_TEST_FRAME::OnMotion( wxMouseEvent& event )
{
}


GAL_TEST_FRAME::GAL_TEST_FRAME(wxFrame *frame, const wxString& title, const wxPoint& pos,
                 const wxSize& size, long style)
    : wxFrame(frame, wxID_ANY, title, pos, size, style)
{
    

    // Make a menubar
    wxMenu *fileMenu = new wxMenu;

    fileMenu->Append(wxID_OPEN, wxT("&Open..."));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, wxT("E&xit"));
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, wxT("&File"));
    SetMenuBar(menuBar);

    Show(true);
    Raise();

    m_galPanel = new EDA_DRAW_PANEL_GAL( this, -1, wxPoint( 0, 0 ), wxDefaultSize, EDA_DRAW_PANEL_GAL::GAL_TYPE_OPENGL ) ;
    m_galPanel->SetEvtHandlerEnabled( true );
    m_galPanel->SetFocus();

    //m_galPanel->Connect ( wxEVT_MOTION, wxMouseEventHandler( GAL_TEST_FRAME::OnMotion ), NULL, this );
    m_galPanel->StartDrawing();

    KIGFX::VIEW *view = m_galPanel->GetView();

    printf("View: %p\n", view );

    view->SetLayerTarget( 0, KIGFX::TARGET_OVERLAY );
    
    view->SetCenter (VECTOR2D (0, 0));
    view->SetScale ( 1.0 );

    m_ovl = m_galPanel->GetView()->MakeOverlay();

    m_ovl->Begin();
    
    for (double angle = 0; angle < 360.0; angle += 5.0)
    {
        VECTOR2D center (0, 0);
        VECTOR2D p ( center.x + 20000000 * cos (angle * M_PI/180.0), center.y + 20000000 * sin (angle * M_PI/180.0) );
        
        VECTOR2D p0 =  center;
        VECTOR2D p1 =  p;
        
//        m_ovl->DrawLine (p0, p1 );
    
        m_ovl->TestLine (p0.x, p0.y, p1.x, p1.y);
        printf("%.0f %.0f %.0f %.0f\n", p0.x, p0.y, p1.x ,p1.y);
    


     //   printf("DrawL %.1f\n", angle);
        
    }
    m_ovl->End();


}

GAL_TEST_FRAME::~GAL_TEST_FRAME()
{
    delete m_galPanel;
}

// Intercept menu commands
void GAL_TEST_FRAME::OnExit( wxCommandEvent& WXUNUSED(event) )
{
    // true is to force the frame to close
    Close(true);
}

