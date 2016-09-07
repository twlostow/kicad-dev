#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE PNS_NODE

#include <boost/test/unit_test.hpp>

#include "pns_item.h"
#include "pns_node.h"

using namespace PNS;
using namespace std;

BOOST_AUTO_TEST_CASE( PnsNode )
{
    unique_ptr<NODE> node ( new NODE );
    BOOST_REQUIRE( node->JointCount() == 0 );

    unique_ptr<SEGMENT> segment( new SEGMENT ( SEG ( VECTOR2I (0, 0), VECTOR2I( 1000, 1000) ), 0 ) );
    node->Add( move ( segment ) );

    BOOST_REQUIRE( node->JointCount() == 2 );
}
