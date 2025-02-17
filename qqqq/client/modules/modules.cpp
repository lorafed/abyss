#include "pch.hpp"
#include "modules.hpp"
#include "context.hpp"

#include "context/g_context.hpp"
#include "interop/java_interop.hpp"

modules* modules::get_instance()
{
    if (!instance_) {
        instance_ = new modules();
    }

    return instance_;
}

void modules::initialize()
{
	modules_.emplace_back(new left_clicker());
	modules_.emplace_back(new right_clicker());
	modules_.emplace_back(new full_bright());
	modules_.emplace_back(new sprint());
	modules_.emplace_back(new esp());
	
	for (auto& module_ : modules_)
	{
		module_->set_module_manager(modules::get_instance());
		module_->initialize();
	}
}

void modules::native_run()
{
	std::thread native_thread = std::thread([&]()
	{
		std::cout << "main thread started" << std::endl;

		java_interop_manager->attach_thread(true);

		do
		{
			JNIEnv* env = java_interop_manager->get_env();

			minecraft_t minecraft;

			if (!minecraft) {
				Sleep(1);
				continue;
			}

			entity_t player = minecraft.player();

			if (!player) {
				Sleep(1);
				continue;
			}

			for (auto& module_ : modules_)
			{
				java_local_frame_t frame;

				module_->run(env, minecraft, player);
			}

			Sleep(1);

		} while (!ctx.unload.load());

		java_interop_manager->detach_thread();
		std::cout << "main thread detach" << std::endl;
	});

	native_thread.detach();

	main_thread_ = std::move(native_thread);
}

void modules::destroy()
{
	for (auto& module_ : modules_)
	{
		module_->destroy();
		delete module_;
	}

	/*if (main_thread_.joinable()) {
		main_thread_.join();
	}*/
}

module* modules::module_by_name(const std::string& name)
{
	for (auto module_ : modules_) {
		if (module_->get_name() == name) {
			return module_;
		}
	}

	return nullptr;
}

void modules::disable_all()
{
	for (auto& module_ : modules_) {
		module_->disable();
	}
}

std::vector<::module*> modules::get_modules()
{
	return modules_;
}
