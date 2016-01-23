#include "Rendering/DrawColorCorrectionPass.h"

DrawColorCorrectionPass::DrawColorCorrectionPass(IRenderer* renderer)
{
    m_Renderer = renderer;

    m_ScreenQuad = ResourceManager::Load<Model>("Models/Core/ScreenQuad.obj");
    m_Exposure = 1; //TODO: Renderer: Fixa så att denna går att ändra på genom komponent eller setting.

    InitializeShaderPrograms();
}

void DrawColorCorrectionPass::InitializeShaderPrograms()
{
    m_ColorCorrectionProgram = ResourceManager::Load<ShaderProgram>("#ColorCorrectionProgram");
    m_ColorCorrectionProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/DrawColorCorrection.vert.glsl")));
    m_ColorCorrectionProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/DrawColorCorrection.frag.glsl")));
    m_ColorCorrectionProgram->Compile();
    m_ColorCorrectionProgram->Link();
}

void DrawColorCorrectionPass::Draw(GLuint sceneTexture, GLuint bloomTexture)
{
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLERROR("DrawScreenQuadPass::Draw: Pre");

    DrawScreenQuadPassState state = DrawScreenQuadPassState();
    m_ColorCorrectionProgram->Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    glUniform1f(glGetUniformLocation(m_ColorCorrectionProgram->GetHandle(), "Exposure"), m_Exposure);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloomTexture);

    glBindVertexArray(m_ScreenQuad->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ScreenQuad->ElementBuffer);
    glDrawElementsBaseVertex(GL_TRIANGLES, m_ScreenQuad->MaterialGroups()[0].EndIndex - m_ScreenQuad->MaterialGroups()[0].StartIndex +1
        , GL_UNSIGNED_INT, 0, m_ScreenQuad->MaterialGroups()[0].StartIndex);
}
