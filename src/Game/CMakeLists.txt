project(TacticalZ-Game)

set(INCLUDE_PATH ${CMAKE_SOURCE_DIR}/include)
include_directories(
    ${INCLUDE_PATH}/Engine
    ${INCLUDE_PATH}/Game
)

file(GLOB SOURCE_FILES
    "${INCLUDE_PATH}*.h"
    "*.cpp"
)
#source_group(Core FILES ${SOURCE_FILES})

set(SOURCE_FILES
    ${SOURCE_FILES}
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

