#ifndef Font_h__
#define Font_h__

#include <ft2build.h>
#include FT_FREETYPE_H  
#include FT_GLYPH_H
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include "../OpenGL.h"
#include "../GLM.h"
#include "../Core/ResourceManager.h"

class Font : public Resource
{
    friend class ResourceManager;
private:
    Font(std::string path);
    
public:
    struct Character {
        GLuint     TextureID;  // ID handle of the glyph texture
        glm::ivec2 Size;       // Size of glyph
        glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
        GLuint     Advance;    // Offset to advance to next glyph
    };

    

    int FontSize = 16;

    ~Font();

    std::map<GLchar, Character> m_Characters;
};
#endif