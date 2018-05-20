#include <sch_test_frame.h>
#include <tool/tool_manager.h>

#if 0
class SCH_LIB_TEST_FRAME : public SCH_TEST_FRAME
{
public:
    SCH_LIB_TEST_FRAME( wxFrame* frame,
            const wxString& title,
            const wxPoint& pos  = wxDefaultPosition,
            const wxSize& size  = wxDefaultSize,
            long style = wxDEFAULT_FRAME_STYLE ) :
        SCH_TEST_FRAME( frame, title, pos, size, style )
    {
        registerTools();
    }

    void registerTools();

    virtual ~SCH_LIB_TEST_FRAME() {}
};

wxFrame* CreateMainFrame( const std::string& aFileName )
{
    auto frame = new SCH_TEST_FRAME( nullptr, wxT( "Symbol Library Editor Test" ) );

    if( aFileName != "" )
    {
//        frame->LoadAndDisplayPart( aFileName );
        frame->LoadAndDisplaySchematic( aFileName );

    }

    return frame;
}

void SCH_LIB_TEST_FRAME::registerTools()
{
    m_toolManager->InitTools();
    m_toolManager->ResetTools( TOOL_BASE::MODEL_RELOAD );
}

#endif
