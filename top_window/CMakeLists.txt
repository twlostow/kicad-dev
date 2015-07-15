# the map generation creates on Windows/gcc a lot of useless warnings
# so disable it on windows
if( WIN32 AND NOT CMAKE_CROSSCOMPILING )
    set( MAKE_LINK_MAPS false )
else()
    set( MAKE_LINK_MAPS true )
endif()

add_definitions(  )

include_directories( BEFORE ${INC_BEFORE} )
include_directories(
    ./dialogs
    ../3d-viewer
    ../pcbnew
    ../pcbnew/dialogs
    ../polygon
    ../common
    ${INC_AFTER}
    )



set( TEST_SRCS
    main.cpp
    )


if( MINGW )
    # CVPCB_RESOURCES variable is set by the macro.
    mingw_resource_compiler( test )
endif()




# the main cvpcb program, in DSO form.
add_library( test_kiface MODULE
    ${TEST_SRCS}
    )

set ( KIFACE_PREFIX "_" )
set ( KIFACE_SUFFIX ".kiface" )

set_target_properties( test_kiface PROPERTIES
    OUTPUT_NAME     test
    PREFIX          ${KIFACE_PREFIX}
    SUFFIX          ${KIFACE_SUFFIX}
    )

target_link_libraries( test_kiface
    common
    bitmaps
    ${wxWidgets_LIBRARIES}
    ${GDI_PLUS_LIBRARIES}
    ${OPENMP_LIBRARIES}         # used by 3d viewer
    )


# Must follow github_plugin
target_link_libraries( test_kiface ${Boost_LIBRARIES} )

if( UNIX AND NOT APPLE )
    # -lrt must follow Boost
    target_link_libraries( test_kiface rt )
endif()

set_source_files_properties( main.cpp PROPERTIES
    # The KIFACE is in cvpcb.cpp, export it:
    COMPILE_DEFINITIONS     "BUILD_KIWAY_DLL;COMPILING_DLL"
    )

set ( TO_LINKER "-Wl" )
if( MAKE_LINK_MAPS )
    set_target_properties( test_kiface PROPERTIES
        LINK_FLAGS "${TO_LINKER},-cref ${TO_LINKER},-Map=_test.kiface.map" )
endif()

# a very small program launcher for pcbnew_kiface
add_executable( test WIN32 MACOSX_BUNDLE
    ../common/single_top.cpp
    )

set_source_files_properties( ../common/single_top.cpp test.cpp PROPERTIES
    COMPILE_DEFINITIONS "TOP_FRAME=FRAME_TEST;PGM_DATA_FILE_EXT=\"kicad_pcb\";BUILD_KIWAY_DLL"
    )

target_link_libraries( test
    #singletop  # replaces common, giving us restrictive control and link warnings.
    # There's way too much crap coming in from common yet.
    bitmaps
    common
    bitmaps
    ${wxWidgets_LIBRARIES}
    )

# these 2 binaries are a matched set, keep them together:
