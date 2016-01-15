#ifndef TextJob_h__
#define TextJob_h__

#include <cstdint>

#include "../Common.h"
#include "../GLM.h"
#include "../Core/ComponentWrapper.h"
#include "Texture.h"
#include "RenderJob.h"
#include "../Core/ResourceManager.h"
#include "Font.h"

struct TextJob : RenderJob
{
    TextJob(glm::mat4 matrix, Font* font, ComponentWrapper textComponent)
        : RenderJob()
    {
        Matrix = matrix;
        Color = (glm::vec4)textComponent["Color"];
        Content = (std::string)textComponent["Content"];
        Resource = font;
    };

    glm::mat4 Matrix;
    glm::vec4 Color;
    std::string Content;
    Font* Resource;


    void CalculateHash() override
    {
        Hash = 0;
    }
};

#endif