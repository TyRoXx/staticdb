file(GLOB sources "*.cpp" "*.hpp")
file(GLOB_RECURSE headers "../staticdb/*.hpp")

add_executable(tests ${sources} ${headers})
target_link_libraries(tests ${Boost_LIBRARIES} ${CONAN_LIBS})
if(SQLITE3_FOUND)
	target_link_libraries(tests ${SQLITE3_LIBRARY})
endif()
