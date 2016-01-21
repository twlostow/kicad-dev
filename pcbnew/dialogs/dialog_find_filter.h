///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DIALOG_FIND_FILTER_H__
#define __DIALOG_FIND_FILTER_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/statline.h>
#include <wx/panel.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_FIND_FILTER
///////////////////////////////////////////////////////////////////////////////
class DIALOG_FIND_FILTER : public wxPanel 
{
	private:
	
	protected:
		wxStaticText* m_staticText1;
		wxButton* m_btnAll;
		wxButton* m_btnNone;
		wxCheckBox* m_chkTracks;
		wxCheckBox* m_chkVias;
		wxCheckBox* m_chkPads;
		wxCheckBox* m_chkComps;
		wxCheckBox* m_chkCompText;
		wxCheckBox* m_chkZones;
		wxCheckBox* m_chkGraphics;
		wxCheckBox* m_chkMech;
		wxCheckBox* m_chkDrc;
		wxCheckBox* m_chkNets;
		wxTextCtrl* m_nets;
		wxCheckBox* m_chkText;
		wxTextCtrl* m_text;
		wxStaticLine* m_staticline4;
		wxCheckBox* m_chkIncludeLocked;
		wxCheckBox* m_chkMaskEnabled;
		wxButton* m_btnFindNext;
		wxButton* m_btnBack;
		wxStaticText* m_staticText2;
	
	public:
		
		DIALOG_FIND_FILTER( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 210,692 ), long style = wxTAB_TRAVERSAL ); 
		~DIALOG_FIND_FILTER();
	
};

#endif //__DIALOG_FIND_FILTER_H__
