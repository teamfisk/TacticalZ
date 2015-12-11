#ifndef TextRenderer_h__
#define TextRenderer_h__

#include <ft2build.h>
#include FT_FREETYPE_H  

#include "OpenGL.h"
#include "GLM.h"
#include "ShaderProgram.h"

class TextRenderer
{
public:
    TextRenderer();
    void Initialize();
    void Update();
    void Draw(glm::mat4 projection, glm::mat4 view);

private:
    

    struct Character {
        GLuint     TextureID;  // ID handle of the glyph texture
        glm::ivec2 Size;       // Size of glyph
        glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
        GLuint     Advance;    // Offset to advance to next glyph
    };

    std::map<GLchar, Character> Characters;

   
    GLuint VAO, VBO;

    void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, glm::mat4 projection, glm::mat4 view);

    ShaderProgram m_TextProgram;

    std::string text = "";

    int counter = 0;
};


#endif