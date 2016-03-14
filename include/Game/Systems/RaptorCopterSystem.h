#include "Common.h"
#include "Core/System.h"



 //struct FSubscriptProxy
 //   {
 //       friend struct ComponentWrapper;
 //       FSubscriptProxy(ComponentWrapper* component, std::string fieldName)
 //           : m_Component(component)
 //           , m_FieldName(fieldName)
 //       { }

 //       ComponentWrapper* m_Component;
 //       std::string m_FieldName;

 //   public:
 //       template <typename T>
 //       operator Field<T>() { return Field<T>(m_Component->Field<T>(m_FieldName)); }

 //   };

class RaptorCopterSystem : public PureSystem
{
public:
    RaptorCopterSystem(SystemParams params)
        : System(params)
        , PureSystem("RaptorCopter")
    { }

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cRaptorCopter, double dt) override
    {
        ComponentWrapper& cTransform = entity["Transform"];
        //FSubscriptProxy subOri(&cTransform, "Orientation");
        //Field<glm::vec3> orientation = subOri;
        (Field<glm::vec3>)cTransform["Orientation"] += (float)(const double&)cRaptorCopter["Speed"] * (float)dt * (glm::vec3)cRaptorCopter["Axis"];
    }
};