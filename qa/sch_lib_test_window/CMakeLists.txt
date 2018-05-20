
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

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package( wxWidgets 3.0.0 COMPONENTS gl aui adv html core net base xml stc REQUIRED )

add_definitions(-DBOOST_TEST_DYN_LINK -DEESCHEMA -DTOP_FRAME=FRAME_SCH)

add_executable(sch_lib_test_window
  test.cpp
  sch_view.cpp
  sch_draw_panel_gal.cpp
  sch_painter.cpp

  ../../common/base_units.cpp
  ../../common/base_screen.cpp
  ../../common/eda_text.cpp
  ../../common/page_info.cpp
  ../../common/project.cpp
  ../../common/single_top.cpp

  ../common/sch-mocks.cpp
  ../../eeschema/lib_draw_item.cpp
  ../../eeschema/lib_bezier.cpp
  ../../eeschema/lib_pin.cpp
  ../../eeschema/lib_arc.cpp
  ../../eeschema/lib_polyline.cpp
#  ../../eeschema/lib_export.cpp
  ../../eeschema/lib_rectangle.cpp
  ../../eeschema/lib_field.cpp
  ../../eeschema/lib_text.cpp
#  ../../eeschema/lib_collectors.cpp
  ../../eeschema/lib_circle.cpp
  ../../eeschema/class_libentry.cpp
  ../../eeschema/pin_type.cpp
  ../../eeschema/pin_shape.cpp
  ../../eeschema/transform.cpp
  #../../eeschema/sch_field.cpp
  ../../eeschema/sch_io_mgr.cpp
  ../../eeschema/sch_bitmap.cpp
  ../../eeschema/sch_plugin.cpp
  ../../eeschema/sch_legacy_plugin.cpp
  ../../eeschema/sch_eagle_plugin.cpp
  ../../eeschema/sch_sheet.cpp
  ../../eeschema/sch_component.cpp
  ../../eeschema/sch_screen.cpp
  ../../eeschema/sch_field.cpp
  ../../eeschema/sch_item_struct.cpp
  ../../eeschema/sch_sheet_path.cpp
  ../../eeschema/sch_sheet_pin.cpp
  ../../eeschema/sch_text.cpp
  ../../eeschema/sch_no_connect.cpp
  ../../eeschema/sch_junction.cpp
  ../../eeschema/sch_bus_entry.cpp
  ../../eeschema/sch_collectors.cpp
  ../../eeschema/sch_line.cpp
  ../../eeschema/sch_marker.cpp
  ../../eeschema/class_library.cpp
  ../../eeschema/netlist_object.cpp
  ../../eeschema/symbol_lib_table.cpp
  ../../eeschema/component_references_lister.cpp

  ../common/sch_test_frame.cpp
  )

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/pcbnew
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/eeschema
    ${CMAKE_SOURCE_DIR}/polygon
    ${CMAKE_SOURCE_DIR}/common/geometry
    ${CMAKE_SOURCE_DIR}/qa/common
    ${CMAKE_SOURCE_DIR}/qa/sch_lib_test_window
    ${Boost_INCLUDE_DIR}
)

target_link_libraries( sch_lib_test_window
    common
    gal
    common
    gal
    common
    gal
    common
    gal
    polygon
    gal
    common
    gal
    bitmaps
    gal
    polygon
    common
    bitmaps
    polygon
    common
    bitmaps
    polygon
    common
    bitmaps
    common
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${wxWidgets_LIBRARIES}
)
