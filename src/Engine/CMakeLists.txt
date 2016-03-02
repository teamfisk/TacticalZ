project(TacticalZ-Engine)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem thread chrono timer program_options)
find_package(assimp REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(Xerces REQUIRED)
find_package(Freetype REQUIRED)
# Because FindOpenAL is retarded
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_SOURCE_DIR}/deps/include/OpenAL")
find_package(OpenAL REQUIRED)
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
    ${Xerces_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
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

file(GLOB SOURCE_FILES_Network
    "${INCLUDE_PATH}/Network/*.h"
    "Network/*.cpp"
)
source_group(Network FILES ${SOURCE_FILES_Network})

file(GLOB SOURCE_FILES_Sound
    "${INCLUDE_PATH}/Sound/*.h"
    "Sound/*.cpp"
)
source_group(Sound FILES ${SOURCE_FILES_Sound})

file(GLOB SOURCE_FILES_Rendering
    "${INCLUDE_PATH}/Rendering/*.h"
    "Rendering/*.cpp"
)
file(GLOB SOURCE_FILES_Rendering_Util
    "${INCLUDE_PATH}/Rendering/Util/*.h"
    "Rendering/Util/*.cpp"
)
source_group(Rendering FILES ${SOURCE_FILES_Rendering})
source_group(Rendering\\Util FILES ${SOURCE_FILES_Rendering_Util})

file(GLOB SOURCE_FILES_GUI
    "${INCLUDE_PATH}/GUI/*.h"
    "GUI/*.cpp"
)
source_group(GUI FILES ${SOURCE_FILES_GUI})

file(GLOB SOURCE_FILES_Collision
    "${INCLUDE_PATH}/Collision/*.h"
    "Collision/*.cpp"
)
source_group(Collision FILES ${SOURCE_FILES_Collision})

file(GLOB SOURCE_FILES_Editor
	"${INCLUDE_PATH}/Editor/*.h"
	"Editor/*.cpp"
)
source_group(Editor FILES ${SOURCE_FILES_Editor})

set(SOURCE_FILES
    ${SOURCE_FILES_Core}
    ${SOURCE_FILES_Core_Util}
    ${SOURCE_FILES_Input}
	${SOURCE_FILES_Network}
	${SOURCE_FILES_Sound}
    ${SOURCE_FILES_GUI}
    ${SOURCE_FILES_Rendering}
    ${SOURCE_FILES_Rendering_Util}
    ${SOURCE_FILES_Collision}
	${SOURCE_FILES_Editor}
	${CMAKE_SOURCE_DIR}/deps/include/imgui/imgui.cpp
	${CMAKE_SOURCE_DIR}/deps/include/imgui/imgui_draw.cpp
	${CMAKE_SOURCE_DIR}/deps/include/imgui/imgui_demo.cpp
	${CMAKE_SOURCE_DIR}/deps/include/nativefiledialog/nfd_common.c
)

# nativefiledialog
if(WIN32)
	set(SOURCE_FILES ${SOURCE_FILES}
		${CMAKE_SOURCE_DIR}/deps/include/nativefiledialog/nfd_win.cpp
	)
endif()
if(UNIX)
	set(SOURCE_FILES ${SOURCE_FILES}
		${CMAKE_SOURCE_DIR}/deps/include/nativefiledialog/nfd_gtk.c
		# TODO: Link with GTK+ here!
	)
endif()


set(LIBRARIES
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${Boost_LIBRARIES}
    ${assimp_LIBRARIES}
    ${PNG_LIBRARIES}
    ${Xerces_LIBRARIES}
    ${FREETYPE_LIBRARIES}
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
#set_target_properties(Engine PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${INCLUDE_PATH}/PrecompiledHeader.h")
#cotire(Engine)
