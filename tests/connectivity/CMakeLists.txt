add_executable ( connectivity test_connectivity.cpp ratsnest_v2.cpp )

target_link_libraries (
    connectivity
    _pcbnew_kiface
    ${wxWidgets_LIBRARIES}
    ${Boost_LIBRARIES}
)