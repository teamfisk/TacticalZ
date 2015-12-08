#include "Rendering/Util/ScreenCoords.h"

glm::vec3 ScreenCoords::ToWorldPos(float x, float y, float depth, float screenWidth, float screenHeight, glm::mat4 cameraProjectionMat, glm::mat4 cameraViewMat)
{
    glm::vec3 ndc;
    ndc.x = (x / screenWidth)*2.f - 1.f;
    ndc.y = (y / screenHeight)*2.f - 1.f;
    ndc.z = depth*2.f - 1.f;
    glm::vec4 clipSpace = glm::vec4(ndc, 1.0f);
    glm::vec4 EyeSpace = glm::inverse(cameraProjectionMat) * clipSpace;
    glm::vec4 WorldSpace = glm::inverse(cameraViewMat) * EyeSpace;
    WorldSpace = glm::vec4(glm::vec3(WorldSpace) / WorldSpace.w, 1.f);

    return glm::vec3(WorldSpace);
}

glm::vec3 ScreenCoords::ToWorldPos(glm::vec2 screenCoord, float depth, Rectangle resolution, glm::mat4 cameraProjectionMat, glm::mat4 cameraViewMat)
{
    return ToWorldPos(screenCoord.x, screenCoord.y, depth, resolution.Width, resolution.Height, cameraProjectionMat, cameraViewMat);
}

glm::vec3 ScreenCoords::ToWorldPos(float x, float y, float depth, Rectangle resolution, glm::mat4 cameraProjectionMat, glm::mat4 cameraViewMat)
{
    return ToWorldPos(x, y, depth, resolution.Width, resolution.Height, cameraProjectionMat, cameraViewMat);
}

glm::vec3 ScreenCoords::ToWorldPos(glm::vec2 screenCoord, float depth, float screenWidth, float screenHeight, glm::mat4 cameraProjectionMat, glm::mat4 cameraViewMat)
{
    return ToWorldPos(screenCoord.x, screenCoord.y, depth, screenWidth, screenHeight, cameraProjectionMat, cameraViewMat);
}

glm::vec3 ScreenCoords::ToPixelData(float x, float y, FrameBuffer* PickDataBuffer, GLuint DepthBuffer)
{
    PickDataBuffer->Bind();
    glm::vec2 pixelData;
    glReadPixels(x, y, 1, 1, GL_RG, GL_FLOAT, &pixelData);
    PickDataBuffer->Unbind();

    glBindFramebuffer(GL_FRAMEBUFFER, DepthBuffer);
    float depthData;
    glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthData);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return glm::vec3(pixelData, depthData);
}

glm::vec3 ScreenCoords::ToPixelData(glm::vec2 screenCoord, FrameBuffer* PickDataBuffer, GLuint DepthBuffer)
{
    return ToPixelData(screenCoord.x, screenCoord.y, PickDataBuffer, DepthBuffer);
}
