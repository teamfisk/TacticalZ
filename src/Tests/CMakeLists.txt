project(TacticalZ-Tests)

find_package(Boost REQUIRED COMPONENTS chrono thread unit_test_framework)

set(INCLUDE_PATH ${CMAKE_SOURCE_DIR}/include/Tests)
include_directories(
    ${CMAKE_SOURCE_DIR}/deps/include
    ${CMAKE_SOURCE_DIR}/include/Engine
    ${CMAKE_SOURCE_DIR}/include/Game
    ${INCLUDE_PATH}
    ${Boost_INCLUDE_DIRS}
)

file(GLOB SOURCE_FILES
    "${INCLUDE_PATH}/Tests/*.h"
    "*.cpp"
)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

add_executable(Tests ${SOURCE_FILES})
target_link_libraries(Tests
	Engine
	Game
    ${Boost_LIBRARIES}
)
