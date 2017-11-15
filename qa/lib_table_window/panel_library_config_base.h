///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Nov 10 2017)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __PANEL_LIBRARY_CONFIG_BASE_H__
#define __PANEL_LIBRARY_CONFIG_BASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/dataview.h>
#include <wx/panel.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class PANEL_LIBRARY_CONFIG_BASE
///////////////////////////////////////////////////////////////////////////////
class PANEL_LIBRARY_CONFIG_BASE : public wxPanel 
{
	private:
	
	protected:
		wxStaticText* m_staticText2;
		wxChoice* m_tableScope;
		wxButton* m_goToAdvanced;
		wxDataViewCtrl* m_view;
		wxButton* m_addButton;
		wxButton* m_removeButton;
		wxButton* m_wizardButton;
		wxButton* m_moveUpButton;
		wxButton* m_moveDownButton;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnLaunchAdvancedSettings( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAddLibrary( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRemoveLibrary( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRunWizard( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMoveUp( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnMoveDown( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		PANEL_LIBRARY_CONFIG_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxTAB_TRAVERSAL ); 
		~PANEL_LIBRARY_CONFIG_BASE();
	
};

#endif //__PANEL_LIBRARY_CONFIG_BASE_H__
