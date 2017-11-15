///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Nov 10 2017)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DIALOG_LIBRARY_SETTINGS_BASE_H__
#define __DIALOG_LIBRARY_SETTINGS_BASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
class PANEL_LIBRARY_CONFIG;

#include <wx/treectrl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/splitter.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_LIBRARY_SETTINGS_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_LIBRARY_SETTINGS_BASE : public wxDialog 
{
	private:
	
	protected:
		wxSplitterWindow* m_splitter2;
		wxPanel* m_panel4;
		wxTreeCtrl* m_optionsTree;
		wxPanel* m_panel5;
		PANEL_LIBRARY_CONFIG* m_optionsPage;
		wxStaticLine* m_staticline1;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;
	
	public:
		
		DIALOG_LIBRARY_SETTINGS_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Library Settings"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~DIALOG_LIBRARY_SETTINGS_BASE();
		
		void m_splitter2OnIdle( wxIdleEvent& )
		{
			m_splitter2->SetSashPosition( 230 );
			m_splitter2->Disconnect( wxEVT_IDLE, wxIdleEventHandler( DIALOG_LIBRARY_SETTINGS_BASE::m_splitter2OnIdle ), NULL, this );
		}
	
};

#endif //__DIALOG_LIBRARY_SETTINGS_BASE_H__
