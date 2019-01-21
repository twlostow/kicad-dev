#include <pcb_draw_panel_gal.h>
#include <pcb_test_frame.h>
#include <tool/tool_manager.h>


#include "router/pns_joint.h"
#include "router/pns_kicad_iface.h"
#include "router/pns_node.h"
#include "router/pns_router.h"

class PNS_TEST_FRAME : public PCB_TEST_FRAME
{
public:
    PNS_TEST_FRAME( wxFrame* frame, const wxString& title, const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE )
            : PCB_TEST_FRAME( frame, title, pos, size, style )
    {
        registerTools();

        m_galPanel->Connect(
                wxEVT_MOTION, wxMouseEventHandler( PNS_TEST_FRAME::OnMotion ), NULL, this );
    }

    void SetupRouter()
    {
        m_iface.reset( new PNS_KICAD_IFACE );

        m_iface->SetBoard( m_board.get() );
        m_iface->SetView( m_galPanel->GetView() );
        //m_iface->SetHostFrame( NULL );

        m_router.reset( new PNS::ROUTER );
        m_router->SetInterface( m_iface.get() );
        m_router->ClearWorld();
        m_router->SyncWorld();
        m_router->Settings().SetMode( PNS::RM_Walkaround );
        m_router->StartRouting( VECTOR2I( 0, 0 ), NULL, F_Cu );

        m_galPanel->GetViewControls()->ShowCursor( true );
    }

    virtual void OnExit( wxCommandEvent& event ) override{};

    virtual void OnMotion( wxMouseEvent& aEvent ) override
    {
        VECTOR2I p = m_galPanel->GetViewControls()->GetCursorPosition();

        //printf( "Move [%d, %d]\n ", p.x, p.y );
        m_router->Move( p, NULL );
        m_galPanel->Refresh();
        aEvent.Skip();
    };

    virtual void OnMenuFileOpen( wxCommandEvent& WXUNUSED( event ) ) override{};

    void registerTools();

    virtual ~PNS_TEST_FRAME()
    {
    }

private:
    std::unique_ptr<PNS_KICAD_IFACE> m_iface;
    std::unique_ptr<PNS::ROUTER>     m_router;
};

wxFrame* CreateMainFrame( wxCmdLineParser& aCmdLine )
{
    auto frame = new PNS_TEST_FRAME( nullptr, wxT( "P&S Test" ) );

    /*if( aFileName != "" )
    {
        frame->LoadAndDisplayBoard( aFileName );
    }
    else
    {
        frame->LoadAndDisplayBoard( "router_test.kicad_pcb" );
    }*/

    frame->SetupRouter();

    return frame;
}

void PNS_TEST_FRAME::registerTools()
{
    m_toolManager->InitTools();
    m_toolManager->ResetTools( TOOL_BASE::MODEL_RELOAD );
}


#if 0
PNS::ITEM* GAL_TEST_FRAME::findItemByPosition( VECTOR2I pos, PNS::ITEM::PnsKind kind )
{

    for( auto item : m_router->QueryHoverItems( pos ).CItems() )
    {
        if( item.item->OfKind( kind ) )
            return item.item;
    }
    return NULL;
}
#endif
