#include <list>
#include <tuple>
#include <boost/any.hpp>
#include "GLM.h"
#include "Core/World.h"
#include "Core/Util/Any.h"

#include <vector>
//last!
#define private public
#include <Engine\Core\OctTree.h>

class HardcodedTestWorld : public World
{
public:
    struct LinkOctTreeAndModel {
        EntityID entId;
        OctTree* child;
        glm::vec3 posxyz;
        LinkOctTreeAndModel(EntityID eId, OctTree* ch, glm::vec3 pos)
        {
            entId = eId;
            child = ch;
            posxyz = pos;
        }
    };
    EntityID anotherBoxTransformId;
    std::vector<LinkOctTreeAndModel> linkOM;
    OctTree someOctTree;

    //constructor
    HardcodedTestWorld()
        : World()
        , someOctTree(AABB(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)), 2)
    {
        registerTestComponents();
        //createTestEntities();
    }

private:
    void registerTestComponents()
    {
        ComponentWrapperFactory f;


        f = ComponentWrapperFactory("Test");
        f.AddProperty("TestInteger", 1337);
        f.AddProperty("TestFloat", 13.37f);
        f.AddProperty("TestString", std::string("Carlito"));
        RegisterComponent(f);

        f = ComponentWrapperFactory("Debug");
        f.AddProperty("Name", std::string("Unnamed"));
        RegisterComponent(f);

        f = ComponentWrapperFactory("Transform");
        f.AddProperty("Position", glm::vec3(0.f, 0.f, 0.f));
        f.AddProperty("Orientation", glm::quat());
        f.AddProperty("Scale", glm::vec3(1.f, 1.f, 1.f));
        RegisterComponent(f);

        f = ComponentWrapperFactory("Model");
        f.AddProperty("Resource", std::string());
        f.AddProperty("Color", glm::vec4(1.f, 1.f, 1.f, 1.f));
        f.AddProperty("Visible", true);
        RegisterComponent(f);
    }

    void createTestEntities()
    {
        World& world = *this;
        EntityID tempId;
        //add octTree
        {
            //copy of mainbox
            auto someAABB = AABB(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));

            //draw main box first
            AddBoxModel(someAABB.Center(), someAABB.HalfSize().x, &someOctTree, tempId);

            //add anotherbox in octTree
            auto anotherBox = AABB(glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.2f, 0.2f, 0.2f));
            //note: have to delete the box in the tree first, since were trying to move the box
            someOctTree.AddDynamicObject(anotherBox);

            //draw anotherbox and save it in anotherBoxTransformId
            AddBoxModel(anotherBox.Center(), anotherBox.HalfSize().x, &someOctTree, anotherBoxTransformId);

            //draw the octTree
            for (size_t j = 0; j < 8; j++)
            {
                AddBoxModel(someOctTree.m_Children[j]->m_Box.Center(),
                    someOctTree.m_Children[j]->m_Box.HalfSize().x, someOctTree.m_Children[j], tempId);

                auto someChild = someOctTree.m_Children[j];

                for (size_t i = 0; i < 8; i++)
                {
                    AddBoxModel(someChild->m_Children[i]->m_Box.Center(),
                        someChild->m_Children[i]->m_Box.HalfSize().x, someChild->m_Children[i], tempId);
                }
            }
        }
    }//end CreateEnt

    void AddBoxModel(const glm::vec3 &center, const float &halfSize, OctTree* child, EntityID &outEntityId) {
        World& world = *this;

        EntityID entityDummyScene = world.CreateEntity();
        outEntityId = entityDummyScene;
        ComponentWrapper transform = world.AttachComponent(entityDummyScene, "Transform");
        transform["Position"] = center;
        transform["Scale"] = glm::vec3(1.0f, 1.0f, 1.0f)*halfSize*2.0f*0.97f;
        ComponentWrapper model = world.AttachComponent(entityDummyScene, "Model");
        model["Resource"] = "Models/Core/UnitBox.obj";
        model["Color"] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        if (child->m_DynamicObjects.size() != 0)
            model["Color"] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

        linkOM.emplace_back(entityDummyScene, child, center);
        //extra
        //allModels.push_back(model);
    }
    void createTestEntitiesTest2()
    {
        World& world = *this;

        EntityID entityCollisionBox = world.CreateEntity();
        ComponentWrapper transform = world.AttachComponent(entityCollisionBox, "Transform");
        transform["Position"] = glm::vec3(0.f, 2.f, 0.f);
        ComponentWrapper model = world.AttachComponent(entityCollisionBox, "Model");
        model["Resource"] = "Models/Core/UnitBox.obj";
    }
};