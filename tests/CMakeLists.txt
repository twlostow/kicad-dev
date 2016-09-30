add_definitions( -DPCBNEW )


include_directories ( 
    ${CMAKE_BINARY_DIR} 
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_SOURCE_DIR}/polygon
    ${CMAKE_SOURCE_DIR}/3d-viewer
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/pcbnew
    ${CMAKE_SOURCE_DIR}/pcbnew/router )

link_directories ( ${CMAKE_BINARY_DIR}/pcbnew )

function( add_test_case name )
    add_executable ( ${name} ${name}.cpp )
    target_link_libraries ( 
    ${name}  
	_pcbnew_kiface
    ${wxWidgets_LIBRARIES}
    ${Boost_LIBRARIES}
)
endfunction( add_test_case )

add_subdirectory ( pns )
add_subdirectory ( connectivity )
add_subdirectory ( poly-test )
add_subdirectory ( triangulator )
#add_subdirectory ( windowed )
