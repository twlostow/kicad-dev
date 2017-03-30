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
static constexpr int ELEM_NUM_FOR_EXPAND_ALL = 45;

FOOTPRINTS_TREE::FOOTPRINTS_TREE( KIWAY* aKiway, wxWindow* parent )
   : SEARCH_TREE( parent )
{
    wxASSERT( getTree() );
    // SetPreview( true, "Preview" );

    m_footprints = FOOTPRINT_LIST::GetInstance( *aKiway );
}


FOOTPRINTS_TREE::~FOOTPRINTS_TREE()
{
}


void FOOTPRINTS_TREE::LoadFootprints( FP_LIB_TABLE* aList )
{
    m_footprints->ReadFootprintFiles( aList );

    if( m_footprints->GetErrorCount() )
    {
        m_footprints->DisplayErrors( GetParent() );
        return;
    }

    Update();
}


void FOOTPRINTS_TREE::Update()
{
    std::map<wxString, wxArrayString> newList;
    wxString    msg;
    wxString    oldSelection;

    EDA_PATTERN_MATCH_WILDCARD patternFilter;
    // Use case insensitive search
    patternFilter.SetPattern( m_filterPattern.Lower() );

    assert( m_footprints->GetCount() );

    for( unsigned ii = 0; ii < m_footprints->GetCount(); ++ii )
    {
        if( ( m_filteringOptions & FILTER_BY_NAME )
            && !matchKeywords( m_footprints->GetItem( ii ).GetFootprintName() ) )
            continue;

        if( ( m_filteringOptions & FILTER_BY_PIN_COUNT )
            && getPinCount() != m_footprints->GetItem( ii ).GetUniquePadCount() )
            continue;

        // We can search (Using case insensitive search) in full FPID or only
        // in the fp name itself.
        // After tests, only in the fp name itself looks better.
        // However, the code to take in account the nickname is just commented, no removed.
        wxString currname = //aList.GetItem( ii ).GetNickname().Lower() + ":" +
                            m_footprints->GetItem( ii ).GetFootprintName().Lower();
        assert( !currname.IsEmpty() );

        if( !m_filterPattern.IsEmpty() && patternFilter.Find( currname ) == EDA_PATTERN_NOT_FOUND )
        {
            continue;
        }

        msg.Printf( wxT( "%s" ), GetChars( m_footprints->GetItem( ii ).GetFootprintName() ) );
        newList[m_footprints->GetItem( ii ).GetNickname()].Add( msg );
    }

    ResetTree();

    for( const auto& item : newList )
    {
        InsertLibrary( item.first );

        for( const auto& foot : item.second )
            InsertItem( foot, item.first );
    }

    if( GetCount() < ELEM_NUM_FOR_EXPAND_ALL )
        ExpandAll();
}


wxArrayTreeItemIds FOOTPRINTS_TREE::GetSelected() const
{
    wxArrayTreeItemIds elems = SEARCH_TREE::GetSelected();

    assert( elems.GetCount() <= 1 );
    return elems;
}


void FOOTPRINTS_TREE::OnLeftDClick( wxMouseEvent& event )
{
    // Validation
    const wxTreeItemId footprintName = getTree()->GetFocusedItem();

    if( !footprintName.IsOk()
        || getTree()->GetItemParent( footprintName ) == getTree()->GetRootItem()
        || getTree()->GetRootItem() == footprintName )
    {
        return;
    }

    assert( dynamic_cast<CVPCB_MAINFRAME*>( SEARCH_TREE_BASE::GetParent() ) );
    static_cast<CVPCB_MAINFRAME*>( SEARCH_TREE_BASE::GetParent() )->SetNewPkg( getTree()->GetItemText( footprintName ) );
}
