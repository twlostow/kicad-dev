///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb 16 2016)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DIALOG_SEARCH_TREE_BASE_H__
#define __DIALOG_SEARCH_TREE_BASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/treectrl.h>
#include <wx/panel.h>

///////////////////////////////////////////////////////////////////////////

#define wxID_SEARCHABLE_TREE 1000

///////////////////////////////////////////////////////////////////////////////
/// Class SEARCH_TREE_BASE
///////////////////////////////////////////////////////////////////////////////
class SEARCH_TREE_BASE : public wxPanel 
{
	private:
		wxBoxSizer* m_sizer;
		wxStaticText* m_staticFilterText;
	
	protected:
		wxBoxSizer* m_searchSizer;
		wxTextCtrl* m_text;
		wxTreeCtrl* m_tree;
		wxBoxSizer* m_labels;
		wxStaticText* m_staticDefaultText;
		wxBoxSizer* m_previews;
		wxTextCtrl* m_preview1;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnTextChanged( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLeftDClick( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnLeftDown( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnRightClick( wxMouseEvent& event ) { event.Skip(); }
		
	
	public:
		
		SEARCH_TREE_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxTAB_TRAVERSAL ); 
		~SEARCH_TREE_BASE();
	
};

#endif //__DIALOG_SEARCH_TREE_BASE_H__
