#ifndef RABBIT_DRAG_MODULE_H_
#define RABBIT_DRAG_MODULE_H_

#include <rabbit_model.h>
#include <optional>
#include <octoon/raycaster.h>

namespace rabbit
{
	class DragModule final : public RabbitModule
	{
	public:
		DragModule() noexcept;
		virtual ~DragModule() noexcept;

		virtual void reset() noexcept override;

		virtual void load(octoon::runtime::json& reader) noexcept override;
		virtual void save(octoon::runtime::json& reader) noexcept override;

	private:
		DragModule(const DragModule&) = delete;
		DragModule& operator=(const DragModule&) = delete;

	public:
		std::optional<octoon::RaycastHit> selectedItem_;
		std::optional<octoon::RaycastHit> selectedItemHover_;
	};
}

#endif