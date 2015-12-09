#include <list>
#include <tuple>
#include <boost/any.hpp>
#include "GLM.h"
#include "Core/World.h"
#include "Core/Util/Any.h"

//octTree
//#include <windows.h>

#include <vector>
//last!
#define private public
#include <Engine\Core\OctTree.h>

class HardcodedTestWorld : public World
{
public:
    struct LinkOctTreeAndModel {
        EntityID entId;
    };
    EntityID OctTreeEntityIdSaved;
    //std::vector<ComponentWrapper> allModels;
    //ComponentWrapper moveModel;
    //OctTree* someOctTreePointer;

    //constructor
    HardcodedTestWorld()
        : World()
    {
        registerTestComponents();
        createTestEntities(AABB(glm::vec3(-0.2f, 0.2f, 0.3f), glm::vec3(0.1f, 0.4f, 0.6f)));
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

    void createTestEntities(AABB anotherBox)
    {
        World& world = *this;
        
        //add octTree
        {
            auto someAABB = AABB(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
            OctTree someOctTree(someAABB, 2);
            //add a box
            //auto anotherBox = AABB(glm::vec3(-0.2f, 0.2f, 0.3f), glm::vec3(0.1f, 0.4f, 0.6f));
            //note: have to delete the box in the tree first, since were trying to move the box
            someOctTree.ClearDynamicObjects();
            someOctTree.AddDynamicObject(anotherBox);

            //the main box first
            AddBoxModel(someAABB.Center(), someAABB.HalfSize().x, someOctTree.m_DynamicObjects.size());

            //draw anotherbox
            AddBoxModel(anotherBox.Center(), anotherBox.HalfSize().x, 0);

            //draw the octTree
            for (size_t j = 0; j < 8; j++)
            {
                AddBoxModel(someOctTree.m_Children[j]->m_Box.Center(),
                    someOctTree.m_Children[j]->m_Box.HalfSize().x, someOctTree.m_Children[j]->m_DynamicObjects.size());

                auto someChild = someOctTree.m_Children[j];

                for (size_t i = 0; i < 8; i++)
                {
                    AddBoxModel(someChild->m_Children[i]->m_Box.Center(),
                        someChild->m_Children[i]->m_Box.HalfSize().x, someChild->m_Children[i]->m_DynamicObjects.size());
                }
            }
        }
    }//end CreateEnt

    void AddBoxModel(const glm::vec3 &center, const float &halfSize, const int &contBoxes) {
        World& world = *this;

        EntityID entityDummyScene = world.CreateEntity();

        ComponentWrapper transform = world.AttachComponent(entityDummyScene, "Transform");
        transform["Position"] = center;
        transform["Scale"] = glm::vec3(1.0f, 1.0f, 1.0f)*halfSize*2.0f;
        ComponentWrapper model = world.AttachComponent(entityDummyScene, "Model");
        model["Resource"] = "Models/Core/UnitBox2.obj";
        model["Color"] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        if (contBoxes != 0)
            model["Color"] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

        //extra
        //allModels.push_back(model);
    }
};