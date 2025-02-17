#include "pch.hpp"
#include "client.hpp"

#include "sdk.hpp"

#include "mapper/mapper.hpp"
#include "interop/java_interop.hpp"

#include "game_classes.hpp"
#include "render/render.hpp"

#include "modules.hpp"

unsigned long __stdcall qqqq::initialize(LPVOID module)
{
	windows::allocate_console();

	std::cout << "> entry" << std::endl;

	std::string window_name = windows::get_main_window_title();

	std::transform(window_name.begin(), window_name.end(), window_name.begin(), [](char c) -> char {
		return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		});

	if (std::strstr(window_name.data(), "badlion") != nullptr) {
		std::cout << "curr_version -> BADLION" << std::endl;
		ctx.curr_version = BADLION;
	}

	if (std::strstr(window_name.data(), "lunar") != nullptr) {
		std::cout << "curr_version -> LUNAR" << std::endl;
		ctx.curr_version = LUNAR;
	}

	if (ctx.curr_version == UNKNOWN) {
		MessageBoxA(0, "unsupported game version", "fatal error", MB_ICONERROR);
		exit(0);
	}

	mapper->load("C:\\Users\\chann\\Documents\\mappings\\1.21\\client.txt");

	java_interop_manager->initialize();
	java_interop_manager->dump_classes();

	std::cout << "jniEnv -> " << java_interop_manager->get_env() << std::endl;
	std::cout << "jvmtiEnv -> " << java_interop_manager->get_jvmti() << std::endl;

	std::cout << "mapper test -> " << mapper->get_class_mapping("net.minecraft.client.Minecraft") << std::endl;

	minecraft_t mc;

	std::cout << "minecraft -> " << mc.instance() << std::endl;

	render::initiaize();

	modules::initialize();
	modules::native_run();

	while (!ctx.unload) {
		Sleep(10);
	}

	ctx.unload.store(true);

	render::destroy();
	modules::destroy();

	java_interop_manager->destroy();

	std::cout << "> exit" << std::endl;

	/*while (!windows::is_key_down(VK_END)) {
		Sleep(10);
	}*/

	windows::deallocate_console();

	Beep(1000, 500);

	FreeLibraryAndExitThread((HMODULE)module, 0);
}
