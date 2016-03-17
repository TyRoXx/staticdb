file(GLOB sources "*.cpp" "*.hpp")
file(GLOB_RECURSE headers "../staticdb/*.hpp")

set(allSources ${sources} ${headers})
set(formatted ${formatted} ${allSources} PARENT_SCOPE)

add_executable(tests ${allSources})
target_link_libraries(tests ${Boost_LIBRARIES} ${CONAN_LIBS})
if(SQLITE3_FOUND)
	target_link_libraries(tests ${SQLITE3_LIBRARY})
endif()
