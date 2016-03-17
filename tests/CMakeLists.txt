file(GLOB sources "*.cpp" "*.hpp")
file(GLOB_RECURSE headers "../staticdb/*.hpp")

set(allSources ${sources} ${headers})
set(formatted ${formatted} ${allSources} PARENT_SCOPE)

add_executable(tests ${allSources})
target_link_libraries(tests ${CONAN_LIBS})
