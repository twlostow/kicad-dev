///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 17 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DIALOG_CRASH_REPORT_PREVIEW_BASE_H__
#define __DIALOG_CRASH_REPORT_PREVIEW_BASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include "dialog_shim.h"
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_CRASH_REPORT_PREVIEW_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_CRASH_REPORT_PREVIEW_BASE : public DIALOG_SHIM
{
	private:
	
	protected:
		wxTextCtrl* m_text;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
	
	public:
		
		DIALOG_CRASH_REPORT_PREVIEW_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Kicad Crash Report Preview"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,658 ), long style = wxDEFAULT_DIALOG_STYLE ); 
		~DIALOG_CRASH_REPORT_PREVIEW_BASE();
	
};

#endif //__DIALOG_CRASH_REPORT_PREVIEW_BASE_H__
