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
#include "TextPassState.h"
#include "FrameBuffer.h"

class TextPass
{
public:
    TextPass();
    void Initialize();
    void Update();
    void Draw(RenderScene& scene, FrameBuffer& frameBuffer);

private:
    void renderText(std::string text, Font* font, TextJob::AlignmentEnum alignment, glm::vec4 color, glm::mat4 modelMatrix, glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
	std::string parseColors(std::string text, std::map<int, glm::vec4>& colorChanges, glm::vec4 originalColor);

    Font* font;
    GLuint VAO, VBO;
    ShaderProgram* m_TextProgram;
};


#endif