#ifndef CubeMapPass_h__
#define CubeMapPass_h__

#include "IRenderer.h"
#include "ShaderProgram.h"

class CubeMapPass
{
public:
    CubeMapPass(IRenderer* renderer);
    ~CubeMapPass() { }

    void LoadTextures(std::string input);
    void FillCubeMap(glm::vec3 originPosition);
    void GenerateCubeMapTexture();

    //GLuint CubeMapTexture() const { return m_CubeMapTexture; }
    GLuint m_CubeMapTexture = -1;

private:
    IRenderer* m_Renderer;
    std::string m_PreviusCubeMapTexture;

    std::vector<Texture*> m_CubeMapTextures;
};

#endif 