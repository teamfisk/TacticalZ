#include <memory>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

// OpenGL
#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#define NOMINMAX
#include <GLFW/glfw3.h>
#include <glext.h>
#include "Core/Util/GLError.h"

// GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Core/Util/Logging.h"
