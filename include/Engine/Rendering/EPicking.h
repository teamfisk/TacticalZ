#ifndef Events_Picking_h__
#define Events_Picking_h__

#include "../OpenGL.h"
#include "../GLM.h"

#include "../Core/EventBroker.h"
#include "Util/ScreenCoords.h"
#include "FrameBuffer.h"
#include "../Core/Entity.h"
#include "Util/UnorderedMapVec2.h"

namespace Events
{

/** Thrown Every frame, use functions to pick*/
struct Picking : Event
{
public:
    Picking(FrameBuffer* pickingBuffer, GLuint* depthBuffer, glm::mat4 projectionMatrix, glm::mat4 viewMatrix, Rectangle resolution, const std::unordered_map<glm::vec2, EntityID>* pickingColorsToEntity)
        : PickingBuffer(pickingBuffer)
        , DepthBuffer(depthBuffer)
        , ProjectionMatrix(projectionMatrix)
        , ViewMatrix(viewMatrix)
        , Resolution(resolution)
        , PickingColorsToEntity(pickingColorsToEntity)
    { }
        
    

    struct PickData
    {
        //Picked Entity
        EntityID Entity;
        //World position of the "pick"
        glm::vec3 Position;
        // Depth
        float Depth;
    };

    PickData Pick(glm::vec2 screenCoord) const
    {
        PickData pickData;

        // Invert screen y coordinate
        screenCoord.y = Resolution.Height - screenCoord.y;
        ScreenCoords::PixelData data = ScreenCoords::ToPixelData(screenCoord, PickingBuffer, *DepthBuffer);
        pickData.Depth = data.Depth;

        auto it = PickingColorsToEntity->find(glm::vec2(data.Color[0], data.Color[1]));
        if (it != PickingColorsToEntity->end()) {
            pickData.Entity = it->second;
        } else {
            pickData.Entity = 0;
        }
        pickData.Position = ScreenCoords::ToWorldPos(screenCoord.x, screenCoord.y, data.Depth, Resolution, ProjectionMatrix, ViewMatrix);

        return pickData;
    }


private:
    FrameBuffer* PickingBuffer;
    GLuint* DepthBuffer;
    const glm::mat4 ProjectionMatrix;
    const glm::mat4 ViewMatrix;
    const Rectangle Resolution;
    const std::unordered_map<glm::vec2, EntityID>* PickingColorsToEntity;
    
};

}

#endif
