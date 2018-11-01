///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 17 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DIALOG_CRASH_REPORT_BASE_H__
#define __DIALOG_CRASH_REPORT_BASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include "dialog_shim.h"
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/statline.h>
#include <wx/textctrl.h>
#include <wx/hyperlink.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_CRASH_REPORT_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_CRASH_REPORT_BASE : public DIALOG_SHIM
{
	private:
	
	protected:
		wxStaticText* m_staticText1;
		wxStaticLine* m_staticline1;
		wxStaticText* m_staticText2;
		wxTextCtrl* m_additionalInfo;
		wxStaticText* m_staticText211;
		wxHyperlinkCtrl* m_hyperlink2;
		wxStaticLine* m_staticline2;
		wxButton* m_btnViewReport;
		wxButton* m_btnSendReport;
		wxButton* m_btnExit;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnViewReport( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSendReport( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnExitKicad( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		DIALOG_CRASH_REPORT_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Kicad Crash Reporter"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 730,649 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~DIALOG_CRASH_REPORT_BASE();
	
};

#endif //__DIALOG_CRASH_REPORT_BASE_H__
