#include "Rendering/Util/CommonFunctions.h"

Texture* CommonFunctions::LoadTexture(std::string path, bool threaded)
{
    Texture* img;
    try {
        if(threaded) {
            img = ResourceManager::Load<Texture, true>(path);
        } else {
            img = ResourceManager::Load<Texture, false>(path);
        }
    } catch (const Resource::StillLoadingException&) {
        img = ResourceManager::Load<Texture>("Textures/Core/ErrorTexture.png");
    } catch (const std::exception&) {
        img = nullptr;
    }

    return img;
}
