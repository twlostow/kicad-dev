/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013-2017 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <wx/timer.h>
#include <wx/math.h>
#include <wx/log.h>
#include <wx/popupwin.h>
#include <wx/cmdline.h>

#include <layers_id_colors_and_visibility.h>

#include <gal/graphics_abstraction_layer.h>
#include <view/view.h>
#include <class_draw_panel_gal.h>
#include <view/wx_view_controls.h>
#include "sch_painter.h"

#include <class_libentry.h>
#include <sch_io_mgr.h>
#include <sch_screen.h>
#include <sch_component.h>

#include <tool/actions.h>
#include <tool/tool_manager.h>
#include <tool/tool_dispatcher.h>
#include "sch_legacy_plugin.h"
#include "sch_draw_panel_gal.h"
#include "sch_test_frame.h"
#include <sch_sheet.h>

#include <kiway.h>
#include <kiface_i.h>
#include <project.h>
#include <kiway_player.h>
#include <symbol_lib_table.h>
#include <class_library.h>

using namespace KIGFX;


#if 0
bool GAL_TEST_APP::OnInit()
{
    if( !wxApp::OnInit() )
        return false;

    // Create the main frame window
    auto frame = CreateMainFrame( (const char*) m_filename.c_str() );

    return frame != nullptr;
}


void GAL_TEST_APP::OnInitCmdLine( wxCmdLineParser& parser )
{
    parser.AddOption( "f", wxEmptyString, "Open schematic file" );
    wxApp::OnInitCmdLine( parser );
}


bool GAL_TEST_APP::OnCmdLineParsed( wxCmdLineParser& parser )
{
    wxString filename;

    if( parser.Found( "f", &filename ) )
    {
        m_filename = filename;
    }

    return true;
}
#endif



class TEST_ACTIONS : public ACTIONS
{
public:

    virtual ~TEST_ACTIONS() {};

    virtual OPT<TOOL_EVENT> TranslateLegacyId( int aId )
    {
        return NULLOPT;
    }

    void RegisterAllTools( TOOL_MANAGER* aToolManager )
    {
    }
};

void SCH_TEST_FRAME::OnMenuFileOpen( wxCommandEvent& WXUNUSED( event ) )
{
}


void SCH_TEST_FRAME::OnMotion( wxMouseEvent& aEvent )
{
    auto vc = m_galPanel->GetViewControls();
    auto pos = vc->GetCursorPosition( );
    //printf("x %d y %d\n", (int)pos.x, (int)pos.y);
    aEvent.Skip();
}


void SCH_TEST_FRAME::SetLibraryComponent( LIB_PART *part )
{
    m_part = part;
    m_galPanel->DisplayComponent( m_part );

    m_toolManager->SetEnvironment( m_part, m_galPanel->GetView(),
            m_galPanel->GetViewControls(), nullptr );

    m_toolManager->ResetTools( TOOL_BASE::MODEL_RELOAD );
}

void SCH_TEST_FRAME::SetSchematic( SCH_SHEET *sheet )
{
    m_sheet = sheet;
    m_galPanel->DisplaySheet( m_sheet );

    m_toolManager->SetEnvironment( m_sheet, m_galPanel->GetView(),
            m_galPanel->GetViewControls(), nullptr );

    m_toolManager->ResetTools( TOOL_BASE::MODEL_RELOAD );
}



LIB_PART* SCH_TEST_FRAME::LoadAndDisplayPart( const std::string& filename )
{
    LIB_ALIAS *sym;
    auto pi = SCH_IO_MGR::FindPlugin( SCH_IO_MGR::SCH_LEGACY );

    try
    {
        sym = pi->LoadSymbol( filename.c_str(), "PIC16(L)F1454-I_SS", NULL );
    }
    catch( const IO_ERROR& ioe )
    {
        wxString msg = wxString::Format( _( "Error loading symbol.\n%s" ),
                ioe.Problem() );

        printf( "%s\n", (const char*) msg.mb_str() );
        return nullptr;
    }

//    printf("sym %p\n", sym);

    LIB_PART *part = sym->GetPart();

    SetLibraryComponent( part );

    auto bb = m_galPanel->GetView()->CalculateExtents() ;

    printf("Extents: %d %d %d %d\n", bb.GetOrigin().x, bb.GetOrigin().y, bb.GetSize().x, bb.GetSize().y );

    BOX2D bb2 ( bb.GetOrigin(), bb.GetSize() );

    m_galPanel->GetView()->SetViewport( bb2 );

    return part;
}

SCH_SHEET* SCH_TEST_FRAME::LoadAndDisplaySchematic( const std::string& filename )
{

    auto pi = SCH_IO_MGR::FindPlugin( SCH_IO_MGR::SCH_LEGACY );

    SCH_SHEET *sheet;

    try
    {
        sheet = pi->Load( filename.c_str(), &Kiway() );
    }
    catch( const IO_ERROR& ioe )
    {
        wxString msg = wxString::Format( _( "Error loading schematic.\n%s" ),
                ioe.Problem() );

        printf( "%s\n", (const char*) msg.mb_str() );
        return nullptr;
    }


    Prj().SetProjectFullName("/home/twl/Kicad-dev/kicad-build/debug-dev/qa/sch_lib_test_window/dsi_shield.pro");
    Prj().SchLibs()->LoadAllLibraries( &Prj(), true );

    //Prj().SchSymbolLibTable()->LoadSymbol( aLibId );

    auto cacheLib = Prj().SchLibs()->GetCacheLibrary();

    //printf("cache library @ %p\n", cacheLib );


    auto item = sheet->GetScreen()->GetDrawItems();
    for ( ; item; item=item->Next() )
    {
        if( item->Type() == SCH_COMPONENT_T )
        {
            auto cmp = static_cast<SCH_COMPONENT*>( item );
    //        printf("process comp %p %s\n", item, (const char *) cmp->GetLibId().Format().c_str() );

            cmp->Resolve( *Prj().SchSymbolLibTable(), cacheLib );

            //cmp->SetLibId( cmp->GetLibId(), Prj().SchSymbolLibTable(), nullptr );


        }
    }

    SetSchematic( sheet );

    auto bb = m_galPanel->GetView()->CalculateExtents() ;

    printf("Extents: %d %d %d %d\n", bb.GetOrigin().x, bb.GetOrigin().y, bb.GetSize().x, bb.GetSize().y );

    BOX2D bb2 ( bb.GetOrigin(), bb.GetSize() );

    m_galPanel->GetView()->SetViewport( bb2 );

    return sheet;
}


SCH_TEST_FRAME::SCH_TEST_FRAME( KIWAY* aKiway, wxWindow* aParent )
        : KIWAY_PLAYER( aParent, wxID_ANY, wxT("Sch test frame"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE )
{
    SetKiway( this, aKiway );

    // Make a menubar
    wxMenu* fileMenu = new wxMenu;

    fileMenu->Append( wxID_OPEN, wxT( "&Open..." ) );
    fileMenu->AppendSeparator();
    fileMenu->Append( wxID_EXIT, wxT( "E&xit" ) );
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append( fileMenu, wxT( "&File" ) );
    SetMenuBar( menuBar );

    Show( true );
    Maximize();
    Raise();

    KIGFX::GAL_DISPLAY_OPTIONS options;

    m_galPanel.reset( new SCH_DRAW_PANEL_GAL( this, -1, wxPoint( 0,
                            0 ), wxDefaultSize, options, EDA_DRAW_PANEL_GAL::GAL_TYPE_OPENGL ) );

    m_galPanel->SetEvtHandlerEnabled( true );
    m_galPanel->SetFocus();
    m_galPanel->Show( true );
    m_galPanel->Raise();
    m_galPanel->StartDrawing();

    auto gal = m_galPanel->GetGAL();

    gal->SetGridVisibility( true );
    gal->SetGridSize( VECTOR2D( 1.0, 1.0 ) );
    gal->SetGridOrigin( VECTOR2D( 0.0, 0.0 ) );

    m_galPanel->Connect( wxEVT_MOTION,
            wxMouseEventHandler( SCH_TEST_FRAME::OnMotion ), NULL, this );

    m_galPanel->GetViewControls()->ShowCursor( true );

    m_toolManager.reset( new TOOL_MANAGER );
    m_toolManager->SetEnvironment( m_part, m_galPanel->GetView(),
            m_galPanel->GetViewControls(), nullptr );

    m_pcbActions.reset( new TEST_ACTIONS() );
    m_toolDispatcher.reset( new TOOL_DISPATCHER( m_toolManager.get(), m_pcbActions.get() ) );

    //m_toolManager->RegisterTool( new SELECTION_TOOL );

    m_toolManager->InitTools();
    m_galPanel->SetEventDispatcher( m_toolDispatcher.get() );
    m_toolManager->InvokeTool( "eeschema.InteractiveSelection" );

    //LoadLibrary("dsi_shield-cache.lib");
    LoadAndDisplaySchematic("/home/twl/Kicad-dev/kicad-build/debug-dev/qa/sch_lib_test_window/dsi_shield.sch");

}


SCH_TEST_FRAME::~SCH_TEST_FRAME()
{
}


// Intercept menu commands
void SCH_TEST_FRAME::OnExit( wxCommandEvent& WXUNUSED( event ) )
{
    // true is to force the frame to close
    Close( true );
}

//IMPLEMENT_APP( GAL_TEST_APP );

SYMBOL_LIB_TABLE* PROJECT::SchSymbolLibTable()
{
    // This is a lazy loading function, it loads the project specific table when
    // that table is asked for, not before.
    SYMBOL_LIB_TABLE* tbl = (SYMBOL_LIB_TABLE*) GetElem( ELEM_SYMBOL_LIB_TABLE );

    // its gotta be NULL or a SYMBOL_LIB_TABLE, or a bug.
    wxASSERT( !tbl || dynamic_cast<SYMBOL_LIB_TABLE*>( tbl ) );

    if( !tbl )
    {
        // Stack the project specific SYMBOL_LIB_TABLE overlay on top of the global table.
        // ~SYMBOL_LIB_TABLE() will not touch the fallback table, so multiple projects may
        // stack this way, all using the same global fallback table.
        tbl = new SYMBOL_LIB_TABLE( &SYMBOL_LIB_TABLE::GetGlobalLibTable() );

        SetElem( ELEM_SYMBOL_LIB_TABLE, tbl );

        wxString prjPath = wxGetCwd();

        wxASSERT( !prjPath.empty() );

        wxFileName fn( prjPath, SYMBOL_LIB_TABLE::GetSymbolLibTableFileName() );

        try
        {
            printf("Loading '%s'",(const char*) fn.GetFullPath().c_str());
            tbl->Load( fn.GetFullPath() );
        }
        catch( const IO_ERROR& ioe )
        {
            wxString msg;
            msg.Printf( _( "An error occurred loading the symbol library table \"%s\"." ),
                        fn.GetFullPath() );
            printf("ERR %s\n", (const char *) msg.c_str() );
            //DisplayErrorMessage( NULL, msg, ioe.What() );
        }
    }

    return tbl;
}
