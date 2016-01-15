#ifndef TextRenderer_h__
#define TextRenderer_h__

#include <ft2build.h>
#include FT_FREETYPE_H  

#include "../OpenGL.h"
#include "../GLM.h"
#include "ShaderProgram.h"
#include "Font.h"
#include "../Core/ResourceManager.h"
#include "RenderQueue.h"

class TextRenderer
{
public:
    TextRenderer();
    void Initialize();
    void Update();
    void Draw(RenderScene& scene);

private:
    Font* font;
   
    GLuint VAO, VBO;

    void RenderText(std::string text, Font* font, GLfloat scale, glm::vec4 color, glm::mat4 modelMatrix, glm::mat4 projectionMatrix, glm::mat4 viewMatrix);

    ShaderProgram* m_TextProgram;

    std::string text = "";

    int counter = 0;
};


#endif