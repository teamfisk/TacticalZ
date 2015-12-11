#ifndef FirstPersonInputController_h__
#define FirstPersonInputController_h__

#include "../GLM.h"
#include "../Core/InputController.h"

template <typename EventContext>
class FirstPersonInputController : public InputController<EventContext>
{
public:
    FirstPersonInputController(EventBroker* eventBroker, unsigned int playerID)
        : InputController(eventBroker)
        , m_PlayerID(playerID)
    { }

    const glm::quat Orientation() const { return m_Orientation; }

    virtual bool OnCommand(const Events::InputCommand& e) override
    {
        if (m_PlayerID != e.PlayerID) {
            return false;
        }

        if (e.Command == "Pitch") {
            float val = glm::radians(e.Value);
            m_Orientation = m_Orientation * glm::angleAxis<float>(-val, glm::vec3(1, 0, 0));
        }

        if (e.Command == "Yaw") {
            float val = glm::radians(e.Value);
		    m_Orientation = glm::angleAxis<float>(-val, glm::vec3(0, 1, 0)) * m_Orientation;
        }
    }

private:
    const unsigned int m_PlayerID;
    glm::quat m_Orientation;
};

#endif