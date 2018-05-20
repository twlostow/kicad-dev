/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 1992-2015 KiCad Developers, see AUTHORS.txt for contributors.
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

/**
 * @file pcbnew.cpp
 * @brief Pcbnew main program.
 */

#ifdef KICAD_SCRIPTING
// #include <python_scripting.h>
 //#include <pcbnew_scripting_helpers.h>
#endif
#include <fctsys.h>
#include <pgm_base.h>
#include <kiface_i.h>
#include <confirm.h>
#include <macros.h>
#include <class_drawpanel.h>
#include <eda_dde.h>
#include <wx/stdpaths.h>

#include <wx/file.h>
#include <wx/snglinst.h>
#include <wx/dir.h>
#include <gestfich.h>

#include <hotkeys.h>
#include <wildcards_and_files_ext.h>
#include <kiway_player.h>


#include "lib_edit_frame.h"
#include "sch_edit_frame.h"
#include "sch_test_frame.h"

static struct IFACE : public KIFACE_I
{
    // Of course all are overloads, implementations of the KIFACE.

    IFACE( const char* aName, KIWAY::FACE_T aType ) :
        KIFACE_I( aName, aType )
    {}

    bool OnKifaceStart( PGM_BASE* aProgram, int aCtlBits )
    {
        return true;
    }

    void OnKifaceEnd() {}

    wxWindow* CreateWindow( wxWindow* aParent, int aClassId, KIWAY* aKiway, int aCtlBits = 0 )
    {
            switch( aClassId )
            {
            case FRAME_SCH:
            {
                auto frame = new SCH_TEST_FRAME( aKiway, aParent );
                return frame;
            }
            default:
                printf("unknown frame id %d\n", aClassId);
                return nullptr;
        }

        return nullptr;
    }

    /**
     * Function IfaceOrAddress
     * return a pointer to the requested object.  The safest way to use this
     * is to retrieve a pointer to a static instance of an interface, similar to
     * how the KIFACE interface is exported.  But if you know what you are doing
     * use it to retrieve anything you want.
     *
     * @param aDataId identifies which object you want the address of.
     *
     * @return void* - and must be cast into the know type.
     */
    void* IfaceOrAddress( int aDataId )
    {
        return NULL;
    }
}
kiface( "sch_test_frame", KIWAY::FACE_SCH );

#if 0
static struct PGM_TEST_FRAME : public PGM_BASE
{
    bool OnPgmInit();

    void OnPgmExit()
    {
        Kiway.OnKiwayEnd();

        // Destroy everything in PGM_BASE, especially wxSingleInstanceCheckerImpl
        // earlier than wxApp and earlier than static destruction would.
        PGM_BASE::Destroy();
    }

    void MacOpenFile( const wxString& aFileName )   override
    {
        wxFileName filename( aFileName );

        if( filename.FileExists() )
        {
    #if 0
            // this pulls in EDA_DRAW_FRAME type info, which we don't want in
            // the single_top link image.
            KIWAY_PLAYER* frame = dynamic_cast<KIWAY_PLAYER*>( App().GetTopWindow() );
    #else
            KIWAY_PLAYER* frame = (KIWAY_PLAYER*) App().GetTopWindow();
    #endif

            if( frame )
                frame->OpenProjectFiles( std::vector<wxString>( 1, aFileName ) );
        }
    }
}
program;

/*PGM_BASE& Pgm()
{
    return program;
}
*/


#endif

KIFACE_I& Kiface()
{
    return kiface;
}

static COLOR4D s_layerColor[SCH_LAYER_ID_COUNT];

COLOR4D GetLayerColor( SCH_LAYER_ID aLayer )
{
    unsigned layer = SCH_LAYER_INDEX( aLayer );
    wxASSERT( layer < DIM( s_layerColor ) );
    return s_layerColor[layer];
}

void SetLayerColor( COLOR4D aColor, SCH_LAYER_ID aLayer )
{
    unsigned layer = SCH_LAYER_INDEX( aLayer );
    wxASSERT( layer < DIM( s_layerColor ) );
    s_layerColor[layer] = aColor;
}

// Color to draw selected items
COLOR4D GetItemSelectedColor()
{
    return COLOR4D( BROWN );
}


// Color to draw items flagged invisible, in libedit (they are invisible
// in Eeschema
COLOR4D GetInvisibleItemColor()
{
    return COLOR4D( DARKGRAY );
}

#include "transform.h"

#include "eda_text.h"
#include "general.h"

TRANSFORM DefaultTransform = TRANSFORM( 1, 0, 0, -1 );

/// Default size for text (not only labels)
static int s_defaultTextSize = DEFAULT_SIZE_TEXT;

int GetDefaultTextSize()
{
    return s_defaultTextSize;
}


void SetDefaultTextSize( int aTextSize )
{
    s_defaultTextSize = aTextSize;
}


/*
 * Default line (in Eeschema units) thickness used to draw/plot items having a
 * default thickness line value (i.e. = 0 ).
 */
static int s_drawDefaultLineThickness  = DEFAULTDRAWLINETHICKNESS;


int GetDefaultLineThickness()
{
    return s_drawDefaultLineThickness;
}

int LIB_EDIT_FRAME::           m_textPinNameDefaultSize = DEFAULTPINNAMESIZE;

const wxString traceFindItem = wxT( "KICAD_TRACE_FIND_ITEM" );

/*std::string SCH_ITEM::FormatInternalUnits( int aValue )
{
    char    buf[50];
    double  engUnits = aValue;
    int     len;

    if( engUnits != 0.0 && fabs( engUnits ) <= 0.0001 )
    {
        len = snprintf( buf, sizeof(buf), "%.10f", engUnits );

        while( --len > 0 && buf[len] == '0' )
            buf[len] = '\0';

        ++len;
    }
    else
    {
        len = snprintf( buf, sizeof(buf), "%.10g", engUnits );
    }

    return std::string( buf, len );
}*/

#include "sch_component.h"

/*SCH_FIELD* SCH_COMPONENT::GetField( int aFieldNdx ) const
{
  return nullptr;
}*/

int LIB_EDIT_FRAME::m_defaultPinLength = 10;
int LIB_EDIT_FRAME::m_textPinNumDefaultSize = 10;

#include "template_fieldnames.h"

const wxString TEMPLATE_FIELDNAME::GetDefaultFieldName( int aFieldNdx )
{
 return wxT("dupa");
}

#include "class_library.h"

//int PART_LIBS::s_modify_generation = 1;

class SCH_SHEET;

SCH_SHEET*  g_RootSheet = NULL;

void DrawDanglingSymbol(EDA_DRAW_PANEL*, wxDC*, wxPoint const&, KIGFX::COLOR4D const&){};
void IncrementLabelMember( wxString& name, int aIncrement ){};
int GetDefaultBusThickness() { return 10; }
wxString DRC_ITEM::GetErrorText() const { return wxT(""); }
wxString DRC_ITEM::ShowCoord(wxPoint const&) { return wxT(""); };
void SCH_EDIT_FRAME::SaveUndoItemInUndoList(SCH_ITEM*){}
void SCH_EDIT_FRAME::OnModify(){}

#include <fctsys.h>
#include <kiface_i.h>
#include <pgm_base.h>
#include <gr_basic.h>
#include <class_drawpanel.h>
#include <gestfich.h>
#include <confirm.h>
#include <base_units.h>
#include <msgpanel.h>
#include <html_messagebox.h>
#include <executable_names.h>

#include <general.h>
#include <eeschema_id.h>
#include <netlist.h>
#include <lib_pin.h>
#include <class_library.h>
#include <sch_edit_frame.h>
#include <sch_component.h>
#include <symbol_lib_table.h>

#include <dialog_helpers.h>
#include <reporter.h>
#include <lib_edit_frame.h>
#include <viewlib_frame.h>
#include <hotkeys.h>
#include <eeschema_config.h>
#include <sch_sheet.h>
#include <sch_sheet_path.h>
#include "sim/sim_plot_frame.h"

#include <invoke_sch_dialog.h>

#include <wx/display.h>
#include <build_version.h>
#include <wildcards_and_files_ext.h>

#include <kiway.h>


// non-member so it can be moved easily, and kept REALLY private.
// Do NOT Clear() in here.
static void add_search_paths( SEARCH_STACK* aDst, const SEARCH_STACK& aSrc, int aIndex )
{
    for( unsigned i=0; i<aSrc.GetCount();  ++i )
        aDst->AddPaths( aSrc[i], aIndex );
}


// non-member so it can be moved easily, and kept REALLY private.
// Do NOT Clear() in here.
static void add_search_paths( SEARCH_STACK* aDst, wxConfigBase* aCfg, int aIndex )
{
    for( int i=1;  true;  ++i )
    {
        wxString key   = wxString::Format( wxT( "LibraryPath%d" ), i );
        wxString upath = aCfg->Read( key, wxEmptyString );

        if( !upath )
            break;

        aDst->AddPaths( upath, aIndex );
    }
}

//-----<SCH "data on demand" functions>-------------------------------------------

SEARCH_STACK* PROJECT::SchSearchS()
{
    SEARCH_STACK* ss = (SEARCH_STACK*) GetElem( PROJECT::ELEM_SCH_SEARCH_STACK );

    wxASSERT( !ss || dynamic_cast<SEARCH_STACK*>( GetElem( PROJECT::ELEM_SCH_SEARCH_STACK ) ) );

    if( !ss )
    {
        ss = new SEARCH_STACK();

        // Make PROJECT the new SEARCH_STACK owner.
        SetElem( PROJECT::ELEM_SCH_SEARCH_STACK, ss );

        // to the empty SEARCH_STACK for SchSearchS(), add project dir as first
        ss->AddPaths( m_project_name.GetPath() );

        // next add the paths found in *.pro, variable "LibDir"
        wxString        libDir;

        try
        {
            PART_LIBS::LibNamesAndPaths( this, false, &libDir );
        }
        catch( const IO_ERROR& DBG( ioe ) )
        {
            DBG(printf( "%s: %s\n", __func__, TO_UTF8( ioe.What() ) );)
        }

        if( !!libDir )
        {
            wxArrayString   paths;

            SEARCH_STACK::Split( &paths, libDir );

            for( unsigned i =0; i<paths.GetCount();  ++i )
            {
                wxString path = AbsolutePath( paths[i] );

                ss->AddPaths( path );     // at the end
            }
        }

        // append all paths from aSList
        add_search_paths( ss, Kiface().KifaceSearch(), -1 );

        // addLibrarySearchPaths( SEARCH_STACK* aSP, wxConfigBase* aCfg )
        // This is undocumented, but somebody wanted to store !schematic!
        // library search paths in the .kicad_common file?
        add_search_paths( ss, Pgm().CommonSettings(), -1 );
    }

    return ss;
}


PART_LIBS* PROJECT::SchLibs()
{
    PART_LIBS* libs = (PART_LIBS*)  GetElem( PROJECT::ELEM_SCH_PART_LIBS );

    wxASSERT( !libs || dynamic_cast<PART_LIBS*>( libs ) );

    if( !libs )
    {
        libs = new PART_LIBS();

        // Make PROJECT the new PART_LIBS owner.
        SetElem( PROJECT::ELEM_SCH_PART_LIBS, libs );

        try
        {
            libs->LoadAllLibraries( this );
        }
        catch( const PARSE_ERROR& pe )
        {
            wxString    lib_list = UTF8( pe.inputLine );
            wxWindow*   parent = 0; // Pgm().App().GetTopWindow();

            // parent of this dialog cannot be NULL since that breaks the Kiway() chain.
            HTML_MESSAGE_BOX dlg( parent, _( "Not Found" ) );

            dlg.MessageSet( _( "The following libraries were not found:" ) );

            dlg.ListSet( lib_list );

            dlg.Layout();

            dlg.ShowModal();
        }
        catch( const IO_ERROR& ioe )
        {
            DisplayError( NULL, ioe.What() );
        }
    }

    return libs;
}

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

        wxString prjPath;

        wxGetEnv( PROJECT_VAR_NAME, &prjPath );

        wxASSERT( !prjPath.empty() );

        wxFileName fn( prjPath, SYMBOL_LIB_TABLE::GetSymbolLibTableFileName() );

        try
        {
            tbl->Load( fn.GetFullPath() );
        }
        catch( const IO_ERROR& ioe )
        {
            wxString msg;
            msg.Printf( _( "An error occurred loading the symbol library table \"%s\"." ),
                        fn.GetFullPath() );
            DisplayErrorMessage( NULL, msg, ioe.What() );
        }
    }

    return tbl;
}

int SCH_EDIT_FRAME::BlockCommand( EDA_KEY aKey ){}
void SCH_EDIT_FRAME::HandleBlockPlace( wxDC* DC ){}
bool SCH_EDIT_FRAME::HandleBlockEnd( wxDC* aDC ){}

void SCH_EDIT_FRAME::PrintPage( wxDC* aDC, LSET aPrintMask,
                        bool aPrintMirrorMode, void* aData ){}
bool SCH_EDIT_FRAME::doAutoSave(){};

bool SCH_EDIT_FRAME::isAutoSaveRequired()const {};
wxString SCH_EDIT_FRAME::GetScreenDesc() const {};
void SCH_EDIT_FRAME::ExecuteRemoteCommand( const char* cmdline ){}

SCH_EDIT_FRAME::SCH_EDIT_FRAME( KIWAY* aKiway, wxWindow* aParent ):
    SCH_BASE_FRAME( aKiway, aParent, FRAME_SCH, wxT( "Eeschema" ),
        wxDefaultPosition, wxDefaultSize, KICAD_DEFAULT_DRAWFRAME_STYLE, SCH_EDIT_FRAME_NAME )

{}

SCH_EDIT_FRAME::~SCH_EDIT_FRAME() {}
void SCH_EDIT_FRAME::LoadSettings(wxConfigBase*) {}
void SCH_EDIT_FRAME::SaveSettings(wxConfigBase*) {}
void SCH_EDIT_FRAME::SaveProjectSettings(bool) {}
void SCH_EDIT_FRAME::ReCreateMenuBar() {}
int SCH_EDIT_FRAME::GetIconScale() {}
void SCH_EDIT_FRAME::SetIconScale(int){}
bool SCH_EDIT_FRAME::OpenProjectFiles(std::vector<wxString, std::allocator<wxString> > const&, int){}
void SCH_EDIT_FRAME::KiwayMailIn(KIWAY_EXPRESS&){}
void SCH_BASE_FRAME::SetPageSettings(PAGE_INFO const&){}
const PAGE_INFO& SCH_BASE_FRAME::GetPageSettings() const{}
const wxSize SCH_BASE_FRAME::GetPageSizeIU() const{}
const wxPoint& SCH_BASE_FRAME::GetAuxOrigin() const{}
const TITLE_BLOCK&  SCH_BASE_FRAME::GetTitleBlock() const{}
void SCH_BASE_FRAME::SetAuxOrigin(wxPoint const&){}
void SCH_BASE_FRAME::SetTitleBlock(TITLE_BLOCK const&){}
KIGFX::COLOR4D SCH_BASE_FRAME::GetDrawBgColor() const{}
void SCH_BASE_FRAME::SetDrawBgColor(KIGFX::COLOR4D){}
SCH_SCREEN* SCH_EDIT_FRAME::GetScreen() const{}
EDA_HOTKEY* SCH_EDIT_FRAME::GetHotKeyDescription(int) const{}
bool SCH_EDIT_FRAME::OnHotKey(wxDC*, int, wxPoint const&, EDA_ITEM*){}
const wxString SCH_BASE_FRAME::GetZoomLevelIndicator() const{}
void SCH_EDIT_FRAME::ReCreateHToolbar(){}
void SCH_EDIT_FRAME::ReCreateVToolbar(){}
bool SCH_EDIT_FRAME::GeneralControl(wxDC*, wxPoint const&, unsigned int){}
double SCH_EDIT_FRAME::BestZoom(){}
void SCH_EDIT_FRAME::RedrawActiveWindow(wxDC*, bool){}
void SCH_EDIT_FRAME::OnLeftClick(wxDC*, wxPoint const&){}
void SCH_EDIT_FRAME::OnLeftDClick(wxDC*, wxPoint const&){}
bool SCH_EDIT_FRAME::OnRightClick(wxPoint const&, wxMenu*){}
void SCH_BASE_FRAME::UpdateStatusBar(){}
void SCH_EDIT_FRAME::InitBlockPasteInfos(){}
void SCH_BASE_FRAME::OnEditSymbolLibTable(wxCommandEvent&){}

BEGIN_EVENT_TABLE( SCH_EDIT_FRAME, EDA_DRAW_FRAME )
END_EVENT_TABLE()

SCH_BASE_FRAME::SCH_BASE_FRAME( KIWAY* aKiway, wxWindow* aParent,
        FRAME_T aWindowType, const wxString& aTitle,
        const wxPoint& aPosition, const wxSize& aSize, long aStyle,
        const wxString& aFrameName ) :
    EDA_DRAW_FRAME( aKiway, aParent, aWindowType, aTitle, aPosition,
            aSize, aStyle, aFrameName )
{
}


SCH_BASE_FRAME::~SCH_BASE_FRAME()
{
}

SCH_SCREEN* SCH_BASE_FRAME::GetScreen() const
{
    return nullptr;
}


static PGM_BASE* process;


MY_API( KIFACE* ) KIFACE_GETTER(  int* aKIFACEversion, int aKiwayVersion, PGM_BASE* aProgram )
{
    process = aProgram;
    return &kiface;
}
