#include "Systems/WeaponSystem.h"

WeaponSystem::WeaponSystem(SystemParams params, IRenderer* renderer)
    : System(params)
    , PureSystem("Player")
    , ImpureSystem()
    , m_Renderer(renderer)
{
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &WeaponSystem::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_EShoot, &WeaponSystem::OnShoot);
}

void WeaponSystem::Update(double dt)
{

}

void WeaponSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cComponent, double dt)
{

}

bool WeaponSystem::OnInputCommand(Events::InputCommand& e)
{
    // Make sure player is alive
    if (!e.Player.Valid()) {
        return false;
    }

    // Weapon selection
    if (e.Command == "SelectWeapon") {
        selectWeapon(e.Player, static_cast<ComponentInfo::EnumType>(e.Value));
    }

    // Only shoot client-side!
    if (!IsClient) {
        return false;
    }

    // Only shoot if the player is alive
    if (!e.Player.Valid()) {
        return false;
    }

    if (e.Command == "PrimaryFire" && e.Value > 0) {
        Events::Shoot eShoot;
        if (e.PlayerID == -1) {
            eShoot.Player = LocalPlayer;
        } else {
            eShoot.Player = e.Player;
        }
        m_EventBroker->Publish(eShoot);
    }

    return true;
}

void WeaponSystem::selectWeapon(EntityWrapper player, ComponentInfo::EnumType slot)
{
    // Primary
    if (slot == 1) {
        // TODO: if class...
        m_ActiveWeapons[player] = std::make_shared<AssaultWeaponBehaviour>();
    }

    // Secondary
    if (slot == 2) {
        //m_ActiveWeapons[player] = std::make_shared<PistolWeaponBehaviour>();
    }
}

bool WeaponSystem::OnPlayerSpawned(Events::PlayerSpawned& e)
{
    // Select primary weapon on player spawn
    // TODO: Select the active one specified by player component
    selectWeapon(e.Player, 1);
    return true;
}

bool WeaponSystem::OnShoot(Events::Shoot& eShoot)
{
    if (!eShoot.Player.Valid()) {
        return false;
    }

    // TODO: Weapon firing effects here

    auto rayRed = ResourceManager::Load<EntityFile>("Schema/Entities/RayRed.xml");
    auto rayBlue = ResourceManager::Load<EntityFile>("Schema/Entities/RayBlue.xml");

    EntityWrapper weapon = eShoot.Player.FirstChildByName("WeaponMuzzle");
    if (weapon.Valid()) {
        EntityWrapper ray;
        if ((ComponentInfo::EnumType)eShoot.Player["Team"]["Team"] == eShoot.Player["Team"]["Team"].Enum("Red")) {
            EntityFileParser parser(rayRed);
            EntityID rayID = parser.MergeEntities(m_World);
            ray = EntityWrapper(m_World, rayID);
        } else {
            EntityFileParser parser(rayBlue);
            EntityID rayID = parser.MergeEntities(m_World);
            ray = EntityWrapper(m_World, rayID);
        }

        glm::mat4 transformation = Transform::AbsoluteTransformation(weapon);
        glm::vec3 scale;
        glm::vec3 translation;
        glm::quat orientation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(transformation, scale, orientation, translation, skew, perspective);
        
        // Matrix to euler angles
        glm::vec3 euler;
        euler.y = glm::asin(-transformation[0][2]);
        if (cos(euler.y) != 0) {
            euler.x = atan2(transformation[1][2], transformation[2][2]);
            euler.z = atan2(transformation[0][1], transformation[0][0]);
        } else {
            euler.x = atan2(-transformation[2][0], transformation[1][1]);
            euler.z = 0;
        }

        //LOG_DEBUG("rotation: %f %f %f", euler.x, euler.y, euler.z);
        (glm::vec3&)ray["Transform"]["Position"] = translation;
        (glm::vec3&)ray["Transform"]["Orientation"] = euler;
        //(glm::vec3&)ray["Transform"]["Orientation"] = Transform::AbsoluteOrientationEuler(weapon);
    }

    // Only run further picking code for the local player!
    if (eShoot.Player != LocalPlayer) {
        return false;
    }

    // Screen center, based on current resolution!
    // TODO: check if player has enough ammo and if weapon has a cooldown or not
    Rectangle screenResolution = m_Renderer->GetViewportSize();
    glm::vec2 centerScreen = glm::vec2(screenResolution.Width / 2, screenResolution.Height / 2);

    // TODO: check if player has enough ammo and if weapon has a cooldown or not

    // Pick middle of screen
    PickData pickData = m_Renderer->Pick(centerScreen);
    if (pickData.Entity == EntityID_Invalid) {
        return false;
    }

    EntityWrapper player(m_World, pickData.Entity);

    // Only care about players being hit
    if (!player.HasComponent("Player")) {
        player = player.FirstParentWithComponent("Player");
    }
    if (!player.Valid()) {
        return false;
    }

    // Check for friendly fire
    EntityWrapper shooter = eShoot.Player;
    if ((ComponentInfo::EnumType)player["Team"]["Team"] == (ComponentInfo::EnumType)shooter["Team"]["Team"]) {
        return false;
    }

    // TODO: Weapon damage calculations etc
    Events::PlayerDamage ePlayerDamage;
    ePlayerDamage.Player = player;
    ePlayerDamage.Damage = 100;
    m_EventBroker->Publish(ePlayerDamage);

    return true;
}
