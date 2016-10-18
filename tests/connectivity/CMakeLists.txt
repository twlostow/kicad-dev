add_executable ( connectivity connectivity.cpp  connectivity_impl.cpp test_connectivity.cpp )

target_link_libraries (
    connectivity
    _pcbnew_kiface
    ${wxWidgets_LIBRARIES}
    ${Boost_LIBRARIES}
)