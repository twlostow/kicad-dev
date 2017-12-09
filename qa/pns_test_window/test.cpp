#include <pcb_test_frame.h>
#include <tool/tool_manager.h>

#include <router/pns_router.h>
#include <router/pns_kicad_iface.h>
#include <class_draw_panel_gal.h>
#include <pcb_draw_panel_gal.h>
#include <profile.h>

class OED_TEST_FRAME : public PCB_TEST_FRAME
{
public:
    OED_TEST_FRAME( wxFrame* frame,
            const wxString& title,
            const wxPoint& pos  = wxDefaultPosition,
            const wxSize& size  = wxDefaultSize,
            long style = wxDEFAULT_FRAME_STYLE ) :
        PCB_TEST_FRAME( frame, title, pos, size, style )
    {
        printf("Load:\n");
        LoadAndDisplayBoard( "noname.kicad_pcb");
        registerTools();


        m_iface.reset(new PNS_KICAD_IFACE);

        m_iface->SetBoard( m_board.get() );
        m_iface->SetView( m_galPanel->GetView() );

        m_router.reset(new PNS::ROUTER);
        m_router->SetInterface(m_iface.get());
        m_router->ClearWorld();
        m_router->SyncWorld();
        m_router->Settings().SetMode( PNS::RM_MarkObstacles );
        m_router->FlipPosture();
        m_router->StartRouting( VECTOR2I(150*1000000, 86 * 1000000), NULL, F_Cu );

        m_galPanel->Connect( wxEVT_MOTION,
                                wxMouseEventHandler( OED_TEST_FRAME::OnMotion ), NULL, this );
    }

    void registerTools();

    void OnMotion( wxMouseEvent& aEvent )
    {
        VECTOR2I p = m_galPanel->GetViewControls()->GetCursorPosition();

        printf("Move [%d, %d]\n ", p.x, p.y);

        PROF_COUNTER cnt("route");
        m_router->Move( p , NULL );
        cnt.Show();
//        m_galPanel->Refresh();
        aEvent.Skip();

    }

    virtual ~OED_TEST_FRAME() {}
private:

    std::unique_ptr<PNS::ROUTER> m_router;
    std::unique_ptr<PNS_KICAD_IFACE> m_iface;
};

wxFrame* CreateMainFrame( const std::string& aFileName )
{
    auto frame = new OED_TEST_FRAME( nullptr, wxT( "Outline Editor Test" ) );

    if( aFileName != "" )
    {

    }

    return frame;
}

void OED_TEST_FRAME::registerTools()
{
//    m_toolManager->RegisterTool( new OUTLINE_EDITOR );
    m_toolManager->InitTools();
    m_toolManager->ResetTools( TOOL_BASE::MODEL_RELOAD );
}
