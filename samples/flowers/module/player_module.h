#ifndef FLOWER_TIME_MODULE_H_
#define FLOWER_TIME_MODULE_H_

#include <flower_model.h>
#include <octoon/raycaster.h>
#include <octoon/game_object.h>
#include <optional>

namespace flower
{
	class PlayerModule final : public FlowerModule
	{
	public:
		PlayerModule() noexcept;
		virtual ~PlayerModule() noexcept;

		virtual void reset() noexcept override;

		virtual void load(octoon::runtime::json& reader) noexcept override;
		virtual void save(octoon::runtime::json& reader) noexcept override;

	private:
		PlayerModule(const PlayerModule&) = delete;
		PlayerModule& operator=(const PlayerModule&) = delete;

	public:
		bool isPlaying;

		std::uint32_t spp;
		std::uint32_t sppCount;

		std::uint32_t endFrame;
		std::uint32_t startFrame;

		float playTimeStep;
		float previewTimeStep;
		float recordFps;

		float curTime;
		float takeupTime;
		float estimatedTime;
		float timeLength;

		std::optional<octoon::RaycastHit> dofTarget;
	};
}

#endif