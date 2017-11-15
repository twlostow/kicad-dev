#include <pcb_test_frame.h>
#include <fp_lib_table.h>
#include <symbol_lib_table.h>

#include "panel_library_config.h"
#include "dialog_library_settings_base.h"




class DIALOG_LIBRARY_SETTINGS : public DIALOG_LIBRARY_SETTINGS_BASE
{
public:
    DIALOG_LIBRARY_SETTINGS( wxWindow* aParent, FP_LIB_TABLE *fpLibTable );



private:


    LIB_LIST_MODEL* getModel( bool aIsSch, bool aIsGlobal )
    {
        int i = aIsSch ? 0 : 2;
        if ( aIsGlobal )
            i++;

        return m_models[i];
    }

    void populate( LIB_LIST_MODEL* aModel, LIB_TABLE *aTable );
    void commit( LIB_LIST_MODEL* aModel, LIB_TABLE *aTable );

    FP_LIB_TABLE *m_fpLibTable;
    FP_LIB_TABLE *m_projectFpLibTable;
    SYMBOL_LIB_TABLE *m_symLibTable;
    SYMBOL_LIB_TABLE *m_projectSymLibTable;

    LIB_LIST_MODEL *m_models[4];
};



DIALOG_LIBRARY_SETTINGS::DIALOG_LIBRARY_SETTINGS( wxWindow* aParent, FP_LIB_TABLE *fpLibTable ) :
   DIALOG_LIBRARY_SETTINGS_BASE( aParent )
   {
       m_fpLibTable = fpLibTable;

       auto root = m_optionsTree->AddRoot ( _("Libraries") );

       m_optionsTree->AppendItem ( root, _("Schematic Symbols") );
       m_optionsTree->AppendItem ( root, _("PCB Footrints") );

       for ( int i = 0; i < 4; i++ )
       {
           m_models[i] = new LIB_LIST_MODEL;
       }

       populate( getModel( false, true ), m_fpLibTable );

       m_optionsPage->SetModel ( getModel( false, true )) ;
   }

   bool isLibraryWritable ( LIB_TABLE *table, wxString name )
   {
       auto tbl = dynamic_cast<FP_LIB_TABLE*>( table );
       if(!tbl)
        return false;

        return tbl->IsFootprintLibWritable( name );
   }

   void DIALOG_LIBRARY_SETTINGS::populate( LIB_LIST_MODEL* aModel, LIB_TABLE *aTable )
   {
       for(int i = 0; i < aTable->GetCount(); i++ )
       {

           auto row = aTable->At(i);
           auto type =  row ->GetType();
           auto uri = row->GetFullURI();
           auto nick = row->GetNickName();
           LIB_LIST_MODEL::ENTRY ent;

           ent.id = i;
           ent.name = nick;
           ent.path = uri;
           ent.pluginType = type;
           ent.enabled = true;
           ent.writable = isLibraryWritable( aTable, nick );

           aModel->Add( ent );

//           printf("pop %s %s %d\n", (const char *)(type.c_str()), (const char *)(uri.c_str()), m_optionsPage->Model().GetRowCount() );
       }

       aModel->Reset( aTable->GetCount() );
   }


class LIB_TEST_FRAME : public PCB_TEST_FRAME
{
public:
    LIB_TEST_FRAME( wxFrame* frame,
            const wxString& title,
            const wxPoint& pos  = wxDefaultPosition,
            const wxSize& size  = wxDefaultSize,
            long style = wxDEFAULT_FRAME_STYLE ) :
        PCB_TEST_FRAME( frame, title, pos, size, style )
    {

        auto menuBar = GetMenuBar();
        //wxMenu* prefsMenu = new wxMenu;

        //menuBar->Append( prefsMenu, wxT( "&Preferences" ) );

        //AddMenuAction( drawMenu, &PCB_ACTIONS::drawOutline );

        std::unique_ptr<FP_LIB_TABLE> fpLibTable ( new FP_LIB_TABLE );
        fpLibTable->Load("fp-lib-table");
        std::unique_ptr<SYMBOL_LIB_TABLE> symLibTable ( new SYMBOL_LIB_TABLE );
        symLibTable->Load("sym-lib-table");

        DIALOG_LIBRARY_SETTINGS dlg(this, fpLibTable.get());
        dlg.ShowModal();

    }


    virtual ~LIB_TEST_FRAME() {}
};

wxFrame* CreateMainFrame( const std::string& aFileName )
{
    if( wxImage::FindHandler( wxBITMAP_TYPE_PNG ) == NULL )
        wxImage::AddHandler( new wxPNGHandler );

    if( wxImage::FindHandler( wxBITMAP_TYPE_GIF ) == NULL )
        wxImage::AddHandler( new wxGIFHandler );

    if( wxImage::FindHandler( wxBITMAP_TYPE_JPEG ) == NULL )
        wxImage::AddHandler( new wxJPEGHandler );

    auto frame = new LIB_TEST_FRAME( nullptr, wxT( "Library Editor Test" ) );


    return frame;
}
