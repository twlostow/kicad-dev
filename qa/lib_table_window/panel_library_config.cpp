#include <wx/filefn.h>

#include "panel_library_config.h"
#include "dialog_pick_library_files_base.h"

#include <dialogs/wizard_add_fplib.h>

static const int FILTER_COUNT = 4;
static const struct
{
    wxString m_Description; ///< Description shown in the file picker dialog
    wxString m_Extension;   ///< In case of folders it stands for extensions of files stored inside
    bool m_IsFile;          ///< Whether it is a folder or a file
    IO_MGR::PCB_FILE_T m_Plugin;
} PCBLibFilters[FILTER_COUNT] =
{
    { "KiCad (folder with .kicad_mod files)",   "kicad_mod",   false,   IO_MGR::KICAD_SEXP },
    { "Eagle 6.x (*.lbr)",                      "lbr",         true,    IO_MGR::EAGLE },
    { "KiCad legacy (*.mod)",                   "mod",         true,    IO_MGR::LEGACY },
    { "Geda (folder with *.fp files)",          "fp",          false,   IO_MGR::GEDA_PCB },
};

class DIALOG_PICK_LIBRARY_FILES : public DIALOG_PICK_LIBRARY_FILES_BASE
{
public:
    DIALOG_PICK_LIBRARY_FILES( wxWindow* aParent ) :
    DIALOG_PICK_LIBRARY_FILES_BASE( aParent ) {};


    void SetPath( wxString path )
    {
        m_dirCtrl->SetPath( path );
    }

    void SetPCBMode ( bool aIsPcb )
    {
        if ( aIsPcb )
        {
            m_dirCtrl->SetFilter( getPCBFilterString() );
        }
    }

private:

    // Filters for the file picker


    // Returns the filter string for the file picker
    wxString getPCBFilterString()
    {
        wxString filterInit = _( "All supported library formats|" );
        wxString filter;

        for( int i = 0; i < FILTER_COUNT; ++i )
        {
            // Init part
            if( i != 0 )
                filterInit += ";";

            filterInit += "*." + PCBLibFilters[i].m_Extension;

            // Rest of the filter string
            filter += "|" + PCBLibFilters[i].m_Description +
                      "|" + ( PCBLibFilters[i].m_IsFile ? "*." + PCBLibFilters[i].m_Extension : "" );
        }

        return filterInit + filter;
    }


};

PANEL_LIBRARY_CONFIG::PANEL_LIBRARY_CONFIG( wxWindow* parent,
        wxWindowID id,
        const wxPoint& pos,
        const wxSize& size,
        long style )
    : PANEL_LIBRARY_CONFIG_BASE( parent, id, pos, size, style )
{
    auto br = new wxDataViewBitmapRenderer ();
    auto col0 = new wxDataViewColumn( "", br, LIB_LIST_MODEL::COL_LIBRARY_WRITABLE, 30, wxALIGN_CENTER );
    m_view->AppendColumn( col0 );

    auto tr = new wxDataViewTextRenderer( "string", wxDATAVIEW_CELL_INERT );
    auto col1 = new wxDataViewColumn( "Name", tr, LIB_LIST_MODEL::COL_LIBRARY_NAME, 200, wxALIGN_LEFT );
    m_view->AppendColumn( col1 );

    auto tr2 = new wxDataViewTextRenderer( "string", wxDATAVIEW_CELL_INERT );
    auto col2 = new wxDataViewColumn( "Type", tr2, LIB_LIST_MODEL::COL_LIBRARY_TYPE, 100, wxALIGN_LEFT );
    m_view->AppendColumn( col2 );

    auto tr4 = new wxDataViewToggleRenderer ( "bool", wxDATAVIEW_CELL_EDITABLE, wxALIGN_CENTER);
    auto col4 = new wxDataViewColumn( "Enabled", tr4, LIB_LIST_MODEL::COL_LIBRARY_ENABLED, 80, wxALIGN_CENTER );
    m_view->AppendColumn( col4 );

    auto tr3 = new wxDataViewTextRenderer( "string", wxDATAVIEW_CELL_INERT );
    auto col3 = new wxDataViewColumn( "Path", tr3, LIB_LIST_MODEL::COL_LIBRARY_PATH, 400, wxALIGN_LEFT );
    m_view->AppendColumn( col3 );

}


PANEL_LIBRARY_CONFIG::~PANEL_LIBRARY_CONFIG()
{
    printf("destroy!\n");
}

void PANEL_LIBRARY_CONFIG::SetModel(  LIB_LIST_MODEL * aModel )
{
    m_model = aModel;
    m_view->AssociateModel( m_model );
    m_model->Reset( m_model->GetRowCount() );
    //printf("SET %d\n", m_view->GetItemCount());
}



void PANEL_LIBRARY_CONFIG::OnAddLibrary( wxCommandEvent& event )
{
    auto path = wxGetCwd ();

    DIALOG_PICK_LIBRARY_FILES dirCtrl ( this );

    printf("PATh :: %s\n", (const char *)path.c_str() );
    dirCtrl.SetPath( path );
    dirCtrl.SetPCBMode( true );

    if (dirCtrl.ShowModal() == wxID_CANCEL)
        return;     // the user changed idea...
}

void PANEL_LIBRARY_CONFIG::OnLaunchAdvancedSettings( wxCommandEvent& event )
{

}

void PANEL_LIBRARY_CONFIG::OnRemoveLibrary( wxCommandEvent& event )
{
    wxDataViewItemArray items;
    int len = m_view->GetSelections( items );
    if (len > 0)
        m_model->Remove( items );
}

void PANEL_LIBRARY_CONFIG::OnRunWizard( wxCommandEvent& event )
{
//    EndModal( 0 );
//    wxMessageBox( wxT("Hello World!") );
    //InvokeFootprintWizard( GetParent(), nullptr, nullptr );

}

void PANEL_LIBRARY_CONFIG::OnMoveUp( wxCommandEvent& event )
{
    wxDataViewItemArray items;
    int len = m_view->GetSelections( items );
    if (len > 0)
        m_model->MoveRows( items, true );

    m_view->SetSelections( items );
    m_view->EnsureVisible( items[0] );
}

void PANEL_LIBRARY_CONFIG::OnMoveDown( wxCommandEvent& event )
{
    wxDataViewItemArray items;
    int len = m_view->GetSelections( items );
    if (len > 0)
        m_model->MoveRows( items, false );
    m_view->SetSelections( items );
    m_view->EnsureVisible( items[0] );
}
