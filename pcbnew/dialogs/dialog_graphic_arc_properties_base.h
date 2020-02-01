///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __DIALOG_GRAPHIC_ARC_PROPERTIES_BASE_H__
#define __DIALOG_GRAPHIC_ARC_PROPERTIES_BASE_H__

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
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/statline.h>
#include <wx/bmpcbox.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class DIALOG_GRAPHIC_ARC_PROPERTIES_BASE
///////////////////////////////////////////////////////////////////////////////
class DIALOG_GRAPHIC_ARC_PROPERTIES_BASE : public DIALOG_SHIM
{
	private:
	
	protected:
		wxStaticText* m_staticText45;
		wxRadioButton* m_defineAsCoordinates;
		wxRadioButton* m_defineAsAngleRadius;
		wxStaticText* m_StartPointXLabel1;
		wxTextCtrl* m_startXCtrl;
		wxStaticText* m_startXUnit;
		wxStaticText* m_StartPointYLabel;
		wxTextCtrl* m_startYCtrl;
		wxStaticText* m_startYUnit;
		wxStaticText* m_EndPointXLabel;
		wxTextCtrl* m_endXCtrl;
		wxStaticText* m_endXUnit;
		wxStaticText* m_EndPointYLabel;
		wxTextCtrl* m_endYCtrl;
		wxStaticText* m_endYUnit;
		wxStaticText* m_EndPointXLabel2;
		wxTextCtrl* m_centerXCtrl;
		wxStaticText* m_centerXUnit;
		wxStaticText* m_EndPointYLabel1;
		wxTextCtrl* m_centerYCtrl;
		wxStaticText* m_centerYUnit;
		wxStaticText* m_staticText55;
		wxTextCtrl* m_radiusCtrl;
		wxStaticText* m_radiusUnit;
		wxStaticText* m_staticText551;
		wxTextCtrl* m_startAngleCtrl;
		wxStaticText* m_startAngleUnit;
		wxStaticText* m_staticText5512;
		wxTextCtrl* m_centralAngleCtrl;
		wxStaticText* m_centralAngleUnit;
		wxStaticLine* m_staticline4;
		wxStaticText* m_ThicknessLabel;
		wxTextCtrl* m_widthCtrl;
		wxStaticText* m_widthUnit;
		wxStaticText* m_LayerLabel;
		PCB_LAYER_BOX_SELECTOR* m_layerCtrl;
		wxStaticLine* m_staticline6;
		wxCheckBox* m_fixPosition;
		wxCheckBox* m_fixCentralAngle;
		wxCheckBox* m_fixStartAngle;
		wxCheckBox* m_fixRadius;
		wxStdDialogButtonSizer* m_stdButtons;
		wxButton* m_stdButtonsOK;
		wxButton* m_stdButtonsCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void onClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnInitDlg( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void onDefineAsCoords( wxCommandEvent& event ) { event.Skip(); }
		virtual void onDefineAsAngleRadius( wxCommandEvent& event ) { event.Skip(); }
		virtual void onCancelClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void onOkClick( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		DIALOG_GRAPHIC_ARC_PROPERTIES_BASE( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Graphic Arc Properties"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 607,533 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxSYSTEM_MENU ); 
		~DIALOG_GRAPHIC_ARC_PROPERTIES_BASE();
	
};

#endif //__DIALOG_GRAPHIC_ARC_PROPERTIES_BASE_H__
