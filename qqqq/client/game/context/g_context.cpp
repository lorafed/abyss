#include "pch.hpp"
#include "g_context.hpp"
//
//void context_manager::update_context()
//{
//	minecraft_t minecraft;
//
//	if (!minecraft) {
//		return;
//	}
//
//	std::unique_lock<::std::shared_mutex> lock(g_mutex_);
//	g_ctx_.minecraft = std::move(minecraft);
//}
//
//g_context context_manager::get_context()
//{
//	std::shared_lock lock(g_mutex_);
//	return g_ctx_;
//}
//
//g_context context_manager::create_context(bool& state)
//{
//	g_context context;
//
//	minecraft_t minecraft;
//	
//	if (!minecraft) {
//		return context;
//	}
//
//	context.minecraft = std::move(minecraft);
//
//	return context;
//}
