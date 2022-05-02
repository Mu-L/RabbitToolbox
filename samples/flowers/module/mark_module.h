#ifndef RABBIT_MARK_MODULE_H_
#define RABBIT_MARK_MODULE_H_

#include <rabbit_model.h>

namespace rabbit
{
	class MarkModule final : public RabbitModule
	{
	public:
		MarkModule() noexcept;
		virtual ~MarkModule() noexcept;

		virtual void reset() noexcept override;

		virtual void load(octoon::runtime::json& reader) noexcept override;
		virtual void save(octoon::runtime::json& reader) noexcept override;

	private:
		MarkModule(const MarkModule&) = delete;
		MarkModule& operator=(const MarkModule&) = delete;

	public:
		bool markEnable;

		std::uint32_t x;
		std::uint32_t y;
		std::uint32_t width;
		std::uint32_t height;
		std::uint32_t channel;
		std::vector<std::uint8_t> pixels;
	};
}

#endif