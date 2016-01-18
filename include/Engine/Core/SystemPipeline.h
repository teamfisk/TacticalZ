#ifndef SystemPipeline_h__
#define SystemPipeline_h__

#include "../Common.h"
#include "EventBroker.h"
#include "System.h"
#include "World.h"

class SystemPipeline
{
public:
    SystemPipeline(World* world, EventBroker* eventBroker)
        : m_World(world)
        , m_EventBroker(eventBroker)
    { }

    ~SystemPipeline()
    {
        for (UnorderedSystems& group : m_OrderedSystemGroups) {
            for (auto& pair : group.Systems) {
                delete pair.second;
            }
        }
    }

    template <typename T, typename... Arguments>
    //All systems with orderlevel 0 will be updated first, then 1, 2, etc.
    void AddSystem(int updateOrderLevel, Arguments... args)
    {
        if (updateOrderLevel + 1 > m_OrderedSystemGroups.size()) {
            m_OrderedSystemGroups.resize(updateOrderLevel + 1);
        }
        UnorderedSystems& group = m_OrderedSystemGroups[updateOrderLevel];
        System* system = new T(m_World, m_EventBroker, args...);
        group.Systems[typeid(T).name()] = system;

        PureSystem* pureSystem = dynamic_cast<PureSystem*>(system);
        if (pureSystem != nullptr) {
            if (!pureSystem->m_ComponentType.empty()) {
                group.PureSystems[pureSystem->m_ComponentType].push_back(pureSystem);
            } else {
                LOG_ERROR("Failed to add pure system \"%s\": Missing component type!", typeid(T).name());
            }
        }

        ImpureSystem* impureSystem = dynamic_cast<ImpureSystem*>(system);
        if (impureSystem != nullptr) {
            group.ImpureSystems.push_back(impureSystem);
        }
    }

    void Update(double dt)
    {
        for (UnorderedSystems& group : m_OrderedSystemGroups) {
            // Process events
            for (auto& pair : group.Systems) {
                m_EventBroker->Process(pair.first);
            }

            // Update
            for (auto& system : group.ImpureSystems) {
                system->Update(dt);
            }
            for (auto& pair : group.PureSystems) {
                const std::string& componentName = pair.first;
                auto& systems = pair.second;
                const ComponentPool* pool = m_World->GetComponents(componentName);
                if (pool == nullptr) {
                    continue;
                }
                for (auto& component : *pool) {
                    for (auto& system : systems) {
                        system->UpdateComponent(EntityWrapper(m_World, component.EntityID), component, dt);
                    }
                }
            }
        }
    }

private:
    World* m_World;
    EventBroker* m_EventBroker;

    struct UnorderedSystems
    {
        std::map<std::string, System*> Systems;
        std::map<std::string, std::vector<PureSystem*>> PureSystems;
        std::vector<ImpureSystem*> ImpureSystems;
    };
    std::vector<UnorderedSystems> m_OrderedSystemGroups;
};

#endif