///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Nov 10 2017)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "panel_library_config_base.h"

///////////////////////////////////////////////////////////////////////////

PANEL_LIBRARY_CONFIG_BASE::PANEL_LIBRARY_CONFIG_BASE( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText2 = new wxStaticText( this, wxID_ANY, _("Scope:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	bSizer6->Add( m_staticText2, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	wxString m_tableScopeChoices[] = { _("Global (all projects)"), _("Current project only") };
	int m_tableScopeNChoices = sizeof( m_tableScopeChoices ) / sizeof( wxString );
	m_tableScope = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_tableScopeNChoices, m_tableScopeChoices, 0 );
	m_tableScope->SetSelection( 0 );
	bSizer6->Add( m_tableScope, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	
	bSizer6->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_goToAdvanced = new wxButton( this, wxID_ANY, _("Advanced settings..."), wxDefaultPosition, wxDefaultSize, wxBU_RIGHT|wxNO_BORDER );
	m_goToAdvanced->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_3DDKSHADOW ) );
	m_goToAdvanced->SetToolTip( _("Go to the full Library Table configuration window. Only for hardcore h4x0rs!") );
	
	bSizer6->Add( m_goToAdvanced, 0, wxALL, 5 );
	
	
	bSizer1->Add( bSizer6, 0, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	m_view = new wxDataViewCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_MULTIPLE|wxDV_ROW_LINES );
	bSizer1->Add( m_view, 1, wxALL|wxEXPAND, 5 );
	
	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer( 0, 2, 0, 0 );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxHORIZONTAL );
	
	m_addButton = new wxButton( this, wxID_ANY, _("Add..."), wxDefaultPosition, wxDefaultSize, 0 );
	m_addButton->SetToolTip( _("Add a new library.") );
	
	bSizer7->Add( m_addButton, 0, wxALL, 5 );
	
	m_removeButton = new wxButton( this, wxID_ANY, _("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
	m_removeButton->SetToolTip( _("Remove selected libraries") );
	
	bSizer7->Add( m_removeButton, 0, wxALL, 5 );
	
	m_wizardButton = new wxButton( this, wxID_ANY, _("Wizard..."), wxDefaultPosition, wxDefaultSize, 0 );
	m_wizardButton->SetToolTip( _("Launch Library Wizard") );
	
	bSizer7->Add( m_wizardButton, 0, wxALL, 5 );
	
	
	gSizer1->Add( bSizer7, 0, 0, 5 );
	
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxHORIZONTAL );
	
	m_moveUpButton = new wxButton( this, wxID_ANY, _("Move Up"), wxDefaultPosition, wxDefaultSize, 0 );
	m_moveUpButton->SetToolTip( _("Move the currently selected row up one position") );
	
	bSizer8->Add( m_moveUpButton, 0, wxALL, 5 );
	
	m_moveDownButton = new wxButton( this, wxID_ANY, _("Move Down"), wxDefaultPosition, wxDefaultSize, 0 );
	m_moveDownButton->SetToolTip( _("Move the currently selected row down one position") );
	
	bSizer8->Add( m_moveDownButton, 0, wxALL, 5 );
	
	
	gSizer1->Add( bSizer8, 1, wxALIGN_RIGHT, 5 );
	
	
	bSizer1->Add( gSizer1, 0, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer1 );
	this->Layout();
	
	// Connect Events
	m_goToAdvanced->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PANEL_LIBRARY_CONFIG_BASE::OnLaunchAdvancedSettings ), NULL, this );
	m_addButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PANEL_LIBRARY_CONFIG_BASE::OnAddLibrary ), NULL, this );
	m_removeButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PANEL_LIBRARY_CONFIG_BASE::OnRemoveLibrary ), NULL, this );
	m_wizardButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PANEL_LIBRARY_CONFIG_BASE::OnRunWizard ), NULL, this );
	m_moveUpButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PANEL_LIBRARY_CONFIG_BASE::OnMoveUp ), NULL, this );
	m_moveDownButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PANEL_LIBRARY_CONFIG_BASE::OnMoveDown ), NULL, this );
}

PANEL_LIBRARY_CONFIG_BASE::~PANEL_LIBRARY_CONFIG_BASE()
{
	// Disconnect Events
	m_goToAdvanced->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PANEL_LIBRARY_CONFIG_BASE::OnLaunchAdvancedSettings ), NULL, this );
	m_addButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PANEL_LIBRARY_CONFIG_BASE::OnAddLibrary ), NULL, this );
	m_removeButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PANEL_LIBRARY_CONFIG_BASE::OnRemoveLibrary ), NULL, this );
	m_wizardButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PANEL_LIBRARY_CONFIG_BASE::OnRunWizard ), NULL, this );
	m_moveUpButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PANEL_LIBRARY_CONFIG_BASE::OnMoveUp ), NULL, this );
	m_moveDownButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( PANEL_LIBRARY_CONFIG_BASE::OnMoveDown ), NULL, this );
	
}
