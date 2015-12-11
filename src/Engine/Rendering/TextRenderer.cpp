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


    
    m_TextProgram.AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Text.vert.glsl")));
    m_TextProgram.AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Text.frag.glsl")));
    m_TextProgram.Compile();
    m_TextProgram.Link();
}

void TextRenderer::Update()
{

}

void TextRenderer::Draw(glm::mat4 projection, glm::mat4 view)
{
    if(counter > 2) {
        if (text == "") {
            text = text + "~wub ";
        } else  if (text == "~wub ") {
            text = text + "wub ";
        } else  if (text == "~wub wub ") {
            text = "";
        }
        counter = 0;
    } else {
        counter++;
    }
    
    RenderText(text, 0.f, 0.f, 1.0f, glm::vec3(1.f, 1.f, 1.f), projection, view);
}

void TextRenderer::RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, glm::mat4 projection, glm::mat4 view)
{
    font = ResourceManager::Load<Font>("fonts/arial.ttf");

    glm::mat4 modelMatrix = glm::translate(glm::mat4(), glm::vec3(0.5f, 0.3f, 0.f)) * glm::toMat4(glm::quat()) * glm::scale(glm::vec3(0.01f));

    // Activate corresponding render state	
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_TextProgram.Bind();
    glUniform3f(glGetUniformLocation(m_TextProgram.GetHandle(), "textColor"), color.x, color.y, color.z);
    glUniformMatrix4fv(glGetUniformLocation(m_TextProgram.GetHandle(), "M"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(m_TextProgram.GetHandle(), "V"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(m_TextProgram.GetHandle(), "P"), 1, GL_FALSE, glm::value_ptr(projection));
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Font::Character ch = font->m_Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos, ypos + h, 0.0, 0.0 },
            { xpos, ypos, 0.0, 1.0 },
            { xpos + w, ypos, 1.0, 1.0 },

            { xpos, ypos + h, 0.0, 0.0 },
            { xpos + w, ypos, 1.0, 1.0 },
            { xpos + w, ypos + h, 1.0, 0.0 }
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLERROR("Text rendering Error");
}

