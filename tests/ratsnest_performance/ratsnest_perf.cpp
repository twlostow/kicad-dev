#include <wx/wx.h>
#include <wx/app.h>

#include <class_board.h>
#include <io_mgr.h>

#include <ratsnest_data.h>

#include <valgrind/callgrind.h>

BOARD *loadBoard( wxString filename )
{
    BOARD* b = new BOARD();

    PLUGIN::RELEASER pi( IO_MGR::PluginFind( IO_MGR::KICAD ) );

    try
    {
        b = pi->Load( filename , NULL, NULL );
        // b=pi -> Load( wxT("../../../tests/conns.kicad_pcb"), NULL, NULL );
    }
    catch( const IO_ERROR& ioe )
    {
        wxString msg = wxString::Format( _( "Error loading board.\n%s" ),
                ioe.errorText.GetData() );

        printf( "%s\n", (const char*) msg.mb_str() );

        delete b;
        return NULL;
    }

    return b;
}


int main()
{
    BOARD *brd = loadBoard ("../../../tests/marblewalrus-switch.kicad_pcb");

    CALLGRIND_START_INSTRUMENTATION;
    brd->GetRatsnest()->Recalculate();
    CALLGRIND_STOP_INSTRUMENTATION;
    CALLGRIND_DUMP_STATS;

    return 0;
}
