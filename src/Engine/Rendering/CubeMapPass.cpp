#include "Rendering/CubeMapPass.h"

CubeMapPass::CubeMapPass(IRenderer* renderer)
    :m_Renderer(renderer)
{
    LoadTextures();
    GenerateCubeMapTexture();
}

/*

*/

void CubeMapPass::LoadTextures()
{
    for (int i = 0; i < 6; i++){
        std::string str;
        str = "Textures/Test/CubeMap/CubeMapTest0" + std::to_string(i) + ".png";
        Texture* img = ResourceManager::Load<Texture>(str);
        m_CubeMapTestTextures.push_back(img);
    }
}

void CubeMapPass::GenerateCubeMapTexture()
{
    glGenTextures(1, &m_CubeMapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubeMapTexture);

    for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_CubeMapTestTextures[i]->Data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    GLERROR("Generate Cubemap");
}

