#
# This program source code file is part of KiCad, a free EDA CAD application.
#
# Copyright (C) 2017 CERN
# @author Alejandro García Montoro <alejandro.garciamontoro@gmail.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, you may find one here:
# http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
# or you may search the http://www.gnu.org website for the version 2 license,
# or you may write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA


find_package(Boost COMPONENTS REQUIRED)
find_package( wxWidgets 3.0.0 COMPONENTS gl aui adv html core net base xml stc REQUIRED )

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)

add_definitions(-DPCBNEW -DUSE_TOOL_MANAGER)

if( BUILD_GITHUB_PLUGIN )
    set( GITHUB_PLUGIN_LIBRARIES github_plugin )
endif()

add_dependencies( pnsrouter pcbcommon pcad2kicadpcb ${GITHUB_PLUGIN_LIBRARIES} )

add_executable(test_toporouter
  test_toporouter.cpp
  ../qa_utils/pcb_test_frame.cpp
  ../qa_utils/mocks.cpp
  ../../common/base_units.cpp
  ../../pcbnew/tools/pcb_tool.cpp
  ../../pcbnew/tools/selection.cpp
  ../../pcbnew/tools/selection_tool.cpp
  ../../pcbnew/tools/tool_event_utils.cpp
gts/kdtree.c
gts/psurface.c
gts/segment.c
gts/oocs.c
gts/stripe.c
gts/partition.c
gts/eheap.c
gts/split.c
gts/tribox3.c
gts/container.c
gts/bbtree.c
gts/point.c
gts/isotetra.c
gts/fifo.c
gts/refine.c
gts/curvature.c
gts/iso.c
gts/hsurface.c
gts/graph.c
gts/edge.c
gts/object.c
gts/cdt.c
gts/surface.c
gts/vertex.c
gts/named.c
gts/misc.c
gts/heap.c
gts/matrix.c
gts/pgraph.c
gts/predicates.c
gts/triangle.c
gts/boolean.c
gts/vopt.c
gts/face.c
toporouter.cpp
toporouter_user.cpp
)

include_directories( BEFORE ${INC_BEFORE} )
include_directories(
    gts
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/3d-viewer
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/pcbnew
    ${CMAKE_SOURCE_DIR}/pcbnew/router
    ${CMAKE_SOURCE_DIR}/pcbnew/tools
    ${CMAKE_SOURCE_DIR}/pcbnew/dialogs
    ${CMAKE_SOURCE_DIR}/polygon
    ${CMAKE_SOURCE_DIR}/common/geometry
    ${CMAKE_SOURCE_DIR}/qa/qa_utils
    ${Boost_INCLUDE_DIR}
    ${GLIB_INCLUDE_DIRS}
    ${INC_AFTER}
)

target_link_libraries( test_toporouter
    polygon
    pnsrouter
    common
    pcbcommon
    bitmaps
    polygon
    pnsrouter
    common
    pcbcommon
    bitmaps
    polygon
    pnsrouter
    common
    pcbcommon
    legacy_gal
    bitmaps
    polygon
    pnsrouter
    common
    pcbcommon
    3d-viewer
    bitmaps
    gal
    pcad2kicadpcb
    common
    pcbcommon
    legacy_gal
    ${GITHUB_PLUGIN_LIBRARIES}
    common
    pcbcommon
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${wxWidgets_LIBRARIES}
    ${GLIB_LDFLAGS}
)


