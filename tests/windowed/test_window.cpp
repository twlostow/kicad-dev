#include <wx/wx.h>
#include <wx/app.h>

#include <layers_id_colors_and_visibility.h>

#include <gal/graphics_abstraction_layer.h>
#include <view/view.h>
#include <pcb_draw_panel_gal.h>

#include <view/wx_view_controls.h>
#include <pcb_painter.h>
#include <pad_shapes.h>
#include <profile.h>

#include <class_pad.h>
#include <class_board.h>
#include <class_track.h>
#include <class_zone.h>

#include <pcb_painter.h>
#include <wxPcbStruct.h>

#include <io_mgr.h>
#include <set>

#include "test_window.h"

#include "router/pns_router.h"
#include "router/pns_kicad_iface.h"
#include "router/pns_node.h"
#include "router/pns_joint.h"

IMPLEMENT_APP( MyApp )

using namespace KIGFX;

bool MyApp::OnInit()
{
    if( !wxApp::OnInit() )
        return false;

    new GAL_TEST_FRAME( NULL, wxT( "P&S Test Window" ) );

    return true;
}




void GAL_TEST_FRAME::OnMotion( wxMouseEvent& aEvent ){
    //VECTOR2I p = m_galPanel->GetViewControls()->GetCursorPosition();

    // printf("Move [%d, %d]\n ", p.x, p.y);
    // m_router->Move( p , NULL );
    //m_galPanel->Refresh();
    aEvent.Skip();

}


GAL_TEST_FRAME::GAL_TEST_FRAME( wxFrame* frame, const wxString& title, const wxPoint& pos,
        const wxSize& size, long style ) :
    wxFrame( frame, wxID_ANY, title, pos, size, style )
{
    // Make a menubar
    wxMenu* fileMenu = new wxMenu;

    fileMenu->Append( wxID_EXIT, wxT( "E&xit" ) );
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append( fileMenu, wxT( "&File" ) );
    SetMenuBar( menuBar );
    Maximize();

    Show( true );
    Raise();

    m_galPanel.reset( new  PCB_DRAW_PANEL_GAL( this, -1, wxPoint( 0,
                    0 ), wxDefaultSize, EDA_DRAW_PANEL_GAL::GAL_TYPE_OPENGL ) );
    m_galPanel->SetEvtHandlerEnabled( true );
    m_galPanel->SetFocus();
    m_galPanel->Show( true );
    m_galPanel->Raise();

    m_galPanel->StartDrawing();

    m_galPanel->Connect( wxEVT_MOTION, wxMouseEventHandler( GAL_TEST_FRAME::OnMotion ), NULL, this );

    PLUGIN::RELEASER pi( IO_MGR::PluginFind( IO_MGR::KICAD ) );

    try
    {
        m_board.reset( pi->Load( wxT( "/home/twl/Kicad-dev/kicad-git/demos/video/video.kicad_pcb" ), NULL, NULL ) );
    }
    catch( const IO_ERROR& ioe )
    {
        wxString msg = wxString::Format( _( "Error loading board.\n%s" ),
                ioe.What() );

        fprintf( stderr, "%s\n", (const char*) msg.mb_str() );
        exit(-1);
    }

    m_galPanel->DisplayBoard( m_board.get() );

    // create the router

    /*m_iface.reset(new PNS_KICAD_IFACE);

    m_iface->SetBoard( m_board.get() );
    m_iface->SetView( m_galPanel->GetView() );
    m_iface->SetHostFrame( NULL );

    m_router.reset(new PNS::ROUTER);
    m_router->SetInterface(m_iface.get());
    m_router->ClearWorld();
    m_router->SyncWorld();
    m_router->Settings().SetMode( PNS::RM_Shove );
    m_router->StartRouting( VECTOR2I(150*1000000, 86 * 1000000), NULL, F_Cu );
    for(int i = 0; i < 15000000; i+=1000000 )
        m_router->Move( VECTOR2I(150*1000000, 86 * 1000000 + i), NULL );
*/
    m_galPanel->GetViewControls()->ShowCursor(true);

}

GAL_TEST_FRAME::~GAL_TEST_FRAME()
{

}

PNS::ITEM *GAL_TEST_FRAME::findItemByPosition( VECTOR2I pos, PNS::ITEM::PnsKind kind)
{

    for ( auto item : m_router->QueryHoverItems( pos ).CItems() )
    {
        if ( item.item->OfKind(kind) )
            return item.item;
    }
    return NULL;
}

// Intercept menu commands
void GAL_TEST_FRAME::OnExit( wxCommandEvent& WXUNUSED( event ) )
{
    // true is to force the frame to close
    Close( true );
}
