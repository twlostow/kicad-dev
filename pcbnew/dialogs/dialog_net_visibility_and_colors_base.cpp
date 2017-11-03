///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_net_visibility_and_colors_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_NET_VISIBILITY_AND_COLORS_BASE::DIALOG_NET_VISIBILITY_AND_COLORS_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DIALOG_SHIM( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
	
	m_MainSizer = new wxBoxSizer( wxVERTICAL );
	
	m_MainSizer->SetMinSize( wxSize( 700,400 ) ); 
	m_useCustom = new wxCheckBox( this, wxID_ANY, _("Use custom visibility and color rules"), wxDefaultPosition, wxDefaultSize, 0 );
	m_useCustom->SetValue(true); 
	m_MainSizer->Add( m_useCustom, 0, wxALL|wxEXPAND, 5 );
	
	m_dataView = new wxDataViewCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_MULTIPLE|wxDV_ROW_LINES );
	m_dataView->SetMinSize( wxSize( 700,400 ) );
	
	m_MainSizer->Add( m_dataView, 1, wxALL, 5 );
	
	m_StdButtons = new wxStdDialogButtonSizer();
	m_StdButtonsOK = new wxButton( this, wxID_OK );
	m_StdButtons->AddButton( m_StdButtonsOK );
	m_StdButtonsCancel = new wxButton( this, wxID_CANCEL );
	m_StdButtons->AddButton( m_StdButtonsCancel );
	m_StdButtons->Realize();
	
	m_MainSizer->Add( m_StdButtons, 0, wxALL|wxEXPAND, 5 );
	
	
	this->SetSizer( m_MainSizer );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DIALOG_NET_VISIBILITY_AND_COLORS_BASE::onClose ) );
	this->Connect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( DIALOG_NET_VISIBILITY_AND_COLORS_BASE::OnInitDlg ) );
	this->Connect( wxID_ANY, wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, wxDataViewEventHandler( DIALOG_NET_VISIBILITY_AND_COLORS_BASE::onCtrlItemActivated ) );
	this->Connect( wxID_ANY, wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler( DIALOG_NET_VISIBILITY_AND_COLORS_BASE::onCtrlItemContextMenu ) );
	m_StdButtonsCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_NET_VISIBILITY_AND_COLORS_BASE::onCancelClick ), NULL, this );
	m_StdButtonsOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_NET_VISIBILITY_AND_COLORS_BASE::onOkClick ), NULL, this );
}

DIALOG_NET_VISIBILITY_AND_COLORS_BASE::~DIALOG_NET_VISIBILITY_AND_COLORS_BASE()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DIALOG_NET_VISIBILITY_AND_COLORS_BASE::onClose ) );
	this->Disconnect( wxEVT_INIT_DIALOG, wxInitDialogEventHandler( DIALOG_NET_VISIBILITY_AND_COLORS_BASE::OnInitDlg ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED, wxDataViewEventHandler( DIALOG_NET_VISIBILITY_AND_COLORS_BASE::onCtrlItemActivated ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, wxDataViewEventHandler( DIALOG_NET_VISIBILITY_AND_COLORS_BASE::onCtrlItemContextMenu ) );
	m_StdButtonsCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_NET_VISIBILITY_AND_COLORS_BASE::onCancelClick ), NULL, this );
	m_StdButtonsOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_NET_VISIBILITY_AND_COLORS_BASE::onOkClick ), NULL, this );
	
}
