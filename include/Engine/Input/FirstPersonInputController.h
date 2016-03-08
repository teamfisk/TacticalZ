#ifndef FirstPersonInputController_h__
#define FirstPersonInputController_h__

#include "../GLM.h"
#include "../Core/InputController.h"
#include "../Core/ELockMouse.h"
#include "../Game/Events/EDashAbility.h"
#include "InputHandler.h"
#include "Rendering/EAutoAnimationBlend.h"
#include "Rendering/ESetBlendWeight.h"

template <typename EventContext>
class FirstPersonInputController : public InputController<EventContext>
{
public:
    FirstPersonInputController(EventBroker* eventBroker, int playerID, EntityWrapper playerEntity);

    virtual const glm::vec3 Movement() const { return m_Movement; }
    virtual const glm::vec3 Rotation() const { return m_Rotation; }
    virtual bool Jumping() const { return m_Jumping; }
    virtual bool Crouching() const { return m_Crouching; }
    virtual bool DoubleJumping() const { return m_DoubleJumping; }
    virtual void SetDoubleJumping(bool isDoubleJumping) {
        m_DoubleJumping = isDoubleJumping;
    }

    void LockMouse();
    void UnlockMouse();
    virtual bool OnCommand(const Events::InputCommand& e) override;
    virtual void Reset();

    void AssaultDashCheck(double dt, bool isJumping, double assaultDashCoolDownMaxTimer, double& assaultDashCoolDownTimer, EntityID playerID);
    virtual bool AssaultDashDoubleTapped() const { return m_AssaultDashDoubleTapped; }
    virtual bool PlayerIsDashing() const { return m_PlayerIsDashing; }
    bool SpecialAbilityKeyDown() const { return m_SpecialAbilityKeyDown; }

protected:
    const int m_PlayerID;
    EntityWrapper m_PlayerEntity;

    bool m_MouseLocked = false;
    glm::vec3 m_Rotation;
    glm::vec3 m_Movement;
    bool m_Jumping = false;
    bool m_DoubleJumping = false;
    bool m_Crouching = false;
    //assault dash membervariables - needed to calculate the doubletap- and dashlogic
    double m_AssaultDashDoubleTapDeltaTime = 0.0;
    //i will let m_AssaultDashDoubleTapSensitivityTimer stay hardcoded, its not really a gamevariable (more an inputvariable), 
    //and its very unlikely that someone wants to change that value
    const float m_AssaultDashDoubleTapSensitivityTimer = 0.25f;
    std::string m_AssaultDashTapDirection = "";
    std::string m_CurrentDirectionVector = "";
    bool m_AssaultDashDoubleTapped = false;
    bool m_PlayerIsDashing = false;
    bool m_ShiftDashing = false;
    bool m_ValidDoubleTap = false;

    //specialabilities
    bool m_MovementKeyDown = false;
    bool m_SpecialAbilityKeyDown = false;
    int m_NumberOfMovementKeysDown = 0;

    EventRelay<EventContext, Events::LockMouse> m_ELockMouse;
    bool OnLockMouse(const Events::LockMouse& e);
    EventRelay<EventContext, Events::UnlockMouse> m_EUnlockMouse;
    bool OnUnlockMouse(const Events::UnlockMouse& e);
};

template <typename EventContext>
FirstPersonInputController<EventContext>::FirstPersonInputController(EventBroker* eventBroker, int playerID, EntityWrapper playerEntity)
    : InputController(eventBroker)
    , m_PlayerID(playerID)
    , m_PlayerEntity(playerEntity)
{
    EVENT_SUBSCRIBE_MEMBER(m_ELockMouse, &FirstPersonInputController::OnLockMouse);
    EVENT_SUBSCRIBE_MEMBER(m_EUnlockMouse, &FirstPersonInputController::OnUnlockMouse);
}

template <typename EventContext>
void FirstPersonInputController<EventContext>::Reset()
{
    m_Rotation = glm::vec3(0.f, 0.f, 0.f);
    m_Jumping = false;
}

template <typename EventContext>
void FirstPersonInputController<EventContext>::LockMouse()
{
    Events::LockMouse e;
    m_EventBroker->Publish(e);
    m_MouseLocked = true;
}

template <typename EventContext>
void FirstPersonInputController<EventContext>::UnlockMouse()
{
    Events::UnlockMouse e;
    m_EventBroker->Publish(e);
    m_MouseLocked = false;
}

template <typename EventContext>
bool FirstPersonInputController<EventContext>::OnCommand(const Events::InputCommand& e)
{
    if (m_PlayerID != e.PlayerID) {
        return false;
    }

    if (e.Command == "Pitch") {
        float val = glm::radians(e.Value);
        m_Rotation.x += -val;
    }

    if (e.Command == "Yaw") {
        float val = glm::radians(e.Value);
        m_Rotation.y += -val;
    }

    if (e.Command == "Forward" || e.Command == "Right") {
        if (e.Command == "Forward") {
            float val = glm::clamp(e.Value, -1.f, 1.f);
            m_Movement.z = -val;

            //Animation
            if (m_PlayerEntity.Valid()) {
                EntityWrapper playerModel = m_PlayerEntity.FirstChildByName("PlayerModel");
                if (playerModel.Valid()) {
                    if (val > 0) {  // Walk/Run
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.1;
                        if (m_Crouching) {
                            aeb.NodeName = "Walk";
                        } else {
                            aeb.NodeName = "Run";
                        }
                        aeb.RootNode = playerModel;
                        aeb.Start = true;
                        aeb.SingleLevelBlend = true;
                        m_EventBroker->Publish(aeb);
                    } else if (val < 0) {   // Walk/run Backwards
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.1;
                        if (m_Crouching) {
                            aeb.NodeName = "Walk";
                        } else {
                            aeb.NodeName = "Run";
                        }
                        aeb.RootNode = playerModel;
                        aeb.Start = true;
                        aeb.SingleLevelBlend = true;
                        aeb.Reverse = true;
                        m_EventBroker->Publish(aeb);
                    }
                }
            }
        }
        if (e.Command == "Right") {
            float val = glm::clamp(e.Value, -1.f, 1.f);
            m_Movement.x = val;


            //Animation
            if (m_PlayerEntity.Valid()) {
                EntityWrapper playerModel = m_PlayerEntity.FirstChildByName("PlayerModel");
                if (playerModel.Valid()) { //Right Strafe
                    if (val > 0) {
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.1;
                        aeb.NodeName = "Right";
                        aeb.RootNode = playerModel;
                        aeb.SingleLevelBlend = true;
                        aeb.Start = true;
                        m_EventBroker->Publish(aeb);
                    } else if (val < 0) { //LeftStrafe
                        Events::AutoAnimationBlend aeb;
                        aeb.Duration = 0.1;
                        aeb.NodeName = "Left";
                        aeb.RootNode = playerModel;
                        aeb.SingleLevelBlend = true;
                        aeb.Start = true;
                        m_EventBroker->Publish(aeb);
                    }
                }
            }
        }
    }

    //Animation
    if (glm::length2(m_Movement) < 0.1f) {
        //Blend to Idle
        if (m_PlayerEntity.Valid()) {
            EntityWrapper playerModel = m_PlayerEntity.FirstChildByName("PlayerModel");
            if (playerModel.Valid()) {
                Events::AutoAnimationBlend aeb;
                aeb.Duration = 0.2;
                aeb.NodeName = "Idle";
                aeb.RootNode = playerModel;
                m_EventBroker->Publish(aeb);
            }
        }
    } else {

        //Blend to movement
        if (m_PlayerEntity.Valid()) {
            EntityWrapper playerModel = m_PlayerEntity.FirstChildByName("PlayerModel");
            if (playerModel.Valid()) {
                Events::AutoAnimationBlend aeb;
                aeb.Duration = 0.2;
                aeb.NodeName = "MovementBlend";
                aeb.RootNode = playerModel;
                m_EventBroker->Publish(aeb);
            }
        }
    }


    if (glm::length2(m_Movement) > 0) {
        m_Movement = glm::normalize(m_Movement);

        //Animation
        // movement direction blend
        if (m_PlayerEntity.Valid()) {
            EntityWrapper playerModel = m_PlayerEntity.FirstChildByName("PlayerModel");
            if (playerModel.Valid()) {
                glm::vec2 direction = glm::normalize(glm::vec2(m_Movement.x, m_Movement.z));
                double weight = glm::abs(glm::dot(glm::vec2(1, 0), direction));
                Events::SetBlendWeight sbw;
                sbw.NodeName = "MovementBlend";
                sbw.Weight = weight;
                sbw.RootNode = playerModel;
                m_EventBroker->Publish(sbw);
            }
        }
    }

        

    if (e.Command == "Forward" || e.Command == "Right") {
        if (e.Value != 0) {
            m_CurrentDirectionVector = e.Command == "Right" ? (e.Value > 0 ? "Right" : "Left") : (e.Value > 0 ? "Forward" : "Backward");
        }
        //if value = 0 then you have just released this key
        if (e.Value != 0) {
            m_NumberOfMovementKeysDown++;
            m_MovementKeyDown = true;
            //if you pressed the same key within m_AssaultDashDoubleTapSensitivityTimer then you have doubletapped it
            if (m_AssaultDashDoubleTapDeltaTime < m_AssaultDashDoubleTapSensitivityTimer && m_AssaultDashTapDirection == m_CurrentDirectionVector) {
                m_ValidDoubleTap = true;
            }
        } else {
            //== 0
            m_NumberOfMovementKeysDown--;
            if (m_NumberOfMovementKeysDown == 0) {
                m_MovementKeyDown = false;
            }
            //you have just released the key, store what key it was and reset the doubletap-sensitivity-timer
            m_AssaultDashTapDirection = m_CurrentDirectionVector;
            m_AssaultDashDoubleTapDeltaTime = 0.f;

        }
    }

    if (e.Command == "Jump") {
        m_Jumping = e.Value > 0;
    }

    if (e.Command == "Crouch") {
        m_Crouching = e.Value > 0;


        //Animation
        if (m_PlayerEntity.Valid()) {
            EntityWrapper playerModel = m_PlayerEntity.FirstChildByName("PlayerModel");
            if (playerModel.Valid()) {
                if (e.Value == 0.f) {
                    Events::AutoAnimationBlend aeb;
                    aeb.Duration = 0.3;
                    aeb.NodeName = "StandMovement";
                    aeb.RootNode = playerModel;
                    aeb.Start = true;
                    aeb.Restart = true;
                    aeb.SingleLevelBlend = true;
                    m_EventBroker->Publish(aeb);
                } else if(e.Value == 1.0f) {
                    Events::AutoAnimationBlend aeb;
                    aeb.Duration = 0.3;
                    aeb.NodeName = "CrouchMovement";
                    aeb.RootNode = playerModel;
                    aeb.Start = true;
                    aeb.Restart = true;
                    aeb.SingleLevelBlend = true;
                    m_EventBroker->Publish(aeb);
                }
            }
        }
        
    }

    if (e.Command == "SpecialAbility") {
        m_SpecialAbilityKeyDown = e.Value > 0;
    }

    m_ShiftDashing = m_SpecialAbilityKeyDown && m_MovementKeyDown;

    return true;
}

template <typename EventContext>
bool FirstPersonInputController<EventContext>::OnUnlockMouse(const Events::UnlockMouse& e)
{
    m_MouseLocked = false;
    return true;
}

template <typename EventContext>
bool FirstPersonInputController<EventContext>::OnLockMouse(const Events::LockMouse& e)
{
    m_MouseLocked = true;
    return true;
}

template <typename EventContext>
void FirstPersonInputController<EventContext>::AssaultDashCheck(double dt, bool isJumping, double assaultDashCoolDownMaxTimer, double& assaultDashCoolDownTimer, EntityID playerID) {
    m_AssaultDashDoubleTapDeltaTime += dt;
    assaultDashCoolDownTimer -= dt;
    //cooldown = assaultDashCoolDownMaxTimer sec, pretend the dash lasts 0.25 sec (for friction to do its work)
    if (assaultDashCoolDownTimer > (assaultDashCoolDownMaxTimer - 0.25f)) {
        m_PlayerIsDashing = true;
    } else {
        m_PlayerIsDashing = false;
    }

    //dashing with shift
    if (m_ShiftDashing && assaultDashCoolDownTimer <= 0.0f) {
        //player is dashing with shift
        //the wanted-direction is set in playermovement already so we dont need to check what direction we want to dash in!
        assaultDashCoolDownTimer = assaultDashCoolDownMaxTimer;
        m_AssaultDashDoubleTapped = true;
        m_AssaultDashDoubleTapDeltaTime = 0.f;

        Events::DashAbility e;
        e.Player = playerID;
        m_EventBroker->Publish(e);
        return;
    }

    //reset the DoubleTapped state in case we recently doubleTapped (doubletap will only happen during 1 frame)
    if (m_AssaultDashDoubleTapped) {
        m_AssaultDashDoubleTapped = false;
    }

    //dashing with doubletap - check if doubletap to dash enabled
    if (!ResourceManager::Load<ConfigFile>("Input.ini")->Get<bool>("Keyboard.DoubleTapToDash", false)) {
        return;
    }

    //check if we have received a valid doubletap
    if (!m_ValidDoubleTap) {
        return;
    }
    m_ValidDoubleTap = false;

    if (!(assaultDashCoolDownTimer <= 0.0f)) {
        //if we cant dash at the moment, then just reset the tap-sensitivity-timer
        m_AssaultDashDoubleTapDeltaTime = 0.f;
        return;
    }
    //ok, we have a valid tap, lets do it
    m_AssaultDashDoubleTapped = true;
    m_AssaultDashDoubleTapDeltaTime = 0.f;
    assaultDashCoolDownTimer = assaultDashCoolDownMaxTimer;

    Events::DashAbility e;
    e.Player = playerID;
    m_EventBroker->Publish(e);
}

#endif