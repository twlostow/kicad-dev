#include <pcb_test_frame.h>
// #include <tools/outline_editor.h>
#include <tool/tool_manager.h>
#include <pcb_draw_panel_gal.h>
#include <view/view.h>
#include <gal/graphics_abstraction_layer.h>

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
        registerTools();
        drawDummySegment();
    }

    void drawDummySegment();
    void registerTools();

    virtual ~OED_TEST_FRAME() {}
};

wxFrame* CreateMainFrame( const std::string& aFileName )
{
    auto frame = new OED_TEST_FRAME( nullptr, wxT( "PCB Test Window" ) );

    if( aFileName != "" )
    {
        frame->LoadAndDisplayBoard( aFileName );
    }

    return frame;
}

void OED_TEST_FRAME::registerTools()
{
//    m_toolManager->RegisterTool( new OUTLINE_EDITOR );
    m_toolManager->InitTools();
    m_toolManager->ResetTools( TOOL_BASE::MODEL_RELOAD );
}

class STUPID_ITEM : public KIGFX::VIEW_ITEM
{
public:
    
    
    /// @copydoc VIEW_ITEM::ViewDraw()
    void ViewDraw( int aLayer, KIGFX::VIEW* aView ) const override
    {
        auto gal = aView->GetGAL();

        gal->SetIsStroke(true);
        gal->SetStrokeColor( COLOR4D( 1.0, 0.0, 0.0, 1.0 ));
        gal->DrawSegment( VECTOR2D( -1000000, 0), VECTOR2D( 1000000, 0), 100000.0);

    }

    /// @copydoc VIEW_ITEM::ViewGetLayers()
    void ViewGetLayers( int aLayers[], int& aCount ) const override
    {
        aCount = 1;
        aLayers[0] = LAYER_GP_OVERLAY;
    }

    const BOX2I ViewBBox() const override
    {
        BOX2I bbox;

        bbox.SetMaximum();
    
        return bbox;
    }

};

void OED_TEST_FRAME::drawDummySegment()
{
   m_galPanel->GetView()->Add( new STUPID_ITEM );
}