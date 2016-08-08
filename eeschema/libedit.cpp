/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2008-2013 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 2004-2015 KiCad Developers, see change_log.txt for contributors.
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
 * @file libedit.cpp
 * @brief Eeschema component library editor.
 */

#include <fctsys.h>
#include <kiway.h>
#include <gr_basic.h>
#include <macros.h>
#include <pgm_base.h>
#include <class_drawpanel.h>
#include <confirm.h>
#include <gestfich.h>
#include <class_sch_screen.h>
#include <wx/tokenzr.h>

#include <eeschema_id.h>
#include <general.h>
#include <libeditframe.h>
#include <class_library.h>
#include <template_fieldnames.h>
#include <wildcards_and_files_ext.h>
#include <dialog_eeschema_tree.h>

#include <dialogs/dialog_lib_new_component.h>


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
