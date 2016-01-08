#ifndef SystemPipeline_h__
#define SystemPipeline_h__

#include "../Common.h"
#include "EventBroker.h"
#include "System.h"
#include "World.h"

class SystemPipeline
{
public:
    SystemPipeline(EventBroker* eventBroker)
        : m_EventBroker(eventBroker)
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
        System* system = new T(m_EventBroker, args...);
        group.Systems[typeid(T).name()] = system;

        if (std::is_base_of<PureSystem, T>::value) {
            PureSystem* pureSystem = static_cast<PureSystem*>(system);
            if (!pureSystem->m_ComponentType.empty()) {
                group.PureSystems[pureSystem->m_ComponentType].push_back(pureSystem);
            } else {
                LOG_ERROR("Failed to add pure system \"%s\": Missing component type!", typeid(T).name());
            }
        }

        if (std::is_base_of<ImpureSystem, T>::value) {
            ImpureSystem* impureSystem = static_cast<ImpureSystem*>(system);
            group.ImpureSystems.push_back(impureSystem);
        }
    }

    void Update(World* world, double dt)
    {
        for (UnorderedSystems& group : m_OrderedSystemGroups) {
            // Process events
            for (auto& pair : group.Systems) {
                m_EventBroker->Process(pair.first);
            }

            // Update
            for (auto& pair : group.PureSystems) {
                const std::string& componentName = pair.first;
                auto& systems = pair.second;
                const ComponentPool* pool = world->GetComponents(componentName);
                if (pool == nullptr) {
                    continue;
                }
                for (auto& component : *pool) {
                    for (auto& system : systems) {
                        system->UpdateComponent(world, component, dt);
                    }
                }
            }
            for (auto& system : group.ImpureSystems) {
                system->Update(world, dt);
            }
        }
    }

private:
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