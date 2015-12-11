#ifndef TextRenderer_h__
#define TextRenderer_h__

#include <ft2build.h>
#include FT_FREETYPE_H  

#include "../OpenGL.h"
#include "../GLM.h"
#include "ShaderProgram.h"
#include "Font.h"
#include "../Core/ResourceManager.h"

class TextRenderer
{
public:
    TextRenderer();
    void Initialize();
    void Update();
    void Draw(glm::mat4 projection, glm::mat4 view);

private:
    Font* font;
   
    GLuint VAO, VBO;

    void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, glm::mat4 projection, glm::mat4 view);

    ShaderProgram m_TextProgram;

    std::string text = "";

    int counter = 0;
};


#endif