///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DIALOG_NET_VISIBILITY_AND_COLORS_BASE_H__
#define __DIALOG_NET_VISIBILITY_AND_COLORS_BASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
class DIALOG_SHIM;

#include "dialog_shim.h"
#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/dataview.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_NET_VISIBILITY_AND_COLORS_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_NET_VISIBILITY_AND_COLORS_BASE : public DIALOG_SHIM
{
	private:
	
	protected:
		wxBoxSizer* m_MainSizer;
		wxCheckBox* m_useCustom;
		wxDataViewCtrl* m_dataView;
		wxStdDialogButtonSizer* m_StdButtons;
		wxButton* m_StdButtonsOK;
		wxButton* m_StdButtonsCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnInitDlg( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void onCtrlItemActivated( wxDataViewEvent& event ) { event.Skip(); }
		virtual void onCtrlItemContextMenu( wxDataViewEvent& event ) { event.Skip(); }
		virtual void onCancelClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void onOkClick( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		DIALOG_NET_VISIBILITY_AND_COLORS_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Net Visibility & Colors"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 573,437 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxSYSTEM_MENU ); 
		~DIALOG_NET_VISIBILITY_AND_COLORS_BASE();
	
};

#endif //__DIALOG_NET_VISIBILITY_AND_COLORS_BASE_H__
