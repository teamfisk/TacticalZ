#include "Common.h"
#include "GLM.h"
#include "Core/System.h"
#include "Core/EPlayerSpawned.h"
#include "Input/FirstPersonInputController.h"
#include <imgui/imgui.h>

class PlayerMovementSystem : public ImpureSystem, PureSystem
{
public:
    PlayerMovementSystem(World* world, EventBroker* eventBroker);
    ~PlayerMovementSystem();

    virtual void Update(double dt) override;
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt);

private:
    // State
    std::unordered_map<EntityWrapper, FirstPersonInputController<PlayerMovementSystem>*> m_PlayerInputControllers;

    EventRelay<PlayerMovementSystem, Events::PlayerSpawned> m_EPlayerSpawned;
    bool OnPlayerSpawned(Events::PlayerSpawned& e);

    double m_AssaultDashDoubleTapDeltaTime = 0.0f;
    double m_AssaultDashCoolDownTimer = 0.0f;
    double m_AssaultDashCoolDownMaxTimer = 3.0f;
    ImGuiKey m_AssaultDashDoubleTapLastKey = ImGuiKey_Escape;
    const float m_AssaultDashDoubleTapSensitivityTimer = 0.25f;
    enum class AssaultDashDirection {
        Left,
        Right,
        None
    };
    AssaultDashDirection m_AssaultDashTapDirection = AssaultDashDirection::None;
    bool m_AssaultDashDoubleTapped = false;
    bool m_PlayerIsDashing = false;

    void assaultDashCheck(glm::vec3 controllerMovement, double dt, bool isJumping);
};