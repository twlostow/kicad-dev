#include <pcb_test_frame.h>
// #include <tools/outline_editor.h>
#include <tool/tool_manager.h>

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
        *(volatile uint32_t *) 0xdeadbeef = 0;
    }

    virtual ~OED_TEST_FRAME() {}
};

wxFrame* CreateMainFrame( const std::string& aFileName )
{
    auto frame = new OED_TEST_FRAME( nullptr, wxT( "Outline Editor Test" ) );

    return frame;
}

