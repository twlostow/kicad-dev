/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * https://www.gnu.org/licenses/gpl-3.0.html
 * or you may search the http://www.gnu.org website for the version 3 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "lib_manager.h"
#include "class_libentry.h"
#include "class_library.h"
#include "libeditframe.h"
#include <kiway.h>

LIB_MANAGER::LIB_MANAGER( LIB_EDIT_FRAME& aFrame )
    : m_frame( aFrame )
{
}


void LIB_MANAGER::Sync()
{
    PART_LIBS* partLibs = m_frame.Kiway().Prj().SchLibs();
    wxString curLib = m_frame.GetCurLibName();

    wxArrayString mgrLibs = GetLibraryNames();
    wxArrayString prjLibs = partLibs->GetLibraryNames();

    mgrLibs.Sort();
    prjLibs.Sort();

    unsigned int mgrIdx = 0;
    unsigned int prjIdx = 0;

    while( prjIdx < prjLibs.GetCount() || mgrIdx < mgrLibs.Count() )
    {
        wxString prjLib( prjIdx < prjLibs.GetCount() ? prjLibs[prjIdx] : "" );
        wxString mgrLib( mgrIdx < mgrLibs.GetCount() ? mgrLibs[mgrIdx] : "" );
        int compare = prjLib.Cmp( mgrLib );

        if( compare == 0 && !prjLib.IsEmpty() )
        {
            // Libraries exist in both the library manager and project libraries, move on
            ++prjIdx;
            ++mgrIdx;
        }
        else if( !prjLib.IsEmpty() && ( compare < 0 || mgrLib.IsEmpty() ) )
        {
            // Missing library in the manager
            PART_LIB* library = partLibs->FindLibrary( prjLib );

            if( library )
                m_libs.emplace( prjLib, library );

            ++prjIdx;
        }
        else if( !mgrLib.IsEmpty() && ( compare > 0 || prjLib.IsEmpty() ) )
        {
            // A library has been removed from the project libraries
            auto it = m_libs.find( mgrLib );
            wxASSERT( it != m_libs.end() );

            // Do not remove libraries that have unsaved changes
            if( curLib != mgrLib && !IsLibraryModified( mgrLib ) )
                m_libs.erase( it );

            ++mgrIdx;
        }
        else
        {
            break;
        }
    }
}


wxArrayString LIB_MANAGER::GetLibraryNames() const
{
    wxArrayString res;
    res.Alloc( m_libs.size() );

    for( const auto& libName : m_libs )
        res.Add( libName.first );

    return res;
}


bool LIB_MANAGER::SaveAll()
{
    bool result = true;

    for( auto& libBuf : m_libs )
        result &= SaveLibrary( libBuf.first );

    return result;
}


bool LIB_MANAGER::CreateLibrary( const wxString& aName, const wxString& aFileName )
{
    wxCHECK( !LibraryExists( aName ), false );
    wxCHECK( touchLibraryFile( aFileName ), false );
    PART_LIB* newLib = m_frame.Kiway().Prj().SchLibs()->AddLibrary( aFileName );
    m_libs.emplace( aName, newLib );
    cfgLibsAdd( newLib );

    return true;
}


bool LIB_MANAGER::AddLibrary( PART_LIB* aLibrary )
{
    wxString libName( aLibrary->GetName() );
    wxCHECK( !LibraryExists( libName ), false );
    wxASSERT( getOriginalLib( aLibrary->GetName() ) );
    m_libs.emplace( libName, aLibrary );
    cfgLibsAdd( aLibrary );

    return true;
}


bool LIB_MANAGER::SaveLibrary( const wxString& aLibName, const wxString& aFileName )
{
    auto bufIt = m_libs.find( aLibName );
    LIB_BUFFER* buf = ( bufIt == m_libs.end() ? nullptr : &bufIt->second );
    PART_LIBS* partLibs = m_frame.Kiway().Prj().SchLibs();
    wxCHECK( partLibs, false );

    PART_LIB* editedLib = buf->getLib();
    wxASSERT( editedLib && editedLib->GetName() == aLibName );
    bool saveAs = !aFileName.IsEmpty();
    wxFileName filename( saveAs ? aFileName : editedLib->GetFullFileName() );

    if( !filename.IsOk() )
        return false;

    if( !wxFileName::IsDirWritable( filename.GetPath() ) )
        return false;

    if( saveAs )
    {
        wxString oldFilename = editedLib->GetFullFileName();
        editedLib->SetFileName( filename.GetFullPath() );
        editedLib->Save();
        editedLib->SetFileName( oldFilename );
    }
    else
    {
        // Save the library and reload it
        partLibs->RemoveLibrary( aLibName );
        editedLib->Save();
        partLibs->AddLibrary( editedLib->GetFullFileName() );

        // Clear modification flag on successful write
        for( auto& screen : buf->m_screens )
            screen.second->ClearModified();

        if( buf )
            buf->m_modified = false;
    }

    return true;
}


/*
bool LIB_MANAGER::RenameLibrary( const wxString& aOld, const wxString& aNew )
{
    wxASSERT_MSG( false, "not tested" );
    auto it = m_libs.find( aOld );

    if( it == m_libs.end() )
        return false;

    // Cannot have duplicates
    wxCHECK( !LibraryExists( aNew ), false );
    m_libs.emplace( aNew, std::move( it->second ) );
    m_libs.erase( it );

    return true;
}
*/


bool LIB_MANAGER::RemoveLibrary( const wxString& aName )
{
    auto it = m_libs.find( aName );

    if( it == m_libs.end() )
        return false;

    cfgLibsRemove( it->second.getLib() );
    m_frame.Kiway().Prj().SchLibs()->RemoveLibrary( it->first );
    m_libs.erase( it );

    return true;
}


bool LIB_MANAGER::IsLibraryModified( const wxString& aLibrary ) const
{
    wxCHECK( LibraryExists( aLibrary ), false );
    const LIB_BUFFER& buf = m_libs.at( aLibrary );
    wxASSERT( buf.getLib() );

    // Check the library modification flag (set when e.g. a component is renamed/removed)
    if( buf.m_modified )
        return true;

    // Check individual parts for modifications
    for( auto& screen : buf.m_screens )
    {
        if( screen.second && screen.second->IsModified() )
            return true;
    }

    return false;
}


bool LIB_MANAGER::IsPartModified( const wxString& aPart, const wxString& aLibrary ) const
{
    wxCHECK( LibraryExists( aLibrary ), false );
    const LIB_BUFFER& buf = m_libs.at( aLibrary );
    PART_LIB* libBuf = buf.getLib();
    wxCHECK( libBuf, false );

    LIB_PART* part = buf.getLib()->FindPart( aPart );
    wxString partName = part ? part->GetName() : aPart;
    auto it = buf.m_screens.find( partName );
    SCH_SCREEN* screen = ( it == buf.m_screens.end() ) ? nullptr : it->second.get();
    return screen ? screen->IsModified() : false;
}


bool LIB_MANAGER::IsLibraryReadOnly( const wxString& aLibrary ) const
{
    wxCHECK( LibraryExists( aLibrary ), true );
    const LIB_BUFFER& buf = m_libs.at( aLibrary );
    return buf.getLib() ? buf.getLib()->IsReadOnly() : true;
}


wxArrayString LIB_MANAGER::GetPartNames( const wxString& aLibrary ) const
{
    wxArrayString names;
    wxCHECK( LibraryExists( aLibrary ), names );
    const LIB_BUFFER& buf = m_libs.at( aLibrary );
    PART_LIB* library = buf.getLib();

    if( !library )
        library = getOriginalLib( aLibrary );

    if( library )
        library->GetAliasNames( names );

    return names;
}


PART_LIB* LIB_MANAGER::GetLibraryCopy( const wxString& aLibrary )
{
    if( aLibrary.IsEmpty() )
        return nullptr;

    LIB_BUFFER& buf = m_libs[aLibrary];

    if( !buf.getLib() )
    {
        PART_LIB* origLib = getOriginalLib( aLibrary );
        wxCHECK( origLib, nullptr );
        buf.setLib( origLib );
    }

    return buf.getLib();
}


LIB_PART* LIB_MANAGER::GetPart( const wxString& aPart, const wxString& aLibrary, bool& aIsAlias ) const
{
    PART_LIB* library = nullptr;

    // Check the library buffers
    auto it = m_libs.find( aLibrary );

    if( it != m_libs.end() )
        library = it->second.getLib();

    // If not found, check the original libraries
    if( !library )
        library = getOriginalLib( aLibrary );

    if( library )
    {
        auto alias = library->FindAlias( aPart );
        aIsAlias = !( alias->GetPart()->GetName() == alias->GetName() );
    }

    return library ? library->FindPart( aPart ) : nullptr;
}

LIB_PART* LIB_MANAGER::GetPart( const wxString& aPart, const wxString& aLibrary ) const
{
    bool dummy;
    return GetPart( aPart, aLibrary, dummy );
}



LIB_PART* LIB_MANAGER::GetPartCopy( const wxString& aPart, const wxString& aLibrary )
{
    wxCHECK( LibraryExists( aLibrary ), nullptr );
    PART_LIB* lib = GetLibraryCopy( aLibrary );
    wxCHECK( lib, nullptr );
    LIB_PART* part = lib->FindPart( aPart );

    // Handle request for a new component
    if( !part )
    {
        part = new LIB_PART( aPart, lib );
        lib->AddPart( part );
    }

    return part;
}


SCH_SCREEN* LIB_MANAGER::GetScreen( const wxString& aPart, const wxString& aLibrary )
{
    wxCHECK( LibraryExists( aLibrary ), nullptr );
    wxCHECK( !aPart.IsEmpty(), nullptr );
    LIB_BUFFER& buf = m_libs.at( aLibrary );
    wxCHECK( buf.getLib(), nullptr );
    SCH_SCREEN* screen = nullptr;

    // All aliases should share the same screen, so it should be referred by the part name
    LIB_PART* part = buf.getLib()->FindPart( aPart );
    wxString partName = part ? part->GetName() : aPart;

    // Let's see if we have a working copy already
    auto it = buf.m_screens.find( partName );

    if( it != buf.m_screens.end() )
        screen = it->second.get();

    if( !screen )
    {
        screen = new SCH_SCREEN( &m_frame.Kiway() );
        buf.m_screens[partName].reset( screen );
    }

    return screen;
}


bool LIB_MANAGER::UpdatePart( LIB_PART* aPart, const wxString& aLibrary )
{
    wxCHECK( LibraryExists( aLibrary ), false );
    wxCHECK( aPart, false );
    const wxString partName = aPart->GetName();
    LIB_BUFFER& buf = m_libs.at( aLibrary );
    PART_LIB* library = buf.getLib();

    bool res = updatePart( library, aPart );

    if( res )
        GetScreen( partName, aLibrary )->SetModified();

    return res;
}


bool LIB_MANAGER::SavePart( const wxString& aPart, const wxString& aLibrary )
{
    wxCHECK( LibraryExists( aLibrary ), false );

    PART_LIB* origLib = getOriginalLib( aLibrary );
    wxCHECK( origLib, false );
    wxCHECK( !origLib->IsReadOnly(), false );

    LIB_BUFFER& buf = m_libs.at( aLibrary );
    PART_LIB* bufLib = buf.getLib();
    wxCHECK( bufLib, false );
    wxASSERT( aLibrary == bufLib->GetName() );

    LIB_PART* part = bufLib->FindPart( aPart );
    wxCHECK( part, false );

    // Replacing the part in the original library automatically saves to disk
    updatePart( origLib, part );
    GetScreen( aPart, aLibrary )->ClearModified();

    // If the part was renamed, remove the associated entry
    buf.m_renamed.erase( aPart );

    return true;
}


bool LIB_MANAGER::RevertPart( const wxString& aPart, const wxString& aLibrary )
{
    wxCHECK( LibraryExists( aLibrary ), false );
    LIB_BUFFER& buf = m_libs.at( aLibrary );
    PART_LIB* bufLib = buf.getLib();
    wxCHECK( bufLib, false );
    wxASSERT( aLibrary == bufLib->GetName() );

    // Check if the part has been renamed, and if so - use the old name to retrieve the part
    auto it = buf.m_renamed.find( aPart );
    wxString origPartName = it == buf.m_renamed.end() ? aPart : it->second;

    PART_LIB* origLib = getOriginalLib( aLibrary );
    wxCHECK( origLib, false );

    LIB_PART* origPart = origLib->FindPart( origPartName );
    LIB_PART* part = bufLib->FindPart( aPart );

    // Use a copy of the original part, so the main part remains untouched
    if( part && origPart )          // part has been modified
    {
        GetScreen( aPart, aLibrary )->ClearModified();
        bufLib->ReplacePart( part, origPart );
    }
    else if( !part && origPart )    // part has been deleted
    {
        bufLib->AddPart( origPart );
    }
    else if( part && !origPart )    // part has been added
    {
        buf.m_screens.erase( part->GetName() );

        for( int i = part->GetAliasCount() - 1; i >= 0; --i )
            bufLib->RemoveAlias( part->GetAlias( i ) );
    }
    else
    {
        return false;
    }

    return true;
}


bool LIB_MANAGER::RevertLibrary( const wxString& aLibrary )
{
    wxCHECK( LibraryExists( aLibrary ), false );
    LIB_BUFFER& buf = m_libs.at( aLibrary );
    wxCHECK( buf.getLib(), false );
    PART_LIB* origLib = getOriginalLib( aLibrary );
    wxCHECK( origLib, false );
    buf.setLib( origLib );

    return true;
}


int LIB_MANAGER::GetHash() const
{
    int res = 0;

    for( const auto& buffer : m_libs )
    {
        if( PART_LIB* lib = buffer.second.getLib() )
            res += lib->GetModHash();
    }

    return res;
}


bool LIB_MANAGER::RenamePart( const wxString& aOld, const wxString& aNew, const wxString& aLibrary )
{
    wxCHECK( LibraryExists( aLibrary ), false );
    LIB_BUFFER& buf = m_libs.at( aLibrary );
    PART_LIB* libBuf = buf.getLib();
    wxCHECK( libBuf, false );
    wxCHECK( !PartExists( aNew, aLibrary ), false ); // Check for duplicates
    LIB_PART* part = buf.getLib()->FindPart( aOld );
    wxCHECK( part, false );
    wxString origName = GetOriginalName( aOld, aLibrary );
    wxCHECK( !origName.IsEmpty(), false );

    LIB_PART tmp( *part );
    tmp.AddAlias( aNew );
    tmp.RemoveAlias( aOld );
    libBuf->ReplacePart( part, &tmp );

    // Screens are referenced by part names, so if part name has changed,
    // we need to transfer the related SCH_SCREEN. If only an alias is renamed, then the
    // part name is not changed, and we can keep the old entry.
    wxString newName = tmp.GetName();

    if( origName != newName )
    {
        auto screenIt = buf.m_screens.find( origName );

        if( screenIt != buf.m_screens.end() )
        {
            wxASSERT( buf.m_screens.count( aNew ) == 0 );

            buf.m_screens[newName] = std::move( screenIt->second );
            buf.m_screens.erase( screenIt );
        }
    }

    GetScreen( newName, aLibrary )->SetModified();
    buf.m_renamed.erase( origName );
    buf.m_renamed[aNew] = origName;

    return true;
}


bool LIB_MANAGER::RemovePart( const wxString& aPart, const wxString& aLibrary )
{
    wxCHECK( LibraryExists( aLibrary ), false );
    LIB_BUFFER& buf = m_libs.at( aLibrary );
    wxCHECK( buf.getLib(), false );
    LIB_ALIAS* alias = buf.getLib()->FindAlias( aPart );
    wxCHECK( alias, false );

    buf.m_modified = true;
    buf.m_screens.erase( alias->GetPart()->GetName() );
    buf.getLib()->RemoveAlias( alias );
    return true;
}


bool LIB_MANAGER::PartExists( const wxString& aPart, const wxString& aLibrary ) const
{
    // No library has been selected
    if( !LibraryExists( aLibrary ) )
        return false;

    const LIB_BUFFER& buf = m_libs.at( aLibrary );
    wxASSERT( buf.getLib() );

    return ( buf.getLib() && buf.getLib()->FindAlias( aPart ) );
}


wxString LIB_MANAGER::ValidateName( const wxString& aName )
{
    wxString name( aName );
    name.Replace( " ", "_" );
    return name;
}


wxString LIB_MANAGER::GetOriginalName( const wxString& aPart, const wxString& aLibrary ) const
{
    wxCHECK( LibraryExists( aLibrary ), wxEmptyString );
    const LIB_BUFFER& buf = m_libs.at( aLibrary );
    PART_LIB* libBuf = buf.getLib();
    wxCHECK( libBuf, wxEmptyString );

    wxString origName = aPart;
    bool updated;

    do
    {
        updated = false;
        auto it = buf.m_renamed.find( origName );

        if( it != buf.m_renamed.end() )
        {
            updated = true;
            origName = it->second;
        }
    }
    while( updated );

    return origName;
}


wxString LIB_MANAGER::GetUniqueLibraryName() const
{
    wxString name = "New_Library";

    if( !LibraryExists( name ) )
        return name;

    name += "_";

    for( unsigned int i = 0; i < std::numeric_limits<unsigned int>::max(); ++i )
    {
        if( !LibraryExists( name + wxString::Format( "%u", i ) ) )
            return name + wxString::Format( "%u", i );
    }

    // Something went terribly wrong here
    wxASSERT( false );

    return wxEmptyString;
}


wxString LIB_MANAGER::GetUniqueComponentName( const wxString& aLibrary ) const
{
    wxString name = "New_Component";

    if( !PartExists( name, aLibrary ) )
        return name;

    name += "_";

    for( unsigned int i = 0; i < std::numeric_limits<unsigned int>::max(); ++i )
    {
        if( !PartExists( name + wxString::Format( "%u", i ), aLibrary ) )
            return name + wxString::Format( "%u", i );
    }

    // Something went terribly wrong here
    wxASSERT( false );

    return wxEmptyString;
}


bool LIB_MANAGER::updatePart( PART_LIB* aLibrary, LIB_PART* aPart )
{
    if( !aPart || !aLibrary )
        return false;

    LIB_PART* oldPart = aLibrary->FindPart( GetOriginalName( aPart->GetName(), aLibrary->GetName() ) );

    if( oldPart )
        aLibrary->ReplacePart( oldPart, aPart );
    else
        aLibrary->AddPart( aPart );

    return true;
}


PART_LIB* LIB_MANAGER::getOriginalLib( const wxString& aLibrary ) const
{
    PART_LIBS* partLibs = m_frame.Kiway().Prj().SchLibs();
    return partLibs ? partLibs->FindLibrary( aLibrary ) : nullptr;
}


bool LIB_MANAGER::touchLibraryFile( const wxString& aFileName ) const
{
    wxRemoveFile( aFileName );
    PART_LIB tmp( LIBRARY_TYPE_EESCHEMA, aFileName );
    tmp.Create();
    return true;
}


void LIB_MANAGER::cfgLibsAdd( const PART_LIB* aLibrary )
{
    wxString libPaths;
    wxArrayString libNames;
    PROJECT* prj = &m_frame.Kiway().Prj();
    SEARCH_STACK* paths = prj->SchSearchS();

    PART_LIBS::LibNamesAndPaths( prj, false, &libPaths, &libNames );
    wxFileName libFileName = aLibrary->GetFullFileName();

    // Try to find a path relative to one in the current search stack
    for( unsigned int i = 0; i < paths->GetCount(); i++ )
    {
        wxFileName relFn( libFileName );

        if( relFn.MakeRelativeTo( (*paths)[i] ) && relFn.GetPath()[0] != '.' )
        {
            libFileName = relFn;
            break;
        }
    }

    // Strip the extension from the library path
    libNames.Add( libFileName.GetPath() + wxFileName::GetPathSeparator() + aLibrary->GetName() );
    // Save the library list
    PART_LIBS::LibNamesAndPaths( prj, true, &libPaths, &libNames );
}


void LIB_MANAGER::cfgLibsRemove( const PART_LIB* aLibrary )
{
    wxString libPaths;
    wxArrayString libNames;
    PROJECT* prj = &m_frame.Kiway().Prj();
    const wxString& remLib( aLibrary->GetName() );
    int remLibLen = remLib.Length();

    PART_LIBS::LibNamesAndPaths( prj, false, &libPaths, &libNames );

    for( unsigned int i = 0; i < libNames.GetCount(); ++i )
    {
        const wxString& lib = libNames[i];
        int libLen = lib.Length();

        if( lib.EndsWith( remLib ) )    // Library can contain its path, hence EndsWith() test
        {
            // Be sure that we are not removing a library that simply has a name ending
            // with the library to be removed
            if( ( libLen == remLibLen )       // meaning remLib == lib
                || ( libLen > remLibLen && wxFileName::IsPathSeparator( lib[libLen - remLibLen - 1] ) ) )
            {
                libNames.RemoveAt( i );
                break;
            }
        }
    }

    // Save the library list
    PART_LIBS::LibNamesAndPaths( prj, true, &libPaths, &libNames );
}


LIB_MANAGER::LIB_BUFFER::LIB_BUFFER( const PART_LIB* aLibrary )
    : m_modified( false ), m_lib( nullptr )
{
    if( aLibrary )
        setLib( aLibrary );
}


void LIB_MANAGER::LIB_BUFFER::setLib( const PART_LIB* aLibrary )
{
    PART_LIB* copy = nullptr;

    if( aLibrary )
    {
        copy = PART_LIB::LoadLibrary( aLibrary->GetFullFileName() );
        copy->EnableBuffering();
    }

    // SCH_SCREENs are not valid anymore, as the parts are gone
    m_screens.clear();
    m_modified = false;

    m_lib.reset( copy );
}
