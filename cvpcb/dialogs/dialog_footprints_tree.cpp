/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) CERN 2016 Michele Castellana, <michele.castellana@cern.ch>
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

#include "dialog_footprints_tree.h"
#include "cvpcb_mainframe.h"

FOOTPRINTS_TREE::FOOTPRINTS_TREE( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* m_sizer;
	m_sizer = new wxBoxSizer( wxHORIZONTAL );

	m_tree = new wxTreeCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT|wxTR_SINGLE );
	m_sizer->Add( m_tree, 1, wxALL|wxEXPAND, 5 );

	this->SetSizer( m_sizer );
	this->Layout();

	// Connect Events
	m_tree->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( FOOTPRINTS_TREE::OnLeftDClick ), NULL, this );
}

FOOTPRINTS_TREE::~FOOTPRINTS_TREE()
{
	// Disconnect Events
	m_tree->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( FOOTPRINTS_TREE::OnLeftDClick ), NULL, this );

}

wxTreeItemId FOOTPRINTS_TREE::FindItem( wxTreeItemId root, const wxString& sSearchFor )
{
   wxTreeItemIdValue cookie;
   wxTreeItemId item = m_tree->GetFirstChild( root, cookie );
   while( item.IsOk() ) {
      wxString sData = m_tree->GetItemText(item);
      if( sSearchFor.CompareTo(sData) == 0 ) {
         return item;
      }
      if( m_tree->ItemHasChildren( item ) ) {
         wxTreeItemId search = FindItem( item, sSearchFor );
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

void FOOTPRINTS_TREE::OnLeftDClick( wxMouseEvent& event )
{
  const wxTreeItemId footprintName = m_tree->GetFocusedItem();
  if( !footprintName.IsOk() )
  {
     return;
  }

  assert(dynamic_cast<CVPCB_MAINFRAME*>(GetParent()));
  static_cast<CVPCB_MAINFRAME*>(GetParent())->SetNewPkg( m_tree->GetItemText(footprintName) );
}
