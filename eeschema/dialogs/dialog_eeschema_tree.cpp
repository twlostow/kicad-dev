/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * @author Michele Castellana <michele.castellana@cern.ch>
 * @author Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A COMPONENTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <dialog_eeschema_tree.h>
#include <eda_pattern_match.h>
#include <libeditframe.h>
#include <eeschema_id.h>
#include <class_library.h>
#include <lib_manager.h>
#include <list>

// If the number of elements is less than this,
// the code will automatically expand the tree elements.
static const int ELEM_NUM_FOR_EXPAND_ALL = 45;

EESCHEMA_TREE::EESCHEMA_TREE( LIB_EDIT_FRAME* aParent )
   : SEARCH_TREE( aParent ), m_menuActive( false ),
    m_staticFootCandText( nullptr ), m_footprintCandidates( nullptr ), m_libMgr( nullptr )
{
    assert( getTree() );

    // SetPreview( true, "Preview" );
    if( hasPreview() )
    {
        wxFont guiFont = wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT );

        // Add the footprinCandidates static text
        m_staticFootCandText = new wxStaticText( this, wxID_ANY, _( "Predefined footprints" ) );
        m_staticFootCandText->Wrap( -1 );
        m_staticFootCandText->SetFont( wxFont( guiFont.GetPointSize(),
                 wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL ) );
        getSizerForLabels()->Add( m_staticFootCandText, 1, wxALL, 5 );

        // Add the footprinCandidates listbox
        m_footprintCandidates = new wxListBox( this, wxID_ANY );
        m_footprintCandidates->SetFont( wxFont( guiFont.GetPointSize(),
                 wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL ) );
        getSizerForPreviews()->Add( m_footprintCandidates, 1, wxALL | wxEXPAND, 5 );
    }

    m_menuLibrary.reset( new wxMenu() );
    m_menuLibrary->Append( ID_LIBEDIT_NEW_LIBRARY, _( "New library..." ) );
    m_menuLibrary->Append( ID_LIBEDIT_ADD_LIBRARY, _( "Add existing library..." ) );
    m_menuLibrary->Append( ID_LIBEDIT_SAVE_LIBRARY, _( "Save library" ) );
    m_menuLibrary->Append( ID_LIBEDIT_SAVE_LIBRARY_AS, _( "Save library as..." ) );
    m_menuLibrary->Append( ID_LIBEDIT_REVERT_LIBRARY, _( "Revert library" ) );
    m_menuLibrary->Append( ID_LIBEDIT_REMOVE_LIBRARY, _( "Remove library" ) );
    m_menuLibrary->AppendSeparator();
    m_menuLibrary->Append( ID_LIBEDIT_NEW_PART, _( "New component..." ) );
    m_menuLibrary->Append( ID_LIBEDIT_PASTE_PART, _( "Paste component" ) );
    m_menuLibrary->Append( ID_LIBEDIT_IMPORT_PART, _( "Import component..." ) );

    m_menuComponent.reset( new wxMenu() );
    m_menuComponent->Append( ID_LIBEDIT_EDIT_PART, _( "Edit" ) );
    m_menuComponent->Append( ID_LIBEDIT_CUT_PART, _( "Cut" ) );
    m_menuComponent->Append( ID_LIBEDIT_COPY_PART, _( "Copy" ) );
    m_menuComponent->Append( ID_LIBEDIT_RENAME_PART, _( "Rename" ) );
    m_menuComponent->Append( ID_LIBEDIT_REMOVE_PART, _( "Remove" ) );
    m_menuComponent->Append( ID_LIBEDIT_EXPORT_PART, _( "Export..." ) );
    m_menuComponent->Append( ID_LIBEDIT_SAVE_PART, _( "Save" ) );
    m_menuComponent->Append( ID_LIBEDIT_REVERT_PART, _( "Revert" ) );
    m_menuComponent->AppendSeparator();

    // Append the library menu to the component menu
    for( size_t i = 0; i < m_menuLibrary->GetMenuItemCount(); ++i )
    {
        wxMenuItem* menuItem = m_menuLibrary->FindItemByPosition( i );
        m_menuComponent->Append( menuItem->GetId(), menuItem->GetItemLabel() );
    }
}


EESCHEMA_TREE::~EESCHEMA_TREE()
{
}


void EESCHEMA_TREE::OnRightClick( wxMouseEvent& aEvent )
{
    int area;
    ITEM_ID item = getTree()->HitTest( aEvent.GetPosition(), area );
    getTree()->UnselectAll();
    wxMenu* menu = nullptr;

    if( item.IsOk() )
    {
        getTree()->SelectItem( item, true );
        wxString itemName = GetText( item );
        int itemLevel = GetLevel( item );

        switch( itemLevel )
        {
            case COMPONENT:
                menu = m_menuComponent.get();
                break;

            case LIBRARY:
                menu = m_menuLibrary.get();
                break;

            default:
                // There is no menu for component subunits
                return;
        }
    }
    else // nothing selected
    {
        menu = m_menuLibrary.get();
    }

    if( menu )
    {
        m_menuActive = true;
        PopupMenu( menu );
        m_menuActive = false;
    }
}


void EESCHEMA_TREE::Update()
{
    for( const auto& libName : m_libMgr->GetLibraryNames() )
        UpdateLibrary( libName );

    getTree()->SortChildren( getTree()->GetRootItem() );

    if( GetCount() < ELEM_NUM_FOR_EXPAND_ALL )
        ExpandAll();
}


void EESCHEMA_TREE::Sync()
{
    std::list<ITEM_ID> toRemove;
    wxArrayString mgrLibs = m_libMgr->GetLibraryNames();
    unsigned int mgrIdx = 0;

    wxTreeItemIdValue cookie;
    ITEM_ID root = m_tree->GetRootItem();
    ITEM_ID libId = m_tree->GetFirstChild( root, cookie );

    while( libId.IsOk() || mgrIdx < mgrLibs.Count() )
    {
        wxString prjLib( libId.IsOk() ? GetText( libId ) : "" );
        wxString mgrLib( mgrIdx < mgrLibs.GetCount() ? mgrLibs[mgrIdx] : "" );
        int compare = prjLib.Cmp( mgrLib );

        if( compare == 0 )
        {
            UpdateLibrary( prjLib );
            libId = libId.IsOk() ? m_tree->GetNextSibling( libId ) : libId;
            ++mgrIdx;
        }
        else if( compare < 0 )
        {
            // New library
            wxASSERT( !mgrLib.IsEmpty() );
            UpdateLibrary( mgrLib );
            ++mgrIdx;
        }
        else //( compare > 0 )
        {
            // Manager has a removed library
            // libId has to be iterated first, as UpdateLibrary may remove it
            libId = libId.IsOk() ? m_tree->GetNextSibling( libId ) : libId;
            wxASSERT( !prjLib.IsEmpty() );
            UpdateLibrary( prjLib );
        }
    }
}


void EESCHEMA_TREE::UpdateLibrary( const wxString& aLibName )
{
    ITEM_ID libId = FindLibrary( aLibName );

    // Remove libraries that do not exist anymore
    if( !m_libMgr->LibraryExists( aLibName ) )
    {
        if( libId.IsOk() )
            getTree()->Delete( libId );

        return;
    }

    bool isExpanded = false;

    // Create a library entry or clear the old one
    if( !libId.IsOk() )
    {
        libId = InsertLibrary( aLibName );
    }
    else
    {
        isExpanded = getTree()->IsExpanded( libId );
        getTree()->DeleteChildren( libId );
    }

    SetModified( libId, m_libMgr->IsLibraryModified( aLibName ) );
    SetItemFont( libId, m_libMgr->IsLibraryModified( aLibName ), false );

    const auto& parts = m_libMgr->GetPartNames( aLibName );

    if( parts.empty() )
        return;

    // Is it the currently modified library?
    bool current = ( aLibName == CurrentLibrary() );

    EDA_PATTERN_MATCH_WILDCARD patternFilter;

    // Use case insensitive search
    bool libNameMatch = false;

    if( !m_filterPattern.IsEmpty() )
    {
        patternFilter.SetPattern( m_filterPattern.Lower() );
        libNameMatch = patternFilter.Find( aLibName.Lower() ) != EDA_PATTERN_NOT_FOUND;
    }

    for( const auto& partName : parts )
    {
        bool isAlias;

        if( ( m_filteringOptions & SEARCH_TREE::FILTER_BY_NAME ) && !libNameMatch
                && !matchKeywords( partName ) )
            continue;

        if( !m_filterPattern.IsEmpty() && !libNameMatch
            && patternFilter.Find( partName.Lower() ) == EDA_PATTERN_NOT_FOUND )
            continue;

        const LIB_PART* part = m_libMgr->GetPart( partName, aLibName, isAlias );

        if( !part )
            continue;

        ITEM_ID itemId = InsertItem( partName, libId );

        if( current && partName == CurrentComponent() )
        {
            SetCurrent( itemId );
            current = false;
        }

        if( part->IsMulti() )
        {
            for( int u = 1; u <= part->GetUnitCount(); ++u )
            {
                wxString unitName = _( "Unit" );
                unitName += " " + LIB_PART::SubReference( u, false );
                InsertItem( unitName, itemId );
            }
        }

        SetModified( itemId, m_libMgr->IsPartModified( partName, aLibName ) );
        SetItemFont( itemId, m_libMgr->IsPartModified( partName, aLibName ), isAlias );
    }

    // After filtering there is nothing left, so do not display the library
    if( !m_filterPattern.IsEmpty() && getTree()->GetChildrenCount( libId ) == 0 )
    {
        getTree()->Delete( libId );
        return;
    }

    if( isExpanded || libNameMatch )
        getTree()->Expand( libId );
}


void EESCHEMA_TREE::LoadFootprints( LIB_MANAGER* aLibManager )
{
    m_libMgr = aLibManager;
    Sync();
    getTree()->SortChildren( getTree()->GetRootItem() );
}


void EESCHEMA_TREE::OnLeftDClick( wxMouseEvent& aEvent )
{
    const ITEM_ID item = getTree()->GetFocusedItem();

    // Double click expands/collapses a library
    if( IsLibrary( item ) )
    {
        wxTreeCtrl* tree = getTree();

        if( tree->IsExpanded( item ) )
            tree->Collapse( item );
        else
            tree->Expand( item );
    }

    aEvent.Skip();
}
