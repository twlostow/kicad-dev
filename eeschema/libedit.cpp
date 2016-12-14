/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2008-2017 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 2004-2017 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <eeschema_id.h>
#include <general.h>
#include <libeditframe.h>
#include <class_library.h>
#include <template_fieldnames.h>
#include <wildcards_and_files_ext.h>
#include <schframe.h>
#include <lib_manager.h>

#include <dialog_choose_component.h>
#include <cmp_tree_model_adapter.h>
#include <dialog_eeschema_tree.h>

#include <dialogs/dialog_lib_new_component.h>


void LIB_EDIT_FRAME::DisplayLibInfos()
{
    PART_LIB* lib = GetCurLib();
    wxString title = wxString::Format( L"Part Library Editor \u2014 %s%s",
            lib ? lib->GetFullFileName() : _( "no library selected" ),
            lib && lib->IsReadOnly() ? _( " [Read Only] ") : wxString( wxEmptyString ) );

    SetTitle( title );
}


bool LIB_EDIT_FRAME::LoadComponentAndSelectLib( LIB_ALIAS* aLibEntry, PART_LIB* aLibrary )
{
    if( !aLibrary || !aLibEntry )
    {
        wxASSERT( false );
        return false;
    }

    aLibrary = m_libMgr->GetLibraryCopy( aLibrary->GetName() );
    SetCurLib( aLibrary );

    return LoadOneLibraryPartAux( aLibEntry, aLibrary );
}


void LIB_EDIT_FRAME::LoadOneLibraryPart( wxTreeEvent& aEvent )
{
    m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor() );
    ITEM_ID itemId = aEvent.GetItem();

    // Double clicking on a library should not do anything
    if( !itemId.IsOk() || m_panelTree->IsLibrary( itemId ) )
        return;

    wxString libName = m_panelTree->GetText( itemId, SEARCH_TREE::LIBRARY );
    wxString cmpName = m_panelTree->GetText( itemId, SEARCH_TREE::COMPONENT );
    int unit = 1;

    // Try to determine the selected unit
    LIB_PART* part = m_libMgr->GetPartCopy( cmpName, libName );
    ITEM_ID unitId = m_panelTree->GetItemAtLevel( itemId, SEARCH_TREE::UNIT );

    if( part && unitId.IsOk() )
    {
        wxString unitTxt = m_panelTree->GetText( unitId );

        for( int u = 1; u <= part->GetUnitCount(); ++u )
        {
            if( unitTxt.EndsWith( LIB_PART::SubReference( u, false ) ) )
            {
                unit = u;
                break;
            }
        }
    }

    LoadPart( cmpName, libName, unit );
}


bool LIB_EDIT_FRAME::LoadOneLibraryPartAux( LIB_ALIAS* aAlias, PART_LIB* aLibrary )
{
    wxString msg, rootName;

    if( !aAlias || !aLibrary )
        return false;

    if( aAlias->GetName().IsEmpty() )
    {
        wxLogWarning( wxT( "Entry in library <%s> has empty name field." ),
                      GetChars( aLibrary->GetName() ) );
        return false;
    }

    m_aliasName = aAlias->GetName();
    LIB_PART* lib_part = m_libMgr->GetPartCopy( m_aliasName, aLibrary->GetName() );
    wxASSERT( lib_part );

    SetScreen( m_libMgr->GetScreen( lib_part->GetName(), aLibrary->GetName() ) );
    SetCurPart( lib_part );

    m_convert = 1;
    SetShowDeMorgan( GetCurPart()->HasConversion() );
    m_editPinsPerPartOrConvert = GetCurPart()->UnitsLocked() ? true : false;

    Zoom_Automatique( false );
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
    LIB_PART* part = GetCurPart();

    if( part )
    {
        // display reference like in schematic (a reference U is shown U? or U?A)
        // although it is stored without ? and part id.
        // So temporary change the reference by a schematic like reference
        LIB_FIELD*  field = part->GetField( REFERENCE );
        wxString    fieldText = field->GetText();
        wxString    fieldfullText = field->GetFullText( m_unit );

        field->EDA_TEXT::SetText( fieldfullText );  // change the field text string only
        auto opts = PART_DRAW_OPTIONS::Default();
        opts.show_elec_type = GetShowElectricalType();
        part->Draw( m_canvas, aDC, aOffset, m_unit, m_convert, opts );
        field->EDA_TEXT::SetText( fieldText );      // restore the field text string
    }
}


void LIB_EDIT_FRAME::RedrawActiveWindow( wxDC* DC, bool EraseBg )
{
    if( GetScreen() == NULL )
        return;

    m_canvas->DrawBackGround( DC );

    RedrawComponent( DC, wxPoint( 0, 0 ) );

#ifdef USE_WX_OVERLAY
    if( IsShown() )
    {
        m_overlay.Reset();
        wxDCOverlay overlaydc( m_overlay, (wxWindowDC*) DC );
        overlaydc.Clear();
    }
#endif

    if( m_canvas->IsMouseCaptured() )
        m_canvas->CallMouseCapture( DC, wxDefaultPosition, false );

    m_canvas->DrawCrossHair( DC );

    DisplayLibInfos();
    UpdateStatusBar();
}


bool LIB_EDIT_FRAME::SaveLibrary( PART_LIB* aLibrary, const wxString& aFileName )
{
    if( !aLibrary )
    {
        DisplayError( this, _( "No library specified." ) );
        return false;
    }

    // Just in case the library hasn't been cached yet.
    aLibrary->GetCount();

    wxString oldFileName = aLibrary->GetFullFileName();
    wxFileName fn = aFileName.IsEmpty() ? aLibrary->GetFullFileName() : aFileName;
    wxString msg;

    // Verify the user has write privileges before attempting to
    // save the aLibrary file.
    if( !IsWritable( fn ) )
    {
        DisplayError( this, _( "No write access" ) );
        return false;
    }

    ClearMsgPanel();

    wxFileName libFileName = fn;
    wxFileName backupFileName = fn;

    // Rename the old .lib file to .bak.
    if( libFileName.FileExists() )
    {
        backupFileName.SetExt( "bak" );

        if( backupFileName.FileExists() )
            wxRemoveFile( backupFileName.GetFullPath() );

        if( !wxRenameFile( libFileName.GetFullPath(), backupFileName.GetFullPath() ) )
        {
            libFileName.MakeAbsolute();
            DisplayError( this, _( "Failed to rename old component library file " )
                    + backupFileName.GetFullPath() );
            return false;
        }
    }

    wxFileName docFileName = libFileName;

    docFileName.SetExt( DOC_EXT );

    // Rename .doc file to .bck.
    if( docFileName.FileExists() )
    {
        backupFileName.SetExt( "bck" );

        if( backupFileName.FileExists() )
            wxRemoveFile( backupFileName.GetFullPath() );

        if( !wxRenameFile( docFileName.GetFullPath(), backupFileName.GetFullPath() ) )
        {
            msg = _( "Failed to save old library document file " ) + backupFileName.GetFullPath();
            DisplayError( this, msg );
        }
    }

    try
    {
        aLibrary->SetFileName( fn.GetFullPath() );
        aLibrary->Save();
    }
    catch( ... /* IO_ERROR ioe */ )
    {
        aLibrary->SetFileName( oldFileName );
        msg.Printf( _( "Failed to create symbol library file '%s'" ),
                    GetChars( docFileName.GetFullPath() ) );
        DisplayError( this, msg );
        return false;
    }

    aLibrary->SetFileName( oldFileName );
    msg.Printf( _( "Library file '%s' saved" ), GetChars( fn.GetFullPath() ) );
    fn.SetExt( DOC_EXT );
    wxString msg1;
    msg1.Printf( _( "Documentation file '%s' saved" ), GetChars( fn.GetFullPath() ) );
    AppendMsgPanel( msg, msg1, BLUE );
    UpdateAliasSelectList();
    UpdatePartSelectList();
    refreshSchematic();

    return true;
}


void LIB_EDIT_FRAME::DisplayCmpDoc()
{
    LIB_ALIAS* alias;
    LIB_PART* part = GetCurPart();

    ClearMsgPanel();

    if( !part )
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


void LIB_EDIT_FRAME::CreateNewPart( wxCommandEvent& aEvent )
{
    m_canvas->EndMouseCapture( ID_NO_TOOL_SELECTED, m_canvas->GetDefaultCursor() );

    m_drawItem = NULL;

    PART_LIB* lib = GetSelectedLib();

    if( !lib )
    {
        lib = SelectLibraryFromList();

        if( !lib )
            return;
    }

    SetCurLib( lib );

    DIALOG_LIB_NEW_COMPONENT dlg( this );

    dlg.SetMinSize( dlg.GetSize() );

    if( dlg.ShowModal() == wxID_CANCEL )
        return;

    if( dlg.GetName().IsEmpty() )
    {
        DisplayError( this, _( "Cannot create a component without a name" ) );
        return;
    }

    wxString name = dlg.GetName();
    name.Replace( wxT( " " ), wxT( "_" ) );
    wxString libName = lib->GetName();

    // Test if there a component with this name already.
    if( lib->FindAlias( name ) )
    {
        wxString msg = wxString::Format( _( "Part '%s' already exists in library '%s'" ),
            GetChars( name ), GetChars( libName ) );
        DisplayError( this, msg );
        return;
    }

    LIB_PART* new_part = m_libMgr->GetPartCopy( name, libName );

    SetScreen( m_libMgr->GetScreen( name, libName ) );
    m_aliasName = name;

    m_panelTree->UpdateLibrary( libName );
    m_panelTree->SelectItem( m_panelTree->FindItem( name, libName ) );

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
    m_convert = 1;

    SetCurPart( new_part );
    DisplayLibInfos();
    DisplayCmpDoc();
    UpdateAliasSelectList();
    UpdatePartSelectList();

    m_editPinsPerPartOrConvert = new_part->UnitsLocked() ? true : false;
    m_lastDrawItem = NULL;

    Zoom_Automatique( false );
    OnModify();

    m_canvas->Refresh();
    m_mainToolBar->Refresh();
    GetScreen()->SetModified();
}


bool LIB_EDIT_FRAME::SaveOnePart( PART_LIB* aLib, bool aPromptUser )
{
    wxString    msg;
    LIB_PART*   part = GetSelectedPart();
    LIB_PART*   old_part = NULL;

    GetScreen()->ClearModified();

    if( !wxFileName::FileExists( aLib->GetFullFileName() ) )
    {
        aLib->Create();
    }
    else
    {
        old_part = aLib->FindPart( part->GetName() );

        if( old_part && aPromptUser )
        {
            msg.Printf( _( "Part '%s' already exists. Change it?" ),
                        GetChars( part->GetName() ) );

            if( !IsOK( this, msg ) )
                return false;
        }
    }

    m_drawItem = m_lastDrawItem = NULL;

    if( old_part )
        aLib->ReplacePart( old_part, part );
    else
        aLib->AddPart( part );

    msg.Printf( _( "Part '%s' saved in library '%s'" ),
                GetChars( part->GetName() ),
                GetChars( aLib->GetName() ) );

    SetStatusText( msg );

    return true;
}
