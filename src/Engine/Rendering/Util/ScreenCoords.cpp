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

ScreenCoords::PixelData ScreenCoords::ToPixelData(float x, float y, FrameBuffer* PickDataBuffer, GLuint DepthBuffer)
{
	GLERROR("Pre");
    PickDataBuffer->Bind();
    unsigned char pdata[3];
    glReadPixels(x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &pdata);
	GLERROR("glReadPixels(pdata) Error");
    PickDataBuffer->Unbind();

    glBindFramebuffer(GL_FRAMEBUFFER, DepthBuffer); 
	GLERROR("glBindFramebuffer(DepthBuffer) Error");
    float depthData;
    glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthData);
	GLERROR("glReadPixels(depthData) Error");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GLERROR("glBindFramebuffer(0) Error");

    PixelData p;
    p.Color[0] = (int)pdata[0];
    p.Color[1] = (int)pdata[1];
    p.Depth = depthData;

    GLERROR("End");

    return p;
}

ScreenCoords::PixelData ScreenCoords::ToPixelData(glm::vec2 screenCoord, FrameBuffer* PickDataBuffer, GLuint DepthBuffer)
{
    return ToPixelData(screenCoord.x, screenCoord.y, PickDataBuffer, DepthBuffer);
}

