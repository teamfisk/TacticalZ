project(Tests)

find_package(Boost REQUIRED COMPONENTS chrono thread unit_test_framework)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
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
