#include "Rendering/CubeMapPass.h"

CubeMapPass::CubeMapPass(IRenderer* renderer)
    :m_Renderer(renderer)
{
    LoadTextures("Nevada");
}

void CubeMapPass::LoadTextures(std::string input)
{
    if (m_PreviusCubeMapTexture != input) {
        m_CubeMapTextures.clear();
        for (int i = 0; i < 6; i++) {
            std::string str;
            str = "Textures/Test/CubeMap/" + input + "/CubeMapTest0" + std::to_string(i) + ".png";
            Texture* img = ResourceManager::Load<Texture>(str);
            m_CubeMapTextures.push_back(img);
        }
        GenerateCubeMapTexture();
		m_PreviusCubeMapTexture = input;
    }
}

void CubeMapPass::GenerateCubeMapTexture()
{
    if (m_CubeMapTexture == -1) {
        glGenTextures(1, &m_CubeMapTexture);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapTexture);

    for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F, m_CubeMapTextures[0]->Width, m_CubeMapTextures[0]->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_CubeMapTextures[i]->Data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    GLERROR("Generate Cubemap");
}

