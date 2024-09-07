#include <the-surprise/TheSurprise.h>
#include <the-surprise/ObjFireworksSpectacle.h>
#include <the-surprise/ObjModScreenshot.h>

hh::fnd::Reference<hh::fnd::ResourceLoader> resourceLoader{};

constexpr size_t appResetAddr = 0x1501A41F0;
constexpr size_t appShutdownAddr = 0x150192E80;

HOOK(uint64_t, __fastcall, GameApplication_Reset, appResetAddr, hh::game::GameApplication* self) {
	auto res = originalGameApplication_Reset(self);

	resourceLoader = hh::fnd::ResourceLoader::Create(hh::fnd::MemoryRouter::GetModuleAllocator());
	resourceLoader->LoadPackfile("mods/angryzor_devtools/devtools.pac", 0);

	InitTheSurprise(*resourceLoader);

	return res;
}

HOOK(uint64_t, __fastcall, GameApplication_Shutdown, appShutdownAddr, hh::game::GameApplication* self) {
	resourceLoader = nullptr;
	return originalGameApplication_Shutdown(self);
}

HOOK(hh::fnd::ResourceTypeRegistry*, __fastcall, ResourceTypeRegistry_Create, 0x152EFD7F0) {
	auto * res = originalResourceTypeRegistry_Create();
	res->RegisterTypeInfo(ResMidiFile::GetTypeInfo());
	res->RegisterExtension("mid", ResMidiFile::GetTypeInfo());
	return res;
}

BOOL WINAPI DllMain(_In_ HINSTANCE hInstance, _In_ DWORD reason, _In_ LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		INSTALL_HOOK(GameApplication_Reset);
		INSTALL_HOOK(GameApplication_Shutdown);
		INSTALL_HOOK(ResourceTypeRegistry_Create);
		break;
	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}
