///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Nov 10 2017)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_lib_table_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_LIB_TABLE_BASE::DIALOG_LIB_TABLE_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : DIALOG_SHIM( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	m_auinotebook = new wxAuiNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP );
	m_global_panel = new wxPanel( m_auinotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* m_global_sizer;
	m_global_sizer = new wxBoxSizer( wxVERTICAL );
	
	m_view = new wxDataViewListCtrl( m_global_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_global_sizer->Add( m_view, 1, wxALL|wxEXPAND, 5 );
	
	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer( 0, 2, 0, 0 );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxHORIZONTAL );
	
	m_append_button = new wxButton( m_global_panel, wxID_ANY, _("Add..."), wxDefaultPosition, wxDefaultSize, 0 );
	m_append_button->SetToolTip( _("Add a PCB library row to this table") );
	
	bSizer7->Add( m_append_button, 0, wxALL, 5 );
	
	m_delete_button = new wxButton( m_global_panel, wxID_ANY, _("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
	m_delete_button->SetToolTip( _("Remove a PCB library from this library table") );
	
	bSizer7->Add( m_delete_button, 0, wxALL, 5 );
	
	m_wizardButton = new wxButton( m_global_panel, wxID_ANY, _("Wizard..."), wxDefaultPosition, wxDefaultSize, 0 );
	m_wizardButton->SetToolTip( _("Remove a PCB library from this library table") );
	
	bSizer7->Add( m_wizardButton, 0, wxALL, 5 );
	
	
	gSizer1->Add( bSizer7, 0, 0, 5 );
	
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxHORIZONTAL );
	
	m_move_up_button = new wxButton( m_global_panel, wxID_ANY, _("Move Up"), wxDefaultPosition, wxDefaultSize, 0 );
	m_move_up_button->SetToolTip( _("Move the currently selected row up one position") );
	
	bSizer8->Add( m_move_up_button, 0, wxALL, 5 );
	
	m_move_down_button = new wxButton( m_global_panel, wxID_ANY, _("Move Down"), wxDefaultPosition, wxDefaultSize, 0 );
	m_move_down_button->SetToolTip( _("Move the currently selected row down one position") );
	
	bSizer8->Add( m_move_down_button, 0, wxALL, 5 );
	
	
	gSizer1->Add( bSizer8, 1, wxALIGN_RIGHT, 5 );
	
	
	m_global_sizer->Add( gSizer1, 0, wxEXPAND, 5 );
	
	
	m_global_panel->SetSizer( m_global_sizer );
	m_global_panel->Layout();
	m_global_sizer->Fit( m_global_panel );
	m_auinotebook->AddPage( m_global_panel, _("Global Libraries"), true, wxNullBitmap );
	m_project_panel = new wxPanel( m_auinotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_auinotebook->AddPage( m_project_panel, _("Project Libraries"), false, wxNullBitmap );
	
	bSizer1->Add( m_auinotebook, 6, wxEXPAND | wxALL, 5 );
	
	wxGridSizer* gSizer2;
	gSizer2 = new wxGridSizer( 0, 2, 0, 0 );
	
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );
	
	m_showAdvancedOptions = new wxCheckBox( this, wxID_ANY, _("Show advanced options (here be dragons!)"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer9->Add( m_showAdvancedOptions, 0, wxALL, 5 );
	
	
	gSizer2->Add( bSizer9, 0, wxRIGHT|wxLEFT, 5 );
	
	m_sdbSizer = new wxStdDialogButtonSizer();
	m_sdbSizerOK = new wxButton( this, wxID_OK );
	m_sdbSizer->AddButton( m_sdbSizerOK );
	m_sdbSizerCancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer->AddButton( m_sdbSizerCancel );
	m_sdbSizer->Realize();
	
	gSizer2->Add( m_sdbSizer, 0, wxALL|wxALIGN_RIGHT, 5 );
	
	
	bSizer1->Add( gSizer2, 0, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer1 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DIALOG_LIB_TABLE_BASE::onCancelCaptionButtonClick ) );
	this->Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( DIALOG_LIB_TABLE_BASE::onKeyDown ) );
	m_auinotebook->Connect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler( DIALOG_LIB_TABLE_BASE::pageChangedHandler ), NULL, this );
	m_append_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_LIB_TABLE_BASE::appendRowHandler ), NULL, this );
	m_delete_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_LIB_TABLE_BASE::deleteRowHandler ), NULL, this );
	m_wizardButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_LIB_TABLE_BASE::deleteRowHandler ), NULL, this );
	m_move_up_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_LIB_TABLE_BASE::moveUpHandler ), NULL, this );
	m_move_down_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_LIB_TABLE_BASE::moveDownHandler ), NULL, this );
	m_sdbSizerCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_LIB_TABLE_BASE::onCancelButtonClick ), NULL, this );
	m_sdbSizerOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_LIB_TABLE_BASE::onOKButtonClick ), NULL, this );
}

DIALOG_LIB_TABLE_BASE::~DIALOG_LIB_TABLE_BASE()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DIALOG_LIB_TABLE_BASE::onCancelCaptionButtonClick ) );
	this->Disconnect( wxEVT_KEY_DOWN, wxKeyEventHandler( DIALOG_LIB_TABLE_BASE::onKeyDown ) );
	m_auinotebook->Disconnect( wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED, wxAuiNotebookEventHandler( DIALOG_LIB_TABLE_BASE::pageChangedHandler ), NULL, this );
	m_append_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_LIB_TABLE_BASE::appendRowHandler ), NULL, this );
	m_delete_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_LIB_TABLE_BASE::deleteRowHandler ), NULL, this );
	m_wizardButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_LIB_TABLE_BASE::deleteRowHandler ), NULL, this );
	m_move_up_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_LIB_TABLE_BASE::moveUpHandler ), NULL, this );
	m_move_down_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_LIB_TABLE_BASE::moveDownHandler ), NULL, this );
	m_sdbSizerCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_LIB_TABLE_BASE::onCancelButtonClick ), NULL, this );
	m_sdbSizerOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_LIB_TABLE_BASE::onOKButtonClick ), NULL, this );
	
}
