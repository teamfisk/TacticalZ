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

        if ((int)textComponent["Alignment"] == textComponent["Alignment"].Enum("Left")) {
            Alignment = AlignmentEnum::Left;
        } else if ((int)textComponent["Alignment"] == textComponent["Alignment"].Enum("Right")) {
            Alignment = AlignmentEnum::Right;
        } else if ((int)textComponent["Alignment"] == textComponent["Alignment"].Enum("Center")) {
            Alignment = AlignmentEnum::Center;
        } else {
            LOG_ERROR("Text alignment invalid");
            Alignment = AlignmentEnum::Left;
        }

    };

    enum class AlignmentEnum
    {
        Left,
        Right,
        Center
    };

    glm::mat4 Matrix;
    glm::vec4 Color;
    std::string Content;
    Font* Resource;
    AlignmentEnum Alignment;

    

    void CalculateHash() override
    {
        Hash = 0;
    }
};

#endif