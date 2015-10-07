file(GLOB sources "*.cpp" "*.hpp")
file(GLOB_RECURSE headers "../staticdb/*.hpp")

add_executable(tests ${sources} ${headers})
target_link_libraries(tests ${Boost_LIBRARIES} ${SQLITE3_LIBRARY})
