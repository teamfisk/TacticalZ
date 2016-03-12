#ifndef CubeMapPass_h__
#define CubeMapPass_h__

#include "IRenderer.h"
#include "ShaderProgram.h"
#include "PNG.h"

class CubeMapPass
{
public:
    CubeMapPass(IRenderer* renderer);
    ~CubeMapPass() { }

    void LoadTextures(std::string input);
    void FillCubeMap(glm::vec3 originPosition);
    void GenerateCubeMapTexture();

    //GLuint CubeMapTexture() const { return m_CubeMapTexture; }
    GLuint m_CubeMapTexture = 0;

private:
    IRenderer* m_Renderer;
    std::string m_PreviusCubeMapTexture;

    std::vector<std::string> m_CubeMapTextures;
};

#endif 