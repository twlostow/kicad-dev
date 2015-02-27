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
    )


add_executable( test_window WIN32 MACOSX_BUNDLE
    ${TEST_SRCS}
    )

target_link_libraries( test_window
    #singletop  # replaces common, giving us restrictive control and link warnings.
    # There's way too much crap coming in from common yet.
    common
    pcbcommon
    common
    gal
    common
    bitmaps
    ${wxWidgets_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GDI_PLUS_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${CAIRO_LIBRARIES}
    ${PIXMAN_LIBRARY}
    ${OPENMP_LIBRARIES}

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

