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

#ifndef __DIALOG_FOOTPRINTS_TREE_H__
#define __DIALOG_FOOTPRINTS_TREE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/treectrl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/panel.h>

///////////////////////////////////////////////////////////////////////////////
/// Class FOOTPRINTS_TREE
///////////////////////////////////////////////////////////////////////////////
class FOOTPRINTS_TREE : public wxPanel
{
	protected:

		// Virtual event handlers, overide them in your derived class
		virtual void OnLeftDClick( wxMouseEvent& event );

	public:
		wxTreeCtrl* m_tree;

		wxTreeItemId FindItem( wxTreeItemId root, const wxString& sSearchFor );
		FOOTPRINTS_TREE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxTAB_TRAVERSAL ); 
		~FOOTPRINTS_TREE();

};

#endif //__DIALOG_FOOTPRINTS_TREE_H__
