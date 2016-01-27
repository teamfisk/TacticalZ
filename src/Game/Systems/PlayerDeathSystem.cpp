#include "Systems/PlayerDeathSystem.h"

PlayerDeathSystem::PlayerDeathSystem(World* m_World, EventBroker* eventBroker)
    : System(m_World, eventBroker)
{
    EVENT_SUBSCRIBE_MEMBER(m_OnInputCommand, &PlayerDeathSystem::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_OnPlayerDeath, &PlayerDeathSystem::OnPlayerDeath);
    m_NetworkEnabled = ResourceManager::Load<ConfigFile>("Config.ini")->Get("Networking.StartNetwork", false);
}

void PlayerDeathSystem::Update(double dt)
{
    auto playerSpawns = m_World->GetComponents("PlayerSpawn");
    if (playerSpawns == nullptr) {
        return;
    }

    for (auto& req : m_SpawnRequests) {
        for (auto& cPlayerSpawn : *playerSpawns) {
            EntityWrapper spawner(m_World, cPlayerSpawn.EntityID);
            if (!spawner.HasComponent("Spawner")) {
                continue;
            }

            // If the spawner has a team affiliation, check it
            if (spawner.HasComponent("Team")) {
                if ((int)spawner["Team"]["Team"] != req.Team) {
                    continue;
                }
            }

            // Spawn the player!
            EntityWrapper player = SpawnerSystem::Spawn(spawner);
            // Set the player team affiliation
            player["Team"]["Team"] = req.Team;

            // Publish a PlayerSpawned event
            Events::PlayerSpawned e;
            e.PlayerID = req.PlayerID;
            e.Player = player;
            e.Spawner = spawner;
            m_EventBroker->Publish(e);

        }
    }
    m_SpawnRequests.clear();
}

bool PlayerDeathSystem::OnInputCommand(const Events::InputCommand& e)
{
    //testing: Jump -> playerdamage
    if (e.Command != "Jump") {
        return false;
    }

    // 0 = released
    if (e.Value != 0) {
        return false;
    }

    auto players = m_World->GetComponents("Player");

    for (auto& cPlayer : *players) {
        EntityWrapper player(m_World, cPlayer.EntityID);
        Events::PlayerDamage e;
        e.Player = player;
        e.Damage = 50;
        m_EventBroker->Publish(e);
    }



    return true;
}

bool PlayerDeathSystem::OnPlayerDeath(Events::PlayerDeath& e)
{
    //// When a player is actually spawned (since the actual spawning is handled on the server)

    //// Check if a player already exists
    //if (m_PlayerEntities.count(e.PlayerID) != 0) {
    //    // TODO: Disallow infinite respawning here
    //    m_World->DeleteEntity(m_PlayerEntities[e.PlayerID].ID);
    //}

    //// Store the player for future reference
    //m_PlayerEntities[e.PlayerID] = e.Player;

    //// Set the camera to the correct entity
    //EntityWrapper cameraEntity = e.Player.FirstChildByName("Camera");
    //if (cameraEntity.Valid()) {
    //    Events::SetCamera e;
    //    e.CameraEntity = cameraEntity;
    //    m_EventBroker->Publish(e);
    //}

    //// HACK: Set the player model color to team color
    //EntityWrapper playerModel = e.Player.FirstChildByName("PlayerModel");
    //if (playerModel.Valid() && e.Player.HasComponent("Team")) {
    //    ComponentWrapper cTeam = e.Player["Team"];
    //    ComponentWrapper cModel = playerModel["Model"];
    //    if ((ComponentInfo::EnumType)cTeam["Team"] == cTeam["Team"].Enum("Red")) {
    //        cModel["Color"] = glm::vec3(1.f, 0.f, 0.f);
    //    } else if ((ComponentInfo::EnumType)cTeam["Team"] == cTeam["Team"].Enum("Blue")) {
    //        cModel["Color"] = glm::vec3(0.f, 0.25f, 1.f);
    //    }
    //}

    //// TODO: Set the player name to whatever
    //EntityWrapper playerName = e.Player.FirstChildByName("PlayerName");
    //if (playerName.Valid()) {
    //    playerName["Text"]["Content"] = e.PlayerName;
    //}

    //m_World->DeleteEntity(e.Player.ID);

    //koppla till en kamera modell för att explodera den

    auto cameras = m_World->GetComponents("Camera");

    for (auto& someCamera : *cameras) {
        EntityWrapper entity = EntityWrapper(m_World, someCamera.EntityID);
        auto cameraID = entity.ID;
        auto modelID = entity.FirstChildByName("HUD");
        auto weapID = entity.FirstChildByName("Weapon").ID;

        //    auto modelID = m_World->GetComponent(e.PlayerID, "Camera");
        ComponentWrapper& explosionEffect = m_World->AttachComponent(weapID, "ExplosionEffect");
        ComponentWrapper& lifeTime = m_World->AttachComponent(weapID, "Lifetime");
        (glm::vec3)explosionEffect["ExplosionOrigin"] = glm::vec3(0, 0, 0);
        //explosionEffect["ExplosionOrigin"] = e.Player["Transform"]["Position"];
        (double)explosionEffect["TimeSinceDeath"] = 0.0;
        (double)explosionEffect["ExplosionDuration"] = 2.0;
        explosionEffect["EndColor"] = glm::vec4(0, 0, 0, 1);
        (bool)explosionEffect["Randomness"] = false;
        (double)explosionEffect["RandomnessScalar"] = 1.0;
        (glm::vec2)explosionEffect["Velocity"] = glm::vec2(1, 1);
        (bool)explosionEffect["ColorByDistance"] = false;
        (bool)explosionEffect["ExponentialAccelaration"] = false;
        lifeTime["Lifetime"] = 2.0;
    }

   
    auto t = e.Player.FirstChildByName("PlayerModel");
    auto playerEntityModel = t["Model"];
    auto playerEntityAnimation = t["Animation"];
    auto playerEntityTransform = t["Transform"];
    //auto playerEntityModel = e.Player["PlayerModel"];
//    ComponentWrapper& playerEntityModel = e.Player.FirstChildByName("PlayerModel");

    auto newEntity = m_World->CreateEntity();
    ComponentWrapper& newEntityModel = m_World->AttachComponent(newEntity, "Model");
    ComponentWrapper& newAnimationModel = m_World->AttachComponent(newEntity, "Animation");
    ComponentWrapper& newTransformModel = m_World->AttachComponent(newEntity, "Transform");

    //ComponentWrapper& playerEntityModel = m_World->GetComponent(e.PlayerID, "PlayerModel");
    playerEntityModel.Copy(newEntityModel);
    playerEntityAnimation.Copy(newAnimationModel);
    playerEntityTransform.Copy(newTransformModel);
    newAnimationModel["Speed"] = (double)0.0;
    auto t2 = e.Player["Transform"]["Position"];
    newTransformModel["Position"] = (glm::vec3)e.Player["Transform"]["Position"];
    newTransformModel["Scale"] = glm::vec3(5, 5, 5);
    //auto tr = m_World->GetComponent(newEntityModel.EntityID, "Transform");
    //tr["Position"] = glm::vec3(0, 50, 0);

    ComponentWrapper& explosionEffect = m_World->AttachComponent(newEntityModel.EntityID, "ExplosionEffect");
    ComponentWrapper& lifeTime = m_World->AttachComponent(newEntityModel.EntityID, "Lifetime");
    (glm::vec3)explosionEffect["ExplosionOrigin"] = (glm::vec3)e.Player["Transform"]["Position"];
    //explosionEffect["ExplosionOrigin"] = e.Player["Transform"]["Position"];
    (double)explosionEffect["TimeSinceDeath"] = 0.0;
    (double)explosionEffect["ExplosionDuration"] = 8.0;
    explosionEffect["EndColor"] = glm::vec4(0, 0, 0, 1);
    (bool)explosionEffect["Randomness"] = false;
    (double)explosionEffect["RandomnessScalar"] = 1.0;
    (glm::vec2)explosionEffect["Velocity"] = glm::vec2(1, 1);
    (bool)explosionEffect["ColorByDistance"] = false;
    (bool)explosionEffect["ExponentialAccelaration"] = false;
    lifeTime["Lifetime"] = 8.0;

    return true;
}
//on deathanim done -> del entity
