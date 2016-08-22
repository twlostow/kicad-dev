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

#ifndef __DIALOG_SEARCH_TREE_H__
#define __DIALOG_SEARCH_TREE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/mediactrl.h>
#include <vector>
#include <dialog_search_tree_base.h>

class wxConfigBase;

///////////////////////////////////////////////////////////////////////////////
/// Class SEARCH_TREE
/// By default this class provides an hidden root node in order to
/// have all the object in the first layer of the tree at the same level
///////////////////////////////////////////////////////////////////////////////
class SEARCH_TREE : public SEARCH_TREE_BASE
{
   static const wxString FilterFootprintEntry;
	private:
      using SEARCH_TREE_BASE::m_tree;
      const long m_style;
      bool m_hasPreview;
      wxArrayString m_keywords;
      unsigned m_pinCount;
      wxTreeItemId m_lastAddedElem;
      wxTreeItemId FindItem( const wxTreeItemId& aRoot, const wxString& aSearchFor ) const;
      void SelectItem( const wxTreeItemId& aItem );
      void SelectItems( const wxArrayTreeItemIds& aItems );

	protected:
      int m_filteringOptions;
      wxString m_filterPattern;
      bool MatchKeywords( const wxString& aSearchFor ) const;
		virtual wxArrayTreeItemIds GetSelected() const;
      wxTreeItemId AddLibrary( const wxString& aLibrary );
      // Depth First Search
      wxTreeItemId FindItem( const wxString& aSearchFor, const wxString& aParent = wxEmptyString ) const;
      // First layer of the tree
      wxTreeItemId FindLibrary( const wxString& aSearchFor ) const;
      /*
       * Interface methods for children classes in order to decouple their implementation
       * with respect to the base class
       */
      wxBoxSizer* GetSizerForLabels() const;
      wxBoxSizer* GetSizerForPreviews() const;
      wxTreeCtrl* GetTree() const;
      unsigned GetPinCount();
      void SetPreview( bool aVal, const wxString& aText = wxString("Preview") );
      bool HasPreview() const;

		/*
       * Virtual event handlers, override them in your derived class
       * By default they do nothing.
       */
		virtual void OnTextChanged( wxCommandEvent& aEvent );
		virtual void OnLeftDClick( wxMouseEvent& aEvent );
      /*
       * If you override this methods you MUST call this function.
       * The idea is that every class save its own internal.
       */
      virtual void LoadSettings( wxConfigBase* aCfg );

	public:
      enum FILTER {
          UNFILTERED                      = 0,
          FILTERING_BY_PIN_COUNT          = 0x0002,
          FILTERING_BY_NAMES              = 0x0001
      };
      /*
       * This function will be called when the widget needs to update itself
       * this is the only context-dependant method;
       */
      virtual void Update() = 0;
		unsigned int GetCount() const;
      void ExpandAll();
      void ExpandSelected();
      void Expand( const wxString& aElem );
      void ResetTree();
      void SelectItem( const wxString& aItem, const wxString& aParent = wxEmptyString );
      virtual void SaveSettings( wxConfigBase* aCfg );
      int GetFilteringOptions() const;
      bool IsSelected( const wxString& aSearchFor ) const;
      void SetFilteringKeywords(const wxArrayString aFilter );
      void SetPinCount( unsigned aNum );
      // Insert as child of the root node
      void InsertLibrary( const wxString& aLibrary );
      void RemoveLibrary( const wxString& aLibrary );
      // Insert as child of the corresponding library
      void InsertAsChildOf( const wxString& aElem, const wxString& aParent, const wxString& aGrandPa = wxEmptyString );
      void RemoveLastAdded();
      virtual const wxArrayString GetSelectedLibraries() const;
      virtual const wxArrayString GetSelectedElements() const;
      void OnFiltering( bool aApply, SEARCH_TREE::FILTER aFilter );
		SEARCH_TREE( wxWindow* aParent, wxWindowID aId = wxID_ANY, const wxPoint& aPos = wxDefaultPosition, const wxSize& aSize = wxSize( 500,300 ), long aStyle = 0 );
		~SEARCH_TREE();
      // The following functions are needed to handle a wxTreeEvent
      wxString GetText( const wxTreeItemId& aElem ) const;
      bool IsLibrary( const wxTreeItemId& aElem ) const;
      wxString GetLibrary( const wxTreeItemId& aElem ) const;

      void SetItemModified( wxTreeItemId& aItem );

};

#endif //__DIALOG_SEARCH_TREE_H__
