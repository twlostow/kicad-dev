/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2008-2013 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 2004-2016 KiCad Developers, see AUTHORS.txt for contributors.
 * Copyright (C) 2016 CERN
 * @author Michele Castellana <michele.castellana@cern.ch>
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
 * @file libeditframe.cpp
 * @brief LIB_EDIT_FRAME class is the component library editor frame.
 */

#include <map>

#include <fctsys.h>
#include <pgm_base.h>
#include <kiface_i.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <eda_doc.h>
#include <gr_basic.h>
#include <schframe.h>
#include <msgpanel.h>
#include <wx/tokenzr.h>

#include <general.h>
#include <eeschema_id.h>
#include <libeditframe.h>
#include <class_library.h>
#include <lib_polyline.h>
#include <lib_pin.h>

#include <kicad_device_context.h>
#include <hotkeys.h>

#include <dialogs/dialog_lib_edit_text.h>
#include <dialogs/dialog_edit_component_in_lib.h>
#include <dialogs/dialog_lib_edit_pin_table.h>
#include <dialogs/dialog_lib_new_component.h>

#include <dialog_eeschema_tree.h>
#include <menus_helpers.h>
#include <wildcards_and_files_ext.h>


/* This method guarantees unique IDs for the library this run of Eeschema
 * which prevents ID conflicts and eliminates the need to recompile every
 * source file in the project when adding IDs to include/id.h. */
int ExportPartId = ::wxNewId();
int ImportPartId = ::wxNewId();
int CreateNewLibAndSavePartId = ::wxNewId();


wxString LIB_EDIT_FRAME::      m_aliasName;
int LIB_EDIT_FRAME::           m_unit    = 1;
int LIB_EDIT_FRAME::           m_convert = 1;
LIB_ITEM* LIB_EDIT_FRAME::m_lastDrawItem = NULL;
LIB_ITEM* LIB_EDIT_FRAME::m_drawItem = NULL;
bool LIB_EDIT_FRAME::          m_showDeMorgan    = false;
wxSize LIB_EDIT_FRAME::        m_clientSize      = wxSize( -1, -1 );
int LIB_EDIT_FRAME::           m_textSize        = -1;
int LIB_EDIT_FRAME::           m_textOrientation = TEXT_ORIENT_HORIZ;
int LIB_EDIT_FRAME::           m_drawLineWidth   = 0;

// these values are overridden when reading the config
int LIB_EDIT_FRAME::           m_textPinNumDefaultSize = DEFAULTPINNUMSIZE;
int LIB_EDIT_FRAME::           m_textPinNameDefaultSize = DEFAULTPINNAMESIZE;
int LIB_EDIT_FRAME::           m_defaultPinLength = DEFAULTPINLENGTH;

FILL_T LIB_EDIT_FRAME::        m_drawFillStyle   = NO_FILL;


BEGIN_EVENT_TABLE( LIB_EDIT_FRAME, EDA_DRAW_FRAME )
    EVT_CLOSE( LIB_EDIT_FRAME::OnCloseWindow )
    EVT_SIZE( LIB_EDIT_FRAME::OnSize )
    EVT_ACTIVATE( LIB_EDIT_FRAME::OnActivate )

    // Main horizontal toolbar.
    EVT_TOOL( ID_LIBEDIT_SAVE_ALL_LIBS, LIB_EDIT_FRAME::SaveAllLibraries )
    EVT_TOOL( ID_LIBEDIT_SAVE_CURRENT_LIB, LIB_EDIT_FRAME::SaveSelectedLibrary )
    EVT_TOOL( ID_LIBEDIT_DELETE_PART, LIB_EDIT_FRAME::DeleteOnePart )
    EVT_TOOL( ID_TO_LIBVIEW, LIB_EDIT_FRAME::OnOpenLibraryViewer )
    EVT_TOOL( ID_LIBEDIT_NEW_PART, LIB_EDIT_FRAME::CreateNewLibraryPart )
    EVT_TOOL( ID_LIBEDIT_NEW_PART_FROM_EXISTING, LIB_EDIT_FRAME::OnCreateNewPartFromExisting )

    EVT_TOOL( ID_LIBEDIT_SAVE_CURRENT_PART, LIB_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( wxID_UNDO, LIB_EDIT_FRAME::GetComponentFromUndoList )
    EVT_TOOL( wxID_REDO, LIB_EDIT_FRAME::GetComponentFromRedoList )
    EVT_TOOL( ID_LIBEDIT_GET_FRAME_EDIT_PART, LIB_EDIT_FRAME::OnEditComponentProperties )
    EVT_TOOL( ID_LIBEDIT_GET_FRAME_EDIT_FIELDS, LIB_EDIT_FRAME::InstallFieldsEditorDialog )
    EVT_TOOL( ID_LIBEDIT_CHECK_PART, LIB_EDIT_FRAME::OnCheckComponent )
    EVT_TOOL( ID_DE_MORGAN_NORMAL_BUTT, LIB_EDIT_FRAME::OnSelectBodyStyle )
    EVT_TOOL( ID_DE_MORGAN_CONVERT_BUTT, LIB_EDIT_FRAME::OnSelectBodyStyle )
    EVT_TOOL( ID_LIBEDIT_VIEW_DOC, LIB_EDIT_FRAME::OnViewEntryDoc )
    EVT_TOOL( ID_LIBEDIT_EDIT_PIN_BY_PIN, LIB_EDIT_FRAME::Process_Special_Functions )
    EVT_TOOL( ID_LIBEDIT_EDIT_PIN_BY_TABLE, LIB_EDIT_FRAME::OnOpenPinTable )
    EVT_TOOL( ExportPartId, LIB_EDIT_FRAME::OnExportPart )
    EVT_TOOL( CreateNewLibAndSavePartId, LIB_EDIT_FRAME::OnExportPart )
    EVT_TOOL( ImportPartId, LIB_EDIT_FRAME::OnImportPart )

    EVT_COMBOBOX( ID_LIBEDIT_SELECT_PART_NUMBER, LIB_EDIT_FRAME::OnSelectPart )
    EVT_COMBOBOX( ID_LIBEDIT_SELECT_ALIAS, LIB_EDIT_FRAME::OnSelectAlias )

    // Right vertical toolbar.
    EVT_TOOL( ID_NO_TOOL_SELECTED, LIB_EDIT_FRAME::OnSelectTool )
    EVT_TOOL( ID_ZOOM_SELECTION, LIB_EDIT_FRAME::OnSelectTool )
    EVT_TOOL_RANGE( ID_LIBEDIT_PIN_BUTT, ID_LIBEDIT_DELETE_ITEM_BUTT,
                    LIB_EDIT_FRAME::OnSelectTool )

    // menubar commands
    EVT_MENU( wxID_EXIT, LIB_EDIT_FRAME::CloseWindow )
    EVT_MENU( ID_LIBEDIT_GEN_PNG_FILE, LIB_EDIT_FRAME::OnPlotCurrentComponent )
    EVT_MENU( ID_LIBEDIT_GEN_SVG_FILE, LIB_EDIT_FRAME::OnPlotCurrentComponent )
    EVT_MENU( wxID_HELP, EDA_DRAW_FRAME::GetKicadHelp )
    EVT_MENU( wxID_INDEX, EDA_DRAW_FRAME::GetKicadHelp )
    EVT_MENU( wxID_ABOUT, EDA_BASE_FRAME::GetKicadAbout )

    EVT_MENU( wxID_PREFERENCES, LIB_EDIT_FRAME::OnPreferencesOptions )
    EVT_MENU( ID_CONFIG_REQ, LIB_EDIT_FRAME::InstallConfigFrame )

    // Multiple item selection context menu commands.
    EVT_MENU_RANGE( ID_SELECT_ITEM_START, ID_SELECT_ITEM_END, LIB_EDIT_FRAME::OnSelectItem )

    EVT_MENU_RANGE( ID_PREFERENCES_HOTKEY_START, ID_PREFERENCES_HOTKEY_END,
                    LIB_EDIT_FRAME::Process_Config )

    // Context menu events and commands.
    EVT_MENU( ID_LIBEDIT_EDIT_PIN, LIB_EDIT_FRAME::OnEditPin )
    EVT_MENU( ID_LIBEDIT_ROTATE_ITEM, LIB_EDIT_FRAME::OnRotateItem )

    EVT_MENU_RANGE( ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_ITEM,
                    ID_POPUP_LIBEDIT_DELETE_CURRENT_POLY_SEGMENT,
                    LIB_EDIT_FRAME::Process_Special_Functions )

    EVT_MENU_RANGE( ID_POPUP_GENERAL_START_RANGE, ID_POPUP_GENERAL_END_RANGE,
                    LIB_EDIT_FRAME::Process_Special_Functions )

    EVT_MENU_RANGE( ID_LIBEDIT_MIRROR_X, ID_LIBEDIT_ORIENT_NORMAL,
                    LIB_EDIT_FRAME::OnOrient )
    // Update user interface elements.
    EVT_UPDATE_UI( ExportPartId, LIB_EDIT_FRAME::OnUpdateEditingPart )
    EVT_UPDATE_UI( CreateNewLibAndSavePartId, LIB_EDIT_FRAME::OnUpdateEditingPart )
    EVT_UPDATE_UI( ID_LIBEDIT_SAVE_CURRENT_PART, LIB_EDIT_FRAME::OnUpdateEditingPart )
    EVT_UPDATE_UI( ID_LIBEDIT_GET_FRAME_EDIT_FIELDS, LIB_EDIT_FRAME::OnUpdateEditingPart )
    EVT_UPDATE_UI( ID_LIBEDIT_CHECK_PART, LIB_EDIT_FRAME::OnUpdateEditingPart )
    EVT_UPDATE_UI( ID_LIBEDIT_GET_FRAME_EDIT_PART, LIB_EDIT_FRAME::OnUpdateEditingPart )
    EVT_UPDATE_UI( ID_LIBEDIT_NEW_PART_FROM_EXISTING, LIB_EDIT_FRAME::OnUpdateEditingPart )
    EVT_UPDATE_UI( wxID_UNDO, LIB_EDIT_FRAME::OnUpdateUndo )
    EVT_UPDATE_UI( wxID_REDO, LIB_EDIT_FRAME::OnUpdateRedo )
    EVT_UPDATE_UI( ID_LIBEDIT_SAVE_CURRENT_LIB, LIB_EDIT_FRAME::OnSaveSelectedLib )
    EVT_UPDATE_UI( ID_LIBEDIT_SAVE_ALL_LIBS, LIB_EDIT_FRAME::OnSaveAllLibraries )
    EVT_UPDATE_UI( ID_LIBEDIT_VIEW_DOC, LIB_EDIT_FRAME::OnUpdateViewDoc )
    EVT_UPDATE_UI( ID_LIBEDIT_EDIT_PIN_BY_PIN, LIB_EDIT_FRAME::OnUpdatePinByPin )
    EVT_UPDATE_UI( ID_LIBEDIT_EDIT_PIN_BY_TABLE, LIB_EDIT_FRAME::OnUpdatePinTable )
    EVT_UPDATE_UI( ID_LIBEDIT_SELECT_PART_NUMBER, LIB_EDIT_FRAME::OnUpdatePartNumber )
    EVT_UPDATE_UI( ID_LIBEDIT_SELECT_ALIAS, LIB_EDIT_FRAME::OnUpdateSelectAlias )
    EVT_UPDATE_UI( ID_DE_MORGAN_NORMAL_BUTT, LIB_EDIT_FRAME::OnUpdateDeMorganNormal )
    EVT_UPDATE_UI( ID_DE_MORGAN_CONVERT_BUTT, LIB_EDIT_FRAME::OnUpdateDeMorganConvert )
    EVT_UPDATE_UI( ID_NO_TOOL_SELECTED, LIB_EDIT_FRAME::OnUpdateEditingPart )
    EVT_UPDATE_UI( ID_ZOOM_SELECTION, LIB_EDIT_FRAME::OnUpdateEditingPart )
    EVT_UPDATE_UI_RANGE( ID_LIBEDIT_PIN_BUTT, ID_LIBEDIT_DELETE_ITEM_BUTT,
                         LIB_EDIT_FRAME::OnUpdateEditingPart )

    EVT_TREE_ITEM_ACTIVATED( wxID_SEARCHABLE_TREE, LIB_EDIT_FRAME::LoadOneLibraryPart )
    EVT_TREE_END_LABEL_EDIT( wxID_SEARCHABLE_TREE, LIB_EDIT_FRAME::Rename )
    EVT_MENU( ID_LIBEDIT_NEW_LIBRARY, LIB_EDIT_FRAME::EditLibs )
    EVT_MENU( ID_LIBEDIT_NEW_COMPONENT, LIB_EDIT_FRAME::EditLibs )
    EVT_MENU( ID_LIBEDIT_CUT, LIB_EDIT_FRAME::EditLibs )
    EVT_MENU( ID_LIBEDIT_PASTE, LIB_EDIT_FRAME::EditLibs )
END_EVENT_TABLE()

class LIB_MANAGER {
public:

    struct LIBRARY {
        PART_LIB *m_lib;
        PART_LIB *m_copy;
        bool m_isExternalFile;
        wxFileName m_fileName;
    };

    struct PART {
        LIB_PART *m_part;
        LIB_PART *m_copy;
        SCH_SCREEN *m_screen;
        PART_LIB *m_lib;
    };

    std::vector<LIBRARY> m_libs;
    std::vector<PART> m_parts;
    std::map<LIB_PART*, SCH_SCREEN* > m_screenMap;

    PART *m_currentPart;

    LIB_EDIT_FRAME *m_frame;

    LIB_MANAGER( LIB_EDIT_FRAME *aFrame ) :
        m_frame ( aFrame )

    {
        m_currentPart = nullptr;
    }

    void LoadLibs( PART_LIBS *aLibs )
    {
        for( const auto it : aLibs->GetLibraryNames() )
        {
            LIBRARY lib;
            lib.m_lib = aLibs->FindLibrary( it );
            if( !lib.m_lib )
                continue;

            lib.m_copy = nullptr;
            lib.m_isExternalFile = false;

            m_libs.push_back( lib );

            wxArrayString names;
            lib.m_lib->GetEntryNames( names );

            for ( auto name : names )
            {
                PART part;
                part.m_part = lib.m_lib->FindPart( name );
                part.m_copy = nullptr;
                part.m_screen = nullptr;
                part.m_lib = lib.m_lib;


                m_parts.push_back(part);

                printf("add part %p\n", part);

            }

        }
    }

    void ModifyPart ( LIB_PART *aPart )
    {

    }

    void OpenExternalLib ( const wxFileName& aName )
    {

    }

    const std::vector<LIBRARY*> GetModifiedLibs ( )
    {

    }

    void CutCopyPaste ( LIB_PART *aPart, PART_LIB* aTarget, bool aCut = false )
    {

    }

    void EditPart( LIB_PART *aPart )
    {
        for ( PART& part : m_parts)
            if(part.m_part == aPart)
            {
                if(!part.m_copy)
                {
                    part.m_copy = new LIB_PART ( *aPart );
                    part.m_screen = new SCH_SCREEN ( &m_frame->Kiway() );

                    m_currentPart = &part;
                }
                m_frame->SetScreen(part.m_screen);

            }

    }

    void NewPart ( LIB_PART *aPart, PART_LIB *aLibrary )
    {

    }

    void DeletePart ( LIB_PART *aPart )
    {

    }
};

LIB_EDIT_FRAME::LIB_EDIT_FRAME( KIWAY* aKiway, wxWindow* aParent ) :
    SCH_BASE_FRAME( aKiway, aParent, FRAME_SCH_LIB_EDITOR, _( "Library Editor" ),
        wxDefaultPosition, wxDefaultSize, KICAD_DEFAULT_DRAWFRAME_STYLE, wxT("LibeditFrame") )
{
    m_showAxis   = true;            // true to draw axis
    SetShowDeMorgan( false );
    m_drawSpecificConvert = true;
    m_drawSpecificUnit    = false;
    m_hotkeysDescrList    = g_Libedit_Hokeys_Descr;
    m_editPinsPerPartOrConvert = false;
    m_repeatPinStep = DEFAULT_REPEAT_OFFSET_PIN;

    m_my_part = NULL;
    m_tempCopyComponent = NULL;
    m_is_new = false;
    m_libMgr.reset ( new LIB_MANAGER ( this ) );
    m_libMgr->LoadLibs( Prj().SchLibs() );

    // Delayed initialization
    if( m_textSize == -1 )
        m_textSize = GetDefaultTextSize();

    // Initialize grid id to the default value 50 mils:
    m_LastGridSizeId = ID_POPUP_GRID_LEVEL_50 - ID_POPUP_GRID_LEVEL_1000;

    wxIcon icon;
    icon.CopyFromBitmap( KiBitmap( libedit_icon_xpm ) );
    SetIcon( icon );

    LoadSettings( config() );


    m_dummyScreen = new SCH_SCREEN ( aKiway );

    SetScreen( m_dummyScreen );
    GetScreen()->m_Center = true;
    GetScreen()->SetMaxUndoItems( m_UndoRedoCountMax );

    SetCrossHairPosition( wxPoint( 0, 0 ) );

    // Ensure m_LastGridSizeId is an offset inside the allowed schematic range
    if( m_LastGridSizeId < ID_POPUP_GRID_LEVEL_50 - ID_POPUP_GRID_LEVEL_1000 )
        m_LastGridSizeId = ID_POPUP_GRID_LEVEL_50 - ID_POPUP_GRID_LEVEL_1000;

    if( m_LastGridSizeId > ID_POPUP_GRID_LEVEL_1 - ID_POPUP_GRID_LEVEL_1000 )
        m_LastGridSizeId = ID_POPUP_GRID_LEVEL_1 - ID_POPUP_GRID_LEVEL_1000;

    SetSize( m_FramePos.x, m_FramePos.y, m_FrameSize.x, m_FrameSize.y );

    GetScreen()->SetGrid( ID_POPUP_GRID_LEVEL_1000 + m_LastGridSizeId  );

    if( m_canvas )
        m_canvas->SetEnableBlockCommands( true );

    m_panelTree = new EESCHEMA_TREE( this );

    PopulateTree();
    ReCreateMenuBar();
    ReCreateHToolbar();
    ReCreateVToolbar();

    // Ensure the current alias name is valid if a part is loaded
    // Sometimes it is not valid. This is the case
    // when a part value (the part lib name), or the alias list was modified
    // during a previous session and the modifications not saved in lib.
    // Reopen libedit in a new session gives a non valid m_aliasName
    // because the curr part is reloaded from the library (and this is the unmodified part)
    // and the old alias name (from the previous edition) can be invalid
    LIB_PART* part = GetCurPart();

    if( part == NULL )
        m_aliasName.Empty();
    else if( m_aliasName != part->GetName() )
    {
        LIB_ALIAS* alias = part->GetAlias( m_aliasName );

        if( !alias )
            m_aliasName = part->GetName();
    }


    CreateOptionToolbar();
    DisplayLibInfos();
    DisplayCmpDoc();
    UpdateAliasSelectList();
    UpdatePartSelectList();

    m_auimgr.SetManagedWindow( this );

    EDA_PANEINFO horiz;
    horiz.HorizontalToolbarPane();

    EDA_PANEINFO vert;
    vert.VerticalToolbarPane();

    EDA_PANEINFO mesg;
    mesg.MessageToolbarPane();

    EDA_PANEINFO dock;
    dock.DockableFramePane();

    m_auimgr.AddPane( m_mainToolBar,
                      wxAuiPaneInfo( horiz ).Name( wxT( "m_mainToolBar" ) ).Top().Row( 0 ) );

    m_auimgr.AddPane( m_drawToolBar,
                      wxAuiPaneInfo( vert ).Name( wxT( "m_VToolBar" ) ).Right() );

    m_auimgr.AddPane( m_optionsToolBar,
                      wxAuiPaneInfo( vert ).Name( wxT( "m_optionsToolBar" ) ).Right() );

    m_auimgr.AddPane( m_panelTree,
                      wxAuiPaneInfo( dock ).Name( wxT( "m_panelTree" ) ).Left() );

    m_auimgr.AddPane( m_canvas,
                      wxAuiPaneInfo().Name( wxT( "DrawFrame" ) ).CentrePane() );

    m_auimgr.AddPane( m_messagePanel,
                      wxAuiPaneInfo( mesg ).Name( wxT( "MsgPanel" ) ).Bottom().Layer(10) );

    m_auimgr.Update();

    Raise();
    Show( true );

    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_ZOOM_PAGE );
    wxPostEvent( this, evt );
}

void LIB_EDIT_FRAME::Rename( wxTreeEvent& aEvent )
{
#if 0
    if( !aEvent.IsAllowed() )
        return;
    wxString new_name = aEvent.GetLabel();
    wxString old_name;
    if(aEvent.GetItem().IsOk())
    {
       old_name = m_panelTree->GetText( aEvent.GetItem() );
    }
    auto elems = m_panelTree->GetSelectedElements();
    if( elems.GetCount() > 1 || elems.empty() )
    {
        aEvent.Veto();
        return;
    }
    assert( m_panelTree->GetSelectedLibraries().GetCount() == 1 );
    PART_LIB* found_lib = GetCurLib();
    if( m_panelTree->IsLibrary(aEvent.GetItem()) )
    {
        // Add to "removed names" and add the new library
    }
    else
    {
        LIB_PART* tmp = found_lib->FindPart( elems[0] );
        if( tmp )
        {
            found_lib->SetModified();
            tmp->SetName( wxString(aEvent.GetLabel()) );
        }
        else
            aEvent.Veto();
    }
#endif
}

wxFileName LIB_EDIT_FRAME::CreateLib()
{
    wxFileName fn = m_panelTree->GetFirstLibraryNameAvailable();
    fn.SetExt( SchematicLibraryFileExtension );

    wxFileDialog dlg( this, _("New Library"), wxEmptyString, fn.GetFullName(),
                      SchematicLibraryFileWildcard, wxFD_SAVE | wxFD_CHANGE_DIR| wxFD_OVERWRITE_PROMPT );

    if( dlg.ShowModal() == wxID_CANCEL )
        return wxFileName();

    fn = dlg.GetPath();
    fn.SetExt( SchematicLibraryFileExtension );

    return fn;
}

void LIB_EDIT_FRAME::EditLibs( wxCommandEvent& aEvent )
{
    PROJECT& prj = Prj();
    wxArrayString   lib_names;
    wxString        lib_paths;
    switch( aEvent.GetId() )
    {
        case ID_LIBEDIT_NEW_LIBRARY:
            {
                wxFileName fn = CreateLib();
                if(fn.GetName().empty())
                {
                    DisplayError( this, _( "No library name specified." ) );
                    return;
                }
                PART_LIB* found_lib = prj.SchLibs()->FindLibrary( fn.GetName() );
                if( found_lib ||
                      std::find_if(m_newLibs.begin(), m_newLibs.end(),
                         [&fn](const PART_LIB* el) -> bool { return el->GetName().CompareTo(fn.GetName()) == 0; })
                      != m_newLibs.end() )
                {
                    DisplayError( this, _( "The library name already exists." ) );
                    return;
                }
                PART_LIB* temp_lib( new PART_LIB( LIBRARY_TYPE_EESCHEMA, fn.GetFullPath() ) );
                m_newLibs.insert( temp_lib );
                m_panelTree->InsertLibrary( fn.GetName() );
            }
            break;
        case ID_LIBEDIT_NEW_COMPONENT:
            {
                CreateNewLibraryPart( aEvent );
            }
            break;
        case ID_LIBEDIT_CUT:
            {
                #if 0
                assert(m_panelTree->GetSelectedLibraries().GetCount() == 1);
                PART_LIB* found_lib = GetCurLib();
                if( found_lib )
                {
                    m_cutOrCopyElems.clear();
                    for( auto& elem : m_panelTree->GetCutOrCopy() )
                    {
                        LIB_ALIAS* tmp = found_lib->FindAlias( elem );
                        if(tmp)
                        {
                            m_cutOrCopyElems.insert(new LIB_PART(*tmp->GetPart()));
                            found_lib->RemoveEntry(tmp);
                        }
                        else
                            wxMessageBox( _( "Component not found" ), _("Error"), wxOK | wxICON_EXCLAMATION | wxCENTRE );
                    }
                }
                #endif
            }
            break;
        case ID_LIBEDIT_PASTE:
            {
                #if 0
                assert(m_panelTree->GetSelectedLibraries().GetCount() == 1);
                PART_LIB* found_lib = GetCurLib();
                if( found_lib )
                {
                    for( const auto& elem : m_cutOrCopyElems )
                    {
                        if( found_lib->FindPart( elem->GetName() ) )
                            wxMessageBox( _( "Not able to add the component because is already present in the destination library" ), _("Error"), wxOK | wxICON_EXCLAMATION | wxCENTRE );
                        else if( !found_lib->AddPart(elem) )
                            wxMessageBox( _( "Not able to add the component" ), _("Error"), wxOK | wxICON_ERROR | wxCENTRE );
                    }
                    m_cutOrCopyElems.clear();
                }
                else
                    wxMessageBox( _( "Destination library not found" ), _("Error"), wxOK | wxICON_EXCLAMATION | wxCENTRE );
                #endif
            }
            break;
    }
}

void LIB_EDIT_FRAME::PopulateTree()
{
    PART_LIBS* libs = Prj().SchLibs();

    if( libs )
       m_panelTree->LoadFootprints(libs);
}

LIB_EDIT_FRAME::~LIB_EDIT_FRAME()
{
    m_drawItem = m_lastDrawItem = NULL;

    delete m_tempCopyComponent;
    m_tempCopyComponent = NULL;
    delete m_my_part;
    m_my_part = NULL;
}


void LIB_EDIT_FRAME::SetDrawItem( LIB_ITEM* aDrawItem )
{
    m_drawItem = aDrawItem;
}


void LIB_EDIT_FRAME::OnCloseWindow( wxCloseEvent& aEvent )
{
    if( GetScreen()->IsModify() )
    {
        int ii = DisplayExitDialog( this, _( "Save the changes in the library before closing?" ) );

        switch( ii )
        {
        case wxID_NO:
            break;

        case wxID_YES:
            if ( this->SaveLibrary() )
                break;

            // fall through: cancel the close because of an error

        case wxID_CANCEL:
            aEvent.Veto();
            return;
        }
        GetScreen()->ClrModify();
    }

    PART_LIBS* libs = Prj().SchLibs();

    for( const PART_LIB& lib : *libs )
    {
        if( lib.IsModified() )
        {
            wxString msg = wxString::Format( _(
                "Library '%s' was modified!\nDiscard changes?" ),
                GetChars( lib.GetName() )
                );

            if( !IsOK( this, msg ) )
            {
                aEvent.Veto();
                return;
            }
        }
    }

    for( auto lib : m_newLibs )
    {
        wxString msg = wxString::Format( _(
              "You added library '%s' but you didn't save it!\nDiscard changes?" ),
              GetChars( lib->GetName() )
              );
        if( !IsOK( this, msg ) )
        {
            aEvent.Veto();
            return;
        }
    }

    Destroy();
}


double LIB_EDIT_FRAME::BestZoom()
{
    /* Please, note: wxMSW before version 2.9 seems have
     * problems with zoom values < 1 ( i.e. userscale > 1) and needs to be patched:
     * edit file <wxWidgets>/src/msw/dc.cpp
     * search for line static const int VIEWPORT_EXTENT = 1000;
     * and replace by static const int VIEWPORT_EXTENT = 10000;
     */
    int      dx, dy;

    LIB_PART*      part = GetCurPart();

    if( part )
    {
        EDA_RECT boundingBox = part->GetBoundingBox( m_unit, m_convert );

        dx = boundingBox.GetWidth();
        dy = boundingBox.GetHeight();
        SetScrollCenterPosition( wxPoint( 0, 0 ) );
    }
    else
    {
        const PAGE_INFO& pageInfo = GetScreen()->GetPageSettings();

        dx = pageInfo.GetSizeIU().x;
        dy = pageInfo.GetSizeIU().y;

        SetScrollCenterPosition( wxPoint( 0, 0 ) );
    }

    wxSize size = m_canvas->GetClientSize();

    // Reserve a 10% margin around component bounding box.
    double margin_scale_factor = 0.8;
    double zx =(double) dx / ( margin_scale_factor * (double)size.x );
    double zy = (double) dy / ( margin_scale_factor * (double)size.y );

    double bestzoom = std::max( zx, zy );

    // keep it >= minimal existing zoom (can happen for very small components
    // for instance when starting a new component
    if( bestzoom  < GetScreen()->m_ZoomList[0] )
        bestzoom  = GetScreen()->m_ZoomList[0];

    return bestzoom;
}


void LIB_EDIT_FRAME::UpdateAliasSelectList()
{
    if( m_aliasSelectBox == NULL )
        return;

    m_aliasSelectBox->Clear();

    LIB_PART*      part = GetCurPart();

    if( !part )
        return;

    m_aliasSelectBox->Append( part->GetAliasNames() );
    m_aliasSelectBox->SetSelection( 0 );

    int index = m_aliasSelectBox->FindString( m_aliasName );

    if( index != wxNOT_FOUND )
        m_aliasSelectBox->SetSelection( index );
}


void LIB_EDIT_FRAME::UpdatePartSelectList()
{
    if( m_partSelectBox == NULL )
        return;

    if( m_partSelectBox->GetCount() != 0 )
        m_partSelectBox->Clear();

    LIB_PART*      part = GetCurPart();

    if( !part || part->GetUnitCount() <= 1 )
    {
        m_unit = 1;
        m_partSelectBox->Append( wxEmptyString );
    }
    else
    {
        for( int i = 0; i < part->GetUnitCount(); i++ )
        {
            wxString sub  = LIB_PART::SubReference( i+1, false );
            wxString unit = wxString::Format( _( "Unit %s" ), GetChars( sub ) );
            m_partSelectBox->Append( unit );
        }
    }

    // Ensure the current selected unit is compatible with
    // the number of units of the current part:
    if( part && part->GetUnitCount() < m_unit )
        m_unit = 1;

    m_partSelectBox->SetSelection( ( m_unit > 0 ) ? m_unit - 1 : 0 );
}


void LIB_EDIT_FRAME::OnUpdateEditingPart( wxUpdateUIEvent& aEvent )
{
    LIB_PART*      part = GetCurPart();

    aEvent.Enable( part != NULL );

    if( part && aEvent.GetEventObject() == m_drawToolBar )
        aEvent.Check( GetToolId() == aEvent.GetId() );
}


void LIB_EDIT_FRAME::OnUpdateNotEditingPart( wxUpdateUIEvent& aEvent )
{
    aEvent.Enable( !GetCurPart() );
}


void LIB_EDIT_FRAME::OnUpdateUndo( wxUpdateUIEvent& aEvent )
{
    aEvent.Enable( GetCurPart() && GetScreen() &&
        GetScreen()->GetUndoCommandCount() != 0 && !IsEditingDrawItem() );
}


void LIB_EDIT_FRAME::OnUpdateRedo( wxUpdateUIEvent& aEvent )
{
    aEvent.Enable( GetCurPart() && GetScreen() &&
        GetScreen()->GetRedoCommandCount() != 0 && !IsEditingDrawItem() );
}

void LIB_EDIT_FRAME::SaveSelectedLibrary( wxCommandEvent& aEvent )
{
    #if 0
    PART_LIB* cur_lib = GetCurLib();
    if( !cur_lib )
       return;
    SaveLibrary( cur_lib );
    if( m_newLibs.find(cur_lib) != m_newLibs.end() )
        m_newLibs.erase(cur_lib);
    #endif
}
void LIB_EDIT_FRAME::OnSaveSelectedLib( wxUpdateUIEvent& aEvent )
{
    if( GetCurPart() && GetScreen()->IsModify() )
    {
        aEvent.Enable( true );
        return;
    }
    auto libs = m_panelTree->GetSelectedLibraries();
    if( libs.GetCount() != 1 )
    {
        aEvent.Enable( false );
        return;
    }
    wxString name = libs[0];
    if( name.empty() )
    {
        aEvent.Enable( false );
        return;
    }
    PART_LIB* lib = Prj().SchLibs()->FindLibrary( name );
    if( lib && lib->IsModified() )
    {
        aEvent.Enable( true );
        return;
    }
    aEvent.Enable( lib && m_newLibs.find(lib) != m_newLibs.end() );
}

void LIB_EDIT_FRAME::OnSaveAllLibraries( wxUpdateUIEvent& aEvent )
{
    if( GetCurPart() && GetScreen()->IsModify() )
    {
        aEvent.Enable( true );
        return;
    }
    PART_LIBS* libs = Prj().SchLibs();
    for( const PART_LIB& lib : *libs )
    {
        if( lib.IsModified() )
        {
            aEvent.Enable( true );
            return;
        }
    }
    aEvent.Enable( !m_newLibs.empty() );
}

void LIB_EDIT_FRAME::OnUpdateSaveCurrentLib( wxUpdateUIEvent& aEvent )
{
    if( GetScreen()->IsModify() || !m_newLibs.empty() )
    {
        aEvent.Enable( true );
        return;
    }
    PART_LIBS* libs = Prj().SchLibs();

    for( const PART_LIB& lib : *libs )
    {
        if( lib.IsModified() )
            aEvent.Enable( true );
    }
    aEvent.Enable( false );
}


void LIB_EDIT_FRAME::OnUpdateViewDoc( wxUpdateUIEvent& aEvent )
{
   /*
    bool enable = false;

    PART_LIB*    lib  = GetCurLib();
    LIB_PART*    part = GetCurPart();

    if( part && lib )
    {
        LIB_ALIAS* alias = part->GetAlias( m_aliasName );

        wxCHECK_RET( alias != NULL, wxT( "Alias <" ) + m_aliasName + wxT( "> not found." ) );

        enable = !alias->GetDocFileName().IsEmpty();
    }

    aEvent.Enable( enable );
    */
}


void LIB_EDIT_FRAME::OnUpdatePinByPin( wxUpdateUIEvent& aEvent )
{
    LIB_PART*      part = GetCurPart();

    aEvent.Enable( part && ( part->GetUnitCount() > 1 || m_showDeMorgan ) );

    aEvent.Check( m_editPinsPerPartOrConvert );
}

void LIB_EDIT_FRAME::OnUpdatePinTable( wxUpdateUIEvent& aEvent )
{
    LIB_PART* part = GetCurPart();
    aEvent.Enable( part != NULL );
}

void LIB_EDIT_FRAME::OnUpdatePartNumber( wxUpdateUIEvent& aEvent )
{
    if( m_partSelectBox == NULL )
        return;

    LIB_PART*      part = GetCurPart();

    // Using the typical aEvent.Enable() call doesn't seem to work with wxGTK
    // so use the pointer to alias combobox to directly enable or disable.
    m_partSelectBox->Enable( part && part->GetUnitCount() > 1 );
}


void LIB_EDIT_FRAME::OnUpdateDeMorganNormal( wxUpdateUIEvent& aEvent )
{
    if( m_mainToolBar == NULL )
        return;

    LIB_PART*      part = GetCurPart();

    aEvent.Enable( GetShowDeMorgan() || ( part && part->HasConversion() ) );
    aEvent.Check( m_convert <= 1 );
}


void LIB_EDIT_FRAME::OnUpdateDeMorganConvert( wxUpdateUIEvent& aEvent )
{
    if( m_mainToolBar == NULL )
        return;

    LIB_PART*      part = GetCurPart();

    aEvent.Enable( GetShowDeMorgan() || ( part && part->HasConversion() ) );
    aEvent.Check( m_convert > 1 );
}


void LIB_EDIT_FRAME::OnUpdateSelectAlias( wxUpdateUIEvent& aEvent )
{
    if( m_aliasSelectBox == NULL )
        return;

    LIB_PART*      part = GetCurPart();

    // Using the typical aEvent.Enable() call doesn't seem to work with wxGTK
    // so use the pointer to alias combobox to directly enable or disable.
    m_aliasSelectBox->Enable( part && part->GetAliasCount() > 1 );
}


void LIB_EDIT_FRAME::OnSelectAlias( wxCommandEvent& aEvent )
{
    if( m_aliasSelectBox == NULL
        || ( m_aliasSelectBox->GetStringSelection().CmpNoCase( m_aliasName ) == 0)  )
        return;

    m_lastDrawItem = NULL;
    m_aliasName = m_aliasSelectBox->GetStringSelection();

    DisplayCmpDoc();
    m_canvas->Refresh();
}


void LIB_EDIT_FRAME::OnSelectPart( wxCommandEvent& aEvent )
{
    int i = aEvent.GetSelection();

    if( ( i == wxNOT_FOUND ) || ( ( i + 1 ) == m_unit ) )
        return;

    m_lastDrawItem = NULL;
    m_unit = i + 1;
    m_canvas->Refresh();
    DisplayCmpDoc();
}


void LIB_EDIT_FRAME::OnViewEntryDoc( wxCommandEvent& aEvent )
{
    LIB_PART* part = GetCurPart();

    if( !part )
        return;

    wxString    fileName;
    LIB_ALIAS*  alias = part->GetAlias( m_aliasName );

    wxCHECK_RET( alias != NULL, wxT( "Alias not found." ) );

    fileName = alias->GetDocFileName();

    if( !fileName.IsEmpty() )
    {
        SEARCH_STACK* lib_search = Prj().SchSearchS();

        GetAssociatedDocument( this, fileName, lib_search );
    }
}


void LIB_EDIT_FRAME::OnSelectBodyStyle( wxCommandEvent& aEvent )
{
    m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor() );

    if( aEvent.GetId() == ID_DE_MORGAN_NORMAL_BUTT )
        m_convert = 1;
    else
        m_convert = 2;

    m_lastDrawItem = NULL;
    m_canvas->Refresh();
}


void LIB_EDIT_FRAME::Process_Special_Functions( wxCommandEvent& aEvent )
{
    int     id = aEvent.GetId();
    wxPoint pos;

    m_canvas->SetIgnoreMouseEvents( true );

    wxGetMousePosition( &pos.x, &pos.y );
    pos.y += 20;

    switch( id )   // Stop placement commands before handling new command.
    {
    case ID_POPUP_LIBEDIT_END_CREATE_ITEM:
    case ID_LIBEDIT_EDIT_PIN:
    case ID_POPUP_LIBEDIT_BODY_EDIT_ITEM:
    case ID_POPUP_LIBEDIT_FIELD_EDIT_ITEM:
    case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINSIZE_ITEM:
    case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINNAMESIZE_ITEM:
    case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINNUMSIZE_ITEM:
    case ID_POPUP_ZOOM_BLOCK:
    case ID_POPUP_DELETE_BLOCK:
    case ID_POPUP_COPY_BLOCK:
    case ID_POPUP_SELECT_ITEMS_BLOCK:
    case ID_POPUP_MIRROR_X_BLOCK:
    case ID_POPUP_MIRROR_Y_BLOCK:
    case ID_POPUP_ROTATE_BLOCK:
    case ID_POPUP_PLACE_BLOCK:
    case ID_POPUP_LIBEDIT_DELETE_CURRENT_POLY_SEGMENT:
        break;

    case ID_POPUP_LIBEDIT_CANCEL_EDITING:
        if( m_canvas->IsMouseCaptured() )
            m_canvas->EndMouseCapture();
        else
            m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor() );
        break;

    case ID_POPUP_LIBEDIT_DELETE_ITEM:
        m_canvas->EndMouseCapture();
        break;

    default:
        m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor(),
                                    wxEmptyString );
        break;
    }

    INSTALL_UNBUFFERED_DC( dc, m_canvas );

    switch( id )
    {
    case ID_POPUP_LIBEDIT_CANCEL_EDITING:
        break;

    case ID_LIBEDIT_SELECT_CURRENT_LIB:
        SelectActiveLibrary();
        break;

    case ID_LIBEDIT_SAVE_CURRENT_PART:
        {
            LIB_PART*   part = GetCurPart();

            if( !part )
            {
                DisplayError( this, _( "No part to save." ) );
                break;
            }

            SaveOnePart( part );
        }
        break;

    case ID_LIBEDIT_EDIT_PIN_BY_PIN:
        m_editPinsPerPartOrConvert = m_mainToolBar->GetToolToggled( ID_LIBEDIT_EDIT_PIN_BY_PIN );
        break;

    case ID_POPUP_LIBEDIT_END_CREATE_ITEM:
        m_canvas->MoveCursorToCrossHair();
        if( m_drawItem )
        {
            EndDrawGraphicItem( &dc );
        }
        break;

    case ID_POPUP_LIBEDIT_BODY_EDIT_ITEM:
        if( m_drawItem )
        {
            m_canvas->CrossHairOff( &dc );

            switch( m_drawItem->Type() )
            {
            case LIB_ARC_T:
            case LIB_CIRCLE_T:
            case LIB_RECTANGLE_T:
            case LIB_POLYLINE_T:
                EditGraphicSymbol( &dc, m_drawItem );
                break;

            case LIB_TEXT_T:
                EditSymbolText( &dc, m_drawItem );
                break;

            default:
                ;
            }

            m_canvas->CrossHairOn( &dc );
        }
        break;

    case ID_POPUP_LIBEDIT_DELETE_CURRENT_POLY_SEGMENT:
        {
            // Delete the last created segment, while creating a polyline draw item
            if( m_drawItem == NULL )
                break;

            m_canvas->MoveCursorToCrossHair();
            STATUS_FLAGS oldFlags = m_drawItem->GetFlags();
            m_drawItem->ClearFlags();
            m_drawItem->Draw( m_canvas, &dc, wxPoint( 0, 0 ), UNSPECIFIED_COLOR, g_XorMode, NULL,
                              DefaultTransform );
            ( (LIB_POLYLINE*) m_drawItem )->DeleteSegment( GetCrossHairPosition( true ) );
            m_drawItem->Draw( m_canvas, &dc, wxPoint( 0, 0 ), UNSPECIFIED_COLOR, g_XorMode, NULL,
                              DefaultTransform );
            m_drawItem->SetFlags( oldFlags );
            m_lastDrawItem = NULL;
        }
        break;

    case ID_POPUP_LIBEDIT_DELETE_ITEM:
        if( m_drawItem )
            deleteItem( &dc );

        break;

    case ID_POPUP_LIBEDIT_MOVE_ITEM_REQUEST:
        if( m_drawItem == NULL )
            break;

        if( m_drawItem->Type() == LIB_PIN_T )
            StartMovePin( &dc );
        else
            StartMoveDrawSymbol( &dc );
        break;

    case ID_POPUP_LIBEDIT_MODIFY_ITEM:

        if( m_drawItem == NULL )
            break;

        m_canvas->MoveCursorToCrossHair();
        if( m_drawItem->Type() == LIB_RECTANGLE_T
            || m_drawItem->Type() == LIB_CIRCLE_T
            || m_drawItem->Type() == LIB_POLYLINE_T
            || m_drawItem->Type() == LIB_ARC_T
            )
        {
            StartModifyDrawSymbol( &dc );
        }

        break;

    case ID_POPUP_LIBEDIT_FIELD_EDIT_ITEM:
        if( m_drawItem == NULL )
            break;

        m_canvas->CrossHairOff( &dc );

        if( m_drawItem->Type() == LIB_FIELD_T )
        {
            EditField( (LIB_FIELD*) m_drawItem );
        }

        m_canvas->MoveCursorToCrossHair();
        m_canvas->CrossHairOn( &dc );
        break;

    case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINSIZE_ITEM:
    case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINNAMESIZE_ITEM:
    case ID_POPUP_LIBEDIT_PIN_GLOBAL_CHANGE_PINNUMSIZE_ITEM:
        {
            if( !m_drawItem || m_drawItem->Type() != LIB_PIN_T )
                break;

            LIB_PART*      part = GetCurPart();

            SaveCopyInUndoList( part );

            GlobalSetPins( (LIB_PIN*) m_drawItem, id );
            m_canvas->MoveCursorToCrossHair();
            m_canvas->Refresh();
        }
        break;

    case ID_POPUP_ZOOM_BLOCK:
        m_canvas->SetAutoPanRequest( false );
        GetScreen()->m_BlockLocate.SetCommand( BLOCK_ZOOM );
        HandleBlockEnd( &dc );
        break;

    case ID_POPUP_DELETE_BLOCK:
        m_canvas->SetAutoPanRequest( false );
        GetScreen()->m_BlockLocate.SetCommand( BLOCK_DELETE );
        m_canvas->MoveCursorToCrossHair();
        HandleBlockEnd( &dc );
        break;

    case ID_POPUP_COPY_BLOCK:
        m_canvas->SetAutoPanRequest( false );
        GetScreen()->m_BlockLocate.SetCommand( BLOCK_COPY );
        m_canvas->MoveCursorToCrossHair();
        HandleBlockEnd( &dc );
        break;

    case ID_POPUP_SELECT_ITEMS_BLOCK:
        m_canvas->SetAutoPanRequest( false );
        GetScreen()->m_BlockLocate.SetCommand( BLOCK_SELECT_ITEMS_ONLY );
        m_canvas->MoveCursorToCrossHair();
        HandleBlockEnd( &dc );
        break;

    case ID_POPUP_MIRROR_Y_BLOCK:
        m_canvas->SetAutoPanRequest( false );
        GetScreen()->m_BlockLocate.SetCommand( BLOCK_MIRROR_Y );
        m_canvas->MoveCursorToCrossHair();
        HandleBlockPlace( &dc );
        break;

    case ID_POPUP_MIRROR_X_BLOCK:
        m_canvas->SetAutoPanRequest( false );
        GetScreen()->m_BlockLocate.SetCommand( BLOCK_MIRROR_X );
        m_canvas->MoveCursorToCrossHair();
        HandleBlockPlace( &dc );
        break;

    case ID_POPUP_ROTATE_BLOCK:
        m_canvas->SetAutoPanRequest( false );
        GetScreen()->m_BlockLocate.SetCommand( BLOCK_ROTATE );
        m_canvas->MoveCursorToCrossHair();
        HandleBlockPlace( &dc );
        break;

    case ID_POPUP_PLACE_BLOCK:
        m_canvas->SetAutoPanRequest( false );
        m_canvas->MoveCursorToCrossHair();
        HandleBlockPlace( &dc );
        break;

    default:
        DisplayError( this, wxT( "LIB_EDIT_FRAME::Process_Special_Functions error" ) );
        break;
    }

    m_canvas->SetIgnoreMouseEvents( false );

    if( GetToolId() == ID_NO_TOOL_SELECTED )
        m_lastDrawItem = NULL;
}


void LIB_EDIT_FRAME::OnActivate( wxActivateEvent& aEvent )
{
    EDA_DRAW_FRAME::OnActivate( aEvent );
}


PART_LIB* LIB_EDIT_FRAME::GetCurLib()
{
    auto libs = m_panelTree->GetSelectedLibraries();
    if( libs.empty() )
       return nullptr;
    wxString name = libs[0];
    if( !name.empty() )
    {
        PART_LIB* lib = Prj().SchLibs()->FindLibrary( name );
        if( lib )
        {
            Prj().SetRString( PROJECT::SCH_LIBEDIT_CUR_LIB, name );
            return lib;
        }
        auto elem = std::find_if(m_newLibs.begin(), m_newLibs.end(),
              [&name](const PART_LIB* el) -> bool { return el->GetName().CompareTo(name) == 0; });
        if( elem != m_newLibs.end() )
        {
            return *elem;
        }
    }
    return nullptr;
}

PART_LIB* LIB_EDIT_FRAME::SetCurLib( PART_LIB* aLib )
{
    PART_LIB* old = GetCurLib();

    if( !aLib || !aLib->GetName())
        Prj().SetRString( PROJECT::SCH_LIBEDIT_CUR_LIB, wxEmptyString );
    else
        Prj().SetRString( PROJECT::SCH_LIBEDIT_CUR_LIB, aLib->GetName() );

    return old;
}

LIB_PART* LIB_EDIT_FRAME::GetCurPart()
{
    if( m_my_part )
        return m_my_part;
    wxString name;
    if(m_panelTree->GetSelectedElements().GetCount() == 1)
        name = m_panelTree->GetSelectedElements()[0];
    LIB_PART*   part;

    if( !name.empty() && ( part = Prj().SchLibs()->FindLibPart( name ) ) )
    {
        // clone it from the PART_LIB and own it.
        m_my_part = new LIB_PART( *part );
        Prj().SetRString( PROJECT::SCH_LIBEDIT_CUR_PART, name );
    }
    else
        Prj().SetRString( PROJECT::SCH_LIBEDIT_CUR_PART, wxEmptyString );
    return m_my_part;
}


void LIB_EDIT_FRAME::SetCurPart( LIB_PART* aPart )
{
    delete m_my_part;
    m_my_part = aPart;      // take ownership here

    printf("SetCurPart %p\n", aPart );

    // retain in case this wxFrame is re-opened later on the same PROJECT
    Prj().SetRString( PROJECT::SCH_LIBEDIT_CUR_PART,
            aPart ? aPart->GetName() : wxString() );
}


void LIB_EDIT_FRAME::TempCopyComponent()
{
    delete m_tempCopyComponent;

    if( LIB_PART* part = GetCurPart() )
        // clone it and own the clone.
        m_tempCopyComponent = new LIB_PART( *part );
    else
        // clear it, there was no CurPart
        m_tempCopyComponent = NULL;
}


void LIB_EDIT_FRAME::RestoreComponent()
{
    if( m_tempCopyComponent )
    {
        // transfer ownership to CurPart
        SetCurPart( m_tempCopyComponent );
        m_tempCopyComponent = NULL;
    }
}


void LIB_EDIT_FRAME::ClearTempCopyComponent()
{
    delete m_tempCopyComponent;
    m_tempCopyComponent = NULL;
}


void LIB_EDIT_FRAME::EditSymbolText( wxDC* aDC, LIB_ITEM* aDrawItem )
{
    if ( ( aDrawItem == NULL ) || ( aDrawItem->Type() != LIB_TEXT_T ) )
        return;

    // Deleting old text
    if( aDC && !aDrawItem->InEditMode() )
        aDrawItem->Draw( m_canvas, aDC, wxPoint( 0, 0 ), UNSPECIFIED_COLOR, g_XorMode, NULL,
                        DefaultTransform );

    DIALOG_LIB_EDIT_TEXT* frame = new DIALOG_LIB_EDIT_TEXT( this, (LIB_TEXT*) aDrawItem );
    frame->ShowModal();
    frame->Destroy();
    OnModify();

    // Display new text
    if( aDC && !aDrawItem->InEditMode() )
        aDrawItem->Draw( m_canvas, aDC, wxPoint( 0, 0 ), UNSPECIFIED_COLOR, GR_DEFAULT_DRAWMODE, NULL,
                        DefaultTransform );
}


void LIB_EDIT_FRAME::OnEditComponentProperties( wxCommandEvent& aEvent )
{
    bool partLocked = GetCurPart()->UnitsLocked();

    DIALOG_EDIT_COMPONENT_IN_LIBRARY dlg( this );

    if( dlg.ShowModal() == wxID_CANCEL )
        return;

    if( partLocked != GetCurPart()->UnitsLocked() )
    {
        // m_editPinsPerPartOrConvert is set to the better value, if m_UnitSelectionLocked
        // has changed
        m_editPinsPerPartOrConvert = GetCurPart()->UnitsLocked() ? true : false;
    }

    UpdateAliasSelectList();
    UpdatePartSelectList();
    DisplayLibInfos();
    DisplayCmpDoc();
    OnModify();
    m_canvas->Refresh();
}


void LIB_EDIT_FRAME::OnCreateNewPartFromExisting( wxCommandEvent& aEvent )
{
    LIB_PART*      part = GetCurPart();

    wxCHECK_RET( part, wxT( "Cannot create new part from non-existent current part." ) );

    INSTALL_UNBUFFERED_DC( dc, m_canvas );
    m_canvas->CrossHairOff( &dc );

    EditField( &part->GetValueField() );

    m_canvas->MoveCursorToCrossHair();
    m_canvas->CrossHairOn( &dc );
}


void LIB_EDIT_FRAME::OnSelectTool( wxCommandEvent& aEvent )
{
    int id = aEvent.GetId();

    if( GetToolId() == ID_NO_TOOL_SELECTED )
        m_lastDrawItem = NULL;

    m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor(),
                               wxEmptyString );

    LIB_PART*      part = GetCurPart();

    switch( id )
    {
    case ID_NO_TOOL_SELECTED:
        SetToolID( id, m_canvas->GetDefaultCursor(), wxEmptyString );
        break;

    case ID_ZOOM_SELECTION:
        SetToolID( id, wxCURSOR_MAGNIFIER, _( "Zoom to selection" ) );
        break;

    case ID_LIBEDIT_PIN_BUTT:
        if( part )
        {
            SetToolID( id, wxCURSOR_PENCIL, _( "Add pin" ) );
        }
        else
        {
            SetToolID( id, wxCURSOR_ARROW, _( "Set pin options" ) );

            wxCommandEvent cmd( wxEVT_COMMAND_MENU_SELECTED );

            cmd.SetId( ID_LIBEDIT_EDIT_PIN );
            GetEventHandler()->ProcessEvent( cmd );
            SetToolID( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor(), wxEmptyString );
        }
        break;

    case ID_LIBEDIT_BODY_TEXT_BUTT:
        SetToolID( id, wxCURSOR_PENCIL, _( "Add text" ) );
        break;

    case ID_LIBEDIT_BODY_RECT_BUTT:
        SetToolID( id, wxCURSOR_PENCIL, _( "Add rectangle" ) );
        break;

    case ID_LIBEDIT_BODY_CIRCLE_BUTT:
        SetToolID( id, wxCURSOR_PENCIL, _( "Add circle" ) );
        break;

    case ID_LIBEDIT_BODY_ARC_BUTT:
        SetToolID( id, wxCURSOR_PENCIL, _( "Add arc" ) );
        break;

    case ID_LIBEDIT_BODY_LINE_BUTT:
        SetToolID( id, wxCURSOR_PENCIL, _( "Add line" ) );
        break;

    case ID_LIBEDIT_ANCHOR_ITEM_BUTT:
        SetToolID( id, wxCURSOR_HAND, _( "Set anchor position" ) );
        break;

    case ID_LIBEDIT_IMPORT_BODY_BUTT:
        SetToolID( id, m_canvas->GetDefaultCursor(), _( "Import" ) );
        LoadOneSymbol();
        SetToolID( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor(), wxEmptyString );
        break;

    case ID_LIBEDIT_EXPORT_BODY_BUTT:
        SetToolID( id, m_canvas->GetDefaultCursor(), _( "Export" ) );
        SaveOneSymbol();
        SetToolID( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor(), wxEmptyString );
        break;

    case ID_LIBEDIT_DELETE_ITEM_BUTT:
        if( !part )
        {
            wxBell();
            break;
        }

        SetToolID( id, wxCURSOR_BULLSEYE, _( "Delete item" ) );
        break;

    default:
        break;
    }

    m_canvas->SetIgnoreMouseEvents( false );
}


void LIB_EDIT_FRAME::OnRotateItem( wxCommandEvent& aEvent )
{
    if( m_drawItem == NULL )
        return;

    if( !m_drawItem->InEditMode() )
    {
        LIB_PART*      part = GetCurPart();

        SaveCopyInUndoList( part );
        m_drawItem->SetUnit( m_unit );
    }

    m_drawItem->Rotate();
    OnModify();

    if( !m_drawItem->InEditMode() )
        m_drawItem->ClearFlags();

    m_canvas->Refresh();

    if( GetToolId() == ID_NO_TOOL_SELECTED )
        m_lastDrawItem = NULL;
}


void LIB_EDIT_FRAME::OnOrient( wxCommandEvent& aEvent )
{
    INSTALL_UNBUFFERED_DC( dc, m_canvas );
    SCH_SCREEN* screen = GetScreen();
    // Allows block rotate operation on hot key.
    if( screen->m_BlockLocate.GetState() != STATE_NO_BLOCK )
    {
        if( aEvent.GetId() == ID_LIBEDIT_MIRROR_X )
        {
            m_canvas->MoveCursorToCrossHair();
            screen->m_BlockLocate.SetMessageBlock( this );
            screen->m_BlockLocate.SetCommand( BLOCK_MIRROR_X );
            HandleBlockEnd( &dc );
        }
        else if( aEvent.GetId() == ID_LIBEDIT_MIRROR_Y )
        {
            m_canvas->MoveCursorToCrossHair();
            screen->m_BlockLocate.SetMessageBlock( this );
            screen->m_BlockLocate.SetCommand( BLOCK_MIRROR_Y );
            HandleBlockEnd( &dc );
        }
    }
}


LIB_ITEM* LIB_EDIT_FRAME::LocateItemUsingCursor( const wxPoint& aPosition,
                                                 const KICAD_T aFilterList[] )
{
    LIB_PART*      part = GetCurPart();

    if( !part )
        return NULL;

    LIB_ITEM* item = locateItem( aPosition, aFilterList );

    wxPoint pos = GetNearestGridPosition( aPosition );

    if( item == NULL && aPosition != pos )
        item = locateItem( pos, aFilterList );

    return item;
}


LIB_ITEM* LIB_EDIT_FRAME::locateItem( const wxPoint& aPosition, const KICAD_T aFilterList[] )
{
    LIB_PART*      part = GetCurPart();

    if( !part )
        return NULL;

    LIB_ITEM* item = NULL;

    m_collectedItems.Collect( part->GetDrawItemList(), aFilterList, aPosition,
                              m_unit, m_convert );

    if( m_collectedItems.GetCount() == 0 )
    {
        ClearMsgPanel();
    }
    else if( m_collectedItems.GetCount() == 1 )
    {
        item = m_collectedItems[0];
    }
    else
    {
        if( item == NULL )
        {
            wxASSERT_MSG( m_collectedItems.GetCount() <= MAX_SELECT_ITEM_IDS,
                          wxT( "Select item clarification context menu size limit exceeded." ) );

            wxMenu selectMenu;
            wxMenuItem* title = new wxMenuItem( &selectMenu, wxID_NONE, _( "Clarify Selection" ) );

            selectMenu.Append( title );
            selectMenu.AppendSeparator();

            for( int i = 0;  i < m_collectedItems.GetCount() && i < MAX_SELECT_ITEM_IDS;  i++ )
            {
                wxString    text = m_collectedItems[i]->GetSelectMenuText();
                BITMAP_DEF  xpm = m_collectedItems[i]->GetMenuImage();

                AddMenuItem( &selectMenu, ID_SELECT_ITEM_START + i, text, KiBitmap( xpm ) );
            }

            // Set to NULL in case user aborts the clarification context menu.
            m_drawItem = NULL;
            m_canvas->SetAbortRequest( true );   // Changed to false if an item is selected
            PopupMenu( &selectMenu );
            m_canvas->MoveCursorToCrossHair();
            item = m_drawItem;
        }
    }

    if( item )
    {
        MSG_PANEL_ITEMS items;
        item->GetMsgPanelInfo( items );
        SetMsgPanel( items );
    }
    else
    {
        ClearMsgPanel();
    }

    return item;
}


void LIB_EDIT_FRAME::deleteItem( wxDC* aDC )
{
    wxCHECK_RET( m_drawItem != NULL, wxT( "No drawing item selected to delete." ) );

    m_canvas->CrossHairOff( aDC );

    LIB_PART*      part = GetCurPart();

    SaveCopyInUndoList( part );

    if( m_drawItem->Type() == LIB_PIN_T )
    {
        LIB_PIN*    pin = (LIB_PIN*) m_drawItem;
        wxPoint     pos = pin->GetPosition();

        part->RemoveDrawItem( (LIB_ITEM*) pin, m_canvas, aDC );

        if( SynchronizePins() )
        {
            LIB_PIN* tmp = part->GetNextPin();

            while( tmp != NULL )
            {
                pin = tmp;
                tmp = part->GetNextPin( pin );

                if( pin->GetPosition() != pos )
                    continue;

                part->RemoveDrawItem( (LIB_ITEM*) pin );
            }
        }

        m_canvas->Refresh();
    }
    else
    {
        if( m_canvas->IsMouseCaptured() )
        {
            m_canvas->CallEndMouseCapture( aDC );
        }
        else
        {
            part->RemoveDrawItem( m_drawItem, m_canvas, aDC );
            m_canvas->Refresh();
        }
    }

    m_drawItem = NULL;
    m_lastDrawItem = NULL;
    OnModify();
    m_canvas->CrossHairOn( aDC );
}


void LIB_EDIT_FRAME::OnSelectItem( wxCommandEvent& aEvent )
{
    int id = aEvent.GetId();
    int index = id - ID_SELECT_ITEM_START;

    if( (id >= ID_SELECT_ITEM_START && id <= ID_SELECT_ITEM_END)
        && (index >= 0 && index < m_collectedItems.GetCount()) )
    {
        LIB_ITEM* item = m_collectedItems[index];
        m_canvas->SetAbortRequest( false );
        m_drawItem = item;
    }
}

void LIB_EDIT_FRAME::OnOpenPinTable( wxCommandEvent& aEvent )
{
    LIB_PART* part = GetCurPart();

    DIALOG_LIB_EDIT_PIN_TABLE dlg( this, *part );

    if( dlg.ShowModal() == wxID_CANCEL )
        return;

    return;
}

bool LIB_EDIT_FRAME::SynchronizePins()
{
    LIB_PART*      part = GetCurPart();

    return !m_editPinsPerPartOrConvert && ( part &&
        ( part->HasConversion() || part->IsMulti() ) );
}


void LIB_EDIT_FRAME::DisplayLibInfos()
{
   /*
    wxString        msg = _( "Part Library Editor: " );
    PART_LIB*    lib = GetCurLib();

    if( lib )
    {
        msg += lib->GetFullFileName();

        if( lib->IsReadOnly() )
            msg += _( " [Read Only]" );
    }
    else
    {
        msg += _( "no library selected" );
    }

    SetTitle( msg );
    */
}


void LIB_EDIT_FRAME::SelectActiveLibrary( PART_LIB* aLibrary )
{
    if( !aLibrary )
        //aLibrary = SelectLibraryFromList();
        return;

    if( aLibrary )
    {
        SetCurLib( aLibrary );
    }

    DisplayLibInfos();
}


bool LIB_EDIT_FRAME::LoadComponentAndSelectLib( LIB_ALIAS* aLibEntry, PART_LIB* aLibrary )
{
    if( GetScreen()->IsModify()
        && !IsOK( this, _( "The current component is not saved.\n\nDiscard current changes?" ) ) )
        return false;

    return LoadComponentFromCurrentLib( aLibEntry );
}


bool LIB_EDIT_FRAME::LoadComponentFromCurrentLib( LIB_ALIAS* aLibEntry )
{
    if( !LoadOneLibraryPartAux( aLibEntry, GetCurLib() ) )
        return false;

    m_editPinsPerPartOrConvert = GetCurPart()->UnitsLocked() ? true : false;

    GetScreen()->ClearUndoRedoList();
    Zoom_Automatique( false );
    SetShowDeMorgan( GetCurPart()->HasConversion() );

    return true;
}


void LIB_EDIT_FRAME::LoadOneLibraryPart( wxTreeEvent& aEvent )
{
    LIB_ALIAS* libEntry = NULL;

    m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor() );


    if( GetScreen()->IsModify()
        && !IsOK( this, _( "The current component is not saved.\n\nDiscard current changes?" ) ) )
        return;
    if( m_is_new )
    {
        m_panelTree->RemoveLastAdded();
        m_is_new = false;
    }

    PART_LIB* lib = GetCurLib();
    if( !lib )
    {
       DisplayError( this, _( "No library specified." ) );
       return;
    }

    /*
    // No current lib, ask user for the library to use.
    if( !lib )
    {
        SelectActiveLibrary();
        lib = GetCurLib();

        if( !lib )
            return;
    }

    wxArrayString dummyHistoryList;
    int dummyLastUnit;
    SCHLIB_FILTER filter;
    filter.LoadFrom( lib->GetName() );
    */
    if( !aEvent.GetItem().IsOk() ||
          m_panelTree->IsLibrary(aEvent.GetItem()))
        return;
    wxString cmp_name = m_panelTree->GetText(aEvent.GetItem());

    printf("LoadOneLibraryPart [%s]: \n", (const char *)cmp_name.c_str());


    GetScreen()->ClrModify();
    m_lastDrawItem = m_drawItem = NULL;

    // Delete previous library component, if any
    SetCurPart( NULL );
    m_aliasName.Empty();

    // Load the new library component
    libEntry = lib->FindAlias( cmp_name );

    if( !libEntry )
    {
        libEntry = lib->FindAlias(m_panelTree->GetComponent());
        LIB_ALIAS* tmp = lib->FindAlias(m_panelTree->GetComponent());
        for( int u = 1; u <= tmp->GetPart()->GetUnitCount(); ++u )
        {
           if(cmp_name.EndsWith(LIB_PART::SubReference( u, false )))
           {
               m_unit = u;
               break;
           }
        }
    }
    else
        m_unit = 1;
    if( !libEntry )
    {
        wxString msg = wxString::Format( _(
            "Part name '%s' not found in library '%s'" ),
            GetChars( cmp_name ),
            GetChars( lib->GetName() )
            );
        DisplayError( this, msg );
        return;
    }
    PART_LIB* old = SetCurLib( lib );

    LoadComponentFromCurrentLib( libEntry );

    SetCurLib( old );

    DisplayLibInfos();
}


bool LIB_EDIT_FRAME::LoadOneLibraryPartAux( LIB_ALIAS* aEntry, PART_LIB* aLibrary )
{
    wxString msg, rootName;

    if( !aEntry || !aLibrary )
        return false;

    if( aEntry->GetName().IsEmpty() )
    {
        wxLogWarning( wxT( "Entry in library <%s> has empty name field." ),
                      GetChars( aLibrary->GetName() ) );
        return false;
    }

    wxString cmpName = m_aliasName = aEntry->GetName();

    LIB_PART* lib_part = aEntry->GetPart();

    wxASSERT( lib_part );

    wxLogDebug( wxT( "\"<%s>\" is alias of \"<%s>\"" ),
                GetChars( cmpName ),
                GetChars( lib_part->GetName() ) );

    LIB_PART* part = new LIB_PART( *lib_part );      // clone it and own it.
    SetCurPart( part );
    m_aliasName = aEntry->GetName();

    m_convert = 1;

    m_showDeMorgan = false;

    if( part->HasConversion() )
        m_showDeMorgan = true;

    GetScreen()->ClrModify();
    DisplayLibInfos();
    UpdateAliasSelectList();
    UpdatePartSelectList();

    // Display the document information based on the entry selected just in
    // case the entry is an alias.
    DisplayCmpDoc();

    return true;
}


void LIB_EDIT_FRAME::RedrawComponent( wxDC* aDC, wxPoint aOffset  )
{
    LIB_PART*      part = GetCurPart();

    if( part )
    {
        printf("redrawC %p\n", part);

        // display reference like in schematic (a reference U is shown U? or U?A)
        // although it is stored without ? and part id.
        // So temporary change the reference by a schematic like reference
        LIB_FIELD*  field = part->GetField( REFERENCE );
        wxString    fieldText = field->GetText();
        wxString    fieldfullText = field->GetFullText( m_unit );

        field->EDA_TEXT::SetText( fieldfullText );  // change the field text string only
        part->Draw( m_canvas, aDC, aOffset, m_unit, m_convert, GR_DEFAULT_DRAWMODE );
        field->EDA_TEXT::SetText( fieldText );      // restore the field text string
    }
}

void LIB_EDIT_FRAME::RedrawActiveWindow( wxDC* aDC, bool EraseBg )
{
    if( GetScreen() == NULL )
        return;

    m_canvas->DrawBackGround( aDC );

    RedrawComponent( aDC, wxPoint( 0, 0 ) );

#ifdef USE_WX_OVERLAY
    if( IsShown() )
    {
        m_overlay.Reset();
        wxDCOverlay overlaydc( m_overlay, (wxWindowDC*)aDC );
        overlaydc.Clear();
    }
#endif

    if( m_canvas->IsMouseCaptured() )
        m_canvas->CallMouseCapture( aDC, wxDefaultPosition, false );

    m_canvas->DrawCrossHair( aDC );

    DisplayLibInfos();
    UpdateStatusBar();
}


void LIB_EDIT_FRAME::SaveAllLibraries( wxCommandEvent& aEvent )
{
    PROJECT& prj = Prj();
    PART_LIBS* libs = prj.SchLibs();
    wxArrayString   lib_names;
    wxString        lib_paths;
    if( GetCurPart() )
    {
        for( PART_LIB& lib : *libs )
        {
            if( lib.GetName().CompareTo(m_my_part_lib) == 0 )
            {
                lib.AddPart( m_my_part );
                SetCurPart( NULL );
                break;
            }
        }
    }
    for( PART_LIB& lib : *libs )
    {
        if( lib.IsModified() )
        {
            SaveLibrary( &lib );
        }
    }
    for( auto elem : m_newLibs )
    {
        SaveLibrary( elem );
        try
        {
            PART_LIBS::LibNamesAndPaths( &prj, false, &lib_paths, &lib_names );
            wxFileName fn = elem->GetFullFileName();
            wxStringTokenizer tokenizer( lib_paths, ";" );
            bool match = false;
            while( !match && tokenizer.HasMoreTokens() )
                match = tokenizer.GetNextToken() == fn.GetPath();
            if( !match )
            {
                if( !lib_paths.empty() )
                    lib_paths.Append(";");
                lib_paths.Append(fn.GetPath());
            }
            lib_names.Add( fn.GetName() );
            PART_LIBS::LibNamesAndPaths( &prj, true, &lib_paths, &lib_names );
            PART_LIB* tmp = libs->AddLibrary( fn.GetFullPath() );
            if( tmp )
                tmp->SetCache();
        }
        catch ( const IO_ERROR& ioe)
        {
            DBG( printf("%s: %s\n", __func__, TO_UTF8( ioe.errorText )); )
            return;
        }
    }
    m_newLibs.clear();
}


bool LIB_EDIT_FRAME::SaveLibrary( PART_LIB* aLibrary )
{
    wxFileName fn;
    wxString   msg;

    m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor() );

    PART_LIB* lib = aLibrary ? aLibrary : GetCurLib();

    if( !lib )
    {
        DisplayError( this, _( "No library specified." ) );
        return false;
    }

    if( GetScreen()->IsModify() )
    {
        if( IsOK( this, _( "Include last component changes?" ) ) )
            SaveOnePart( GetCurPart(), false );
    }

    fn = aLibrary->GetFullFileName();

    // Verify the user has write privileges before attempting to
    // save the library file.
    if( !IsWritable( fn ) )
        return false;

    ClearMsgPanel();

    wxFileName libFileName = fn;
    wxFileName backupFileName = fn;

    // Rename the old .lib file to .bak.
    if( libFileName.FileExists() )
    {
        backupFileName.SetExt( wxT( "bak" ) );

        if( backupFileName.FileExists() )
            wxRemoveFile( backupFileName.GetFullPath() );

        if( !wxRenameFile( libFileName.GetFullPath(), backupFileName.GetFullPath() ) )
        {
            libFileName.MakeAbsolute();
            msg = wxT( "Failed to rename old component library file " ) +
                  backupFileName.GetFullPath();
            DisplayError( this, msg );
        }
    }

    try
    {
        FILE_OUTPUTFORMATTER    libFormatter( libFileName.GetFullPath() );

        if( !lib->Save( libFormatter ) )
        {
            msg.Printf( _( "Error occurred while saving library file '%s'" ),
                        GetChars( fn.GetFullPath() ) );
            AppendMsgPanel( _( "*** ERROR: ***" ), msg, RED );
            DisplayError( this, msg );
            return false;
        }
    }
    catch( ... /* IO_ERROR ioe */ )
    {
        libFileName.MakeAbsolute();
        msg.Printf( _( "Failed to create component library file '%s'" ),
                    GetChars( libFileName.GetFullPath() ) );
        DisplayError( this, msg );
        return false;
    }

    wxFileName docFileName = libFileName;

    docFileName.SetExt( DOC_EXT );

    // Rename .doc file to .bck.
    if( docFileName.FileExists() )
    {
        backupFileName.SetExt( wxT( "bck" ) );

        if( backupFileName.FileExists() )
            wxRemoveFile( backupFileName.GetFullPath() );

        if( !wxRenameFile( docFileName.GetFullPath(), backupFileName.GetFullPath() ) )
        {
            msg = wxT( "Failed to save old library document file " ) +
                  backupFileName.GetFullPath();
            DisplayError( this, msg );
        }
    }

    try
    {
        FILE_OUTPUTFORMATTER    docFormatter( docFileName.GetFullPath() );

        if( !lib->SaveDocs( docFormatter ) )
        {
            msg.Printf( _( "Error occurred while saving library documentation file <%s>" ),
                        GetChars( docFileName.GetFullPath() ) );
            AppendMsgPanel( _( "*** ERROR: ***" ), msg, RED );
            DisplayError( this, msg );
            return false;
        }
    }
    catch( ... /* IO_ERROR ioe */ )
    {
        docFileName.MakeAbsolute();
        msg.Printf( _( "Failed to create component document library file <%s>" ),
                    GetChars( docFileName.GetFullPath() ) );
        DisplayError( this, msg );
        return false;
    }

    msg.Printf( _( "Library file '%s' OK" ), GetChars( fn.GetFullName() ) );
    fn.SetExt( DOC_EXT );
    wxString msg1;
    msg1.Printf( _( "Documentation file '%s' OK" ), GetChars( fn.GetFullPath() ) );
    AppendMsgPanel( msg, msg1, BLUE );

    return true;
}


void LIB_EDIT_FRAME::DisplayCmpDoc()
{
    LIB_ALIAS*      alias;
    PART_LIB*    lib = GetCurLib();
    LIB_PART*       part = GetCurPart();

    ClearMsgPanel();

    if( !lib || !part )
        return;

    wxString msg = part->GetName();

    AppendMsgPanel( _( "Name" ), msg, BLUE, 8 );

    if( m_aliasName == part->GetName() )
        msg = _( "None" );
    else
        msg = m_aliasName;

    alias = part->GetAlias( m_aliasName );

    wxCHECK_RET( alias != NULL, wxT( "Alias not found in component." ) );

    AppendMsgPanel( _( "Alias" ), msg, RED, 8 );

    static wxChar UnitLetter[] = wxT( "?ABCDEFGHIJKLMNOPQRSTUVWXYZ" );
    msg = UnitLetter[m_unit];

    AppendMsgPanel( _( "Unit" ), msg, BROWN, 8 );

    if( m_convert > 1 )
        msg = _( "Convert" );
    else
        msg = _( "Normal" );

    AppendMsgPanel( _( "Body" ), msg, GREEN, 8 );

    if( part->IsPower() )
        msg = _( "Power Symbol" );
    else
        msg = _( "Part" );

    AppendMsgPanel( _( "Type" ), msg, MAGENTA, 8 );
    AppendMsgPanel( _( "Description" ), alias->GetDescription(), CYAN, 8 );
    AppendMsgPanel( _( "Key words" ), alias->GetKeyWords(), DARKDARKGRAY );
    AppendMsgPanel( _( "Datasheet" ), alias->GetDocFileName(), DARKDARKGRAY );
}


void LIB_EDIT_FRAME::DeleteOnePart( wxCommandEvent& aEvent )
{
    wxString      cmp_name;
    LIB_ALIAS*    libEntry;
    wxArrayString nameList;
    wxString      msg;

    m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor() );

    m_lastDrawItem = NULL;
    m_drawItem = NULL;

    PART_LIB* lib = GetCurLib();

    if( !lib )
    {
        SelectActiveLibrary();

        lib = GetCurLib();
        if( !lib )
        {
            DisplayError( this, _( "Please select a component library." ) );
            return;
        }
    }

    lib->GetEntryNames( nameList );

    if( nameList.IsEmpty() )
    {
        msg.Printf( _( "Part library '%s' is empty." ), GetChars( lib->GetName() ) );
        wxMessageBox( msg, _( "Delete Entry Error" ), wxID_OK | wxICON_EXCLAMATION, this );
        return;
    }

    msg.Printf( _( "Select one of %d components to delete\nfrom library '%s'." ),
                int( nameList.GetCount() ),
                GetChars( lib->GetName() ) );

    wxSingleChoiceDialog dlg( this, msg, _( "Delete Part" ), nameList );

    if( dlg.ShowModal() == wxID_CANCEL || dlg.GetStringSelection().IsEmpty() )
        return;

    libEntry = lib->FindEntry( dlg.GetStringSelection() );

    if( !libEntry )
    {
        msg.Printf( _( "Entry '%s' not found in library '%s'." ),
                    GetChars( dlg.GetStringSelection() ),
                    GetChars( lib->GetName() ) );
        DisplayError( this, msg );
        return;
    }

    msg.Printf( _( "Delete component '%s' from library '%s' ?" ),
                GetChars( libEntry->GetName() ),
                GetChars( lib->GetName() ) );

    if( !IsOK( this, msg ) )
        return;

    LIB_PART* part = GetCurPart();

    if( !part || !part->HasAlias( libEntry->GetName() ) )
    {
        lib->RemoveEntry( libEntry );
        return;
    }

    // If deleting the current entry or removing one of the aliases for
    // the current entry, sync the changes in the current entry as well.

    if( GetScreen()->IsModify() && !IsOK( this, _(
        "The component being deleted has been modified."
        " All changes will be lost. Discard changes?" ) ) )
    {
        return;
    }

    LIB_ALIAS* nextEntry = lib->RemoveEntry( libEntry );

    if( nextEntry != NULL )
    {
        if( LoadOneLibraryPartAux( nextEntry, lib ) )
            Zoom_Automatique( false );
    }
    else
    {
        SetCurPart( NULL );     // delete CurPart
        m_aliasName.Empty();
    }

    m_canvas->Refresh();
}



void LIB_EDIT_FRAME::CreateNewLibraryPart( wxCommandEvent& aEvent )
{

    if( GetCurPart() && GetScreen()->IsModify() && !IsOK( this, _(
        "All changes to the current component will be lost!\n\n"
        "Clear the current component from the screen?" ) ) )
    {
        return;
    }

    m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor() );

    m_drawItem = NULL;

    DIALOG_LIB_NEW_COMPONENT dlg( this );

    dlg.SetMinSize( dlg.GetSize() );

    if( dlg.ShowModal() == wxID_CANCEL )
        return;

    if( dlg.GetName().IsEmpty() )
    {
        wxMessageBox( _( "This new component has no name and cannot be created. Aborted." ), _("Error"), wxOK | wxICON_ERROR | wxCENTRE  );
        return;
    }

    wxString name = dlg.GetName();
    name.Replace( wxT( " " ), wxT( "_" ) );

    PART_LIB* lib = GetCurLib();
    if(!lib)
    {
        wxMessageBox( _( "Library not found. Aborted." ), _("Error"), wxOK | wxICON_ERROR | wxCENTRE  );
        return;
    }

    // Test if there a component with this name already.
    if( lib && lib->FindEntry( name ) )
    {
        wxString msg = wxString::Format( _(
            "Part '%s' already exists in library '%s'" ),
            GetChars( name ),
            GetChars( lib->GetName() )
            );
        DisplayError( this, msg );
        return;
    }

    m_my_part_lib = lib->GetName();
    m_panelTree->InsertAsChildOf(name, lib->GetName());
    m_panelTree->SelectItem( name, lib->GetName());
    LIB_PART* new_part = new LIB_PART( name );
    m_is_new = true;

    SetCurPart( new_part );
    m_aliasName = new_part->GetName();

    new_part->GetReferenceField().SetText( dlg.GetReference() );
    new_part->SetUnitCount( dlg.GetUnitCount() );

    // Initialize new_part->m_TextInside member:
    // if 0, pin text is outside the body (on the pin)
    // if > 0, pin text is inside the body
    new_part->SetConversion( dlg.GetAlternateBodyStyle() );
    SetShowDeMorgan( dlg.GetAlternateBodyStyle() );

    if( dlg.GetPinNameInside() )
    {
        new_part->SetPinNameOffset( dlg.GetPinTextPosition() );

        if( new_part->GetPinNameOffset() == 0 )
            new_part->SetPinNameOffset( 1 );
    }
    else
    {
        new_part->SetPinNameOffset( 0 );
    }

    ( dlg.GetPowerSymbol() ) ? new_part->SetPower() : new_part->SetNormal();
    new_part->SetShowPinNumbers( dlg.GetShowPinNumber() );
    new_part->SetShowPinNames( dlg.GetShowPinName() );
    new_part->LockUnits( dlg.GetLockItems() );

    if( dlg.GetUnitCount() < 2 )
        new_part->LockUnits( false );

    m_unit = 1;
    m_convert  = 1;

    DisplayLibInfos();
    DisplayCmpDoc();
    UpdateAliasSelectList();
    UpdatePartSelectList();

    m_editPinsPerPartOrConvert = new_part->UnitsLocked() ? true : false;
    m_lastDrawItem = NULL;

    GetScreen()->ClearUndoRedoList();
    OnModify();

    m_canvas->Refresh();
    m_mainToolBar->Refresh();
    GetScreen()->SetModify();
}


bool LIB_EDIT_FRAME::SaveOnePart(LIB_PART* aToSave, bool aPromptUser )
{
    assert(aToSave);
    wxString    msg;
    PART_LIB* lib = GetCurLib();

    if( !lib )
    {
        DisplayError( this, _( "No library specified." ) );
        return false;
    }
    GetScreen()->ClrModify();

    LIB_PART* old_part = lib->FindPart( aToSave->GetName() );

    if( old_part && aPromptUser )
    {
        msg.Printf( _( "Part '%s' already exists. Change it?" ),
                    GetChars( aToSave->GetName() ) );

        if( !IsOK( this, msg ) )
            return false;
    }

    m_drawItem = m_lastDrawItem = NULL;

    if( old_part )
        lib->ReplacePart( old_part, aToSave );
    else
        lib->AddPart( aToSave );

    msg.Printf( _( "Part '%s' saved in library '%s'" ),
                GetChars( aToSave->GetName() ),
                GetChars( lib->GetName() ) );

    SetStatusText( msg );

    return true;
}
