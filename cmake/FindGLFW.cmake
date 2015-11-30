# GLFW_FOUND
# GLFW_INCLUDE_DIRS
# GLFW_LIBRARYS

find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h
    $ENV{GLFWDIR}/include
    $ENV{GLFW_HOME}/include
    ${OPENGL_INCLUDE_DIR}
    /usr/include
    /usr/local/include
    /usr/include/GL
    /sw/include
    /opt/local/include
    /opt/graphics/OpenGL/include
    /opt/graphics/OpenGL/contrib/libglfw
    DOC "The directory where GL/glfw.h resides"
)

find_library(GLFW_LIBRARY
    NAMES glfw3 glfw glfw3dll glfwdll
    PATHS
    $ENV{GLFWDIR}/lib
    $ENV{GLFWDIR}/lib/x64
    $ENV{GLFWDIR}/lib/cocoa
    $ENV{GLFWDIR}/lib-msvc120
    $ENV{GLFW_HOME}/lib
    $ENV{GLFW_HOME}/lib/x64
    $ENV{GLFW_HOME}/lib/cocoa
    $ENV{GLFW_HOME}/lib-msvc120
    $ENV{GLFW_HOME}/build/src
    $ENV{GLFW_HOME}/build-debug/src
    $ENV{GLFW_HOME}/build-release/src
    /usr/lib64
    /usr/local/lib64
    /sw/lib64
    /opt/local/lib64
    /usr/lib
    /usr/local/lib
    /sw/lib
    /opt/local/lib
    DOC "The GLFW library"
)

set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})
set(GLFW_LIBRARIES ${GLFW_LIBRARY})

find_package_handle_standard_args(GLFW DEFAULT_MSG GLFW_LIBRARY GLFW_INCLUDE_DIR)
mark_as_advanced(GLFW_FOUND GLFW_INCLUDE_DIR GLFW_LIBRARY)