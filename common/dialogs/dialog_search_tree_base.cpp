///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Feb 16 2016)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "dialog_search_tree_base.h"

///////////////////////////////////////////////////////////////////////////

SEARCH_TREE_BASE::SEARCH_TREE_BASE( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	m_sizer = new wxBoxSizer( wxVERTICAL );
	
	m_searchSizer = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticFilterText = new wxStaticText( this, wxID_ANY, wxT("Filter:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticFilterText->Wrap( -1 );
	m_searchSizer->Add( m_staticFilterText, 0, wxALL, 5 );
	
	m_text = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_NO_VSCROLL );
	m_searchSizer->Add( m_text, 1, wxALL, 5 );
	
	
	m_sizer->Add( m_searchSizer, 0, wxEXPAND, 5 );
	
	m_tree = new wxTreeCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT|wxTR_SINGLE );
	m_sizer->Add( m_tree, 1, wxALL|wxEXPAND, 5 );
	
	m_labels = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticDefaultText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_staticDefaultText->Wrap( -1 );
	m_labels->Add( m_staticDefaultText, 1, wxALL, 5 );
	
	
	m_sizer->Add( m_labels, 0, wxEXPAND, 5 );
	
	m_previews = new wxBoxSizer( wxHORIZONTAL );
	
	m_preview1 = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_previews->Add( m_preview1, 1, wxALL|wxEXPAND, 5 );
	
	
	m_sizer->Add( m_previews, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( m_sizer );
	this->Layout();
	
	// Connect Events
	m_text->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( SEARCH_TREE_BASE::OnTextChanged ), NULL, this );
	m_tree->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( SEARCH_TREE_BASE::OnLeftDClick ), NULL, this );
}

SEARCH_TREE_BASE::~SEARCH_TREE_BASE()
{
	// Disconnect Events
	m_text->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( SEARCH_TREE_BASE::OnTextChanged ), NULL, this );
	m_tree->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( SEARCH_TREE_BASE::OnLeftDClick ), NULL, this );
	
}
