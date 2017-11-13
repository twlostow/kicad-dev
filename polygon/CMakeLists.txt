# .cpp files are compiled with extra ${WSHADOW_FLAGS}
if( COMPILER_SUPPORTS_WSHADOW )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WSHADOW_FLAGS}")
endif()

include_directories(BEFORE ${INC_BEFORE})
include_directories(
    ${INC_AFTER}
    )

set(POLYGON_SRCS
    math_for_graphics.cpp
#    PolyLine.cpp
    polygon_test_point_inside.cpp
    clipper.cpp
)

add_library(polygon STATIC ${POLYGON_SRCS})

if( KICAD_PRECOMPILE_HEADERS )
  set_target_properties( polygon PROPERTIES
    COTIRE_ADD_UNITY_BUILD FALSE
    COTIRE_CXX_PREFIX_HEADER_INIT "${CMAKE_SOURCE_DIR}/stdafx.h"
 )

  cotire(polygon)
endif()

add_dependencies( polygon lib-dependencies )
