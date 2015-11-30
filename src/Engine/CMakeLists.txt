project(TacticalZ-Engine)

find_package(OpenGL REQUIRED)
#find_package(GLEW REQUIRED)
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

set(INCLUDE_PATH ${CMAKE_SOURCE_DIR}/include/Engine)
include_directories(
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

set(SOURCE_FILES
    ${SOURCE_FILES_Core}
    ${SOURCE_FILES_Core_Util}
    #${SOURCE_FILES_Input}
    ${SOURCE_FILES_Rendering}
)

set(LIBRARIES
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${Boost_LIBRARIES}
    ${assimp_LIBRARIES}
    ${PNG_LIBRARIES}
    ${OPENAL_LIBRARY}
    ${X11_LIBRARIES}
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
