#ifndef FLOWER_PROFILE_H_
#define FLOWER_PROFILE_H_

#include <memory>
#include <string>

#include "module/encode_module.h"
#include "module/physics_module.h"
#include "module/player_module.h"
#include "module/file_module.h"
#include "module/entities_module.h"
#include "module/offline_module.h"
#include "module/record_module.h"
#include "module/mark_module.h"
#include "module/sun_module.h"
#include "module/environment_module.h"
#include "module/client_module.h"
#include "module/resource_module.h"
#include "module/selector_module.h"
#include "module/grid_module.h"

namespace flower
{
	class FlowerProfile
	{
	public:
		FlowerProfile() noexcept;
		FlowerProfile(std::string_view path) noexcept(false);
		virtual ~FlowerProfile() noexcept;

		static std::unique_ptr<FlowerProfile> load(std::string_view path) noexcept(false);
		static void save(std::string_view path, const FlowerProfile& profile) noexcept(false);

	private:
		FlowerProfile(const FlowerProfile&) = delete;
		FlowerProfile& operator=(const FlowerProfile&) = delete;

	public:
		std::shared_ptr<RecordModule> recordModule;
		std::shared_ptr<FileModule> fileModule;
		std::shared_ptr<EntitiesModule> entitiesModule;
		std::shared_ptr<EncodeModule> encodeModule;
		std::shared_ptr<PhysicsModule> physicsModule;
		std::shared_ptr<PlayerModule> playerModule;
		std::shared_ptr<OfflineModule> offlineModule;
		std::shared_ptr<MarkModule> markModule;
		std::shared_ptr<ClientModule> clientModule;
		std::shared_ptr<SunModule> sunModule;
		std::shared_ptr<EnvironmentModule> environmentModule;
		std::shared_ptr<ResourceModule> resourceModule;
		std::shared_ptr<SelectorModule> selectorModule;
		std::shared_ptr<GridModule> gridModule;
	};
}

#endif