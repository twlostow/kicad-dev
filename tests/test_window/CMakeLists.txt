# the map generation creates on Windows/gcc a lot of useless warnings
# so disable it on windows
if( WIN32 AND NOT CMAKE_CROSSCOMPILING )
    set( MAKE_LINK_MAPS false )
else()
    set( MAKE_LINK_MAPS true )
endif()


if( KICAD_USE_WEBKIT AND BUILD_GITHUB_PLUGIN )
    set( WEBVIEWER_WXLIB "webviewer" )
endif()


include_directories( BEFORE ${INC_BEFORE} )
include_directories(
    ../../3d-viewer
    ../../pcbnew
    ../../pcbnew/dialogs
    ../../polygon
    ../../common
    ${INC_AFTER}
    )



set( TEST_SRCS
    test_window.cpp
    globals.cpp
    draw_panel_gal_ng.cpp
    view_ng.cpp
    view_controls_ng.cpp
    wx_view_controls_ng.cpp

    )


add_executable( test_window WIN32 MACOSX_BUNDLE
    ${TEST_SRCS}
    )

#  set_target_properties( test_window  PROPERTIES
#        LINK_FLAGS "-Wl,--start-group"
#        LINK_FLAGS "-rdynamic"
#        )

target_link_libraries( test_window
    -Wl,--start-group
    common
    pcbcommon
    common
    gal
    common
    bitmaps
    libpcbnew
    polygon
    3d-viewer
    pcbcommon
    pnsrouter
    common
    pcad2kicadpcb
    polygon
    bitmaps
    gal
    lib_dxf
    idf3
    github_plugin
    ${GITHUB_PLUGIN_LIBRARIES}
    ${wxWidgets_LIBRARIES}
    ${GDI_PLUS_LIBRARIES}
    ${PYTHON_LIBRARIES}
    ${Boost_LIBRARIES}      # must follow GITHUB
    ${PCBNEW_EXTRA_LIBS}    # -lrt must follow Boost
    ${OPENMP_LIBRARIES}
    -Wl,--end-group
    )


# Only for win32 cross compilation using MXE
if( WIN32 AND MSYS AND CMAKE_CROSSCOMPILING )
    target_link_libraries( test_window
        opengl32
        glu32
        pixman-1
        fontconfig
        freetype
        bz2
        )
endif()

if( UNIX AND NOT APPLE )
    # -lrt must follow Boost
    target_link_libraries( test_window rt )
endif()

