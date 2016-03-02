#ifndef CommonFuntions_h__
#define CommonFuntions_h__

#include "../../Common.h"
#include "../../OpenGL.h"
#include "../../GLM.h"
#include "../Texture.h"

namespace CommonFunctions
{ 
Texture* LoadTexture(std::string path, bool threaded);
};

#endif