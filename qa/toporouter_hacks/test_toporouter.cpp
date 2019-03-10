#include <pcb_test_frame.h>
#include <tool/tool_manager.h>
#include <tool/actions.h>


#include <gal/graphics_abstraction_layer.h>
#include <view/view.h>
#include <view/view_item.h>
#include <pcb_draw_panel_gal.h>

#include "toporouter.h"


class MY_PCB_TEST_FRAME : public PCB_TEST_FRAME
{
public:
    MY_PCB_TEST_FRAME( wxFrame* frame, const wxString& title,
            const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
            long style = wxDEFAULT_FRAME_STYLE )
            : PCB_TEST_FRAME( frame, title, pos, size, style, PCB_DRAW_PANEL_GAL::GAL_TYPE_OPENGL )
    {
        registerTools();
        //printf("Create2?\n");
    }

    void registerTools();

    virtual ~MY_PCB_TEST_FRAME()
    {
    }

    void zoomToFit()
    {
        auto bBox = m_board->ViewBBox();

        //printf("bb %d %d %d %d\n", bBox.GetOrigin().x, bBox.GetOrigin().y, bBox.GetSize().x, bBox.GetSize().y );

        if( bBox.GetWidth() == 0 || bBox.GetHeight() == 0 )
        {
            bBox = m_galPanel->GetDefaultViewBBox();
        }

        auto view = m_galPanel->GetView();

        auto size = m_galPanel->GetClientSize();
        //printf("sz %d %d\n", size.x, size.y );
        auto screenSize = view->ToWorld( m_galPanel->GetClientSize(), false );


        VECTOR2D vsize = bBox.GetSize();
        double scale = view->GetScale() / std::max( fabs( vsize.x / screenSize.x ),
                                                    fabs( vsize.y / screenSize.y ) );

        double margin_scale_factor = 1.1;

        view->SetScale( scale / margin_scale_factor );
        view->SetCenter( bBox.Centre() );
        m_galPanel->Refresh();

    }

    void SetupRouter()
    {
        m_router = new TOPOROUTER_ENGINE( m_galPanel.get() ); //::GetInstance();
        m_router->SetBoard( m_board.get() );

        zoomToFit();

        m_router->SyncWorld();
        auto view = m_galPanel->GetView();
        view->Add( m_router->GetPreview() );

        m_router->Run();
        m_router->ImportRoutes();

    }

    TOPOROUTER_ENGINE* m_router;
};

wxFrame* CreateMainFrame( const std::string& aFileName )
{
    auto frame = new MY_PCB_TEST_FRAME( nullptr, wxT( "Test PCB Frame" ) );

    if( aFileName != "" )
    {
        frame->LoadAndDisplayBoard( aFileName );
    }

    frame->Raise();
    frame->Show();
    frame->SetupRouter();
    
    

    return frame;
}

void MY_PCB_TEST_FRAME::registerTools()
{
    m_toolManager->InitTools();
    m_toolManager->ResetTools( TOOL_BASE::MODEL_RELOAD );

    
}
