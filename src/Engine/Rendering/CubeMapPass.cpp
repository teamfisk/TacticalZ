#include "Rendering/CubeMapPass.h"

CubeMapPass::CubeMapPass(IRenderer* renderer)
    :m_Renderer(renderer)
{
    LoadTextures("Nevada");
}

void CubeMapPass::LoadTextures(std::string cubemapName)
{
    if (m_PreviusCubeMapTexture != cubemapName) {
        m_CubeMapTextures.clear();
        for (int i = 0; i < 6; i++) {
            std::string path = "Textures/Test/CubeMap/" + cubemapName + "/CubeMapTest0" + std::to_string(i) + ".png";
            m_CubeMapTextures.push_back(path);
        }
        GenerateCubeMapTexture();
        m_PreviusCubeMapTexture = cubemapName;
    }
}

void CubeMapPass::GenerateCubeMapTexture()
{
    if (m_CubeMapTexture == -1) {
        glGenTextures(1, &m_CubeMapTexture);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapTexture);

    for (int i = 0; i < 6; i++) {
        PNG* img = ResourceManager::Load<PNG>(m_CubeMapTextures[i]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F, img->Width, img->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->Data);
        ResourceManager::Release("PNG", m_CubeMapTextures[i]);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    GLERROR("Generate Cubemap");
}

