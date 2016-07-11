/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) CERN 2016 Michele Castellana, <michele.castellana@cern.ch>
 * Copyright (C) 2016 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2011-2016 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2016 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file cvframe.cpp
 */

#include <wx/list.h>
#include <fctsys.h>
#include <build_version.h>
#include <kiway_express.h>
#include <pgm_base.h>
#include <kiface_i.h>
#include <macros.h>
#include <confirm.h>
#include <eda_doc.h>
#include <eda_dde.h>
#include <gestfich.h>
#include <html_messagebox.h>
#include <wildcards_and_files_ext.h>
#include <fp_lib_table.h>
#include <netlist_reader.h>

#include <cvpcb_mainframe.h>
#include <cvpcb.h>
#include <listview_classes.h>
#include <invoke_pcb_dialog.h>
#include <class_DisplayFootprintsFrame.h>
#include <eda_pattern_match.h>
#include <cvpcb_id.h>
#include <dialog_footprints_tree.h>


#define FRAME_MIN_SIZE_X 450
#define FRAME_MIN_SIZE_Y 300


///@{
/// \ingroup config

/// Nonzero iff cvpcb should be kept open after saving files
static const wxString KeepCvpcbOpenEntry = "KeepCvpcbOpen";

static const wxString FootprintDocFileEntry = "footprints_doc_file";

static const wxString FilterFootprintEntry = "FilterFootprint";
///@}

BEGIN_EVENT_TABLE( CVPCB_MAINFRAME, KIWAY_PLAYER )

    // Menu events
    EVT_MENU( wxID_SAVE, CVPCB_MAINFRAME::SaveQuitCvpcb )
    EVT_MENU( wxID_EXIT, CVPCB_MAINFRAME::OnQuit )
    EVT_MENU( wxID_HELP, CVPCB_MAINFRAME::GetKicadHelp )
    EVT_MENU( wxID_ABOUT, CVPCB_MAINFRAME::GetKicadAbout )
    EVT_MENU( ID_PREFERENCES_CONFIGURE_PATHS, CVPCB_MAINFRAME::OnConfigurePaths )
    EVT_MENU( ID_CVPCB_CONFIG_KEEP_OPEN_ON_SAVE, CVPCB_MAINFRAME::OnKeepOpenOnSave )
    EVT_MENU( ID_CVPCB_EQUFILES_LIST_EDIT, CVPCB_MAINFRAME::OnEditEquFilesList )

    // Toolbar events
    EVT_TOOL( ID_CVPCB_QUIT, CVPCB_MAINFRAME::OnQuit )

    EVT_TOOL( ID_CVPCB_LIB_TABLE_EDIT, CVPCB_MAINFRAME::OnEditFootprintLibraryTable )
    EVT_TOOL( ID_CVPCB_CREATE_SCREENCMP, CVPCB_MAINFRAME::DisplayModule )
    EVT_TOOL( ID_CVPCB_GOTO_FIRSTNA, CVPCB_MAINFRAME::ToFirstNA )
    EVT_TOOL( ID_CVPCB_GOTO_PREVIOUSNA, CVPCB_MAINFRAME::ToPreviousNA )
    EVT_TOOL( ID_CVPCB_DEL_ASSOCIATIONS, CVPCB_MAINFRAME::DelAssociations )
    EVT_TOOL( ID_CVPCB_AUTO_ASSOCIE, CVPCB_MAINFRAME::AutomaticFootprintMatching )
    EVT_TOOL( ID_PCB_DISPLAY_FOOTPRINT_DOC, CVPCB_MAINFRAME::DisplayDocFile )
    EVT_TOOL( ID_CVPCB_FOOTPRINT_DISPLAY_FILTERED_LIST,
              CVPCB_MAINFRAME::OnSelectFilteringFootprint )
    EVT_TOOL( ID_CVPCB_FOOTPRINT_DISPLAY_PIN_FILTERED_LIST,
              CVPCB_MAINFRAME::OnSelectFilteringFootprint )
    EVT_TOOL( ID_CVPCB_FOOTPRINT_DISPLAY_BY_LIBRARY_LIST,
              CVPCB_MAINFRAME::OnSelectFilteringFootprint )
    EVT_TOOL( ID_CVPCB_FOOTPRINT_DISPLAY_BY_NAME,
              CVPCB_MAINFRAME::OnSelectFilteringFootprint )
    EVT_TEXT( ID_CVPCB_FILTER_TEXT_EDIT, CVPCB_MAINFRAME::OnEnterFilteringText )

    // Frame events
    EVT_CLOSE( CVPCB_MAINFRAME::OnCloseWindow )
    EVT_SIZE( CVPCB_MAINFRAME::OnSize )

    // UI event handlers
    EVT_UPDATE_UI( ID_CVPCB_CONFIG_KEEP_OPEN_ON_SAVE, CVPCB_MAINFRAME::OnUpdateKeepOpenOnSave )
    EVT_UPDATE_UI( ID_CVPCB_FOOTPRINT_DISPLAY_FILTERED_LIST, CVPCB_MAINFRAME::OnFilterFPbyKeywords)
    EVT_UPDATE_UI( ID_CVPCB_FOOTPRINT_DISPLAY_PIN_FILTERED_LIST, CVPCB_MAINFRAME::OnFilterFPbyPinCount )
    EVT_UPDATE_UI( ID_CVPCB_FOOTPRINT_DISPLAY_BY_NAME, CVPCB_MAINFRAME::OnFilterFPbyKeyName )

END_EVENT_TABLE()


CVPCB_MAINFRAME::CVPCB_MAINFRAME( KIWAY* aKiway, wxWindow* aParent ) :
    KIWAY_PLAYER( aKiway, aParent, FRAME_CVPCB, wxT( "CvPCB" ), wxDefaultPosition,
        wxDefaultSize, KICAD_DEFAULT_DRAWFRAME_STYLE, wxT( "CvpcbFrame" ) )
{
    m_compListBox           = nullptr;
    m_panelTree             = nullptr;
    m_mainToolBar           = nullptr;
    m_modified              = false;
    m_keepCvpcbOpen         = false;
    m_undefinedComponentCnt = 0;
    m_skipComponentSelect   = false;
    m_filteringOptions      = 0;
    m_tcFilterString        = nullptr;

    /* Name of the document footprint list
     * usually located in share/modules/footprints_doc
     * this is of the responsibility to users to create this file
     * if they want to have a list of footprints
     */
    m_DocModulesFileName = DEFAULT_FOOTPRINTS_LIST_FILENAME;

    // Give an icon
    wxIcon icon;
    icon.CopyFromBitmap( KiBitmap( icon_cvpcb_xpm ) );
    SetIcon( icon );

    SetAutoLayout( true );

    LoadSettings( config() );

    if( m_FrameSize.x < FRAME_MIN_SIZE_X )
        m_FrameSize.x = FRAME_MIN_SIZE_X;

    if( m_FrameSize.y < FRAME_MIN_SIZE_Y )
        m_FrameSize.y = FRAME_MIN_SIZE_Y;

    // Set minimal frame width and height
    SetSizeHints( FRAME_MIN_SIZE_X, FRAME_MIN_SIZE_Y, -1, -1, -1, -1 );

    // Frame size and position
    SetSize( m_FramePos.x, m_FramePos.y, m_FrameSize.x, m_FrameSize.y );

    // create the status bar
    static const int dims[3] = { -1, -1, 250 };

    CreateStatusBar( 3 );
    SetStatusWidths( 3, dims );

    ReCreateMenuBar();
    ReCreateHToolbar();

    // Create list of available modules and components of the schematic
    BuildCmpListBox();
    BuildFOOTPRINTS_TREE();

    m_auimgr.SetManagedWindow( this );

    UpdateTitle();

    EDA_PANEINFO horiz;
    horiz.HorizontalToolbarPane();

    EDA_PANEINFO info;
    info.InfoToolbarPane();

    if( m_mainToolBar )
        m_auimgr.AddPane( m_mainToolBar,
                          wxAuiPaneInfo( horiz ).Name( wxT( "m_mainToolBar" ) ).Top() );

    if( m_compListBox )
        m_auimgr.AddPane( m_compListBox,
                          wxAuiPaneInfo( horiz ).Name( wxT( "m_compListBox" ) ).
                          CentrePane() );

    if( m_panelTree )
        m_auimgr.AddPane( m_panelTree,
                          wxAuiPaneInfo( horiz ).Name( wxT( "m_panelTree" ) ).
                          Right().BestSize( (int) (m_FrameSize.x * 0.3 ), m_FrameSize.y ) );

    m_auimgr.Update();
}


CVPCB_MAINFRAME::~CVPCB_MAINFRAME()
{
    m_auimgr.UnInit();
}


void CVPCB_MAINFRAME::LoadSettings( wxConfigBase* aCfg )
{
    EDA_BASE_FRAME::LoadSettings( aCfg );

    aCfg->Read( KeepCvpcbOpenEntry, &m_keepCvpcbOpen, true );
    aCfg->Read( FootprintDocFileEntry, &m_DocModulesFileName,
                DEFAULT_FOOTPRINTS_LIST_FILENAME );
    aCfg->Read( FilterFootprintEntry, &m_filteringOptions, FOOTPRINTS_LISTBOX::UNFILTERED_FP_LIST );
}


void CVPCB_MAINFRAME::SaveSettings( wxConfigBase* aCfg )
{
    EDA_BASE_FRAME::SaveSettings( aCfg );

    aCfg->Write( KeepCvpcbOpenEntry, m_keepCvpcbOpen );
    aCfg->Write( FootprintDocFileEntry, m_DocModulesFileName );
    aCfg->Write( FilterFootprintEntry, m_filteringOptions );
}


void CVPCB_MAINFRAME::OnSize( wxSizeEvent& event )
{
    event.Skip();
}


void CVPCB_MAINFRAME::OnQuit( wxCommandEvent& event )
{
    Close( false );
}


void CVPCB_MAINFRAME::OnCloseWindow( wxCloseEvent& Event )
{
    if( m_modified )
    {
        wxString msg = _( "Component to Footprint links modified.\nSave before exit ?" );
        int ii = DisplayExitDialog( this, msg );

        switch( ii )
        {
        case wxID_CANCEL:
            Event.Veto();
            return;

        case wxID_NO:
            break;

        case wxID_YES:
            SaveFootprintAssociation();
            break;
        }
    }

    // Close module display frame
    if( GetFootprintViewerFrame() )
        GetFootprintViewerFrame()->Close( true );

    m_modified = false;

    Destroy();
    return;
}


void CVPCB_MAINFRAME::ChangeFocus( bool aMoveRight )
{
    wxWindow* hasFocus = wxWindow::FindFocus();

    if( aMoveRight && hasFocus == m_compListBox )
       m_panelTree->m_tree->SetFocus();
    else if( !aMoveRight && hasFocus == m_panelTree->m_tree )
       m_compListBox->SetFocus();
}


void CVPCB_MAINFRAME::ToFirstNA( wxCommandEvent& event )
{
    if( m_netlist.IsEmpty() )
        return;

    long first_selected = m_compListBox->GetFirstSelected();

    if( first_selected < 0 )
        first_selected = -1;     // We will start to 0 for the first search , if no item selected

    int candidate = -1;

    for( unsigned jj = first_selected+1; jj < m_netlist.GetCount(); ++jj )
    {
        if( m_netlist.GetComponent( jj )->GetFPID().empty() )
        {
            candidate = jj;
            break;
        }
    }

    if( candidate >= 0 )
    {
        m_compListBox->DeselectAll();
        m_compListBox->SetSelection( candidate );
        SendMessageToEESCHEMA();
    }
}


void CVPCB_MAINFRAME::ToPreviousNA( wxCommandEvent& event )
{
    if( m_netlist.IsEmpty() )
        return;

    int first_selected = m_compListBox->GetFirstSelected();

    if( first_selected < 0 )
        first_selected = m_compListBox->GetCount();

    int candidate = -1;

    for( int jj = first_selected-1; jj >= 0; jj-- )
    {
        if( m_netlist.GetComponent( jj )->GetFPID().empty() )
        {
            candidate = jj;
            break;
        }
    }

    if( candidate >= 0 )
    {
        m_compListBox->DeselectAll();
        m_compListBox->SetSelection( candidate );
        SendMessageToEESCHEMA();
    }
}


void CVPCB_MAINFRAME::SaveQuitCvpcb( wxCommandEvent& aEvent )
{
    SaveFootprintAssociation();

    m_modified = false;

    if( !m_keepCvpcbOpen )
        Close( true );
}


void CVPCB_MAINFRAME::DelAssociations( wxCommandEvent& event )
{
    if( IsOK( this, _( "Delete selections" ) ) )
    {
        m_skipComponentSelect = true;

        // Remove all selections to avoid issues when setting the fpids
        m_compListBox->DeselectAll();

        for( unsigned i = 0;  i < m_netlist.GetCount();  ++i )
        {
            FPID fpid;

            m_netlist.GetComponent( i )->SetFPID( fpid );
            SetNewPkg( wxEmptyString );
        }

        // Remove all selections after setting the fpids
        m_compListBox->DeselectAll();

        m_skipComponentSelect = false;
        m_compListBox->SetSelection( 0 );
        m_undefinedComponentCnt = m_netlist.GetCount();
    }

    DisplayStatus();
}


bool CVPCB_MAINFRAME::OpenProjectFiles( const std::vector<wxString>& aFileSet, int aCtl )
{
    return true;
}


void CVPCB_MAINFRAME::OnEditFootprintLibraryTable( wxCommandEvent& aEvent )
{
    bool    tableChanged = false;
    int     r = InvokePcbLibTableEditor( this, &GFootprintTable, Prj().PcbFootprintLibs() );

    if( r & 1 )
    {
        wxString fileName = FP_LIB_TABLE::GetGlobalTableFileName();

        try
        {
            GFootprintTable.Save( fileName );
            tableChanged = true;
        }
        catch( const IO_ERROR& ioe )
        {
            wxString msg = wxString::Format(
                    _( "Error occurred saving the global footprint library table:\n'%s'\n%s" ),
                    GetChars( fileName ),
                    GetChars( ioe.errorText )
                    );
            wxMessageBox( msg, _( "File Save Error" ), wxOK | wxICON_ERROR );
        }
    }

    if( r & 2 )
    {
        wxString fileName = Prj().FootprintLibTblName();

        try
        {
            Prj().PcbFootprintLibs()->Save( fileName );
            tableChanged = true;
        }
        catch( const IO_ERROR& ioe )
        {
            wxString msg = wxString::Format(
                    _( "Error occurred saving the project footprint library table:\n'%s'\n%s" ),
                    GetChars( fileName ),
                    GetChars( ioe.errorText )
                    );
            wxMessageBox( msg, _( "File Save Error" ), wxOK | wxICON_ERROR );
        }
    }

    if( tableChanged )
    {
        wxBusyCursor dummy;
        BuildFOOTPRINTS_TREE();
        m_FootprintsList.ReadFootprintFiles( Prj().PcbFootprintLibs() );
    }
}


void CVPCB_MAINFRAME::OnKeepOpenOnSave( wxCommandEvent& event )
{
    m_keepCvpcbOpen = event.IsChecked();
}


void CVPCB_MAINFRAME::DisplayModule( wxCommandEvent& event )
{
    CreateScreenCmp();
    GetFootprintViewerFrame()->RedrawScreen( wxPoint( 0, 0 ), false );
}


void CVPCB_MAINFRAME::DisplayDocFile( wxCommandEvent& event )
{
    GetAssociatedDocument( this, m_DocModulesFileName, &Kiface().KifaceSearch() );
}


void CVPCB_MAINFRAME::OnSelectComponent( wxListEvent& event )
{
    if( m_skipComponentSelect )
        return;

    COMPONENT* component = GetSelectedComponent();

    SetFootprints( m_FootprintsList, component, \
                                       m_currentSearchPattern, m_filteringOptions);

    refreshAfterComponentSearch(component);
}

void CVPCB_MAINFRAME::SetFootprints( FOOTPRINT_LIST& aList, COMPONENT* aComponent,
                                        const wxString &aFootPrintFilterPattern,
                                        int aFilterType )
{
    std::map<wxString, wxArrayString>   newList;
    wxString        msg;
    wxString        oldSelection;

    EDA_PATTERN_MATCH_WILDCARD patternFilter;
    patternFilter.SetPattern( aFootPrintFilterPattern.Lower() );    // Use case insensitive search

    for( unsigned ii = 0, j = 1; ii < aList.GetCount(); ++j, ++ii )
    {
        if( (aFilterType & FOOTPRINTS_LISTBOX::FILTERING_BY_COMPONENT_KEYWORD) && aComponent
            && !aComponent->MatchesFootprintFilters( aList.GetItem( ii ).GetFootprintName() ) )
            continue;

        if( (aFilterType & FOOTPRINTS_LISTBOX::FILTERING_BY_PIN_COUNT) && aComponent
            && aComponent->GetNetCount() != aList.GetItem( ii ).GetUniquePadCount() )
            continue;

        // We can search (Using case insensitive search) in full FPID or only
        // in the fp name itself.
        // After tests, only in the fp name itself looks better.
        // However, the code to take in account the nickname is just commented, no removed.
        wxString currname = //aList.GetItem( ii ).GetNickname().Lower() + ":" +
                            aList.GetItem( ii ).GetFootprintName().Lower();
        assert(!currname.IsEmpty());

        if( (aFilterType & FOOTPRINTS_LISTBOX::FILTERING_BY_NAME) && !aFootPrintFilterPattern.IsEmpty()
            && patternFilter.Find( currname ) == EDA_PATTERN_NOT_FOUND )
            continue;

        msg.Printf( wxT( "%3u %s:%s" ), j,
                    GetChars( aList.GetItem( ii ).GetNickname() ),
                    GetChars( aList.GetItem( ii ).GetFootprintName() ) );
        newList[aList.GetItem(ii).GetNickname()].Add( msg );
    }

    m_panelTree->m_tree->DeleteAllItems();
    m_panelTree->m_tree->AddRoot("Hidden root");

    for( const auto item : newList ) {
       wxTreeItemId root = m_panelTree->m_tree->GetRootItem();
       assert( root.IsOk() );
       wxTreeItemId libroot = m_panelTree->m_tree->InsertItem( root, 0, item.first );
       assert( libroot.IsOk() );
       for( const auto foot : item.second )
          m_panelTree->m_tree->InsertItem( libroot, 0, foot );
    }
}

void CVPCB_MAINFRAME::refreshAfterComponentSearch( COMPONENT* component )
{
    // Tell AuiMgr that objects are changed !
    if( m_auimgr.GetManagedWindow() )   // Be sure Aui Manager is initialized
                                        // (could be not the case when starting CvPcb
        m_auimgr.Update();

    if( !component )
        return;

    // Preview of the already assigned footprint.
    // Find the footprint that was already chosen for this component and select it,
    // but only if the selection is made from the component list or the library list.
    // If the selection is made from the footprint list, do not change the current
    // selected footprint.
    if( FindFocus() == m_compListBox )
    {
        wxString module = FROM_UTF8( component->GetFPID().Format().c_str() );

        bool found = false;

        wxTreeItemId root = m_panelTree->m_tree->GetRootItem();
        assert(root.IsOk());
        wxTreeItemIdValue cookie;
        for( wxTreeItemId ii = m_panelTree->m_tree->GetFirstChild(root, cookie); ii.IsOk();
              ii = m_panelTree->m_tree->GetNextChild(root, cookie) )
        {
            if( m_panelTree->FindItem( ii, module ).IsOk() )
            {
                found = true;
                //m_panelTree->m_tree->SelectItem( ii, true );
                break;
            }
        }

        if( !found )
        {
            if( GetFootprintViewerFrame() )
            {
                CreateScreenCmp();
            }
        }
    }

    SendMessageToEESCHEMA();
    DisplayStatus();
}

void CVPCB_MAINFRAME::OnSelectFilteringFootprint( wxCommandEvent& event )
{
    int option = 0;

    switch( event.GetId() )
    {
    case ID_CVPCB_FOOTPRINT_DISPLAY_FILTERED_LIST:
        option = FOOTPRINTS_LISTBOX::FILTERING_BY_COMPONENT_KEYWORD;
        break;

    case ID_CVPCB_FOOTPRINT_DISPLAY_PIN_FILTERED_LIST:
        option = FOOTPRINTS_LISTBOX::FILTERING_BY_PIN_COUNT;
        break;

    case ID_CVPCB_FOOTPRINT_DISPLAY_BY_NAME:
        m_currentSearchPattern = m_tcFilterString->GetValue();
        option = FOOTPRINTS_LISTBOX::FILTERING_BY_NAME;
        break;
    default:
        break;
    }

    if( event.IsChecked() )
        m_filteringOptions |= option;
    else
        m_filteringOptions &= ~option;

    wxListEvent l_event;
    OnSelectComponent( l_event );
}


void CVPCB_MAINFRAME::OnUpdateKeepOpenOnSave( wxUpdateUIEvent& event )
{
    event.Check( m_keepCvpcbOpen );
}


void CVPCB_MAINFRAME::OnFilterFPbyKeywords( wxUpdateUIEvent& event )
{
    event.Check( m_filteringOptions & FOOTPRINTS_LISTBOX::FILTERING_BY_COMPONENT_KEYWORD );
}

void CVPCB_MAINFRAME::OnFilterFPbyPinCount( wxUpdateUIEvent& event )
{
    event.Check( m_filteringOptions & FOOTPRINTS_LISTBOX::FILTERING_BY_PIN_COUNT );
}

void CVPCB_MAINFRAME::OnFilterFPbyKeyName( wxUpdateUIEvent& event )
{
    event.Check( m_filteringOptions & FOOTPRINTS_LISTBOX::FILTERING_BY_NAME );
}


void CVPCB_MAINFRAME::OnEnterFilteringText( wxCommandEvent& aEvent )
{
    // Called when changing the filter string in main toolbar.
    // If the option FOOTPRINTS_LISTBOX::FILTERING_BY_NAME is set, update the list of
    // available footprints which match the filter

    m_currentSearchPattern = m_tcFilterString->GetValue();

    if( ( m_filteringOptions & FOOTPRINTS_LISTBOX::FILTERING_BY_NAME ) == 0 )
        return;

    OnSelectFilteringFootprint( aEvent );
}


void CVPCB_MAINFRAME::DisplayStatus()
{
    wxString   msg;
    COMPONENT* component;

    if( wxWindow::FindFocus() == m_compListBox ||
          wxWindow::FindFocus() == m_panelTree->m_tree ) {
        msg.Printf( _( "Components: %d, unassigned: %d" ), (int) m_netlist.GetCount(),
                    m_undefinedComponentCnt );
        SetStatusText( msg, 0 );

        msg.Empty();

        component = GetSelectedComponent();

        if( component ) {
            for( unsigned ii = 0;  ii < component->GetFootprintFilters().GetCount();  ++ii ) {
                if( msg.IsEmpty() )
                    msg += component->GetFootprintFilters()[ii];
                else
                    msg += wxT( ", " ) + component->GetFootprintFilters()[ii];
            }

            msg = _( "Filter list: " ) + msg;
        }

        SetStatusText( msg, 1 );
    } else {
        wxString footprintName = GetSelectedFootprint();

           FOOTPRINT_INFO* module =
              footprintName.IsEmpty() ? nullptr : m_FootprintsList.GetModuleInfo( footprintName );

           if( module )    // can be nullptr if no netlist loaded
           {
              msg = _( "Description: " ) + module->GetDoc();
              SetStatusText( msg, 0 );

              msg  = _( "Key words: " ) + module->GetKeywords();
              SetStatusText( msg, 1 );
           }
    }

    msg.Empty();
    wxString filters;

    if( m_panelTree )
    {
        if( ( m_filteringOptions & FOOTPRINTS_LISTBOX::FILTERING_BY_COMPONENT_KEYWORD ) )
            filters = _( "key words" );

        if( ( m_filteringOptions & FOOTPRINTS_LISTBOX::FILTERING_BY_PIN_COUNT ) )
        {
            if( !filters.IsEmpty() )
                filters += wxT( "+" );

            filters += _( "pin count" );
        }

        if( ( m_filteringOptions & FOOTPRINTS_LISTBOX::FILTERING_BY_LIBRARY ) )
        {
            if( !filters.IsEmpty() )
                filters += wxT( "+" );

            filters += _( "library" );
        }

        if( ( m_filteringOptions & FOOTPRINTS_LISTBOX::FILTERING_BY_NAME ) )
        {
            if( !filters.IsEmpty() )
                filters += wxT( "+" );

            filters += _( "name" );
        }

        if( filters.IsEmpty() )
            msg = _( "No filtering" );
        else
            msg.Printf( _( "Filtered by %s" ), GetChars( filters ) );

        msg << wxT( ": " ) << m_panelTree->m_tree->GetCount();

        SetStatusText( msg, 2 );
    }
}


bool CVPCB_MAINFRAME::LoadFootprintFiles()
{
    FP_LIB_TABLE* fptbl = Prj().PcbFootprintLibs();

    // Check if there are footprint libraries in the footprint library table.
    if( !fptbl || !fptbl->GetLogicalLibs().size() )
    {
        wxMessageBox( _( "No PCB footprint libraries are listed in the current footprint "
                         "library table." ), _( "Configuration Error" ), wxOK | wxICON_ERROR );
        return false;
    }

    {
    wxBusyCursor dummy;  // Let the user know something is happening.

    m_FootprintsList.ReadFootprintFiles( fptbl );
    }

    if( m_FootprintsList.GetErrorCount() )
    {
        m_FootprintsList.DisplayErrors( this );
    }

    return true;
}


void CVPCB_MAINFRAME::UpdateTitle()
{
    wxString    title = wxString::Format( wxT( "Cvpcb %s  " ), GetChars( GetBuildVersion() ) );
    PROJECT&    prj = Prj();
    wxFileName fn = prj.GetProjectFullName();

    if( fn.IsOk() && !prj.GetProjectFullName().IsEmpty() && fn.FileExists() )
    {
        title += wxString::Format( _("Project: '%s'"),
                                   GetChars( fn.GetFullPath() )
                                 );

        if( !fn.IsFileWritable() )
            title += _( " [Read Only]" );
    }
    else
        title += _( "[no project]" );

    SetTitle( title );
}


void CVPCB_MAINFRAME::SendMessageToEESCHEMA()
{
    if( m_netlist.IsEmpty() )
        return;

    int selection = m_compListBox->GetSelection();

    if ( selection < 0 )
        selection = 0;

    if( m_netlist.GetComponent( selection ) == nullptr )
        return;

    COMPONENT* component = m_netlist.GetComponent( selection );

    std::string packet = StrPrintf( "$PART: \"%s\"", TO_UTF8( component->GetReference() ) );

    if( Kiface().IsSingle() )
        SendCommand( MSG_TO_SCH, packet.c_str() );
    else
        Kiway().ExpressMail( FRAME_SCH, MAIL_CROSS_PROBE, packet, this );
}


int CVPCB_MAINFRAME::ReadSchematicNetlist( const std::string& aNetlist )
{
    STRING_LINE_READER*     strrdr = new STRING_LINE_READER( aNetlist, "Eeschema via Kiway" );
    KICAD_NETLIST_READER    netrdr( strrdr, &m_netlist );

    m_netlist.Clear();

    try
    {
        netrdr.LoadNetlist();
    }
    catch( const IO_ERROR& ioe )
    {
        wxString msg = wxString::Format( _( "Error loading netlist.\n%s" ), ioe.errorText.GetData() );
        wxMessageBox( msg, _( "Netlist Load Error" ), wxOK | wxICON_ERROR );
        return 1;
    }

    // We also remove footprint name if it is "$noname" because this is a dummy name,
    // not the actual name of the footprint.
    for( unsigned ii = 0; ii < m_netlist.GetCount(); ++ii )
    {
        if( m_netlist.GetComponent( ii )->GetFPID().GetFootprintName() == std::string( "$noname" ) )
            m_netlist.GetComponent( ii )->SetFPID( FPID( wxEmptyString ) );
    }

    // Sort components by reference:
    m_netlist.SortByReference();

    return 0;
}


void CVPCB_MAINFRAME::CreateScreenCmp()
{
    DISPLAY_FOOTPRINTS_FRAME* fpframe = GetFootprintViewerFrame();

    if( !fpframe )
    {
        fpframe = new DISPLAY_FOOTPRINTS_FRAME( &Kiway(), this );
        fpframe->Show( true );
    }
    else
    {
        if( fpframe->IsIconized() )
             fpframe->Iconize( false );

        // The display footprint window might be buried under some other
        // windows, so CreateScreenCmp() on an existing window would not
        // show any difference, leaving the user confused.
        // So we want to put it to front, second after our CVPCB_MAINFRAME.
        // We do this by a little dance of bringing it to front then the main
        // frame back.
        fpframe->Raise();   // Make sure that is visible.
        Raise();            // .. but still we want the focus.
    }

    fpframe->InitDisplay();
}


void CVPCB_MAINFRAME::BuildFOOTPRINTS_TREE()
{
    if( !m_panelTree )
       BuildLIBRARY_TREE();

   wxTreeItemId root = m_panelTree->m_tree->GetRootItem();
   assert(root.IsOk());
   for( auto item : m_FootprintsList.GetList() ) {
      wxTreeItemIdValue cookie;
      wxTreeItemId tmp = m_panelTree->m_tree->GetFirstChild( root, cookie);
      for( ; tmp.IsOk();
            tmp = m_panelTree->m_tree->GetNextChild( root, cookie) ) {
         if( item.InLibrary(m_panelTree->m_tree->GetItemText(tmp)) ) {
            m_panelTree->m_tree->InsertItem( tmp, 0, item.GetFootprintName() );
            break;
         }
      }
      if( !tmp.IsOk() ) {
         // TODO: error handling here
      }
   }
   DisplayStatus();
}


void CVPCB_MAINFRAME::BuildCmpListBox()
{
    wxString    msg;
    COMPONENT*  component;

    if( !m_compListBox )
    {
        wxFont guiFont = wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT );
        m_compListBox = new COMPONENTS_LISTBOX( this, ID_CVPCB_COMPONENT_LIST,
             wxDefaultPosition, wxDefaultSize );
        m_compListBox->SetFont( wxFont( guiFont.GetPointSize(),
                 wxFONTFAMILY_MODERN,
                 wxFONTSTYLE_NORMAL,
                 wxFONTWEIGHT_NORMAL) );
    }

    m_compListBox->m_ComponentList.Clear();

    for( unsigned i = 0;  i < m_netlist.GetCount();  ++i )
    {
        component = m_netlist.GetComponent( i );

        msg.Printf( CMP_FORMAT, m_panelTree->m_tree->GetCount() + 1, \
                    GetChars( component->GetReference() ), \
                    GetChars( component->GetValue() ), \
                    GetChars( FROM_UTF8( component->GetFPID().Format().c_str() ) ) );
        m_compListBox->m_ComponentList.Add( msg );
    }
}


void CVPCB_MAINFRAME::BuildLIBRARY_TREE()
{
    if( !m_compListBox )
       BuildCmpListBox();

    if( !m_panelTree )
    {
        wxFont guiFont = wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT );
        m_panelTree = new FOOTPRINTS_TREE( this );
        m_panelTree->SetFont( wxFont( guiFont.GetPointSize(),
                                        wxFONTFAMILY_MODERN,
                                        wxFONTSTYLE_NORMAL,
                                        wxFONTWEIGHT_NORMAL ) );
        m_panelTree->m_tree->AddRoot("Hidden root");
    }

    FP_LIB_TABLE* tbl = Prj().PcbFootprintLibs();

    if( tbl )
    {
        std::vector< wxString > libNickNames = tbl->GetLogicalLibs();

        for( const auto item : libNickNames)
           m_panelTree->m_tree->InsertItem( m_panelTree->m_tree->GetRootItem(), 0, item );
    }
}


COMPONENT* CVPCB_MAINFRAME::GetSelectedComponent()
{
    int selection = m_compListBox->GetSelection();

    if( selection >= 0 && selection < (int) m_netlist.GetCount() )
        return m_netlist.GetComponent( selection );

    return nullptr;
}


DISPLAY_FOOTPRINTS_FRAME* CVPCB_MAINFRAME::GetFootprintViewerFrame()
{
    // returns the Footprint Viewer frame, if exists, or nullptr
    return dynamic_cast<DISPLAY_FOOTPRINTS_FRAME*>
            ( wxWindow::FindWindowByName( FOOTPRINTVIEWER_FRAME_NAME ) );
}

const wxString CVPCB_MAINFRAME::GetSelectedFootprint()
{
    // returns the FPID of the selected footprint in footprint listview
    // or a empty string
    wxTreeItemId tmp = m_panelTree->m_tree->GetFocusedItem();
    return tmp.IsOk() ? m_panelTree->m_tree->GetItemText( m_panelTree->m_tree->GetFocusedItem() )
       : wxString();
}


void CVPCB_MAINFRAME::OnConfigurePaths( wxCommandEvent& aEvent )
{
    Pgm().ConfigurePaths( this );
}


void CVPCB_MAINFRAME::KiwayMailIn( KIWAY_EXPRESS& mail )
{
    const std::string& payload = mail.GetPayload();

    DBG(printf( "%s: %s\n", __func__, payload.c_str() );)

    switch( mail.Command() )
    {
    case MAIL_EESCHEMA_NETLIST:
        ReadNetListAndLinkFiles( payload );
        /* @todo
        Go into SCH_EDIT_FRAME::OnOpenCvpcb( wxCommandEvent& event ) and trim GNL_ALL down.
        */
        break;

    default:
        ;       // ignore most
    }
}
