#ifndef AmmunitionHUDSystem_h__
#define AmmunitionHUDSystem_h__

#include <boost/lexical_cast.hpp>
#include <sstream>
#include <iomanip>
#include "../../Engine/Core/System.h"
#include "../../Engine/GLM.h"

class TextFieldReader : public PureSystem
{
public:
    TextFieldReader(SystemParams params)
        : System(params)
        , PureSystem("TextFieldReader")
    { }

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cTextFieldReader, double dt) override;
};

#endif