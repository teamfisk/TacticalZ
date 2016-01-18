project(TacticalZ-Game)

find_package(Boost REQUIRED COMPONENTS system filesystem thread chrono)

set(INCLUDE_PATH ${CMAKE_SOURCE_DIR}/include/Game)
include_directories(
    ${CMAKE_SOURCE_DIR}/deps/include
    ${CMAKE_SOURCE_DIR}/include/Engine
    ${INCLUDE_PATH}
    ${Boost_INCLUDE_DIRS}
)

file(GLOB SOURCE_FILES_Systems
    "${INCLUDE_PATH}/Systems/*.h"
    "Systems/*.cpp"
)
source_group(Systems FILES ${SOURCE_FILES_Systems})

file(GLOB SOURCE_FILES_Events
	"${INCLUDE_PATH}/Events/*.h"
	"Events/*.cpp"
)
source_group(Events FILES ${SOURCE_FILES_Events})

set(SOURCE_FILES
    ${SOURCE_FILES}
    "Game.cpp"
    ${SOURCE_FILES_Systems}
	${SOURCE_FILES_Events}
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

