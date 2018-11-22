///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 17 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_crash_report_base.h"

///////////////////////////////////////////////////////////////////////////

DIALOG_CRASH_REPORT_BASE::DIALOG_CRASH_REPORT_BASE( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("We are sorry."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	m_staticText1->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	
	bSizer1->Add( m_staticText1, 0, wxALL|wxEXPAND, 5 );
	
	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer1->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );
	
	m_staticText2 = new wxStaticText( this, wxID_ANY, wxT("It looks like KiCad encountered a serious problem and crashed. \n\nTo help us diagnose and fix the problem we generated a crash report that you can send to us.\n\nIf you would like to give us some additional information with the report (e.g. a brief description \nof what were you doing when the crash happened), you can write it down below:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	bSizer1->Add( m_staticText2, 0, wxEXPAND|wxALL, 5 );
	
	m_additionalInfo = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	m_additionalInfo->SetMinSize( wxSize( -1,120 ) );
	
	bSizer1->Add( m_additionalInfo, 1, wxALL|wxEXPAND, 5 );
	
	m_staticText211 = new wxStaticText( this, wxID_ANY, wxT("The report contains only the information about the crash and your operating system\nand does not include any private data. You can view the full text of the report that will be sent to us \nby clicking the View Report button. \n\nNote: if you are offline, we encourage you to click the View Report button, copy the report text\nand send it to us later by creating a bug on:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText211->Wrap( -1 );
	bSizer1->Add( m_staticText211, 0, wxALL|wxEXPAND, 5 );
	
	m_hyperlink2 = new wxHyperlinkCtrl( this, wxID_ANY, wxT("http://bugs.launchpad.net/kicad"), wxT("http://www.wxformbuilder.orghttp://bugs.launchpad.net/kicad"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
	bSizer1->Add( m_hyperlink2, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );
	
	m_staticline2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer1->Add( m_staticline2, 0, wxEXPAND | wxALL, 5 );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 1, 3, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_btnViewReport = new wxButton( this, wxID_ANY, wxT("View Report..."), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_btnViewReport, 0, wxALL, 5 );
	
	m_btnSendReport = new wxButton( this, wxID_ANY, wxT("Send Crash Report"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_btnSendReport, 1, wxALL|wxALIGN_RIGHT, 5 );
	
	m_btnExit = new wxButton( this, wxID_ANY, wxT("Exit Kicad"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_btnExit, 0, wxALL, 5 );
	
	
	bSizer1->Add( fgSizer1, 0, wxEXPAND, 5 );
	
	
	bSizer2->Add( bSizer1, 1, wxEXPAND|wxALL, 5 );
	
	
	this->SetSizer( bSizer2 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_btnViewReport->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_CRASH_REPORT_BASE::OnViewReport ), NULL, this );
	m_btnSendReport->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_CRASH_REPORT_BASE::OnSendReport ), NULL, this );
	m_btnExit->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_CRASH_REPORT_BASE::OnExitKicad ), NULL, this );
}

DIALOG_CRASH_REPORT_BASE::~DIALOG_CRASH_REPORT_BASE()
{
	// Disconnect Events
	m_btnViewReport->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_CRASH_REPORT_BASE::OnViewReport ), NULL, this );
	m_btnSendReport->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_CRASH_REPORT_BASE::OnSendReport ), NULL, this );
	m_btnExit->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DIALOG_CRASH_REPORT_BASE::OnExitKicad ), NULL, this );
	
}
