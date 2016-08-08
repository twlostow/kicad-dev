/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * @author Michele Castellana <michele.castellana@cern.ch>
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

const wxString SEARCH_TREE::FilterFootprintEntry = "FilterFootprint";

SEARCH_TREE::SEARCH_TREE( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
   : SEARCH_TREE_BASE( parent, id , pos, size, style ), m_style(wxTR_DEFAULT_STYLE | style), m_hasPreview(false),
   m_keywords(), m_pinCount(0), m_lastAddedElem(), m_filteringOptions(UNFILTERED), m_filterPattern()
{
    GetSizerForLabels()->Show( m_hasPreview );
    GetSizerForPreviews()->Show( m_hasPreview );
    wxFont guiFont = wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT );
    SetFont( wxFont( guiFont.GetPointSize(),
             wxFONTFAMILY_MODERN,
             wxFONTSTYLE_NORMAL,
             wxFONTWEIGHT_NORMAL ) );
    // Add a fake root node
    GetTree()->AddRoot("Hidden root");
}

SEARCH_TREE::~SEARCH_TREE()
{
}
wxString SEARCH_TREE::GetLibrary( const wxTreeItemId& aElem ) const
{
    if( !aElem.IsOk() ||
          aElem == GetTree()->GetRootItem() )
        return wxEmptyString;
    if(IsLibrary(aElem))
       return GetTree()->GetItemText(aElem);
    for(wxTreeItemId tmp = aElem;
          tmp.IsOk(); tmp = GetTree()->GetItemParent(tmp))
    {
        if( IsLibrary(tmp) )
            return GetTree()->GetItemText((tmp));
    }
    return wxEmptyString;
}

bool SEARCH_TREE::HasPreview() const
{
   return m_hasPreview;
}

wxString SEARCH_TREE::GetText( const wxTreeItemId& aElem ) const
{
   return GetTree()->GetItemText(aElem);
}

bool SEARCH_TREE::IsLibrary( const wxTreeItemId& aElem ) const
{
    return GetTree()->GetItemParent(aElem) == GetTree()->GetRootItem();
}
void SEARCH_TREE::RemoveLibrary( const wxString& aLibrary )
{
    wxTreeItemId root = GetTree()->GetRootItem();
    if( !root.IsOk() )
        return;
    wxTreeItemId elem = FindLibrary( aLibrary );
    if( elem.IsOk() )
        GetTree()->Delete( elem );
}

void SEARCH_TREE::InsertLibrary( const wxString& aLibrary )
{
    wxTreeItemId root = GetTree()->GetRootItem();
    if( !root.IsOk() )
        return;
    if( !FindLibrary( aLibrary ).IsOk() )
        GetTree()->InsertItem( root, 0, aLibrary );
}
wxTreeItemId SEARCH_TREE::AddLibrary( const wxString& aLibrary )
{
    wxTreeItemId root = GetTree()->GetRootItem();
    if( !root.IsOk() )
        return wxTreeItemId();
    if( !FindLibrary( aLibrary ).IsOk() )
        return GetTree()->InsertItem( root, 0, aLibrary );
    wxTreeItemId dummy;
    return dummy;
}

void SEARCH_TREE::RemoveLastAdded()
{
    if(m_lastAddedElem.IsOk())
        GetTree()->Delete(m_lastAddedElem);
}
// aGranPa == wxEmptyString means basically to insert a part
void SEARCH_TREE::InsertAsChildOf( const wxString& aElem, const wxString& aParent, const wxString& aGrandPa )
{
    if( !aGrandPa.empty() )
    {
        wxTreeItemId libId = FindLibrary( aGrandPa );
        if( libId.IsOk() )
        {
            wxTreeItemId comp = FindItem( libId, aParent );
            if( comp.IsOk() )
            {
                wxTreeItemId new_elem = GetTree()->InsertItem( comp, 0, aElem );
                if( new_elem.IsOk() )
                    m_lastAddedElem = new_elem;
            }
        }
    }
    else
    {
        wxTreeItemId parent = FindItem( aParent );
        if( parent.IsOk() )
        {
            wxTreeItemId new_elem = GetTree()->InsertItem( parent, 0, aElem );
            if( new_elem.IsOk() )
                m_lastAddedElem = new_elem;
        }
    }
}

/*
void SEARCH_TREE::Remove( const wxString& aElem, const wxString& aParent )
{
    wxTreeItemId libId = FindLibrary( aParent );
    if( libId.IsOk() )
    {
        wxTreeItemId comp = FindItem( libId, aParent );
        if( comp.IsOk() )
            GetTree()->Delete( comp );
    }
}
*/
void SEARCH_TREE::SetPinCount( unsigned aNum )
{
    if(m_pinCount == aNum)
        return;
    m_pinCount = aNum;
    // If the filter is applied, update the view
    if( m_filteringOptions & SEARCH_TREE::FILTERING_BY_PIN_COUNT )
        Update();
}
unsigned SEARCH_TREE::GetPinCount()
{
    return m_pinCount;
}

void SEARCH_TREE::SetPreview( bool aVal, const wxString& aTitle )
{
    if( m_hasPreview == aVal )
        return;
    assert(m_staticDefaultText);
    m_staticDefaultText->SetLabelText(aTitle);
    m_hasPreview = aVal;
    GetSizerForLabels()->Show( m_hasPreview );
    GetSizerForPreviews()->Show( m_hasPreview );
}

int SEARCH_TREE::GetFilteringOptions() const
{
    return m_filteringOptions;
}

wxBoxSizer* SEARCH_TREE::GetSizerForLabels() const
{
    assert( !m_hasPreview || m_labels );
    return m_labels;
}
wxBoxSizer* SEARCH_TREE::GetSizerForPreviews() const
{
    assert( !m_hasPreview || m_previews );
    return m_previews;
}
wxTreeCtrl* SEARCH_TREE::GetTree() const
{
    assert(m_tree);
    return m_tree;
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
void SEARCH_TREE::SetFilteringKeywords( const wxArrayString aFilter )
{
    m_keywords = aFilter;
    // If the filter is applied, update the view
    if( m_filteringOptions & SEARCH_TREE::FILTERING_BY_NAMES )
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

bool SEARCH_TREE::MatchKeywords( const wxString& aSearchFor ) const
{
    if( m_keywords.empty() )
        return true;
    wxString name = aSearchFor.Upper();
    for(unsigned ii = 0; ii < m_keywords.GetCount(); ++ii)
    {
        if( name.Matches( m_keywords[ii].Upper() ) )
            return true;
    }
    return false;

}
wxTreeItemId SEARCH_TREE::FindLibrary( const wxString& aSearchFor ) const
{
    wxTreeItemIdValue cookie;
    wxTreeItemId root = m_tree->GetRootItem();
    if( !root.IsOk() )
        return wxTreeItemId();
    wxTreeItemId item = m_tree->GetFirstChild( root, cookie );
    while( item.IsOk() )
    {
        wxString sData = m_tree->GetItemText(item);
        if( aSearchFor.CompareTo(sData) == 0 )
            return item;
        item = m_tree->GetNextChild( root, cookie);
    }
    /* Not found */
    wxTreeItemId dummy;
    return dummy;
}

unsigned int SEARCH_TREE::GetCount() const
{
    return m_tree->GetCount();
}

const wxArrayString SEARCH_TREE::GetSelectedElements() const
{
    wxArrayString ret_val;
    wxArrayTreeItemIds elems = GetSelected();
    for( unsigned num = 0;
          num < elems.GetCount();
          ++num )
    {
        assert( elems[num].IsOk() );
        if( elems[num] != m_tree->GetRootItem() &&
              m_tree->GetItemParent( elems[num] ) != m_tree->GetRootItem() )
            ret_val.Add( m_tree->GetItemText(elems[num]) );
    }
    return ret_val;
}

void SEARCH_TREE::LoadSettings( wxConfigBase* aCfg )
{
    assert(aCfg);
    //aCfg->Read( FilterFootprintEntry, &m_filteringOptions, SEARCH_TREE::UNFILTERED );
};
void SEARCH_TREE::SaveSettings( wxConfigBase* aCfg )
{
    assert(aCfg);
    //aCfg->Write( FilterFootprintEntry, m_filteringOptions );
};

void SEARCH_TREE::ExpandSelected()
{
    wxArrayTreeItemIds elems = GetSelected();
    for( unsigned num = 0;
         num < elems.GetCount();
         ++num )
    {
        assert( elems[num].IsOk() );
        if( m_tree->ItemHasChildren(elems[num]) 
              && !m_tree->IsExpanded(elems[num]))
            m_tree->Expand( elems[num] );
        for( wxTreeItemId tmp = m_tree->GetItemParent(elems[num]);
              tmp.IsOk() && tmp != m_tree->GetRootItem();
              tmp = m_tree->GetItemParent(tmp) )
            m_tree->Expand(tmp);
    }
}

const wxArrayString SEARCH_TREE::GetSelectedLibraries() const
{
    wxArrayString ret_val;
    wxArrayTreeItemIds elems = GetSelected();
    for( unsigned num = 0;
          num < elems.GetCount();
          ++num )
    {
        assert( elems[num].IsOk() );
        if(elems[num] == m_tree->GetRootItem())
           continue;
        wxTreeItemId tmp = elems[num];
        while( m_tree->GetItemParent( tmp ) != m_tree->GetRootItem() )
        {
            tmp = m_tree->GetItemParent(tmp);
            assert( tmp.IsOk() );
        }
        if( ret_val.Index( m_tree->GetItemText( tmp ) ) == wxNOT_FOUND )
            ret_val.Add( m_tree->GetItemText( tmp ) );
    }
    assert( elems.GetCount() >= ret_val.GetCount() );
    assert( elems.empty() ||
          (elems.GetCount() == 1 && elems[0] == m_tree->GetRootItem() ) ||
          !ret_val.empty() );
    return ret_val;
}

wxArrayTreeItemIds SEARCH_TREE::GetSelected() const
{
    wxArrayTreeItemIds elems;
    if( m_style & wxTR_MULTIPLE )
    {
        m_tree->GetSelections( elems );
    }
    else
    {
        wxTreeItemId tmp = m_tree->GetSelection();
        if(tmp.IsOk())
            elems.Add(tmp);
        else
            assert(elems.empty());
    }
    return elems;
};

void SEARCH_TREE::SelectItems( const wxArrayTreeItemIds& aItems )
{
    for( const auto& elem : aItems )
        SelectItem( elem );
}
bool SEARCH_TREE::IsSelected( const wxString& aItem ) const
{
    wxTreeItemId tmp = FindItem(aItem);
    if( tmp.IsOk() && GetTree()->IsSelected(tmp) )
        return true;
    return false;
}
void SEARCH_TREE::SelectItem( const wxString& aItem, const wxString& aParent )
{
    SelectItem(FindItem(aItem, aParent));
}
void SEARCH_TREE::SelectItem( const wxTreeItemId& aItem)
{
    if(!aItem.IsOk())
        return;
    m_tree->SelectItem(aItem);
    ExpandSelected();
}

wxTreeItemId SEARCH_TREE::FindItem( const wxTreeItemId& aRoot, const wxString& aSearchFor ) const
{
    wxTreeItemIdValue cookie;
    wxTreeItemId item = m_tree->GetFirstChild( aRoot, cookie );
    while( item.IsOk() )
    {
        wxString sData = m_tree->GetItemText(item);
        if( aSearchFor.CompareTo(sData) == 0 )
        {
            return item;
        }
        if( m_tree->ItemHasChildren( item ) )
        {
            wxTreeItemId search = FindItem( item, aSearchFor );
            if( search.IsOk() ) {
                return search;
            }
        }
        item = m_tree->GetNextChild( aRoot, cookie);
    }
    wxTreeItemId dummy;
    return dummy;
}

void SEARCH_TREE::Expand(const wxString& aElem )
{
    wxTreeItemId lib = FindLibrary(aElem);
    if( lib.IsOk()
          && m_tree->ItemHasChildren(lib)
          && !m_tree->IsExpanded(lib) )
        m_tree->Expand(lib);
}

wxTreeItemId SEARCH_TREE::FindItem( const wxString& aSearchFor, const wxString& aParent ) const
{
    wxTreeItemIdValue cookie;
    wxTreeItemId root = m_tree->GetRootItem();
    if( !root.IsOk() )
        return wxTreeItemId();
    wxTreeItemId item = m_tree->GetFirstChild( root, cookie );
    while( item.IsOk() )
    {
        wxString sData = m_tree->GetItemText(item);
        if( aParent.empty() && aSearchFor.CompareTo(sData) == 0 )
        {
            return item;
        }
        if( m_tree->ItemHasChildren( item ) )
        {
            wxTreeItemId search;
            if( !aParent.empty() && aParent.CompareTo(sData) == 0 )
                search = FindItem( item, aSearchFor );
            if( search.IsOk() )
            {
                return search;
            }
        }
        item = m_tree->GetNextChild( root, cookie);
    }
    wxTreeItemId dummy;
    return dummy;
}

void SEARCH_TREE::ResetTree()
{
    m_tree->DeleteAllItems();
    m_tree->AddRoot("Hidden root");
}

void SEARCH_TREE::ExpandAll()
{
    m_tree->ExpandAll();
}
