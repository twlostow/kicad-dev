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
#include <dialog_search_tree.h>
#include <footprint_info.h>

class COMPONENT;
class PART_LIBS;
class wxConfigBase;

///////////////////////////////////////////////////////////////////////////////
/// Class EESCHEMA_TREE
/// > library_name_1
///   > comp_1
///     > part A
///     > part B
///   > comp_2
///   > comp_3
///   > comp_4
/// > library_name_2
///   > comp_1
///   > comp_2
///     > part A
///     > part B
///   > comp_3
///   > comp_4
/// > library_name_3
///   > comp_1
///   > comp_2
///   > comp_3
///   > comp_4
///     > part A
///     > part B
///////////////////////////////////////////////////////////////////////////////
class EESCHEMA_TREE : public SEARCH_TREE
{
   private:
      wxMenu* m_menu;
      wxStaticText* m_staticFootCandText;
      wxListBox* m_footprintCandidates;
      wxArrayString m_cutOrCopyElems;
      PART_LIBS* m_bufLibs;
      void CheckValidity(wxTreeEvent& aEvent);
      wxFileName CreateLib( wxString aName );
      void Update();

	protected:
		// Overriden methods
		void OnLeftDClick( wxMouseEvent& aEvent ) override;
		void OnRightClick( wxMouseEvent& aEvent ) override;
      void LoadSettings( wxConfigBase* aCfg ) override;

	public:
      wxString GetFirstLibraryNameAvailable() const;
      wxString GetFirstComponentNameAvailable(const wxString& aName) const;
      void OnPopupClick( wxCommandEvent &evt );
      void LoadFootprints( PART_LIBS* aLibs );
      void SaveSettings( wxConfigBase* aCfg ) override;
      wxString GetComponent() const;
      wxArrayString GetCutOrCopy() const;
		EESCHEMA_TREE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT|wxTR_SINGLE ); 
		~EESCHEMA_TREE();

   wxDECLARE_EVENT_TABLE();
};

#endif //__DIALOG_FOOTPRINTS_TREE_H__
