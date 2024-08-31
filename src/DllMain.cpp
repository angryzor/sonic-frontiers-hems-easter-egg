#include <the-surprise/TheSurprise.h>
#include <the-surprise/ObjFireworksSpectacle.h>
#include <the-surprise/ObjModScreenshot.h>

hh::fnd::Reference<hh::fnd::ResourceLoader> resourceLoader{};

HOOK(hh::fnd::ResourceTypeRegistry*, __fastcall, ResourceTypeRegistry_Create, 0x152EFD7F0) {
	auto * res = originalResourceTypeRegistry_Create();
	res->RegisterTypeInfo(ResMidiFile::GetTypeInfo());
	res->RegisterExtension("mid", ResMidiFile::GetTypeInfo());
	return res;
}

HOOK(uint64_t, __fastcall, GameModeBootInit, 0x14734FB80, app::game::GameMode* self) {
	auto res = originalGameModeBootInit(self);

	resourceLoader = hh::fnd::ResourceLoader::Create(hh::fnd::MemoryRouter::GetModuleAllocator());
	resourceLoader->LoadPackfile("mods/angryzor_devtools/devtools.pac", 0);

	InitTheSurprise(*resourceLoader);

	return res;
}

BOOL WINAPI DllMain(_In_ HINSTANCE hInstance, _In_ DWORD reason, _In_ LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		INSTALL_HOOK(GameModeBootInit);
		INSTALL_HOOK(ResourceTypeRegistry_Create);
		break;
	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}
