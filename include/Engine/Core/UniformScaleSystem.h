#ifndef UniformScaleSystem_h__
#define UniformScaleSystem_h__

#include "../GLM.h"
#include "System.h"
#include "../Rendering/ESetCamera.h"

class UniformScaleSystem : public PureSystem
{
public:
    UniformScaleSystem(SystemParams params);

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cUniformScale, double dt) override;

private:
    EntityWrapper m_Camera = EntityWrapper::Invalid;

    EventRelay<UniformScaleSystem, Events::SetCamera> m_ESetCamera;
    bool OnSetCamera(const Events::SetCamera& e);
};

#endif