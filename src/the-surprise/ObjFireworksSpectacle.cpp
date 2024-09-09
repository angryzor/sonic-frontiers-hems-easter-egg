#include "ObjFireworksSpectacle.h"
#include "SurpriseService.h"

using namespace hh::fnd;
using namespace hh::game;
using namespace hh::eff;

void* ObjFireworksSpectacleSpawner::Construct(void* pInstance, csl::fnd::IAllocator* pAllocator) {
    auto* self = static_cast<ObjFireworksSpectacleSpawner*>(pInstance);
    self->dummy = 0;
    return self;
}

void ObjFireworksSpectacleSpawner::Finish(void* pInstance) {}
void ObjFireworksSpectacleSpawner::Clean(void* pInstance) {}

const RflClassMember ObjFireworksSpectacleSpawner::rflClassMembers[1] = {
    { "dummy", nullptr, nullptr, RflClassMember::Type::TYPE_SINT32, RflClassMember::Type::TYPE_VOID, 0, 0, offsetof(ObjFireworksSpectacleSpawner, dummy), nullptr},
};

const RflTypeInfo ObjFireworksSpectacleSpawner::typeInfo = RflTypeInfo{ "ObjFireworksSpectacleSpawner", "ObjFireworksSpectacleSpawner", &ObjFireworksSpectacleSpawner::Construct, &ObjFireworksSpectacleSpawner::Finish, &ObjFireworksSpectacleSpawner::Clean, sizeof(ObjFireworksSpectacleSpawner) };
const RflClass ObjFireworksSpectacleSpawner::rflClass = RflClass{ "ObjFireworksSpectacleSpawner", nullptr, sizeof(ObjFireworksSpectacleSpawner), nullptr, 0, ObjFireworksSpectacleSpawner::rflClassMembers, sizeof(ObjFireworksSpectacleSpawner::rflClassMembers) / sizeof(RflClassMember), nullptr };

constexpr size_t postFxJobId = 0x698ca345ull;

constexpr float secondsPerMeasure = 1.5894039735099337748344370860927f;
constexpr float endOfMainSection = 177.01324503311258278145695364238f;
//constexpr float startFadeOut = 292.45033112582781456953642384106f;
//constexpr float fadeOutLength = 12.715231788079470198675496688742f;
constexpr float startFadeOut = 298.80794701986754966887417218543f;
constexpr float fadeOutLength = 6.3576158940397350993377483443709f;

void ObjFireworksSpectacle::Update(UpdatingPhase phase, const SUpdateInfo& updateInfo) {
    switch (phase) {
    case UpdatingPhase::PRE_ANIM:
        switch (state) {
        case State::IDLE:
            break;

        case State::PREPARATION:
            prepTime += updateInfo.deltaTime;

            if (auto* timeSrv = gameManager->GetService<app::game::TimeService>())
                timeSrv->currentTime.SetTimeOfDayInHours(startTime + prepTime * (22.0f - startTime) / 3.0f);

            if (prepTime >= 3.0f) {
                state = State::STARTED;

                auto* soundDirector = gameManager->GetService<app::snd::SoundDirector>();
                soundDirector->PlayBgm({ "bgm_lastboss", 0, 0.0f, 0.0f, 0.0f, 1.0f, 0, 0x10001, 0 });
            }
            break;

        case State::STARTED:
            if (auto* soundDirector = gameManager->GetService<app::snd::SoundDirector>())
            if (auto* surpriseService = gameManager->GetService<SurpriseService>()) {
                float playTime = soundDirector->GetBgmSoundHandle(0).GetPlayTime();

                if (inFirstSection && playTime > endOfMainSection) {
                    soundDirector->PlayBgm({ "bgm_lastboss", 2, 0.0f, 0.0f, 0.0f, 1.0f, 0, 0x10001, 0 });
                    inFirstSection = false;
                }

                if (playTime >= startFadeOut)
                    soundDirector->unkA0[0].unk101a.dwordC = 1.0f - (playTime - startFadeOut) / fadeOutLength;

                while (index < midiResource->midiFile.getEventCount(0)) {
                    auto& midiEvent = midiResource->midiFile.getEvent(0, index);

                    if (static_cast<float>(midiEvent.tick) > playTime * 151.0f * static_cast<float>(midiResource->midiFile.getTicksPerQuarterNote()) / 120.0f)
                        break;

                    if (midiEvent.track > NUM_BANKS) {
                        index++;
                        continue;
                    }

                    if (midiEvent.isNoteOn()) {
                        auto channel = static_cast<short>(midiEvent.track);

                        if (channel == 11) {
                            SpawnMeteor();
                            SpawnMeteor();
                        }
                        else {
                            SpectacleSignalId signalId{ channel, static_cast<short>(midiEvent.getKeyNumber() - 60) };

                            for (auto listener : surpriseService->GetSignalListeners(signalId)) {
                                MsgSpectacleMidiNoteOn msg{ signalId };
                                SendMessageImmToGameObject(listener, msg);
                            }
                        }
                    }
                    else if (midiEvent.isNoteOff()) {
                        SpectacleSignalId signalId{ static_cast<short>(midiEvent.track), static_cast<short>(midiEvent.getKeyNumber() - 60) };

                        for (auto listener : surpriseService->GetSignalListeners(signalId)) {
                            MsgSpectacleMidiNoteOff msg{ signalId };
                            SendMessageImmToGameObject(listener, msg);
                        }
                    }
                    else if (midiEvent.isController()) {
                        short controlId = midiEvent.getControllerNumber();

                        switch (controlId) {
                        //case 20:
                        //    if (auto* fxParamManager = hh::game::GameManager::GetInstance()->GetService<app::gfx::FxParamManager>()) {
                        //        hh::needle::FxCameraControlParameter param;
                        //        RESOLVE_STATIC_VARIABLE(hh::needle::FxCameraControlParameter::typeInfo).m_fpConstruct(&param, GetAllocator());
                        //        param.autoExposure.middleGray = static_cast<float>(midiEvent.getControllerValue()) / 127.0f;
                        //        fxParamManager->mutex.Lock();
                        //        fxParamManager->paramInterpolators.cameraControlInterpolator->UpdateJob(postFxJobId, &param);
                        //        fxParamManager->mutex.Unlock();
                        //    }
                        //    break;
                        default:
                            for (auto listener : surpriseService->GetControlListeners(controlId)) {
                                MsgSpectacleMidiControlChange msg{ controlId, static_cast<uint8_t>(midiEvent.getControllerValue()) };
                                SendMessageImmToGameObject(listener, msg);
                            }
                            break;
                        }
                    }

                    index++;
                }
            }

            nextMeteor -= updateInfo.deltaTime;
            if (nextMeteor <= 0.0f) {
                SpawnMeteor();

                nextMeteor = heur::RandomBetween(meteorShowerParam->GetData()->effect.spawnIntervalMin, meteorShowerParam->GetData()->effect.spawnIntervalMax);
            }

            if (index >= midiResource->midiFile.getEventCount(0))
                Kill();

            break;
        }

        break;
    }
}

bool ObjFireworksSpectacle::ProcessMessage(Message& message)
{
    switch (message.ID) {
    case MessageID::TRIGGER_ENTER: {
        auto* surpServ = gameManager->GetService<SurpriseService>();
        size_t numMembers{};

        if (surpServ->GetFoundMemberCount() >= 0) {
            GetComponent<hh::physics::GOCSphereCollider>()->SetEnabled(false);
            auto* eff = GetComponent<hh::eff::GOCEffect>();
            eff->StopEffectAll();
            eff->CreateEffect("ef_ob_guidecircle_out01", nullptr);
            StartSpectacle();
        }
        else {
            if (auto* overlayService = gameManager->GetService<app::ui::UIOverlayService>()) {
                auto* request = app::ui::RequestOverlayCaption::Create(GetAllocator());
                request->Setup("devtools_birthday_not_enough_members", nullptr, 2.0f);
                request->unk206 = 3;
                overlayService->QueueBeginRequest(request);
            }
        }

        return true;
    }
    default:
        return false;
    }
}

void ObjFireworksSpectacle::AddCallback(GameManager* gameManager) {
    auto* resourceMgr = ResourceManager::GetInstance();
    midiResource = resourceMgr->GetResource<ResMidiFile>("surprising_timings");
    meteorShowerParam = resourceMgr->GetResource<ResReflectionT<heur::rfl::MeteorShowerParameter>>("custom_meteorshower");

    auto* gocEffect = CreateComponent<GOCEffect>();
    gocEffect->Setup({ 0, 1, 1.0f, 0, -1, 0, 0 });
    AddComponent(gocEffect);

    auto* startCollider = CreateComponent<hh::physics::GOCSphereCollider>();
    hh::physics::GOCSphereCollider::SetupInfo colliderSetupInfo{};
    colliderSetupInfo.radius = 1.0f;
    colliderSetupInfo.unk3 |= 1;
    colliderSetupInfo.filterCategory = 25;
    colliderSetupInfo.unk4 = 0x8000;
    colliderSetupInfo.SetPosition({ 0.0f, 0.0f, 0.0f });
    startCollider->Setup(colliderSetupInfo);
    AddComponent(startCollider);

    gocEffect->CreateEffect("ef_ob_guidecircle_loop01", nullptr);
}

void ObjFireworksSpectacle::RemoveCallback(GameManager* gameManager) {
    if (state == State::STARTED)
        EndSpectacle();
}


void ObjFireworksSpectacle::StartSpectacle() {
    state = State::PREPARATION;
    inFirstSection = true;

    if (auto* resMgr = hh::fnd::ResourceManager::GetInstance()) {
        // Prefer hearing music over ObjKodamaFollower yapping.
        if (auto* islandParamResource = resMgr->GetResource<hh::fnd::ResReflectionT<heur::rfl::IslandParameter>>("island_param"))
        if (auto* islandParam = islandParamResource->GetData())
            islandParam->kodama.normal.follow.maxFollowNum = 0;
        
        // Don't dim music volume when watching.
        if (auto* soundCommonParamResource = resMgr->GetResource<hh::fnd::ResReflectionT<heur::rfl::SoundCommonParameter>>("sound_common_param"))
        if (auto* soundCommonParam = soundCommonParamResource->GetData())
            soundCommonParam->bgmVolumeInfo.playerStop.value = 1.0f;

        // End please shut up...
        if (auto* scriptParamResource = resMgr->GetResource<hh::fnd::ResReflectionT<heur::rfl::ScriptParameter>>("script"))
        if (auto* scriptParam = scriptParamResource->GetData())
            scriptParam->intervalMinuntes = 10000;
    }

    if (auto* monologueSrv = gameManager->GetService<app::game::MonologueService>()) {
        monologueSrv->monologueTimeout = 2000000.0f;
        monologueSrv->timeUntilNextMonologue = 2000000.0f;
    }

    if (auto* soundDirector = gameManager->GetService<app::snd::SoundDirector>()) {
        for (size_t i = 0; i < soundDirector->extensions.size(); i++) {
            auto* extension = soundDirector->extensions[i];

            if (extension->GetNameHash() == 0x8B8D888) {
                extension->UnkFunc3();
                soundDirector->extensions.remove(i);
                extension->GetAllocator()->Free(extension);
                break;
            }
        }
    }

    if (auto* weatherSrv = gameManager->GetService<app::game::WeatherService>())
        weatherSrv->SetWeather(app::game::WeatherService::WeatherType::UNK4, -1.0f);

    //if (auto* fxParamManager = hh::game::GameManager::GetInstance()->GetService<app::gfx::FxParamManager>()) {
    //    hh::needle::FxCameraControlParameter param;
    //    RESOLVE_STATIC_VARIABLE(hh::needle::FxCameraControlParameter::typeInfo).m_fpConstruct(&param, GetAllocator());
    //    param.autoExposure.middleGray = 0.5f;
    //    fxParamManager->mutex.Lock();
    //    fxParamManager->paramInterpolators.cameraControlInterpolator->AddJob(postFxJobId, &param, 0x2, 15000, 0.0f, -1);
    //    fxParamManager->mutex.Unlock();
    //}

    if (auto* timeSrv = gameManager->GetService<app::game::TimeService>())
        startTime = timeSrv->currentTime.GetTimeOfDayInHours();
}

void ObjFireworksSpectacle::EndSpectacle() {
    auto* soundDirector = gameManager->GetService<app::snd::SoundDirector>();
    soundDirector->PlayBgm({ "", 0, 0.0f, 0.0f, 0.0f, 1.0f, 0, 0x10001, 0 });
    soundDirector->unkA0[0].unk101a.dwordC = 1.0f;

    //if (auto* fxParamManager = hh::game::GameManager::GetInstance()->GetService<app::gfx::FxParamManager>()) {
    //    fxParamManager->mutex.Lock();
    //    fxParamManager->paramInterpolators.cameraControlInterpolator->ReverseJob(postFxJobId, 0.0f);
    //    fxParamManager->mutex.Unlock();
    //}
}

void ObjFireworksSpectacle::SpawnMeteor()
{
    auto& param = meteorShowerParam->GetData()->effect;
    app::MeteorShowerEffect::SetupInfo setupInfo{
        .moveTime = heur::RandomBetween(param.moveTimeMin, param.moveTimeMax),
        .fadeTime = heur::RandomBetween(param.fadeTimeMin, param.fadeTimeMax),
        .length = heur::RandomBetween(param.lengthMin, param.lengthMax),
        .width = param.width,
        .speed = heur::RandomBetween(param.speedMin, param.speedMax),
        .angle = heur::RandomBetween(param.angleMin, param.angleMax),
        .color = param.color,
    };

    auto inverseViewMat = hh::gfnd::GraphicsContext::GetInstance()->GetViewportData(0).inverseViewMatrix;

    Eigen::Vector3f inFront = (inverseViewMat.rotation() * Eigen::Vector3f{ 0.0f, 0.0f, -1.0f });
    inFront.y() = 0.0f;
    auto inFrontNormalized = inFront.normalized();

    auto spawnAngle = heur::RandomBetween(-param.spawnAngle, param.spawnAngle);
    auto spawnHorizon = heur::RandomBetween(param.spawnHorizonMin, param.spawnHorizonMax);
    auto spawnHeight = heur::RandomBetween(param.spawnHeightMin, param.spawnHeightMax);

    Eigen::Vector3f camPos = inverseViewMat.translation();

    auto spawnPos = camPos + Eigen::Vector3f{ 0.0f, spawnHeight, 0.0f } + spawnHorizon * (Eigen::AngleAxisf{ spawnAngle * 0.017453292f, Eigen::Vector3f::UnitY() } * inFront);

    WorldPosition worldPos{};
    worldPos.m_Position = spawnPos;

    auto* meteorShower = hh::game::GameObject::Create<app::MeteorShowerEffect>(GetAllocator());
    meteorShower->Setup(setupInfo);
    gameManager->AddGameObject(meteorShower, "HEMSMeteorShower", false, &worldPos, nullptr);
}

ObjFireworksSpectacle::ObjFireworksSpectacle(csl::fnd::IAllocator* allocator) : GameObject{ allocator } {
    SetLayer(0);
    SetUpdateFlag(UpdatingPhase::PRE_ANIM, true);
}

GameObject* ObjFireworksSpectacle::Create(csl::fnd::IAllocator* allocator) {
    return new (allocator) ObjFireworksSpectacle{ allocator };
}

const RflClassMember::Value ObjFireworksSpectacle::attributes[]{
    { "category", RflClassMember::Type::TYPE_CSTRING, "HEMS" },
};

const GameObjectClass ObjFireworksSpectacle::gameObjectClass{
    "FireworksSpectacle",
    "FireworksSpectacle",
    sizeof(ObjFireworksSpectacle),
    &ObjFireworksSpectacle::Create,
    1,
    ObjFireworksSpectacle::attributes,
    &ObjFireworksSpectacleSpawner::rflClass,
};

const GameObjectClass* ObjFireworksSpectacle::GetClass() {
    return &gameObjectClass;
}
