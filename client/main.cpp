#include "pch.hpp"
#include "sdk.hpp"

#include "context.hpp"
#include "client/client.hpp"

context ctx;

int __stdcall DllMain(void* module, unsigned long reason, void* reserved)
{
	if (reason != DLL_PROCESS_ATTACH) {
		return 1;
	}

	void* thread = CreateThread(0, 0, qqqq::initialize, module, 0, 0);

	CloseHandle(thread);

	DisableThreadLibraryCalls(static_cast<HMODULE>(module));

	return 1;
}
