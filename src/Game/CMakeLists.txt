project(TacticalZ-Game)

find_package(Boost REQUIRED COMPONENTS system filesystem thread chrono)

set(INCLUDE_PATH ${CMAKE_SOURCE_DIR}/include/Game)
include_directories(
    ${CMAKE_SOURCE_DIR}/deps/include
    ${CMAKE_SOURCE_DIR}/include/Engine
    ${INCLUDE_PATH}
    ${Boost_INCLUDE_DIRS}
)

file(GLOB SOURCE_FILES
    "${INCLUDE_PATH}/*.h"
    #"*.cpp"
)
#source_group(Core FILES ${SOURCE_FILES})

set(SOURCE_FILES
    ${SOURCE_FILES}
    "Game.cpp"
    "HealthSystem.cpp"
	"PlayerSystem.cpp"
    "CapturePointSystem.cpp"
)

set(LIBRARIES

)

add_library(Game ${SOURCE_FILES})
target_link_libraries(Game
    Engine
    ${LIBRARIES}
)

add_executable(TacticalZ
	main.cpp
)
target_link_libraries(TacticalZ
    Game
    ${LIBRARIES}
)

