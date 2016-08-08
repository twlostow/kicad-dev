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

#include <dialog_eeschema_tree.h>
#include <eda_pattern_match.h>
#include <wx/config.h>
#include <wx/string.h>
#include <macros.h>
#include <viewlib_frame.h>
#include <eeschema_id.h>
#include <class_library.h>
#include <climits>
#include <wildcards_and_files_ext.h>
#include <kiway.h>

// If the number of elements is less than this,
// the code will automatically expand the tree elements.
#define ELEM_NUM_FOR_EXPAND_ALL 45
#define WRONG_SELECTION "Wrong selection"
#define TOO_MANY_LIBRARIES_SELECTED_MESSAGE_BOX wxMessageBox( _( "More than one library selected. Choose only one!" ), _( WRONG_SELECTION ), wxOK | wxICON_ERROR | wxCENTRE )
#define NO_LIBRARY_SELECTED_MESSAGE_BOX wxMessageBox( _( "Select one library, please." ), _( WRONG_SELECTION ), wxOK | wxICON_ERROR | wxCENTRE )

BEGIN_EVENT_TABLE( EESCHEMA_TREE, SEARCH_TREE )
    EVT_TREE_END_LABEL_EDIT( wxID_SEARCHABLE_TREE, EESCHEMA_TREE::CheckValidity )
END_EVENT_TABLE()

EESCHEMA_TREE::EESCHEMA_TREE( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
   : SEARCH_TREE( parent, id, pos, size, wxTR_HIDE_ROOT | wxTR_MULTIPLE | style ), m_menu(nullptr),
   m_staticFootCandText(nullptr), m_footprintCandidates(nullptr), m_cutOrCopyElems(), m_bufLibs(nullptr)
{
    assert(GetTree());
    //SetPreview( true, "Preview" );
    if( HasPreview() )
    {
        wxFont guiFont = wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT );
        // Add the footprinCandidates static text
        m_staticFootCandText = new wxStaticText( this, wxID_ANY, wxT("Predefined footprints"), wxDefaultPosition, wxDefaultSize, 0 );
        m_staticFootCandText->Wrap( -1 );
        m_staticFootCandText->SetFont( wxFont( guiFont.GetPointSize(),
                 wxFONTFAMILY_MODERN,
                 wxFONTSTYLE_NORMAL,
                 wxFONTWEIGHT_NORMAL ) );
        GetSizerForLabels()->Add( m_staticFootCandText, 1, wxALL, 5);
        // Add the footprinCandidates listbox
        m_footprintCandidates = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
        m_footprintCandidates->SetFont( wxFont( guiFont.GetPointSize(),
                 wxFONTFAMILY_MODERN,
                 wxFONTSTYLE_NORMAL,
                 wxFONTWEIGHT_NORMAL ) );
        GetSizerForPreviews()->Add( m_footprintCandidates, 1, wxALL | wxEXPAND, 5);
    }
}
EESCHEMA_TREE::~EESCHEMA_TREE()
{
    if( m_menu )
    {
        delete m_menu;
        m_menu = nullptr;
    }
}

void EESCHEMA_TREE::CheckValidity(wxTreeEvent& aEvent)
{
    wxString new_name = aEvent.GetLabel();
    wxString old_name;
    if(aEvent.GetItem().IsOk())
    {
       old_name = GetTree()->GetItemText( aEvent.GetItem() );
    }
    if( new_name.Trim().empty()
          || FindLibrary( new_name ) )
    {
        aEvent.Veto();
    }
    aEvent.Skip();
}

wxString EESCHEMA_TREE::GetFirstLibraryNameAvailable() const
{
   wxString name = "New_Library";
   if(!FindItem(name).IsOk())
       return name;
   name += "_";
   for(unsigned int i = 0; i < UINT_MAX; ++i)
   {
       if( !FindItem(name + wxString::Format("%u", i)).IsOk() )
          return name + wxString::Format("%u", i);
   }
   // Something went terribly wrong here
   assert(false);
}
wxString EESCHEMA_TREE::GetFirstComponentNameAvailable(const wxString& aName) const
{
   wxString name = "New_Component";
   if(!FindItem(name).IsOk())
       return name;
   name += "_";
   for(unsigned int i = 0; i < UINT_MAX; ++i)
   {
       if( !FindItem(name + wxString::Format("%u", i), aName).IsOk() )
          return name + wxString::Format("%u", i);
   }
   // Something went terribly wrong here
   assert(false);
}

void EESCHEMA_TREE::OnPopupClick( wxCommandEvent &aEvt )
{
    bool is_skipped = true;
    switch( aEvt.GetId() )
    {
        case ID_LIBEDIT_NEW_COMPONENT:
            {
                wxArrayString tmp = GetSelectedLibraries();
                if(tmp.empty() )
                {
                    NO_LIBRARY_SELECTED_MESSAGE_BOX;
                    is_skipped = false;
                }
                if( tmp.GetCount() > 1 )
                {
                    TOO_MANY_LIBRARIES_SELECTED_MESSAGE_BOX;
                    is_skipped = false;
                }
            }
            break;
        case ID_LIBEDIT_NEW_PART:
            {
                wxArrayTreeItemIds tmp = GetSelected();
                if(tmp.empty() )
                {
                    NO_LIBRARY_SELECTED_MESSAGE_BOX;
                    is_skipped = false;
                }
                if( tmp.GetCount() > 1 )
                {
                    TOO_MANY_LIBRARIES_SELECTED_MESSAGE_BOX;
                    is_skipped = false;
                }
            }
            break;
        case ID_LIBEDIT_CUT:
            {
                if( GetSelectedLibraries().GetCount() == 1 )
                {
                    wxArrayTreeItemIds tmp;
                    GetTree()->GetSelections(tmp);
                    if( IsLibrary(tmp[0]) )
                    {
                        wxMessageBox( _( "You cannot cut a whole library" ) );
                        is_skipped = false;
                    }
                    wxTreeItemId lib_to_select = FindLibrary( GetSelectedLibraries()[0] );
                    assert(lib_to_select.IsOk());
                    m_cutOrCopyElems.Clear();
                    for( auto elem : tmp )
                    {
                        if( !IsLibrary( elem )
                              && !GetTree()->GetItemText(elem).StartsWith("Unit ") )
                        {
                            m_cutOrCopyElems.Add(GetTree()->GetItemText(elem));
                            GetTree()->Delete(elem);
                        }
                    }
                    GetTree()->SelectItem(lib_to_select);
                }
                else if( GetSelectedLibraries().empty() )
                {
                    NO_LIBRARY_SELECTED_MESSAGE_BOX;
                    is_skipped = false;
                }
                else if( GetSelectedLibraries().GetCount() > 1 )
                {
                    TOO_MANY_LIBRARIES_SELECTED_MESSAGE_BOX;
                    is_skipped = false;
                }
            }
            break;
        case ID_LIBEDIT_COPY:
            {
                if( GetSelectedLibraries().GetCount() == 1 )
                {
                    wxArrayTreeItemIds tmp;
                    GetTree()->GetSelections(tmp);
                    if( IsLibrary(tmp[0]) )
                    {
                        wxMessageBox( _( "You cannot copy a whole library" ), _( WRONG_SELECTION ), wxICON_ERROR | wxCENTRE );
                        is_skipped = false;
                    }
                    wxTreeItemId lib_to_select = FindLibrary( GetSelectedLibraries()[0] );
                    m_cutOrCopyElems.Clear();
                    for( auto elem : tmp )
                    {
                        if( !IsLibrary( elem )
                              && !GetTree()->GetItemText(elem).StartsWith("Unit ") )
                            m_cutOrCopyElems.Add(GetTree()->GetItemText(elem));
                    }
                    GetTree()->SelectItem(lib_to_select);
                }
                else if( GetSelectedLibraries().empty() )
                {
                    NO_LIBRARY_SELECTED_MESSAGE_BOX;
                    is_skipped = false;
                }
                else if( GetSelectedLibraries().GetCount() > 1 )
                {
                    TOO_MANY_LIBRARIES_SELECTED_MESSAGE_BOX;
                    is_skipped = false;
                }
            }
            break;
        case ID_LIBEDIT_PASTE:
            {
                wxArrayString tmp = GetSelectedLibraries();
                if( tmp.GetCount() == 1)
                {
                    for( auto elem : m_cutOrCopyElems )
                    {
                        InsertAsChildOf(elem, tmp[0]);
                    }
                    Expand( tmp[0] );
                }
                else if( tmp.empty() )
                {
                    NO_LIBRARY_SELECTED_MESSAGE_BOX;
                    is_skipped = false;
                }
                else if( tmp.GetCount() > 1 )
                {
                    TOO_MANY_LIBRARIES_SELECTED_MESSAGE_BOX;
                    is_skipped = false;
                }
            }
            break;
        case ID_LIBEDIT_RENAME:
            {
                wxArrayTreeItemIds tmp = GetSelected();
                if(tmp.empty()
                      || tmp.GetCount() > 1)
                    is_skipped = false;
                else
                    GetTree()->EditLabel(tmp[0]);
            }
            break;
        case ID_LIBEDIT_NEW_LIBRARY:
        default:
            break;
    }
    aEvt.Skip(is_skipped);
}
wxArrayString EESCHEMA_TREE::GetCutOrCopy() const
{
    return m_cutOrCopyElems;
}
void EESCHEMA_TREE::OnRightClick( wxMouseEvent& aEvent )
{
    if( !m_menu )
    {
        m_menu = new wxMenu ;
        m_menu->Append(ID_LIBEDIT_NEW_LIBRARY, "Add Library" );
        m_menu->Append(ID_LIBEDIT_NEW_COMPONENT, "Add Component" );
        m_menu->Append(ID_LIBEDIT_NEW_PART, "Add Part" );
        m_menu->AppendSeparator();
        m_menu->Append(ID_LIBEDIT_CUT, "Cut" );
        m_menu->Append(ID_LIBEDIT_COPY, "Copy" );
        m_menu->Append(ID_LIBEDIT_PASTE, "Paste" );
        m_menu->Append(ID_LIBEDIT_RENAME, "Rename" );
        //m_menu->Append(ID_LIBEDIT_DUPLICATE, "Duplicate" );
        m_menu->Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(EESCHEMA_TREE::OnPopupClick),NULL,this);
    }
    int area;
    wxTreeItemId hypItem = GetTree()->HitTest(aEvent.GetPosition(), area);
    GetTree()->UnselectAll();
    if( hypItem.IsOk() )
    {
        GetTree()->SelectItem(hypItem, true);
    }
    PopupMenu(m_menu);
}

void EESCHEMA_TREE::LoadSettings( wxConfigBase* aCfg )
{
    assert(aCfg); 
}
void EESCHEMA_TREE::SaveSettings( wxConfigBase* aCfg )
{
    assert(aCfg);
    SEARCH_TREE::SaveSettings(aCfg);
}

void EESCHEMA_TREE::LoadFootprints( PART_LIBS* aLibs )
{
    m_bufLibs = aLibs;
    Update();
}

void EESCHEMA_TREE::Update()
{
    std::map<wxString, std::map<wxString, wxArrayString>>   newList;

    EDA_PATTERN_MATCH_WILDCARD patternFilter;
    // Use case insensitive search
    patternFilter.SetPattern( m_filterPattern.Lower() );

    for( const auto it : m_bufLibs->GetLibraryNames() )
    {
        PART_LIB* lib = m_bufLibs->FindLibrary( it );
        if( !lib )
            continue;

        wxArrayString entries;
        lib->GetEntryNames(entries);
        newList[it];
        for( const auto elem : entries )
        {
            if( (m_filteringOptions & SEARCH_TREE::FILTERING_BY_NAMES )
                  && !MatchKeywords( elem ) )
                continue;
            if( !m_filterPattern.IsEmpty() && patternFilter.Find( elem.Lower() ) == EDA_PATTERN_NOT_FOUND )
                continue;
            LIB_ALIAS* al = lib->FindAlias( elem );
            newList[it][al->GetName()];
            if( al->GetPart()->IsMulti() )
                for( int u = 1; u <= al->GetPart()->GetUnitCount(); ++u )
                {
                    wxString unitName = _("Unit");
                    unitName += wxT( " " ) + LIB_PART::SubReference( u, false );
                    newList[it][elem].Add(unitName);
                }
        }
    }
    ResetTree();

    for( const auto item : newList ) {
        if( item.second.empty() )
            continue;
        InsertLibrary( item.first );
        for( const auto comp : item.second )
        {
            InsertAsChildOf( comp.first, item.first );
            for( auto x : comp.second )
                InsertAsChildOf( x, comp.first, item.first );
        }
    }
    if( GetCount() < ELEM_NUM_FOR_EXPAND_ALL )
        ExpandAll();
}

wxString EESCHEMA_TREE::GetComponent() const
{
    wxArrayTreeItemIds elems = GetSelected();
    if(elems.empty())
       return wxEmptyString;
    for( unsigned num = 0;
          num < elems.GetCount();
          ++num )
    {
        assert( elems[num].IsOk() );

        if( elems[num] == GetTree()->GetRootItem() ||
              GetTree()->GetItemParent( elems[num] ) == GetTree()->GetRootItem() )
           return wxEmptyString;
        if( num > 0 &&
              GetTree()->GetItemParent( elems[num] ) != GetTree()->GetItemParent( elems[num-1] ) )
           return wxEmptyString;
    }
    return GetTree()->GetItemText(GetTree()->GetItemParent(elems[0]));
}
void EESCHEMA_TREE::OnLeftDClick( wxMouseEvent& aEvent )
{
    const wxTreeItemId item = GetTree()->GetFocusedItem();
    if( !item.IsOk() ||
          item == GetTree()->GetRootItem() )
    {
        return;
    }
    if( GetTree()->GetItemParent(item) == GetTree()->GetRootItem() )
    {
        wxMessageBox( _( "Cannot show a library. Select a component instead." ), _( WRONG_SELECTION ), wxOK | wxICON_EXCLAMATION | wxCENTRE );
        return;
    }
    aEvent.Skip();

    //assert(dynamic_cast<LIB_VIEW_FRAME*>(GetParent()));
    //static_cast<LIB_VIEW_FRAME*>(GetParent())->SetSelectedComponent( GetTree()->GetItemText(footprintName) );
}
