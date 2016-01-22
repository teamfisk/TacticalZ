#include "Rendering/TextRenderer.h"

TextRenderer::TextRenderer()
{

}

void TextRenderer::Initialize()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_TextProgram = ResourceManager::Load<ShaderProgram>("#TextProgram");
    m_TextProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Text.vert.glsl")));
    m_TextProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Text.frag.glsl")));
    m_TextProgram->Compile();
    m_TextProgram->Link();
}

void TextRenderer::Update()
{

}

void TextRenderer::Draw(RenderScene& scene)
{
    for (auto &job : scene.TextJobs) {
        auto textJob = std::dynamic_pointer_cast<TextJob>(job);
        if (textJob) {

            renderText(textJob->Content, textJob->Resource, textJob->Alignment, textJob->Color, textJob->Matrix, scene.Camera->ProjectionMatrix(), scene.Camera->ViewMatrix());
        }
    }
}

void TextRenderer::renderText(std::string text, Font* font, TextJob::AlignmentEnum alignment, glm::vec4 color, glm::mat4 modelMatrix, glm::mat4 projectionMatrix, glm::mat4 viewMatrix)
{
    GLfloat penX = 0;
    GLfloat penY = 0;
    float scale = 1.0/font->FontSize;

    GLfloat stringWidth = 0.f;

    for (std::string::const_iterator c = text.begin(); c != text.end(); c++) {
        Font::Character ch = font->m_Characters[*c];
        stringWidth += (ch.Advance >> 6) * scale;
    }

    if(alignment == TextJob::AlignmentEnum::Center) {
        penX = -stringWidth/2.f;
    } else if (alignment == TextJob::AlignmentEnum::Right) {
        penX = -stringWidth;
    } else {
        penX = 0;
    }	
    
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_TextProgram->Bind();
    glUniform3f(glGetUniformLocation(m_TextProgram->GetHandle(), "textColor"), color.x, color.y, color.z);
    glUniformMatrix4fv(glGetUniformLocation(m_TextProgram->GetHandle(), "M"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(m_TextProgram->GetHandle(), "V"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(m_TextProgram->GetHandle(), "P"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);


    for (std::string::const_iterator c = text.begin(); c != text.end(); c++) {
        Font::Character ch = font->m_Characters[*c];

        GLfloat xpos = penX + ch.Bearing.x * scale;
        GLfloat ypos = penY - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        GLfloat vertices[6][4] = {
            { xpos, ypos + h, 0.0, 0.0 },
            { xpos, ypos, 0.0, 1.0 },
            { xpos + w, ypos, 1.0, 1.0 },

            { xpos, ypos + h, 0.0, 0.0 },
            { xpos + w, ypos, 1.0, 1.0 },
            { xpos + w, ypos + h, 1.0, 0.0 }
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        penX += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLERROR("Text rendering Error");
}

