#include "Game.h"
#include "Collision/FillOctreeSystem.h"
#include "Collision/FillFrustumOctreeSystem.h"
#include "Collision/EntityAABB.h"
#include "Collision/TriggerSystem.h"
#include "Collision/CollisionSystem.h"
#include "Systems/RaptorCopterSystem.h"
#include "Systems/HealthSystem.h"
#include "Systems/PlayerMovementSystem.h"
#include "Systems/SpawnerSystem.h"
#include "Systems/PlayerSpawnSystem.h"
#include "Systems/PlayerDeathSystem.h"
#include "Systems/FloatingEffectSystem.h"
#include "Core/EntityFile.h"
#include "Core/EntityXMLFileWriter.h"
#include "Game/Systems/CapturePointSystem.h"
#include "Game/Systems/CapturePointHUDSystem.h"
#include "Game/Systems/PickupSpawnSystem.h"
#include "Game/Systems/AmmoPickupSystem.h"
#include "Game/Systems/DamageIndicatorSystem.h"
#include "Game/Systems/Weapon/DefenderWeaponBehaviour.h"
#include "Rendering/AnimationSystem.h"
#include "Game/Systems/HealthHUDSystem.h"
#include "Rendering/BoneAttachmentSystem.h"
#include "Game/Systems/LifetimeSystem.h"
#include "../Engine/Core/UniformScaleSystem.h"
#include "Rendering/AnimationSystem.h"
#include "Network/MultiplayerSnapshotFilter.h"
#include "Game/Systems/AmmunitionHUDSystem.h"
#include "Game/Systems/AbilityCooldownHUDSystem.h"
#include "Game/Systems/CapturePointArrowHUDSystem.h"
#include "Game/Systems/KillFeedSystem.h"
#include "Game/Systems/BoostSystem.h"
#include "Game/Systems/BoostIconsHUDSystem.h"
#include "Game/Systems/ScoreScreenSystem.h"
#include "GUI/ButtonSystem.h"
#include "GUI/MainMenuSystem.h"


Game::Game(int argc, char* argv[])
{
    parseArgs(argc, argv);

    ResourceManager::RegisterType<ConfigFile>("ConfigFile");
    ResourceManager::RegisterType<Sound>("Sound");
    ResourceManager::RegisterType<Model>("Model");
    ResourceManager::RegisterType<RawModel>("RawModel");
    ResourceManager::RegisterType<Texture>("Texture");
    ResourceManager::RegisterType<PNG>("Png");
    ResourceManager::RegisterType<ShaderProgram>("ShaderProgram");
    ResourceManager::RegisterType<EntityFile>("EntityFile");
    ResourceManager::RegisterType<EntityXMLFile>("EntityXMLFile");
    ResourceManager::RegisterType<Font>("FontFile");

    m_Config = ResourceManager::Load<ConfigFile>("Config.ini");
    ResourceManager::UseThreading = m_Config->Get<bool>("Multithreading.ResourceLoading", true);
    DisableMemoryPool::Value = m_Config->Get<bool>("Debug.DisableMemoryPool", false);
    LOG_LEVEL = static_cast<_LOG_LEVEL>(m_Config->Get<int>("Debug.LogLevel", 1));

    // Create the core event broker
    m_EventBroker = new EventBroker();

    // Create the renderer
    m_Renderer = new Renderer(m_EventBroker, m_Config);
    m_Renderer->SetFullscreen(m_Config->Get<bool>("Video.Fullscreen", false));
    m_Renderer->SetVSYNC(m_Config->Get<bool>("Video.VSYNC", false));
    m_Renderer->SetResolution(Rectangle::Rectangle(
        0,
        0,
        m_Config->Get<int>("Video.Width", 1280),
        m_Config->Get<int>("Video.Height", 720)
    ));
    m_Renderer->Initialize();
    //m_Renderer->Camera()->SetFOV(glm::radians(m_Config->Get<float>("Video.FOV", 90.f)));
    m_RenderFrame = new RenderFrame();

    // Create input manager
    m_InputManager = new InputManager(m_Renderer->Window(), m_EventBroker);
    m_InputProxy = new InputProxy(m_EventBroker);
    m_InputProxy->AddHandler<KeyboardInputHandler>();
    m_InputProxy->AddHandler<MouseInputHandler>();
    m_InputProxy->LoadBindings("Input.ini");

    // Create a world
    m_World = new World(m_EventBroker);
    std::string mapToLoad = m_Config->Get<std::string>("Debug.LoadMap", "");
    if (!mapToLoad.empty()) {
        auto file = ResourceManager::Load<EntityFile>(mapToLoad);
        file->MergeInto(m_World);
    }

    // Create the sound manager
    m_SoundManager = new SoundManager(m_World, m_EventBroker);

    // Initialize network
    if (m_Config->Get<bool>("Networking.StartNetwork", false)) {
        if (m_IsServer) {
            m_NetworkServer = new Server(m_World, m_EventBroker, m_NetworkPort);
            m_Renderer->SetWindowTitle(m_Renderer->WindowTitle() + " SERVER");
        } else if (m_IsClient) {
            m_NetworkClient = new Client(m_World, m_EventBroker, std::make_unique<MultiplayerSnapshotFilter>(m_EventBroker));
            m_NetworkClient->Connect(m_NetworkAddress, m_NetworkPort);
            m_Renderer->SetWindowTitle(m_Renderer->WindowTitle() + " CLIENT");
        }
    } else {
        // If network is disabled, pretend we're a server
        m_IsClient = true;
        m_IsServer = true;
    }

    // Create Octrees
    // TODO: Perhaps the world bounds should be set in some non-arbitrary way instead of this.
    AABB boxContainingTheWorld(glm::vec3(-300), glm::vec3(300));
    m_OctreeCollision = new Octree<EntityAABB>(boxContainingTheWorld, 4);
    m_OctreeTrigger = new Octree<EntityAABB>(boxContainingTheWorld, 4);
    m_OctreeFrustrumCulling = new Octree<EntityAABB>(boxContainingTheWorld, 4);
    // Create system pipeline
    m_SystemPipeline = new SystemPipeline(m_World, m_EventBroker, m_IsClient, m_IsServer);

    // All systems with orderlevel 0 will be updated first.
    unsigned int updateOrderLevel = 0;
    m_SystemPipeline->AddSystem<InterpolationSystem>(updateOrderLevel);
    ++updateOrderLevel;
    m_SystemPipeline->AddSystem<SoundSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<RaptorCopterSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<FloatingEffectSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<ExplosionEffectSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<HealthSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<PlayerMovementSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<SpawnerSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<PlayerSpawnSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<DefenderWeaponBehaviour>(updateOrderLevel, m_Renderer, m_OctreeCollision);
    m_SystemPipeline->AddSystem<LifetimeSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<CapturePointSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<CapturePointHUDSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<PickupSpawnSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<AmmoPickupSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<DamageIndicatorSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<AmmunitionHUDSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<AbilityCooldownHUDSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<CapturePointArrowHUDSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<KillFeedSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<LifetimeSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<CapturePointSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<CapturePointHUDSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<PickupSpawnSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<AmmoPickupSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<DamageIndicatorSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<AmmunitionHUDSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<KillFeedSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<BoostSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<ButtonSystem>(updateOrderLevel, m_Renderer);
    m_SystemPipeline->AddSystem<MainMenuSystem>(updateOrderLevel, m_Renderer);
    m_SystemPipeline->AddSystem<BoostIconsHUDSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<ScoreScreenSystem>(updateOrderLevel);
    // Populate Octree with collidables
    ++updateOrderLevel;
    m_SystemPipeline->AddSystem<FillOctreeSystem>(updateOrderLevel, m_OctreeCollision, "Collidable");
    m_SystemPipeline->AddSystem<FillOctreeSystem>(updateOrderLevel, m_OctreeTrigger, "Player");
    m_SystemPipeline->AddSystem<AnimationSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<UniformScaleSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<HealthHUDSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<PlayerDeathSystem>(updateOrderLevel);
    // Collision and TriggerSystem should update after player.
    ++updateOrderLevel;
    m_SystemPipeline->AddSystem<BoneAttachmentSystem>(updateOrderLevel);
    m_SystemPipeline->AddSystem<CollisionSystem>(updateOrderLevel, m_OctreeCollision);
    m_SystemPipeline->AddSystem<TriggerSystem>(updateOrderLevel, m_OctreeTrigger);
    // Octree for frustum culling must be updated after collisions, otherwise players frustum may be moved after tree is filled, and wrong things are culled.
    ++updateOrderLevel;
    m_SystemPipeline->AddSystem<FillFrustumOctreeSystem>(updateOrderLevel, m_OctreeFrustrumCulling);
    ++updateOrderLevel;
    m_SystemPipeline->AddSystem<RenderSystem>(updateOrderLevel, m_Renderer, m_RenderFrame, m_OctreeFrustrumCulling);
    ++updateOrderLevel;
    m_SystemPipeline->AddSystem<EditorSystem>(updateOrderLevel, m_Renderer, m_RenderFrame);

    m_LastTime = glfwGetTime();
}

Game::~Game()
{
    delete m_SystemPipeline;
    delete m_OctreeFrustrumCulling;
    delete m_OctreeCollision;
    delete m_OctreeTrigger;
    delete m_SoundManager;
    if (m_NetworkClient != nullptr) {
        delete m_NetworkClient;
    }
    if (m_NetworkServer != nullptr) {
        delete m_NetworkServer;
    }
    delete m_World;
    delete m_InputProxy;
    delete m_InputManager;
    delete m_RenderFrame;
    delete m_Renderer;
    delete m_EventBroker;
}

void Game::Tick()
{
    glfwPollEvents();

    double currentTime = glfwGetTime();
    double dt = currentTime - m_LastTime;
    m_LastTime = currentTime;

    // Handle input in a weird looking but responsive way
    m_EventBroker->Process<InputManager>();
    m_EventBroker->Swap();
    PerformanceTimer::StartTimer("InputManager");
    m_InputManager->Update(dt);
    m_EventBroker->Swap();
    PerformanceTimer::StartTimerAndStopPrevious("InputProxy");
    m_InputProxy->Update(dt);
    m_EventBroker->Swap();
    m_InputProxy->Process();
    m_EventBroker->Swap();

    PerformanceTimer::StartTimerAndStopPrevious("SoundManager");
    m_SoundManager->Update(dt);

    // Update network
    PerformanceTimer::StartTimerAndStopPrevious("Network");
    m_EventBroker->Process<MultiplayerSnapshotFilter>();
    if (m_NetworkClient != nullptr) {
        m_NetworkClient->Update();
    }
    if (m_NetworkServer != nullptr) {
        m_NetworkServer->Update();
    }
    //m_SoundManager->Update(dt);

    // Iterate through systems and update world!
    PerformanceTimer::StartTimerAndStopPrevious("SystemPipeline");
    m_EventBroker->Process<SystemPipeline>();
    m_SystemPipeline->Update(dt);
    PerformanceTimer::StartTimerAndStopPrevious("RendererUpdate");
    m_Renderer->Update(dt);
    PerformanceTimer::StartTimerAndStopPrevious("RendererDraw");
    m_Renderer->Draw(*m_RenderFrame);
    PerformanceTimer::StopTimer("RendererDraw");
    m_RenderFrame->Clear();
    m_EventBroker->Swap();
    m_EventBroker->Clear();
}

int Game::parseArgs(int argc, char* argv[])
{
    namespace po = boost::program_options;

    po::options_description desc("Options");
    desc.add_options()
        ("help", "Help")
        ("server,s", po::bool_switch(&m_IsServer), "Launch game in server mode")
        ("connect", po::value<std::string>(&m_NetworkAddress)->default_value(""), "Connect to this address in client mode")
        ("port,p", po::value<int>(&m_NetworkPort), "Port to listen on or connect to");
    ;

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (std::exception& e) {
        LOG_ERROR(e.what());
        return 1;
    }

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        exit(1);
    }

    // HACK: Right now, client and server are mutually exclusive
    m_IsClient = true;
    if (m_IsServer) {
        m_IsClient = false;
    }

    return 0;
}
