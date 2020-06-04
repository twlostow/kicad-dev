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
        //printf("*** createVarRef %p %d\n", this, aItemIndex );
    }

    virtual LIBEVAL::VAR_TYPE_T GetType( const LIBEVAL::UCODE* aUcode ) const override
    {
        return LIBEVAL::VT_NUMERIC; // fixme
    }

    virtual LIBEVAL::VALUE GetValue( const LIBEVAL::UCODE* aUcode ) const override
    {
        auto ucode = static_cast<const PCB_UCODE*> (aUcode);
        auto item = ucode->GetItem( m_itemIndex );

        auto any = item->Get( m_prop );
        const wxAnyValueType *type = any.GetType();

        return LIBEVAL::VALUE( (double) item->Get<int>( m_prop ) );
    }

private:
    PROPERTY_BASE* m_prop;
    int m_itemIndex;
};

LIBEVAL::UCODE::VAR_REF *PCB_UCODE::createVarRef( const std::string &var, const std::string &field )
{
    PCB_VAR_REF *rv;

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
            double v = atof(aString.c_str());
            switch(unitId)
            {
                case 0 :
                    return Mils2iu( v );
                case 1:
                    return Millimeter2iu( v );
                case 2:
                    return Mils2iu( v * 1000.0 );
                default:
                    return v;
            }
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

bool testEvalExpr( const std::string expr, double expectedResult, bool expectError = false, BOARD_ITEM* itemA = nullptr, BOARD_ITEM* itemB = nullptr )
{
    PCB_EXPR_COMPILER compiler;
    PCB_UCODE ucode;
    bool ok = true;

    ucode.SetItems( itemA, itemB );

    bool error = !compiler.Compile( expr, &ucode );

    if( error )
    {
        if ( expectError )
        {
            printf("result: OK (expected parse error)\n");
            ok = true;
            return ok;
        } else {
            printf("result: FAIL (unexpected parse error)\n");
            ok = false;
        }
    }

    double result;

    if( ok )
    {
        result = ucode.Run()->AsDouble();
        ok = (result == expectedResult);
    }

    printf("result: %s (got %.10f expected: %.10f)\n", ok ? "OK" : "FAIL", result, expectedResult );

    if (!ok )
    {
        printf("Offending code dump: \n%s\n", ucode.Dump().c_str() );
    }
}

int main( int argc, char *argv[] )
{
    PROPERTY_MANAGER& propMgr = PROPERTY_MANAGER::Instance();
    propMgr.Rebuild();

    
    testEvalExpr( "10mm + 20 mm", 30e6 );
    testEvalExpr( "3*(7+8)", 3*(7+8) );
    testEvalExpr( "3*7+8", 3*7+8 );
    testEvalExpr( "(3*7)+8", 3*7+8 );
    testEvalExpr( "10mm + 20)", 0, true );
    

    TRACK trackA(nullptr);
    TRACK trackB(nullptr);
    
    trackA.SetWidth( Mils2iu( 10 ));
    trackB.SetWidth( Mils2iu( 20 ));

    testEvalExpr( "A.Width > B.Width", 0.0, false, &trackA, &trackB );

    return 0;
}
