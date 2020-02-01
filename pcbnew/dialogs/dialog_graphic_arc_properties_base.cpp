///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "pcb_layer_box_selector.h"

#include "dialog_graphic_arc_properties_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_GRAPHIC_ARC_PROPERTIES_BASE::DIALOG_GRAPHIC_ARC_PROPERTIES_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DIALOG_SHIM( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
	
	wxBoxSizer* bMainSizer;
	bMainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* m_sizer1;
	m_sizer1 = new wxFlexGridSizer( 1, 3, 0, 0 );
	m_sizer1->SetFlexibleDirection( wxBOTH );
	m_sizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText45 = new wxStaticText( this, wxID_ANY, _("Define as:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText45->Wrap( -1 );
	m_sizer1->Add( m_staticText45, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_defineAsCoordinates = new wxRadioButton( this, wxID_ANY, _("Endpoint coordinates"), wxDefaultPosition, wxDefaultSize, 0 );
	m_sizer1->Add( m_defineAsCoordinates, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_defineAsAngleRadius = new wxRadioButton( this, wxID_ANY, _("Center/Angle/Radius"), wxDefaultPosition, wxDefaultSize, 0 );
	m_sizer1->Add( m_defineAsAngleRadius, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bMainSizer->Add( m_sizer1, 0, wxEXPAND, 5 );
	
	wxFlexGridSizer* m_sizer2;
	m_sizer2 = new wxFlexGridSizer( 6, 6, 0, 0 );
	m_sizer2->SetFlexibleDirection( wxBOTH );
	m_sizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	m_StartPointXLabel1 = new wxStaticText( this, wxID_ANY, _("Start X:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_StartPointXLabel1->Wrap( -1 );
	m_sizer2->Add( m_StartPointXLabel1, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_startXCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_startXCtrl->SetMaxLength( 0 ); 
	m_sizer2->Add( m_startXCtrl, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxLEFT|wxTOP, 5 );
	
	m_startXUnit = new wxStaticText( this, wxID_ANY, _("Unit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_startXUnit->Wrap( -1 );
	m_sizer2->Add( m_startXUnit, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_StartPointYLabel = new wxStaticText( this, wxID_ANY, _("Y:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_StartPointYLabel->Wrap( -1 );
	m_sizer2->Add( m_StartPointYLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxBOTTOM|wxLEFT|wxTOP, 5 );
	
	m_startYCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_startYCtrl->SetMaxLength( 0 ); 
	m_sizer2->Add( m_startYCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_startYUnit = new wxStaticText( this, wxID_ANY, _("Unit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_startYUnit->Wrap( -1 );
	m_sizer2->Add( m_startYUnit, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );
	
	m_EndPointXLabel = new wxStaticText( this, wxID_ANY, _("End X:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_EndPointXLabel->Wrap( -1 );
	m_sizer2->Add( m_EndPointXLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxBOTTOM|wxLEFT|wxTOP, 5 );
	
	m_endXCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_endXCtrl->SetMaxLength( 0 ); 
	m_sizer2->Add( m_endXCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_endXUnit = new wxStaticText( this, wxID_ANY, _("Unit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_endXUnit->Wrap( -1 );
	m_sizer2->Add( m_endXUnit, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	m_EndPointYLabel = new wxStaticText( this, wxID_ANY, _("Y:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_EndPointYLabel->Wrap( -1 );
	m_sizer2->Add( m_EndPointYLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxBOTTOM|wxLEFT|wxTOP, 5 );
	
	m_endYCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_endYCtrl->SetMaxLength( 0 ); 
	m_sizer2->Add( m_endYCtrl, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxALL, 5 );
	
	m_endYUnit = new wxStaticText( this, wxID_ANY, _("Unit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_endYUnit->Wrap( -1 );
	m_sizer2->Add( m_endYUnit, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );
	
	m_EndPointXLabel2 = new wxStaticText( this, wxID_ANY, _("Center X:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_EndPointXLabel2->Wrap( -1 );
	m_sizer2->Add( m_EndPointXLabel2, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );
	
	m_centerXCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_sizer2->Add( m_centerXCtrl, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_centerXUnit = new wxStaticText( this, wxID_ANY, _("Unit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_centerXUnit->Wrap( -1 );
	m_sizer2->Add( m_centerXUnit, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_EndPointYLabel1 = new wxStaticText( this, wxID_ANY, _("Y:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_EndPointYLabel1->Wrap( -1 );
	m_sizer2->Add( m_EndPointYLabel1, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_centerYCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_sizer2->Add( m_centerYCtrl, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_centerYUnit = new wxStaticText( this, wxID_ANY, _("Unit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_centerYUnit->Wrap( -1 );
	m_sizer2->Add( m_centerYUnit, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );
	
	m_staticText55 = new wxStaticText( this, wxID_ANY, _("Radius:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText55->Wrap( -1 );
	m_sizer2->Add( m_staticText55, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );
	
	m_radiusCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_sizer2->Add( m_radiusCtrl, 0, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_radiusUnit = new wxStaticText( this, wxID_ANY, _("Unit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radiusUnit->Wrap( -1 );
	m_sizer2->Add( m_radiusUnit, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	m_sizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	m_sizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	m_sizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText551 = new wxStaticText( this, wxID_ANY, _("Start Angle:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText551->Wrap( -1 );
	m_sizer2->Add( m_staticText551, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );
	
	m_startAngleCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_sizer2->Add( m_startAngleCtrl, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_startAngleUnit = new wxStaticText( this, wxID_ANY, _("degrees"), wxDefaultPosition, wxDefaultSize, 0 );
	m_startAngleUnit->Wrap( -1 );
	m_sizer2->Add( m_startAngleUnit, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	m_sizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	m_sizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	m_sizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText5512 = new wxStaticText( this, wxID_ANY, _("Central Angle:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5512->Wrap( -1 );
	m_sizer2->Add( m_staticText5512, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_centralAngleCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_sizer2->Add( m_centralAngleCtrl, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_centralAngleUnit = new wxStaticText( this, wxID_ANY, _("degrees"), wxDefaultPosition, wxDefaultSize, 0 );
	m_centralAngleUnit->Wrap( -1 );
	m_sizer2->Add( m_centralAngleUnit, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bMainSizer->Add( m_sizer2, 1, wxBOTTOM, 5 );
	
	m_staticline4 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bMainSizer->Add( m_staticline4, 0, wxEXPAND | wxALL, 5 );
	
	wxFlexGridSizer* sizer2;
	sizer2 = new wxFlexGridSizer( 0, 3, 0, 0 );
	sizer2->SetFlexibleDirection( wxBOTH );
	sizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_ThicknessLabel = new wxStaticText( this, wxID_ANY, _("Width:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ThicknessLabel->Wrap( -1 );
	sizer2->Add( m_ThicknessLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_widthCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_widthCtrl->SetMaxLength( 0 ); 
	sizer2->Add( m_widthCtrl, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxLEFT|wxTOP, 5 );
	
	m_widthUnit = new wxStaticText( this, wxID_ANY, _("Unit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_widthUnit->Wrap( -1 );
	sizer2->Add( m_widthUnit, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_LayerLabel = new wxStaticText( this, wxID_ANY, _("Layer:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_LayerLabel->Wrap( -1 );
	sizer2->Add( m_LayerLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_layerCtrl = new PCB_LAYER_BOX_SELECTOR( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	sizer2->Add( m_layerCtrl, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxLEFT|wxTOP, 5 );
	
	
	sizer2->Add( 0, 0, 1, wxEXPAND, 5 );
	
	
	bMainSizer->Add( sizer2, 0, wxEXPAND, 5 );
	
	m_staticline6 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bMainSizer->Add( m_staticline6, 0, wxEXPAND | wxALL, 5 );
	
	wxFlexGridSizer* sizer3;
	sizer3 = new wxFlexGridSizer( 0, 4, 0, 0 );
	sizer3->SetFlexibleDirection( wxBOTH );
	sizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_fixPosition = new wxCheckBox( this, wxID_ANY, _("Fix Position"), wxDefaultPosition, wxDefaultSize, 0 );
	sizer3->Add( m_fixPosition, 0, wxALL, 5 );
	
	m_fixCentralAngle = new wxCheckBox( this, wxID_ANY, _("Fix Central Angle"), wxDefaultPosition, wxDefaultSize, 0 );
	sizer3->Add( m_fixCentralAngle, 0, wxALL, 5 );
	
	m_fixStartAngle = new wxCheckBox( this, wxID_ANY, _("Fix Start Angle"), wxDefaultPosition, wxDefaultSize, 0 );
	sizer3->Add( m_fixStartAngle, 0, wxALL, 5 );
	
	m_fixRadius = new wxCheckBox( this, wxID_ANY, _("Fix Radius"), wxDefaultPosition, wxDefaultSize, 0 );
	sizer3->Add( m_fixRadius, 0, wxALL, 5 );
	
	
	bMainSizer->Add( sizer3, 1, wxEXPAND, 5 );
	
	m_stdButtons = new wxStdDialogButtonSizer();
	m_stdButtonsOK = new wxButton( this, wxID_OK );
	m_stdButtons->AddButton( m_stdButtonsOK );
	m_stdButtonsCancel = new wxButton( this, wxID_CANCEL );
	m_stdButtons->AddButton( m_stdButtonsCancel );
	m_stdButtons->Realize();
	
	bMainSizer->Add( m_stdButtons, 0, wxALIGN_RIGHT|wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( bMainSizer );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DIALOG_GRAPHIC_ARC_PROPERTIES_BASE::onClose ) );
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( DIALOG_GRAPHIC_ARC_PROPERTIES_BASE::OnInitDlg ) );
	m_defineAsCoordinates->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DIALOG_GRAPHIC_ARC_PROPERTIES_BASE::onDefineAsCoords ), NULL, this );
	m_defineAsAngleRadius->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DIALOG_GRAPHIC_ARC_PROPERTIES_BASE::onDefineAsAngleRadius ), NULL, this );
	m_stdButtonsCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_GRAPHIC_ARC_PROPERTIES_BASE::onCancelClick ), NULL, this );
	m_stdButtonsOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_GRAPHIC_ARC_PROPERTIES_BASE::onOkClick ), NULL, this );
}

DIALOG_GRAPHIC_ARC_PROPERTIES_BASE::~DIALOG_GRAPHIC_ARC_PROPERTIES_BASE()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DIALOG_GRAPHIC_ARC_PROPERTIES_BASE::onClose ) );
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( DIALOG_GRAPHIC_ARC_PROPERTIES_BASE::OnInitDlg ) );
	m_defineAsCoordinates->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DIALOG_GRAPHIC_ARC_PROPERTIES_BASE::onDefineAsCoords ), NULL, this );
	m_defineAsAngleRadius->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( DIALOG_GRAPHIC_ARC_PROPERTIES_BASE::onDefineAsAngleRadius ), NULL, this );
	m_stdButtonsCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_GRAPHIC_ARC_PROPERTIES_BASE::onCancelClick ), NULL, this );
	m_stdButtonsOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_GRAPHIC_ARC_PROPERTIES_BASE::onOkClick ), NULL, this );
	
}
