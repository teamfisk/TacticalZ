#ifndef ScreenCoords_h__
#define ScreenCoords_h__

#include "../../Common.h"
#include "../../OpenGL.h"
#include "../../GLM.h"
#include "../../Core/Util/Rectangle.h"
#include "../FrameBuffer.h"

class ScreenCoords 
{
public: 
    ScreenCoords() = delete;

    struct PixelData
    {
        int Color[2];
        float Depth;
    };

    //Return world position from given screenspace coordinates and depth value in viewspace.
    static glm::vec3 ToWorldPos(glm::vec2 screenCoord, float depth, Rectangle resolution, glm::mat4 cameraProjectionMat, glm::mat4 cameraViewMat);
    static glm::vec3 ToWorldPos(float x, float y, float depth, Rectangle resolution, glm::mat4 cameraProjectionMat, glm::mat4 cameraViewMat);
    static glm::vec3 ToWorldPos(glm::vec2 screenCoord, float depth, float screenWidth, float screenHeight, glm::mat4 cameraProjectionMat, glm::mat4 cameraViewMat);
    static glm::vec3 ToWorldPos(float x, float y, float depth, float screenWidth, float screenHeight, glm::mat4 cameraProjectionMat, glm::mat4 cameraViewMat);
    //Return data from the given buffers at the coordinates given in screenspace. Buffer should probably have a texture that covers the screen.
    //Data is given as R = x, B = y, and 
    static PixelData ToPixelData(glm::vec2 screenCoord, FrameBuffer* PickDataBuffer, GLuint DepthBuffer);
    static PixelData ToPixelData(float x, float y, FrameBuffer* PickDataBuffer, GLuint DepthBuffer);
    //Return EntityID of the clicked coordinate in given screenspace coordinates.
    //EntityID ScreenCoordsToEntityID(glm::vec2 screenCoord, float depth);

private:

};

#endif