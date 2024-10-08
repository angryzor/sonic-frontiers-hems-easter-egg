#include "TheSurprise.h"

#include <the-surprise/ResMidiFile.h>
#include <the-surprise/ObjFireworksSpectacle.h>
#include <the-surprise/ObjHEMSMember.h>
#include <the-surprise/spectacle-responders/ObjSpectacleEffectEmitter.h>
#include <the-surprise/spectacle-responders/ObjSpectacleObjectStatusController.h>
#include <the-surprise/spectacle-responders/ObjSpectacleLaserProjector.h>
#include <the-surprise/spectacle-responders/ObjModScreenshotSource.h>
#include <the-surprise/SurpriseService.h>

using namespace hh::fnd;

void InitTheSurprise(ResourceLoader& resourceLoader)
{
	BuiltinTypeRegistry::GetTypeInfoRegistry()->Register(&ObjSpectacleEffectEmitterSpawner::typeInfo);
	BuiltinTypeRegistry::GetClassNameRegistry()->Register(&ObjSpectacleEffectEmitterSpawner::rflClass);
	BuiltinTypeRegistry::GetTypeInfoRegistry()->Register(&ObjSpectacleObjectStatusControllerSpawner::typeInfo);
	BuiltinTypeRegistry::GetClassNameRegistry()->Register(&ObjSpectacleObjectStatusControllerSpawner::rflClass);
	BuiltinTypeRegistry::GetTypeInfoRegistry()->Register(&ObjSpectacleLaserProjectorSpawner::typeInfo);
	BuiltinTypeRegistry::GetClassNameRegistry()->Register(&ObjSpectacleLaserProjectorSpawner::rflClass);
	BuiltinTypeRegistry::GetTypeInfoRegistry()->Register(&ObjModScreenshotSourceSpawner::typeInfo);
	BuiltinTypeRegistry::GetClassNameRegistry()->Register(&ObjModScreenshotSourceSpawner::rflClass);
	BuiltinTypeRegistry::GetTypeInfoRegistry()->Register(&ObjHEMSMemberSpawner::typeInfo);
	BuiltinTypeRegistry::GetClassNameRegistry()->Register(&ObjHEMSMemberSpawner::rflClass);
	BuiltinTypeRegistry::GetTypeInfoRegistry()->Register(&ObjFireworksSpectacleSpawner::typeInfo);
	BuiltinTypeRegistry::GetClassNameRegistry()->Register(&ObjFireworksSpectacleSpawner::rflClass);

	const hh::game::GameObjectClass* goClass;
	goClass = ObjSpectacleEffectEmitter::GetClass();
	GameObjectSystem::GetInstance()->gameObjectRegistry->AddObject(&goClass);
	goClass = ObjSpectacleObjectStatusController::GetClass();
	GameObjectSystem::GetInstance()->gameObjectRegistry->AddObject(&goClass);
	goClass = ObjSpectacleLaserProjector::GetClass();
	GameObjectSystem::GetInstance()->gameObjectRegistry->AddObject(&goClass);
	goClass = ObjModScreenshotSource::GetClass();
	GameObjectSystem::GetInstance()->gameObjectRegistry->AddObject(&goClass);
	goClass = ObjHEMSMember::GetClass();
	GameObjectSystem::GetInstance()->gameObjectRegistry->AddObject(&goClass);
	goClass = ObjFireworksSpectacle::GetClass();
	GameObjectSystem::GetInstance()->gameObjectRegistry->AddObject(&goClass);

	ResourceLoader::Locale locale{};

	resourceLoader.LoadResource(InplaceTempUri{ "sound/the_end01_sound/bgm_lastboss" }, hh::snd::ResAtomCueSheet::GetTypeInfo(), 0, 1, locale);
	resourceLoader.LoadResource(InplaceTempUri{ "mods/hems_easter_egg/surprising_timings.mid" }, ResMidiFile::GetTypeInfo(), 0, 1, locale);
	resourceLoader.LoadResource(InplaceTempUri{ "mods/hems_easter_egg/surprising_objects.gedit" }, hh::game::ResObjectWorld::GetTypeInfo(), 0, 1, locale);
	resourceLoader.LoadResource(InplaceTempUri{ "mods/hems_easter_egg/surprising_pous.gedit" }, hh::game::ResObjectWorld::GetTypeInfo(), 0, 1, locale);
	resourceLoader.LoadResource(InplaceTempUri{ "mods/hems_easter_egg/surprising_text" }, hh::text::ResText::GetTypeInfo(), 0, 1, locale);
	resourceLoader.LoadResource(InplaceTempUri{ "mods/hems_easter_egg/custom_meteorshower" }, hh::fnd::ResReflection::GetTypeInfo(), 0, 1, locale);

	auto* gameManager = hh::game::GameManager::GetInstance();
	auto* s = gameManager->CreateService<SurpriseService>(hh::fnd::MemoryRouter::GetModuleAllocator());
	gameManager->RegisterService(s);
}
