/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2008-2016 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 2004-2016 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file lib_export.cpp
 * @brief Eeschema library maintenance routines to backup modified libraries and
 *        create, edit, and delete components.
 */

#include <fctsys.h>
#include <class_drawpanel.h>
#include <confirm.h>

#include <general.h>
#include <libeditframe.h>
#include <class_library.h>
#include <wildcards_and_files_ext.h>

#include <wx/filename.h>
#include <dialog_eeschema_tree.h>
#include <lib_manager.h>


void LIB_EDIT_FRAME::ImportPart( wxCommandEvent& event )
{
    m_lastDrawItem = NULL;

    // Select either the library chosen in the panel tree or the currently modified library
    PART_LIB* curLib = GetSelectedLib();

    if( !curLib )
    {
        curLib = SelectLibraryFromList();

        if( !curLib )
            return;
    }

    wxFileDialog dlg( this, _( "Import Component" ), m_mruPath,
            wxEmptyString, SchematicLibraryFileWildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST );

    if( dlg.ShowModal() == wxID_CANCEL )
        return;

    wxFileName fn = dlg.GetPath();
    m_mruPath = fn.GetPath();
    std::unique_ptr<PART_LIB> lib;

    try
    {
        std::unique_ptr<PART_LIB> new_lib( PART_LIB::LoadLibrary( fn.GetFullPath() ) );
        lib = std::move( new_lib );
    }
    catch( const IO_ERROR& )
    {
        wxString msg = wxString::Format( _( "Unable to import library '%s'." ),
            GetChars( fn.GetFullPath() ) );
        DisplayError( this, msg );
        return;
    }

    wxArrayString aliasNames;
    lib->GetAliasNames( aliasNames );

    if( aliasNames.IsEmpty() )
    {
        wxString msg = wxString::Format( _( "Part library file '%s' is empty." ),
                                         GetChars( fn.GetFullPath() ) );
        DisplayError( this, msg );
        return;
    }

    LIB_ALIAS* entry = lib->FindAlias( aliasNames[0] );
    m_libMgr->UpdatePart( entry->GetPart(), curLib->GetName() );
    m_panelTree->UpdateLibrary( curLib->GetName() );
}


void LIB_EDIT_FRAME::ExportPart( wxCommandEvent& event )
{
    // Select either the library chosen in the panel tree or the currently modified library
    PART_LIB* curLib = GetSelectedLib();

    if( !curLib )
    {
        DisplayError( this, _( "You need to select destination library first." ) );
        return;
    }

    wxString msg;
    LIB_PART* part = GetSelectedPart();

    if( !part )
    {
        DisplayError( this, _( "There is no component selected to save." ) );
        return;
    }

    wxFileName fn = part->GetName().Lower();
    fn.SetExt( SchematicLibraryFileExtension );

    wxFileDialog dlg( this, _( "Export Component" ), m_mruPath, fn.GetFullName(),
                      SchematicLibraryFileWildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

    if( dlg.ShowModal() == wxID_CANCEL )
        return;

    fn = dlg.GetPath();

    std::unique_ptr<PART_LIB> temp_lib( new PART_LIB( LIBRARY_TYPE_EESCHEMA, fn.GetFullPath() ) );

    try
    {
        SaveOnePart( temp_lib.get() );
    }
    catch( ... /* IO_ERROR ioe */ )
    {
        fn.MakeAbsolute();
        msg = wxT( "Failed to create symbol library file " ) + fn.GetFullPath();
        DisplayError( this, msg );
        msg.Printf( _( "Error creating symbol library '%s'" ), fn.GetFullName() );
        SetStatusText( msg );
        return;
    }

    m_mruPath = fn.GetPath();

    msg.Printf( _( "'%s' - Export OK" ), GetChars( fn.GetFullPath() ) );
    SetStatusText( msg );
}
