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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dialog_search_tree.h"
#include <wx/config.h>

SEARCH_TREE::SEARCH_TREE( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
   : SEARCH_TREE_BASE( parent, id , pos, size, style | wxTR_DEFAULT_STYLE | wxTR_SINGLE | wxTR_HIDE_ROOT ),
    m_filteringOptions( UNFILTERED ), m_style( wxTR_DEFAULT_STYLE | style ),
    m_hasPreview( false ), m_pinCount( 0 )
{
    getSizerForLabels()->Show( m_hasPreview );
    getSizerForPreviews()->Show( m_hasPreview );

    wxFont guiFont = wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT );
    SetFont( wxFont( guiFont.GetPointSize(), wxFONTFAMILY_MODERN,
             wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL ) );

    ResetTree();
}


SEARCH_TREE::~SEARCH_TREE()
{
}


ITEM_ID SEARCH_TREE::InsertLibrary( const wxString& aLibrary, USER_DATA* aData )
{
    if( FindLibrary( aLibrary ).IsOk() )
        return ITEM_ID();

    // Find the right place to insert the new library (lexical order)
    wxTreeItemIdValue cookie;
    ITEM_ID root = m_tree->GetRootItem();
    ITEM_ID item = m_tree->GetFirstChild( root, cookie );

    while( item.IsOk() )
    {
        if( m_tree->GetItemText( item ) > aLibrary )
            break;

        item = m_tree->GetNextChild( root, cookie );
    }

    if( item.IsOk() )
        item = m_tree->GetPrevSibling( item );

    if( item.IsOk() )
        return m_tree->InsertItem( root, item, aLibrary, -1, -1, new ITEM_DATA( 0, aData, false ) );
    else
        return m_tree->AppendItem( root, aLibrary, -1, -1, new ITEM_DATA( 0, aData, false ) );
}


bool SEARCH_TREE::RemoveLibrary( const wxString& aLibrary )
{
    ITEM_ID elem = FindLibrary( aLibrary );

    if( elem.IsOk() )
        m_tree->Delete( elem );

    return true;
}


ITEM_ID SEARCH_TREE::InsertItem( const wxString& aItem, const ITEM_ID& aParent, USER_DATA* aData )
{
    if( !aParent.IsOk() )
        return ITEM_ID();

    return m_tree->AppendItem( aParent, aItem, -1, -1,
            new ITEM_DATA( GetLevel( aParent ) + 1, aData, false ) );
}


bool SEARCH_TREE::RemoveItem( const ITEM_ID& aItem )
{
    if( !aItem.IsOk() )
        return false;

    m_tree->Delete( aItem );

    return true;
}


bool SEARCH_TREE::SetData( const ITEM_ID& aItem, USER_DATA* aData )
{
    if( !aItem.IsOk() )
        return false;

    ITEM_DATA* data = static_cast<ITEM_DATA*>( m_tree->GetItemData( aItem ) );
    delete data->m_userData;    // remove the previous data, so it does not leak
    data->m_userData = aData;
    return true;
}


void SEARCH_TREE::SetPinCountFilter( unsigned aNum )
{
    if( m_pinCount == aNum )
        return;

    m_pinCount = aNum;

    // If the filter is applied, update the view
    if( m_filteringOptions & SEARCH_TREE::FILTER_BY_PIN_COUNT )
        Update();
}


void SEARCH_TREE::SetFilteringKeywords( const wxArrayString aFilter )
{
    m_keywords = aFilter;

    // If the filter is applied, update the view
    if( m_filteringOptions & SEARCH_TREE::FILTER_BY_NAME )
        Update();
}


void SEARCH_TREE::OnFiltering( bool aApply, SEARCH_TREE::FILTER aFilter )
{
    if( aApply )
        m_filteringOptions |= aFilter;
    else
        m_filteringOptions &= ~aFilter;

    Update();
}


int SEARCH_TREE::GetLevel( const ITEM_ID& aItem ) const
{
    if( !aItem.IsOk() )
        return -1;

    const ITEM_DATA* data = static_cast<const ITEM_DATA*>( m_tree->GetItemData( aItem ) );
    assert( data );

    if( !data )
        return -1;

    return data->m_level;
}


ITEM_ID SEARCH_TREE::GetItemAtLevel( const ITEM_ID& aItem, int aLevel ) const
{
    int itemLevel = GetLevel( aItem );

    if( itemLevel >= aLevel )
    {
        ITEM_ID res( aItem );

        for( int i = 0; i < ( itemLevel - aLevel ); ++i )
        {
            res = m_tree->GetItemParent( res );
            assert( res.IsOk() );
        }

        return res;
    }

    return ITEM_ID();
}


wxString SEARCH_TREE::GetLibrary( const ITEM_ID& aItem ) const
{
    return GetText( GetItemAtLevel( aItem, 0 ) );
}


wxString SEARCH_TREE::GetComponent( const ITEM_ID& aItem ) const
{
    return GetText( GetItemAtLevel( aItem, 1 ) );
}


bool SEARCH_TREE::SetModified( const ITEM_ID& aItem, bool aModified )
{
    if( !aItem.IsOk() )
        return false;

    m_tree->SetItemBold( aItem, aModified );
    ITEM_DATA* data = static_cast<ITEM_DATA*>( m_tree->GetItemData( aItem ) );
    assert( data );
    data->m_modified = aModified;

    return true;
}


bool SEARCH_TREE::IsModified( const ITEM_ID& aItem ) const
{
    if( !aItem.IsOk() )
        return false;

    const ITEM_DATA* data = static_cast<const ITEM_DATA*>( m_tree->GetItemData( aItem ) );
    assert( data );
    return data->m_modified;
}


bool SEARCH_TREE::SetCurrent( const ITEM_ID& aItem )
{
    // Restore colors for the previous item
    invertColors( FindItem( m_currentComp, m_currentLib ), false );
    invertColors( FindLibrary( m_currentLib ), false );

    // Invert colors for the current selection
    invertColors( aItem, true );
    invertColors( GetItemAtLevel( aItem, LIBRARY ), true );

    m_currentComp = GetComponent( aItem );
    m_currentLib = GetLibrary( aItem );

    return true;
}


bool SEARCH_TREE::IsCurrent( const ITEM_ID& aItem ) const
{
    return ( GetComponent( aItem ) == m_currentComp ) && ( GetLibrary( aItem ) == m_currentLib );
}


ITEM_ID SEARCH_TREE::GetSingleSelected( int aLevel ) const
{
    ITEM_IDS elems = GetSelected();

    if( elems.size() != 1 )
        return ITEM_ID();

    ITEM_ID id = elems[0];
    int level = GetLevel( id );

    // Too high in the hierarchy
    if( level < aLevel )
        return ITEM_ID();

    for( int i = 0; i < level - aLevel; ++i )
    {
        if( !id.IsOk() )
            break;

        id = getTree()->GetItemParent( id );
    }

    return id;
}


wxArrayString SEARCH_TREE::GetSelectedLibraries() const
{
    wxArrayString ret_val;
    ITEM_IDS elems = GetSelected();

    for( unsigned num = 0; num < elems.GetCount(); ++num )
    {
        assert( elems[num].IsOk() );

        if( elems[num] == m_tree->GetRootItem() )
            continue;

        ITEM_ID tmp = elems[num];

        while( m_tree->GetItemParent( tmp ) != m_tree->GetRootItem() )
        {
            tmp = m_tree->GetItemParent( tmp );
            assert( tmp.IsOk() );
        }

        if( ret_val.Index( m_tree->GetItemText( tmp ) ) == wxNOT_FOUND )
            ret_val.Add( m_tree->GetItemText( tmp ) );
    }

    assert( elems.GetCount() >= ret_val.GetCount() );
    assert( elems.empty() || ( elems.GetCount() == 1 && elems[0] == m_tree->GetRootItem() )
            || !ret_val.empty() );

    return ret_val;
}


wxArrayString SEARCH_TREE::GetSelectedComponents() const
{
    wxArrayString ret_val;
    ITEM_IDS elems = GetSelected();

    for( unsigned num = 0; num < elems.GetCount(); ++num )
    {
        assert( elems[num].IsOk() );

        if( IsComponent( elems[num] ) )
            ret_val.Add( m_tree->GetItemText( elems[num] ) );
    }

    return ret_val;
}


void SEARCH_TREE::SaveSettings( wxConfigBase* aCfg )
{
    assert( aCfg );
    // aCfg->Write( FILTER_FOOTPRINT_CFG, m_filteringOptions );
}


void SEARCH_TREE::ExpandSelected()
{
    ITEM_IDS elems = GetSelected();

    for( unsigned num = 0; num < elems.GetCount(); ++num )
    {
        assert( elems[num].IsOk() );

        if( m_tree->ItemHasChildren( elems[num] ) && !m_tree->IsExpanded( elems[num] ) )
            m_tree->Expand( elems[num] );

        for( ITEM_ID tmp = m_tree->GetItemParent( elems[num] );
                 tmp.IsOk() && tmp != m_tree->GetRootItem();
                 tmp = m_tree->GetItemParent( tmp ) )
        {
            m_tree->Expand( tmp );
        }
    }
}


void SEARCH_TREE::SelectItem( const ITEM_ID& aItem )
{
    if( !aItem.IsOk() )
        return;

    m_tree->SelectItem( aItem );
    ExpandSelected();
}


void SEARCH_TREE::ResetTree()
{
    m_tree->DeleteAllItems();

    // Add a fake root node
    m_tree->AddRoot( "Hidden root", -1, -1, new ITEM_DATA( -1, nullptr, false ) );
}


bool SEARCH_TREE::matchKeywords( const wxString& aSearchFor ) const
{
    if( m_keywords.empty() )
        return true;

    wxString name = aSearchFor.Upper();

    for( unsigned ii = 0; ii < m_keywords.GetCount(); ++ii )
    {
        if( name.Matches( m_keywords[ii].Upper() ) )
            return true;
    }

    return false;
}


ITEM_IDS SEARCH_TREE::GetSelected() const
{
    ITEM_IDS elems;

    if( m_style & wxTR_MULTIPLE )
    {
        m_tree->GetSelections( elems );
    }
    else
    {
        ITEM_ID tmp = m_tree->GetSelection();

        // Apparently an item returned by wxTreeCtrl::GetSelection() is not necessarily selected,,
        // so we need to check first..
        if( tmp.IsOk() && m_tree->IsSelected( tmp ) )
            elems.Add( tmp );
    }

    return elems;
}


void SEARCH_TREE::Visit( std::function<void(const ITEM_ID&)> aFunc, int aMinLevel, int aMaxLevel )
{
    visitRecursive( m_tree->GetRootItem(), aFunc, aMinLevel, aMaxLevel );
}


void SEARCH_TREE::visitRecursive( const ITEM_ID& aNode, std::function<void(const ITEM_ID&)> aFunc,
        int aMinLevel, int aMaxLevel )
{
    if( !aNode.IsOk() )
        return;

    int level = GetLevel( aNode );

    if( level >= aMaxLevel )
        return;

    wxTreeItemIdValue cookie;
    ITEM_ID item = m_tree->GetFirstChild( aNode, cookie );

    while( item.IsOk() )
    {
        if( level + 1 >= aMinLevel )
            aFunc( item );

        visitRecursive( item, aFunc, aMinLevel, aMaxLevel );
        item = m_tree->GetNextChild( aNode, cookie );
    }
}


ITEM_ID SEARCH_TREE::FindItem( const wxString& aSearchFor, const wxString& aParent ) const
{
    wxTreeItemIdValue cookie;
    ITEM_ID root = m_tree->GetRootItem();

    if( !root.IsOk() )
        return ITEM_ID();

    ITEM_ID item = m_tree->GetFirstChild( root, cookie );

    while( item.IsOk() )
    {
        wxString sData = m_tree->GetItemText( item );

        if( aParent.empty() && aSearchFor.CompareTo( sData ) == 0 )
            return item;

        if( m_tree->ItemHasChildren( item ) )
        {
            ITEM_ID search;

            if( !aParent.empty() && aParent.CompareTo( sData ) == 0 )
                search = findItem( item, aSearchFor );

            if( search.IsOk() )
                return search;
        }

        item = m_tree->GetNextChild( root, cookie );
    }

    return ITEM_ID();
}


ITEM_ID SEARCH_TREE::FindLibrary( const wxString& aSearchFor ) const
{
    wxTreeItemIdValue cookie;
    ITEM_ID root = m_tree->GetRootItem();

    if( !root.IsOk() )
        return ITEM_ID();

    ITEM_ID item = m_tree->GetFirstChild( root, cookie );

    while( item.IsOk() )
    {
        wxString sData = m_tree->GetItemText( item );

        if( aSearchFor.CompareTo( sData ) == 0 )
            return item;

        item = m_tree->GetNextChild( root, cookie );
    }

    return ITEM_ID();
}


void SEARCH_TREE::setPreview( bool aVal, const wxString& aTitle )
{
    if( m_hasPreview == aVal )
        return;

    assert( m_staticDefaultText );
    m_staticDefaultText->SetLabelText( aTitle );
    m_hasPreview = aVal;
    getSizerForLabels()->Show( m_hasPreview );
    getSizerForPreviews()->Show( m_hasPreview );
}


void SEARCH_TREE::OnTextChanged( wxCommandEvent& aEvent )
{
    m_filterPattern = m_text->GetValue();
    Update();
}


void SEARCH_TREE::OnLeftDClick( wxMouseEvent& aEvent )
{
    aEvent.Skip();
}


void SEARCH_TREE::loadSettings( wxConfigBase* aCfg )
{
    assert( aCfg );
    // aCfg->Read( FILTER_FOOTPRINT_CFG, &m_filteringOptions, SEARCH_TREE::UNFILTERED );
}


void SEARCH_TREE::invertColors( ITEM_ID aItem, bool aInvert )
{
    if( !aItem.IsOk() )
        return;

    wxColour textColor = wxSystemSettings::GetColour( wxSYS_COLOUR_LISTBOXTEXT );
    wxColour bgColor = wxSystemSettings::GetColour( wxSYS_COLOUR_LISTBOX );

    m_tree->SetItemTextColour( aItem, aInvert ? bgColor : textColor );
    m_tree->SetItemBackgroundColour( aItem, aInvert ? textColor : bgColor );
}


ITEM_ID SEARCH_TREE::findItem( const ITEM_ID& aRoot, const wxString& aSearchFor ) const
{
    wxTreeItemIdValue cookie;
    ITEM_ID item = m_tree->GetFirstChild( aRoot, cookie );

    while( item.IsOk() )
    {
        wxString sData = m_tree->GetItemText( item );

        if( aSearchFor.CompareTo( sData ) == 0 )
            return item;

        if( m_tree->ItemHasChildren( item ) )
        {
            ITEM_ID search = findItem( item, aSearchFor );

            if( search.IsOk() )
                return search;
        }

        item = m_tree->GetNextChild( aRoot, cookie );
    }

    return ITEM_ID();
}


void SEARCH_TREE::selectItems( const ITEM_IDS& aItems )
{
    for( const auto& elem : aItems )
        SelectItem( elem );
}
