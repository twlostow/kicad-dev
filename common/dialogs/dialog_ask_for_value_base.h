
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DIALOG_ASK_FOR_VALUE_BASE_H__
#define __DIALOG_ASK_FOR_VALUE_BASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
class DIALOG_SHIM;

#include "dialog_shim.h"
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_ASK_FOR_VALUE_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_ASK_FOR_VALUE_BASE : public DIALOG_SHIM
{
	private:
	
	protected:
		wxStaticText* m_valueLabel;
		wxTextCtrl* m_valueCtrl;
		wxStaticText* m_valueUnit;
		wxStdDialogButtonSizer* m_stdButtons;
		wxButton* m_stdButtonsOK;
		wxButton* m_stdButtonsCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnInitDlg( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void onCancelClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void onOkClick( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		DIALOG_ASK_FOR_VALUE_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Input Value"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 619,348 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxSYSTEM_MENU ); 
		~DIALOG_ASK_FOR_VALUE_BASE();
	
};

#endif //__DIALOG_ASK_FOR_VALUE_BASE_H__
