#ifndef FLOWER_RESOURCE_MODULE_H_
#define FLOWER_RESOURCE_MODULE_H_

#include <flower_model.h>

namespace flower
{
	class ResourceModule final : public FlowerModule
	{
	public:
		ResourceModule() noexcept;
		virtual ~ResourceModule() noexcept;

		virtual void reset() noexcept override;

		virtual void load(octoon::runtime::json& reader) noexcept override;
		virtual void save(octoon::runtime::json& reader) noexcept override;

	private:
		ResourceModule(const ResourceModule&) = delete;
		ResourceModule& operator=(const ResourceModule&) = delete;

	public:
		std::string rootPath;
		std::string hdriPath;
		std::string materialPath;
	};
}

#endif