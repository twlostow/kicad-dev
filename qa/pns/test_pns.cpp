#include <qa_utils/utility_registry.h>



int pns1_main_func( int argc, char* argv[] )
{
    printf("Dupa\n");

    App().SetTopWindow( frame );      // wxApp gets a face.


    return 0;
}

dupa

static bool registered = UTILITY_REGISTRY::Register( {
        "pns1",
        "PNS test1",
        pns1_main_func,
} );
