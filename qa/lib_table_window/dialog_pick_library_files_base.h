///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Nov 10 2017)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DIALOG_PICK_LIBRARY_FILES_BASE_H__
#define __DIALOG_PICK_LIBRARY_FILES_BASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include "dialog_shim.h"
#include <wx/string.h>
#include <wx/dirctrl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_PICK_LIBRARY_FILES_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_PICK_LIBRARY_FILES_BASE : public DIALOG_SHIM
{
	private:
	
	protected:
		wxGenericDirCtrl* m_dirCtrl;
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;
	
	public:
		
		DIALOG_PICK_LIBRARY_FILES_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Choose Libraries"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE ); 
		~DIALOG_PICK_LIBRARY_FILES_BASE();
	
};

#endif //__DIALOG_PICK_LIBRARY_FILES_BASE_H__
