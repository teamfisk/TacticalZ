project(TacticalZ-Game)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem thread chrono)
find_package(assimp REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
# Because FindOpenAL is retarded
#set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/deps/include/AL")
#find_package(OpenAL REQUIRED)
if(UNIX)
    find_package(X11 REQUIRED)
endif()

set(INCLUDE_PATH ${CMAKE_SOURCE_DIR}/include/Game)
include_directories(
    ${CMAKE_SOURCE_DIR}/include/Engine
    ${INCLUDE_PATH}
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${assimp_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIR}
    ${X11_INCLUDE_DIRS}
)

file(GLOB SOURCE_FILES
    "${INCLUDE_PATH}/*.h"
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

