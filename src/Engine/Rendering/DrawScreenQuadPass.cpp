#include "Rendering/DrawScreenQuadPass.h"

DrawScreenQuadPass::DrawScreenQuadPass(IRenderer* renderer)
{
    m_Renderer = renderer;

    m_ScreenQuad = ResourceManager::Load<Model>("Models/Core/ScreenQuad.obj");

    InitializeShaderPrograms();
}

void DrawScreenQuadPass::InitializeShaderPrograms()
{
    m_DrawQuadProgram = ResourceManager::Load<ShaderProgram>("#DrawScreenQuadProgram");
    m_DrawQuadProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/DrawScreenQuad.vert.glsl")));
    m_DrawQuadProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/DrawScreenQuad.frag.glsl")));
    m_DrawQuadProgram->Compile();
    m_DrawQuadProgram->Link();
}

void DrawScreenQuadPass::Draw(GLuint texture)
{
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLERROR("DrawScreenQuadPass::Draw: Pre");

    DrawScreenQuadPassState state = DrawScreenQuadPassState();
    m_DrawQuadProgram->Bind();
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(m_ScreenQuad->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
    glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].EndIndex - m_ScreenQuad->MaterialGroups()[0].StartIndex +1
        , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].StartIndex);
}
