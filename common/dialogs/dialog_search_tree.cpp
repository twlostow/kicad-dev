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
   : SEARCH_TREE_BASE( parent, id , pos, size, style ), m_filteringOptions(UNFILTERED), m_filterPattern(),
   m_style( wxTR_DEFAULT_STYLE | style), m_hasPreview(false), m_keywords(), m_pinCount(0)
{
    GetSizerForLabels()->Show( m_hasPreview );
    GetSizerForLabels()->Show( m_hasPreview );
    wxFont guiFont = wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT );
    SetFont( wxFont( guiFont.GetPointSize(),
             wxFONTFAMILY_MODERN,
             wxFONTSTYLE_NORMAL,
             wxFONTWEIGHT_NORMAL ) );
}

SEARCH_TREE::~SEARCH_TREE()
{
}

void SEARCH_TREE::InsertLibrary( const wxString& aLibrary )
{
    assert( GetTree() );
    wxTreeItemId root = GetTree()->GetRootItem();
    assert( root.IsOk() );
    if( !FindLibrary( aLibrary ).IsOk() )
    {
        std::cerr << aLibrary << " added" << std::endl;
        GetTree()->InsertItem( root, 0, aLibrary );
    }
}

void SEARCH_TREE::InsertAsChildOf( const wxString& aFootprint, const wxString& aLibrary )
{
    wxTreeItemId library = FindItem( aLibrary );
    if( library.IsOk() )
    {
        GetTree()->InsertItem( library, 0, aFootprint );
        return;
    }
}
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

void SEARCH_TREE::SetPreview( bool val, const wxString& title )
{
    if( m_hasPreview == val)
        return;
    assert(m_staticDefaultText);
    m_staticDefaultText->SetLabelText(title);
    m_hasPreview = val;
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
void SEARCH_TREE::OnLeftDClick( wxMouseEvent& event )
{
    event.Skip();
}
void SEARCH_TREE::SetFilteringKeywords( const wxArrayString aFilter )
{
    m_keywords = aFilter;
    // If the filter is applied, update the view
    if( m_filteringOptions & SEARCH_TREE::FILTERING_BY_NAMES )
        Update();
}

void SEARCH_TREE::OnFiltering( bool apply, SEARCH_TREE::FILTER filter )
{
    if( apply )
        m_filteringOptions |= filter;
    else
        m_filteringOptions &= ~filter;

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
    assert(root.IsOk());
    wxTreeItemId item = m_tree->GetFirstChild( root, cookie );
    while( item.IsOk() ) {
        wxString sData = m_tree->GetItemText(item);
        if( aSearchFor.CompareTo(sData) == 0 ) {
            return item;
        }
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
        if( m_tree->GetItemParent( elems[num] ) != m_tree->GetRootItem() )
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
        {
            m_tree->Expand(tmp);
        }
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
        if( m_tree->GetItemParent( elems[num] ) == m_tree->GetRootItem() )
           ret_val.Add( m_tree->GetItemText(elems[num]) );
    }
    return ret_val;
}

wxArrayTreeItemIds SEARCH_TREE::GetSelected() const
{
    wxArrayTreeItemIds elems;
    if( m_style & wxTR_MULTIPLE )
    {
        assert(false);
        m_tree->GetSelections( elems );
    }
    else
    {
        wxTreeItemId tmp = m_tree->GetSelection();
        if(tmp.IsOk())
            elems.Add(tmp);
    }
    return elems;
};

void SEARCH_TREE::SelectItems( const wxArrayTreeItemIds& items )
{
    for( const auto& elem : items )
        SelectItem( elem );
}
bool SEARCH_TREE::IsSelected( const wxString& item ) const
{
   wxTreeItemId tmp = FindItem(item);
   if( tmp.IsOk() && GetTree()->IsSelected(tmp))
       return true;
   return false;
}
void SEARCH_TREE::SelectItem( const wxString& item )
{
    SelectItem(FindItem(item));
}
void SEARCH_TREE::SelectItem( const wxTreeItemId& item)
{
    if(!item.IsOk())
        return;
    m_tree->SelectItem(item);
    ExpandSelected();
}

wxTreeItemId SEARCH_TREE::FindItem( const wxTreeItemId& root, const wxString& aSearchFor ) const
{
    wxTreeItemIdValue cookie;
    wxTreeItemId item = m_tree->GetFirstChild( root, cookie );
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
        item = m_tree->GetNextChild( root, cookie);
    }
    /* Not found */
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

wxTreeItemId SEARCH_TREE::FindItem( const wxString& aSearchFor ) const
{
    wxTreeItemIdValue cookie;
    wxTreeItemId root = m_tree->GetRootItem();
    assert(root.IsOk());
    wxTreeItemId item = m_tree->GetFirstChild( root, cookie );
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
            if( search.IsOk() )
            {
                return search;
            }
        }
        item = m_tree->GetNextChild( root, cookie);
    }
    /* Not found */
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
