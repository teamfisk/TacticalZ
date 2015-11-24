project(TacticalZ)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem thread chrono)
find_package(assimp REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
# Because FindOpenAL is retarded
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/deps/include/AL")
find_package(OpenAL REQUIRED)
if(UNIX)
    find_package(X11 REQUIRED)
endif()

set(INCLUDE_PATH ${CMAKE_SOURCE_DIR}/include)
include_directories(
    ${INCLUDE_PATH}
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${assimp_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIR}
)

file(GLOB SOURCE_FILES_Core
    "${INCLUDE_PATH}/Core/*.h"
    "Core/*.cpp"
)
file(GLOB SOURCE_FILES_Core_Util
    "${INCLUDE_PATH}/Core/Util/*.h"
    "Core/Util/*.cpp"
)
source_group(Core FILES ${SOURCE_FILES_Core})
source_group(Core\\Util FILES ${SOURCE_FILES_Core_Util})

file(GLOB SOURCE_FILES_Input
    "${INCLUDE_PATH}/Input/*.h"
    "Input/*.cpp"
)
source_group(Input FILES ${SOURCE_FILES_Input})

file(GLOB SOURCE_FILES_Rendering
    "${INCLUDE_PATH}/Rendering/*.h"
    "Rendering/*.cpp"
)
source_group(Rendering FILES ${SOURCE_FILES_Rendering})
file(GLOB SOURCE_FILES_Rendering_OpenGL
    "${INCLUDE_PATH}/Rendering/OpenGL/Rendering/*.h"
    "Rendering/OpenGL/*.cpp"
)
source_group(Rendering\\OpenGL FILES ${SOURCE_FILES_Rendering_OpenGL})
file(GLOB SOURCE_FILES_Rendering_DirectX
    "${INCLUDE_PATH}/Rendering/DirectX/Rendering/*.h"
    "Rendering/DirectX/*.cpp"
)
source_group(Rendering\\DirectX FILES ${SOURCE_FILES_Rendering_DirectX})

file(GLOB SOURCE_FILES_Transform
    "${INCLUDE_PATH}/Transform/*.h"
    "Transform/*.cpp"
)
source_group(Transform FILES ${SOURCE_FILES_Transform})

file(GLOB SOURCE_FILES_Physics
    "${INCLUDE_PATH}/Physics/*.h"
    "Physics/*.cpp"
)
source_group(Physics FILES ${SOURCE_FILES_Physics})

file(GLOB SOURCE_FILES_Game
    "${INCLUDE_PATH}/Game/*.h"
    "Game/*.cpp"
)
source_group(Game FILES ${SOURCE_FILES_Game})
file(GLOB SOURCE_FILES_Game_GUI
    "${INCLUDE_PATH}/Game/GUI/*.h"
    "Game/GUI/*.cpp"
)
source_group(Game\\GUI FILES ${SOURCE_FILES_Game_GUI})

file(GLOB SOURCE_FILES_Sound
    "${INCLUDE_PATH}/Sound/*.h"
    "Sound/*.cpp"
)
source_group(Sound FILES ${SOURCE_FILES_Sound})

file(GLOB SOURCE_FILES_GUI
    "${INCLUDE_PATH}/GUI/*.h"
    "GUI/*.cpp"
)
source_group(GUI FILES ${SOURCE_FILES_GUI})

set(SOURCE_FILES
    ${SOURCE_FILES_Core}
    ${SOURCE_FILES_Core_Util}
    ${SOURCE_FILES_Input}
    ${SOURCE_FILES_Rendering}
    ${SOURCE_FILES_Transform}
    ${SOURCE_FILES_Physics}
    ${SOURCE_FILES_Game}
    ${SOURCE_FILES_Game_GUI}
    ${SOURCE_FILES_Sound}
    ${SOURCE_FILES_GUI}
)

set(LIBRARIES
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${Boost_LIBRARIES}
    ${assimp_LIBRARIES}
    ${PNG_LIBRARIES}
    ${X11_LIBRARIES}
    ${OPENAL_LIBRARY}
)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
    set(STATIC_LIBRARY_FLAGS "${STATIC_LIBRARY_FLAGS}")
endif()

add_library(Engine ${SOURCE_FILES})
target_link_libraries(Engine
    ${LIBRARIES}
)

add_executable(Game 
	main.cpp
)
target_link_libraries(Game
	Engine
    ${LIBRARIES}
)

