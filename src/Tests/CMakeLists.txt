project(TacticalZ-Tests)

find_package(Boost REQUIRED COMPONENTS chrono thread unit_test_framework)

set(INCLUDE_PATH ${CMAKE_SOURCE_DIR}/include)
include_directories(
    ${INCLUDE_PATH}/Engine
    ${INCLUDE_PATH}/Game
    ${INCLUDE_PATH}/Tests
    ${Boost_INCLUDE_DIRS}
)

file(GLOB SOURCE_FILES
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
