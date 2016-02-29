#ifndef Frustum_h__
#define Frustum_h__

#include "../GLM.h"
#include "AABB.h"
#include <bitset>

//A frustum defined by 6 planes.
struct Frustum
{
    //Contains points P in: dot(normal, P) + d = 0
    struct Plane
    {
        glm::vec3 Normal;
        float Distance;
    };

    enum class Output
    {
        Inside,
        Outside,
        Intersects
    };
    Plane Planes[6];

    Frustum() = default;
    Frustum(glm::mat4x4 viewProjMatrix)
    {
        //Order: Right, left, top, bottom, far, near.
        int sign = 1;
        for (int i = 0; i < 6; ++i) {
            sign = -sign;
            int index = i / 2;
            Plane& plane = Planes[i];
            plane.Normal.x = viewProjMatrix[0].w + sign * viewProjMatrix[0][index];
            plane.Normal.y = viewProjMatrix[1].w + sign * viewProjMatrix[1][index];
            plane.Normal.z = viewProjMatrix[2].w + sign * viewProjMatrix[2][index];
            plane.Distance = viewProjMatrix[3].w + sign * viewProjMatrix[3][index];
            float divByNormalLength = 1.0f / glm::length(plane.Normal);
            plane.Normal *= divByNormalLength;
            plane.Distance *= divByNormalLength;
        }
    }

    Output VsAABB(const AABB& box) const
    {
        const glm::vec3& maxCorner = box.MaxCorner();
        const glm::vec3& minCorner = box.MinCorner();
        bool completelyInside = true;
        for (const Plane& p : Planes) {
            bool anyWasInside = false;
            bool anyWasOutside = false;
            //If points are on both sides of the plane, we can stop.
            for (int i = 0; i < 8 && (!anyWasInside || !anyWasOutside); ++i) {
                std::bitset<3> bits(i);
                glm::vec3 corner;
                corner.x = bits.test(0) ? maxCorner.x : minCorner.x;
                corner.y = bits.test(1) ? maxCorner.y : minCorner.y;
                corner.z = bits.test(2) ? maxCorner.z : minCorner.z;
                if (glm::dot(p.Normal, corner) > -p.Distance) {
                    anyWasInside = true;
                } else {
                    anyWasOutside = true;
                }
            }
            if (!anyWasInside) {
                return Output::Outside;
            }
            if (anyWasOutside) {
                completelyInside = false;
            }
        }
        return completelyInside ? Output::Inside : Output::Intersects;
    }
};

#endif