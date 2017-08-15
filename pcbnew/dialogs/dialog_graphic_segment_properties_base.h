///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DIALOG_GRAPHIC_SEGMENT_PROPERTIES_BASE_H__
#define __DIALOG_GRAPHIC_SEGMENT_PROPERTIES_BASE_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
class DIALOG_SHIM;
class PCB_LAYER_BOX_SELECTOR;

#include "dialog_shim.h"
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/bmpcbox.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_GRAPHIC_SEGMENT_PROPERTIES_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_GRAPHIC_SEGMENT_PROPERTIES_BASE : public DIALOG_SHIM
{
	private:
	
	protected:
		wxStaticText* m_StartPointXLabel1;
		wxStaticText* m_StartPointXLabel11;
		wxTextCtrl* m_startXCtrl;
		wxStaticText* m_startXUnit;
		wxStaticText* m_StartPointYLabel;
		wxTextCtrl* m_startYCtrl;
		wxStaticText* m_startYUnit;
		wxStaticText* m_EndPointXLabel;
		wxStaticText* m_EndPointXLabel1;
		wxTextCtrl* m_endXCtrl;
		wxStaticText* m_endXUnit;
		wxStaticText* m_EndPointYLabel;
		wxTextCtrl* m_endYCtrl;
		wxStaticText* m_endYUnit;
		wxStaticLine* m_staticline4;
		wxStaticText* m_ThicknessLabel;
		wxTextCtrl* m_widthCtrl;
		wxStaticText* m_widthUnit;
		wxStaticText* m_LayerLabel;
		PCB_LAYER_BOX_SELECTOR* m_layerCtrl;
		wxStaticLine* m_staticline6;
		wxCheckBox* m_fixLength;
		wxCheckBox* m_fixDirection;
		wxStdDialogButtonSizer* m_stdButtons;
		wxButton* m_stdButtonsOK;
		wxButton* m_stdButtonsCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnInitDlg( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void onCancelClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void onOkClick( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		DIALOG_GRAPHIC_SEGMENT_PROPERTIES_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Graphic Segment Properties"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 619,348 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxSYSTEM_MENU ); 
		~DIALOG_GRAPHIC_SEGMENT_PROPERTIES_BASE();
	
};

#endif //__DIALOG_GRAPHIC_SEGMENT_PROPERTIES_BASE_H__
