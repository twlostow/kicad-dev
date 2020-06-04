#include <wx/wx.h>
#include <cstdio>

#include "class_board.h"
#include "class_track.h"
#include "libeval_compiler.h"


#include <io_mgr.h>
#include <kicad_plugin.h>

#include <profile.h>

class PCB_VAR_REF;

class PCB_UCODE : public LIBEVAL::UCODE
{
public:

    virtual VAR_REF *createVarRef( const std::string &var, const std::string &field ) override;
    
    void SetItems(  BOARD_ITEM *a,  BOARD_ITEM* b )
    {
        m_items[0] = a;
        m_items[1] = b;
    }

     BOARD_ITEM *GetItem( int index ) const 
    {
        return m_items[index];
    }

private:
    BOARD_ITEM *m_items[2];
};


class PCB_VAR_REF : public LIBEVAL::UCODE::VAR_REF
{
public:
    PCB_VAR_REF ( PROPERTY_BASE* aProp, int aItemIndex )
        : m_prop( aProp ), m_itemIndex(aItemIndex)
    {
        //printf("*** createVarRef %p %d\n", aProp, aItemIndex );
    }

    virtual LIBEVAL::VAR_TYPE_T GetType( const LIBEVAL::UCODE* aUcode ) const override
    {
        return LIBEVAL::VT_NUMERIC; // fixme
    }

    virtual LIBEVAL::VALUE GetValue( const LIBEVAL::UCODE* aUcode ) const override
    {
        auto ucode = static_cast<const PCB_UCODE*> (aUcode);
        auto item = ucode->GetItem( m_itemIndex );
        return LIBEVAL::VALUE( (double) item->Get<int>( m_prop ) );
    }

private:
    PROPERTY_BASE* m_prop;
    int m_itemIndex;
};

LIBEVAL::UCODE::VAR_REF *PCB_UCODE::createVarRef( const std::string &var, const std::string &field )
{
    PCB_VAR_REF *rv;
    printf("createRef: var '%s' field '%s'\n", (const char *) var.c_str(), (const char *) field.c_str() );

    PROPERTY_MANAGER& propMgr = PROPERTY_MANAGER::Instance();
    PROPERTY_BASE* prop = propMgr.GetProperty( TYPE_HASH( TRACK ), field );
    rv = new PCB_VAR_REF( prop , var == "A" ? 0 : 1 );

    return rv;
}


BOARD* loadBoard( const std::string& filename )
{
    PLUGIN::RELEASER pi( new PCB_IO );
    BOARD* brd = nullptr;

    try
    {
        brd = pi->Load( wxString( filename.c_str() ), NULL, NULL );
    }
    catch( const IO_ERROR& ioe )
    {
        wxString msg = wxString::Format( _( "Error loading board.\n%s" ),
                ioe.Problem() );

        printf( "%s\n", (const char*) msg.mb_str() );
        return nullptr;
    }

    return brd;
}


class PCB_UNIT_RESOLVER : public LIBEVAL::UNIT_RESOLVER
{
    public:
        virtual ~PCB_UNIT_RESOLVER()
        {
        }

        virtual const std::vector<std::string>& GetSupportedUnits() const override
        {
            static const std::vector<std::string> pcbUnits = {"mil", "mm", "in"};

            return pcbUnits;
        }

        virtual double Convert( const std::string aString, int unitId ) const override
        {
            return 0.0;
        };
};


class PCB_EXPR_COMPILER : public LIBEVAL::COMPILER
{
    public:
        PCB_EXPR_COMPILER()
        {
            m_unitResolver.reset( new PCB_UNIT_RESOLVER );
        }
};

void testEvalExpr( const std::string expr, double expectedResult, bool expectError = false )
{
    PCB_EXPR_COMPILER compiler;
    PCB_UCODE ucode;

    bool error = compiler.Compile( expr, &ucode );
    ucode.Dump();
    //double result = ucode.Run();
}

int main( int argc, char *argv[] )
{
    PROPERTY_MANAGER& propMgr = PROPERTY_MANAGER::Instance();
    propMgr.Rebuild();

    
    testEvalExpr( "10mm + 20 mm", 30e9 );
    testEvalExpr( "3*(7+8)", 3*(7+8) );
    testEvalExpr( "3*7+8", 3*7+8 );
    testEvalExpr( "(3*7)+8", 3*7+8 );
    

    return 0;

    LIBEVAL::COMPILER compiler;
    PCB_UCODE ucode;

//    auto board = loadBoard( argv[1] );

    //compiler.Compile("A.NetClass==(1+2)*3   ", &ucode);
    compiler.Compile("A.NetClass==\"HV\" || A.NetClass==\"BigClearance\"", &ucode);
    ucode.Dump();

#if 0

    printf("Track segments: %d\n", board->Tracks().size() );

    PROF_COUNTER cnt("run-exprs");
    // run the microcode for all tracks
    for( auto t1: board->Tracks() )
    {
        for( auto t2: board->Tracks() )
        {
            if ( t1->Type() != PCB_TRACE_T )
                break;

            if ( t2->Type() != PCB_TRACE_T )
                continue;

            if ( t1->GetLayer() != t2->GetLayer() )
                continue;


            ucode.SetItems( t1, nullptr );
            auto result = ucode.Run();
            (void ) result;
        }
        //printf("Trk %p w %d result %d\n", t1, t1->GetWidth(), (int) result );
    }

    cnt.Show();
#endif
    return 0;
}
