
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_ask_for_value_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_ASK_FOR_VALUE_BASE::DIALOG_ASK_FOR_VALUE_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DIALOG_SHIM( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
	
	wxBoxSizer* bMainSizer;
	bMainSizer = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* sizer1;
	sizer1 = new wxFlexGridSizer( 1, 3, 0, 0 );
	sizer1->AddGrowableCol( 1 );
	sizer1->SetFlexibleDirection( wxBOTH );
	sizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_valueLabel = new wxStaticText( this, wxID_ANY, _("Label:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_valueLabel->Wrap( -1 );
	sizer1->Add( m_valueLabel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );
	
	m_valueCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_valueCtrl->SetMaxLength( 0 ); 
	sizer1->Add( m_valueCtrl, 1, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxLEFT|wxTOP|wxEXPAND, 5 );
	
	m_valueUnit = new wxStaticText( this, wxID_ANY, _("Unit"), wxDefaultPosition, wxDefaultSize, 0 );
	m_valueUnit->Wrap( -1 );
	sizer1->Add( m_valueUnit, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	
	bMainSizer->Add( sizer1, 1, wxEXPAND, 5 );
	
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
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DIALOG_ASK_FOR_VALUE_BASE::onClose ) );
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( DIALOG_ASK_FOR_VALUE_BASE::OnInitDlg ) );
	m_stdButtonsCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_ASK_FOR_VALUE_BASE::onCancelClick ), NULL, this );
	m_stdButtonsOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_ASK_FOR_VALUE_BASE::onOkClick ), NULL, this );
}

DIALOG_ASK_FOR_VALUE_BASE::~DIALOG_ASK_FOR_VALUE_BASE()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DIALOG_ASK_FOR_VALUE_BASE::onClose ) );
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( DIALOG_ASK_FOR_VALUE_BASE::OnInitDlg ) );
	m_stdButtonsCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_ASK_FOR_VALUE_BASE::onCancelClick ), NULL, this );
	m_stdButtonsOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_ASK_FOR_VALUE_BASE::onOkClick ), NULL, this );
	
}
