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

#include "dialog_footprints_tree.h"
#include <cvpcb_mainframe.h>
#include <cvpcb_id.h>
#include <eda_pattern_match.h>
#include <listview_classes.h>
#include <fp_lib_table.h>

// If the number of elements is less than this,
// the code will automatically expand the tree elements.
#define ELEM_NUM_FOR_EXPAND_ALL 45

FOOTPRINTS_TREE::FOOTPRINTS_TREE( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
   : SEARCH_TREE( parent, id, pos, size, wxTR_HIDE_ROOT|wxTR_SINGLE | style )
{
   assert(GetTree());
   //SetPreview( true, "Preview" );
}

FOOTPRINTS_TREE::~FOOTPRINTS_TREE()
{}

const wxArrayString FOOTPRINTS_TREE::GetSelectedLibraries() const
{
   wxArrayString ret_val = SEARCH_TREE::GetSelectedLibraries();
   assert(ret_val.GetCount() <= 1);
   return ret_val;
}

const wxArrayString FOOTPRINTS_TREE::GetSelectedElements() const
{
   wxArrayString ret_val = SEARCH_TREE::GetSelectedElements();
   assert(ret_val.GetCount() <= 1);
   return ret_val;
}

void FOOTPRINTS_TREE::LoadFootprints( FP_LIB_TABLE* aList)
{
    m_footPrints.ReadFootprintFiles( aList );
    if( m_footPrints.GetErrorCount() )
        return;
    Update();
}
void FOOTPRINTS_TREE::Update()
{
    std::map<wxString, wxArrayString>   newList;
    wxString        msg;
    wxString        oldSelection;

    EDA_PATTERN_MATCH_WILDCARD patternFilter;
    // Use case insensitive search
    patternFilter.SetPattern( m_filterPattern.Lower() );

    assert( m_footPrints.GetCount() );
    for( unsigned ii = 0; ii < m_footPrints.GetCount(); ++ii )
    {
        if( (m_filteringOptions & SEARCH_TREE::FILTERING_BY_NAMES )
              && !MatchKeywords( m_footPrints.GetItem( ii ).GetFootprintName() ) )
            continue;

        if( (m_filteringOptions & SEARCH_TREE::FILTERING_BY_PIN_COUNT)
            && GetPinCount() != m_footPrints.GetItem( ii ).GetUniquePadCount() )
            continue;

        // We can search (Using case insensitive search) in full FPID or only
        // in the fp name itself.
        // After tests, only in the fp name itself looks better.
        // However, the code to take in account the nickname is just commented, no removed.
        wxString currname = //aList.GetItem( ii ).GetNickname().Lower() + ":" +
                            m_footPrints.GetItem( ii ).GetFootprintName().Lower();
        assert(!currname.IsEmpty());

        if( !m_filterPattern.IsEmpty() && patternFilter.Find( currname ) == EDA_PATTERN_NOT_FOUND )
        {
            continue;
        }

        msg.Printf( wxT( "%s" ), GetChars( m_footPrints.GetItem( ii ).GetFootprintName() ) );
        newList[m_footPrints.GetItem(ii).GetNickname()].Add( msg );
    }
    ResetTree();

    for( const auto item : newList ) {
       InsertLibrary( item.first );
       for( const auto foot : item.second )
          InsertAsChildOf( foot, item.first );
    }
    if( GetCount() < ELEM_NUM_FOR_EXPAND_ALL )
       ExpandAll();
}

wxArrayTreeItemIds FOOTPRINTS_TREE::GetSelected() const
{
   wxArrayTreeItemIds elems = SEARCH_TREE::GetSelected();
   assert(elems.GetCount() <= 1);
   return elems;
};

void FOOTPRINTS_TREE::OnLeftDClick( wxMouseEvent& event )
{
  // Validation
  const wxTreeItemId footprintName = GetTree()->GetFocusedItem();
  if( !footprintName.IsOk()
        || GetTree()->GetItemParent(footprintName) == GetTree()->GetRootItem()
        || GetTree()->GetRootItem() == footprintName )
  {
     return;
  }

  assert(dynamic_cast<CVPCB_MAINFRAME*>(GetParent()));
  static_cast<CVPCB_MAINFRAME*>(GetParent())->SetNewPkg( GetTree()->GetItemText(footprintName) );
}
