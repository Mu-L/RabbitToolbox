#include "material_component.h"

#include <octoon/mdl_loader.h>
#include <octoon/PMREM_loader.h>
#include <octoon/texture_loader.h>
#include <octoon/video_feature.h>
#include <octoon/environment_light_component.h>

#include <filesystem>
#include <fstream>

#include <qimage.h>
#include <qstring.h>
#include <quuid.h>

#include "../flower_profile.h"
#include "../flower_behaviour.h"

namespace flower
{
	MaterialComponent::MaterialComponent() noexcept
		: previewWidth_(128)
		, previewHeight_(128)
	{
	}

	MaterialComponent::~MaterialComponent() noexcept
	{
	}

	nlohmann::json
	MaterialComponent::importMdl(std::string_view path) noexcept(false)
	{
		octoon::io::ifstream stream(QString::fromStdString(std::string(path)).toStdWString());
		if (stream)
		{
			octoon::MDLLoader loader;
			loader.load("resource", stream);

			nlohmann::json items;

			auto writeTexture = [](const octoon::hal::GraphicsTexturePtr& texture, std::filesystem::path rootPath) -> nlohmann::json
			{
				if (texture)
				{
					auto textureDesc = texture->getTextureDesc();
					auto width = textureDesc.getWidth();
					auto height = textureDesc.getHeight();
					auto textureFormat = textureDesc.getTexFormat();

					std::uint8_t* data_ = nullptr;

					if (texture->map(0, 0, width, height, 0, (void**)&data_))
					{
						auto format = QImage::Format::Format_RGB888;

						if (textureFormat == octoon::hal::GraphicsFormat::R8G8B8UNorm)
							format = QImage::Format::Format_RGB888;
						else if (textureFormat == octoon::hal::GraphicsFormat::R8G8B8A8UNorm)
							format = QImage::Format::Format_RGBA8888;

						QImage qimage(data_, width, height, format);

						texture->unmap();

						auto id = QUuid::createUuid().toString();
						auto uuid = id.toStdString().substr(1, id.length() - 2);
						auto outputPath = rootPath.append(uuid + ".png").string();

						qimage.save(QString::fromStdString(outputPath), "png");

						return outputPath;
					}
				}

				return nlohmann::json();
			};

			auto writePreview = [this](const std::shared_ptr<octoon::MeshStandardMaterial> material, std::filesystem::path outputPath) -> nlohmann::json
			{
				QPixmap pixmap;
				auto id = QUuid::createUuid().toString();
				auto uuid = id.toStdString().substr(1, id.length() - 2);
				auto previewPath = std::filesystem::path(outputPath).append(uuid + ".png");
				this->createMaterialPreview(material, pixmap, previewWidth_, previewHeight_);
				pixmap.save(QString::fromStdString(previewPath.string()), "png");
				return previewPath.string();
			};

			auto writeFloat2 = [](const octoon::math::float2& v)
			{
				return nlohmann::json({ v.x, v.y });
			};

			auto writeFloat3 = [](const octoon::math::float3& v)
			{
				return nlohmann::json({ v.x, v.y, v.z });
			};

			for (auto& mat : loader.getMaterials())
			{
				auto id = QUuid::createUuid().toString();
				auto uuid = id.toStdString().substr(1, id.length() - 2);
				auto outputPath = std::filesystem::path(this->getModel()->materialPath).append(uuid);

				std::filesystem::create_directory(this->getModel()->materialPath);
				std::filesystem::create_directory(outputPath);

				nlohmann::json item;
				item["uuid"] = uuid;
				item["name"] = mat->getName();
				item["preview"] = writePreview(mat, outputPath);
				item["colorMap"] = writeTexture(mat->getColorMap(), outputPath);
				item["opacityMap"] = writeTexture(mat->getOpacityMap(), outputPath);
				item["normalMap"] = writeTexture(mat->getNormalMap(), outputPath);
				item["roughnessMap"] = writeTexture(mat->getRoughnessMap(), outputPath);
				item["specularMap"] = writeTexture(mat->getSpecularMap(), outputPath);
				item["metalnessMap"] = writeTexture(mat->getMetalnessMap(), outputPath);
				item["emissiveMap"] = writeTexture(mat->getEmissiveMap(), outputPath);
				item["anisotropyMap"] = writeTexture(mat->getAnisotropyMap(), outputPath);
				item["clearCoatMap"] = writeTexture(mat->getClearCoatMap(), outputPath);
				item["clearCoatRoughnessMap"] = writeTexture(mat->getClearCoatRoughnessMap(), outputPath);
				item["subsurfaceMap"] = writeTexture(mat->getSubsurfaceMap(), outputPath);
				item["subsurfaceColorMap"] = writeTexture(mat->getSubsurfaceColorMap(), outputPath);
				item["sheenMap"] = writeTexture(mat->getSheenMap(), outputPath);
				item["lightMap"] = writeTexture(mat->getLightMap(), outputPath);
				item["emissiveIntensity"] = mat->getEmissiveIntensity();
				item["opacity"] = mat->getOpacity();
				item["smoothness"] = mat->getSmoothness();
				item["roughness"] = mat->getRoughness();
				item["metalness"] = mat->getMetalness();
				item["anisotropy"] = mat->getAnisotropy();
				item["sheen"] = mat->getSheen();
				item["specular"] = mat->getSpecular();
				item["refractionRatio"] = mat->getRefractionRatio();
				item["clearCoat"] = mat->getClearCoat();
				item["clearCoatRoughness"] = mat->getClearCoatRoughness();
				item["subsurface"] = mat->getSubsurface();
				item["reflectionRatio"] = mat->getReflectionRatio();
				item["transmission"] = mat->getTransmission();
				item["lightMapIntensity"] = mat->getLightMapIntensity();
				item["gamma"] = mat->getGamma();
				item["offset"] = writeFloat2(mat->getOffset());
				item["repeat"] = writeFloat2(mat->getRepeat());
				item["normalScale"] = writeFloat2(mat->getNormalScale());
				item["color"] = writeFloat3(mat->getColor());
				item["emissive"] = writeFloat3(mat->getEmissive());
				item["subsurfaceColor"] = writeFloat3(mat->getSubsurfaceColor());
				item["depthEnable"] = mat->getDepthEnable();
				item["depthBiasEnable"] = mat->getDepthBiasEnable();
				item["depthBoundsEnable"] = mat->getDepthBoundsEnable();
				item["depthClampEnable"] = mat->getDepthClampEnable();
				item["depthWriteEnable"] = mat->getDepthWriteEnable();
				item["stencilEnable"] = mat->getStencilEnable();
				item["scissorTestEnable"] = mat->getScissorTestEnable();
				item["depthMin"] = mat->getDepthMin();
				item["depthMax"] = mat->getDepthMax();
				item["depthBias"] = mat->getDepthBias();
				item["depthSlopeScaleBias"] = mat->getDepthSlopeScaleBias();

				std::ofstream ifs(std::filesystem::path(outputPath).append("package.json"), std::ios_base::binary);
				if (ifs)
				{
					auto dump = item.dump();
					ifs.write(dump.c_str(), dump.size());

					items.push_back(uuid);

					this->indexList_.push_back(uuid);
				}
			}

			this->save();
			this->sendMessage("editor:material:change");

			return items;
		}

		return nlohmann::json();
	}

	const nlohmann::json&
	MaterialComponent::getIndexList() const noexcept
	{
		return this->indexList_;
	}

	const nlohmann::json&
	MaterialComponent::getSceneList() const noexcept
	{
		return this->sceneList_;
	}

	nlohmann::json
	MaterialComponent::getPackage(std::string_view uuid) noexcept(false)
	{
		auto it = this->packageList_.find(std::string(uuid));
		if (it == this->packageList_.end())
		{
			std::ifstream ifs(std::filesystem::path(this->getModel()->materialPath).append(uuid).append("package.json"));
			if (ifs)
			{
				auto package = nlohmann::json::parse(ifs);
				this->packageList_[std::string(uuid)] = package;
				return package;
			}
			else
			{
				return nlohmann::json();
			}			
		}

		return this->packageList_[std::string(uuid)];
	}

	const std::shared_ptr<octoon::MeshStandardMaterial>
	MaterialComponent::getMaterial(std::string_view uuid) noexcept(false)
	{
		auto material = this->materials_.find(uuid);
		if (material == this->materials_.end())
		{
			auto package = this->getPackage(uuid);
			if (package.is_null())
				return nullptr;

			auto standard = std::make_shared<octoon::MeshStandardMaterial>();
			
			auto name = package.find("name");
			auto colorMap = package.find("colorMap");
			auto opacityMap = package.find("opacityMap");
			auto normalMap = package.find("normalMap");
			auto roughnessMap = package.find("roughnessMap");
			auto specularMap = package.find("specularMap");
			auto metalnessMap = package.find("metalnessMap");
			auto emissiveMap = package.find("emissiveMap");
			auto anisotropyMap = package.find("anisotropyMap");
			auto clearCoatMap = package.find("clearCoatMap");
			auto clearCoatRoughnessMap = package.find("clearCoatRoughnessMap");
			auto subsurfaceMap = package.find("subsurfaceMap");
			auto subsurfaceColorMap = package.find("subsurfaceColorMap");
			auto sheenMap = package.find("sheenMap");
			auto lightMap = package.find("lightMap");

			if (name != package.end() && (*name).is_string())
				standard->setName((*name).get<nlohmann::json::string_t>());			
			if (colorMap != package.end() && (*colorMap).is_string())
				standard->setColorMap(octoon::TextureLoader::load((*colorMap).get<nlohmann::json::string_t>()));			
			if (opacityMap != package.end() && (*opacityMap).is_string())
				standard->setOpacityMap(octoon::TextureLoader::load((*opacityMap).get<nlohmann::json::string_t>()));			
			if (normalMap != package.end() && (*normalMap).is_string())
				standard->setNormalMap(octoon::TextureLoader::load((*normalMap).get<nlohmann::json::string_t>()));			
			if (roughnessMap != package.end() && (*roughnessMap).is_string())
				standard->setRoughnessMap(octoon::TextureLoader::load((*roughnessMap).get<nlohmann::json::string_t>()));			
			if (specularMap != package.end() && (*specularMap).is_string())
				standard->setSpecularMap(octoon::TextureLoader::load((*specularMap).get<nlohmann::json::string_t>()));			
			if (metalnessMap != package.end() && (*metalnessMap).is_string())
				standard->setMetalnessMap(octoon::TextureLoader::load((*metalnessMap).get<nlohmann::json::string_t>()));			
			if (emissiveMap != package.end() && (*emissiveMap).is_string())
				standard->setEmissiveMap(octoon::TextureLoader::load((*emissiveMap).get<nlohmann::json::string_t>()));			
			if (anisotropyMap != package.end() && (*anisotropyMap).is_string())
				standard->setAnisotropyMap(octoon::TextureLoader::load((*anisotropyMap).get<nlohmann::json::string_t>()));			
			if (clearCoatMap != package.end() && (*clearCoatMap).is_string())
				standard->setClearCoatMap(octoon::TextureLoader::load((*clearCoatMap).get<nlohmann::json::string_t>()));			
			if (clearCoatRoughnessMap != package.end() && (*clearCoatRoughnessMap).is_string())
				standard->setClearCoatRoughnessMap(octoon::TextureLoader::load((*clearCoatRoughnessMap).get<nlohmann::json::string_t>()));			
			if (subsurfaceMap != package.end() && (*subsurfaceMap).is_string())
				standard->setSubsurfaceMap(octoon::TextureLoader::load((*subsurfaceMap).get<nlohmann::json::string_t>()));			
			if (subsurfaceColorMap != package.end() && (*subsurfaceColorMap).is_string())
				standard->setSubsurfaceColorMap(octoon::TextureLoader::load((*subsurfaceColorMap).get<nlohmann::json::string_t>()));			
			if (sheenMap != package.end() && (*sheenMap).is_string())
				standard->setSheenMap(octoon::TextureLoader::load((*sheenMap).get<nlohmann::json::string_t>()));			
			if (lightMap != package.end() && (*lightMap).is_string())
				standard->setLightMap(octoon::TextureLoader::load((*lightMap).get<nlohmann::json::string_t>()));

			auto depthEnable = package.find("depthEnable");
			auto depthBiasEnable = package.find("depthBiasEnable");
			auto depthBoundsEnable = package.find("depthBoundsEnable");
			auto depthClampEnable = package.find("depthClampEnable");
			auto depthWriteEnable = package.find("depthWriteEnable");
			auto stencilEnable = package.find("stencilEnable");
			auto scissorTestEnable = package.find("scissorTestEnable");

			if (depthEnable != package.end() && (*depthEnable).is_boolean())
				standard->setDepthEnable((*depthEnable).get<nlohmann::json::boolean_t>());
			if (depthBiasEnable != package.end() && (*depthBiasEnable).is_boolean())
				standard->setDepthBiasEnable((*depthBiasEnable).get<nlohmann::json::boolean_t>());
			if (depthBoundsEnable != package.end() && (*depthBoundsEnable).is_boolean())
				standard->setDepthBoundsEnable((*depthBoundsEnable).get<nlohmann::json::boolean_t>());
			if (depthClampEnable != package.end() && (*depthClampEnable).is_boolean())
				standard->setDepthClampEnable((*depthClampEnable).get<nlohmann::json::boolean_t>());
			if (depthWriteEnable != package.end() && (*depthWriteEnable).is_boolean())
				standard->setDepthWriteEnable((*depthWriteEnable).get<nlohmann::json::boolean_t>());
			if (stencilEnable != package.end() && (*stencilEnable).is_boolean())
				standard->setStencilEnable((*stencilEnable).get<nlohmann::json::boolean_t>());
			if (scissorTestEnable != package.end() && (*scissorTestEnable).is_boolean())
				standard->setScissorTestEnable((*scissorTestEnable).get<nlohmann::json::boolean_t>());

			auto emissiveIntensity = package.find("emissiveIntensity");
			auto opacity = package.find("opacity");
			auto smoothness = package.find("smoothness");
			auto roughness = package.find("roughness");
			auto metalness = package.find("metalness");
			auto anisotropy = package.find("anisotropy");
			auto sheen = package.find("sheen");
			auto specular = package.find("specular");
			auto refractionRatio = package.find("refractionRatio");
			auto clearCoat = package.find("clearCoat");
			auto clearCoatRoughness = package.find("clearCoatRoughness");
			auto subsurface = package.find("subsurface");
			auto reflectionRatio = package.find("reflectionRatio");
			auto transmission = package.find("transmission");
			auto lightMapIntensity = package.find("lightMapIntensity");
			auto gamma = package.find("gamma");
			auto depthMin = package.find("depthMin");
			auto depthMax = package.find("depthMax");
			auto depthBias = package.find("depthBias");
			auto depthSlopeScaleBias = package.find("depthSlopeScaleBias");

			if (emissiveIntensity != package.end() && (*emissiveIntensity).is_number_float())
				standard->setEmissiveIntensity((*emissiveIntensity).get<nlohmann::json::number_float_t>());
			if (opacity != package.end() && (*opacity).is_number_float())
				standard->setOpacity((*opacity).get<nlohmann::json::number_float_t>());
			if (smoothness != package.end() && (*smoothness).is_number_float())
				standard->setSmoothness((*smoothness).get<nlohmann::json::number_float_t>());
			if (roughness != package.end() && (*roughness).is_number_float())
				standard->setRoughness((*roughness).get<nlohmann::json::number_float_t>());
			if (metalness != package.end() && (*metalness).is_number_float())
				standard->setMetalness((*metalness).get<nlohmann::json::number_float_t>());
			if (anisotropy != package.end() && (*anisotropy).is_number_float())
				standard->setAnisotropy((*anisotropy).get<nlohmann::json::number_float_t>());
			if (sheen != package.end() && (*sheen).is_number_float())
				standard->setSheen((*sheen).get<nlohmann::json::number_float_t>());
			if (specular != package.end() && (*specular).is_number_float())
				standard->setSpecular((*specular).get<nlohmann::json::number_float_t>());
			if (refractionRatio != package.end() && (*refractionRatio).is_number_float())
				standard->setRefractionRatio((*refractionRatio).get<nlohmann::json::number_float_t>());
			if (clearCoat != package.end() && (*clearCoat).is_number_float())
				standard->setClearCoat((*clearCoat).get<nlohmann::json::number_float_t>());
			if (clearCoatRoughness != package.end() && (*clearCoatRoughness).is_number_float())
				standard->setClearCoatRoughness((*clearCoatRoughness).get<nlohmann::json::number_float_t>());
			if (subsurface != package.end() && (*subsurface).is_number_float())
				standard->setSubsurface((*subsurface).get<nlohmann::json::number_float_t>());
			if (reflectionRatio != package.end() && (*reflectionRatio).is_number_float())
				standard->setReflectionRatio((*reflectionRatio).get<nlohmann::json::number_float_t>());
			if (transmission != package.end() && (*transmission).is_number_float())
				standard->setTransmission((*transmission).get<nlohmann::json::number_float_t>());
			if (lightMapIntensity != package.end() && (*lightMapIntensity).is_number_float())
				standard->setLightMapIntensity((*lightMapIntensity).get<nlohmann::json::number_float_t>());
			if (gamma != package.end() && (*gamma).is_number_float())
				standard->setGamma((*gamma).get<nlohmann::json::number_float_t>());
			if (depthMin != package.end() && (*depthMin).is_number_float())
				standard->setDepthMin((*depthMin).get<nlohmann::json::number_float_t>());
			if (depthMax != package.end() && (*depthMax).is_number_float())
				standard->setDepthMax((*depthMax).get<nlohmann::json::number_float_t>());
			if (depthBias != package.end() && (*depthBias).is_number_float())
				standard->setDepthBias((*depthBias).get<nlohmann::json::number_float_t>());
			if (depthSlopeScaleBias != package.end() && (*depthSlopeScaleBias).is_number_float())
				standard->setDepthSlopeScaleBias((*depthSlopeScaleBias).get<nlohmann::json::number_float_t>());

			auto offset = package.find("offset");
			auto repeat = package.find("repeat");
			auto normalScale = package.find("normalScale");
			auto color = package.find("color");
			auto emissive = package.find("emissive");
			auto subsurfaceColor = package.find("subsurfaceColor");

			if (offset != package.end() && (*offset).is_array())
				standard->setOffset(octoon::math::float2((*offset)[0].get<nlohmann::json::number_float_t>(), (*offset)[1].get<nlohmann::json::number_float_t>()));
			if (repeat != package.end() && (*repeat).is_array())
				standard->setOffset(octoon::math::float2((*repeat)[0].get<nlohmann::json::number_float_t>(), (*repeat)[1].get<nlohmann::json::number_float_t>()));
			if (normalScale != package.end() && (*normalScale).is_array())
				standard->setOffset(octoon::math::float2((*normalScale)[0].get<nlohmann::json::number_float_t>(), (*normalScale)[1].get<nlohmann::json::number_float_t>()));
			if (color != package.end() && (*color).is_array())
				standard->setColor(octoon::math::float3((*color)[0].get<nlohmann::json::number_float_t>(), (*color)[1].get<nlohmann::json::number_float_t>(), (*color)[2].get<nlohmann::json::number_float_t>()));
			if (emissive != package.end() && (*emissive).is_array())
				standard->setEmissive(octoon::math::float3((*emissive)[0].get<nlohmann::json::number_float_t>(), (*emissive)[1].get<nlohmann::json::number_float_t>(), (*emissive)[2].get<nlohmann::json::number_float_t>()));
			if (subsurfaceColor != package.end() && (*subsurfaceColor).is_array())
				standard->setSubsurfaceColor(octoon::math::float3((*subsurfaceColor)[0].get<nlohmann::json::number_float_t>(), (*subsurfaceColor)[1].get<nlohmann::json::number_float_t>(), (*subsurfaceColor)[2].get<nlohmann::json::number_float_t>()));

			this->materials_[std::string(uuid)] = standard;

			return standard;
		}
		else
		{
			return (*material).second;
		}
	}

	bool
	MaterialComponent::addMaterial(const std::shared_ptr<octoon::Material>& mat)
	{
		if (this->materialSets_.find((void*)mat.get()) == this->materialSets_.end())
		{
			auto standard = mat->downcast_pointer<octoon::MeshStandardMaterial>();

			auto id = QUuid::createUuid().toString();
			auto uuid = id.toStdString().substr(1, id.length() - 2);

			auto& color = standard->getColor();
			auto& colorMap = standard->getColorMap();

			nlohmann::json item;
			item["uuid"] = uuid;
			item["name"] = mat->getName();
			item["color"] = { color.x, color.y, color.z };

			if (colorMap)
				item["colorMap"] = colorMap->getTextureDesc().getName();

			this->sceneList_ += uuid;
			this->packageList_[uuid] = item;

			this->materials_[uuid] = standard;
			this->materialSets_.insert((void*)mat.get());
			this->materialsRemap_[mat] = uuid;

			return true;
		}

		return false;
	}

	void
	MaterialComponent::createMaterialPreview(const std::shared_ptr<octoon::Material>& material, QPixmap& pixmap, int w, int h)
	{
		assert(material);

		if (scene_)
		{
			auto renderer = this->getFeature<octoon::VideoFeature>()->getRenderer();
			if (renderer)
			{
				geometry_->setMaterial(material);
				renderer->render(scene_);
				material->setDirty(true);
			}

			auto framebufferDesc = framebuffer_->getFramebufferDesc();
			auto width = framebufferDesc.getWidth();
			auto height = framebufferDesc.getHeight();

			auto colorTexture = framebufferDesc.getColorAttachment(0).getBindingTexture();

			std::uint8_t* data;
			if (colorTexture->map(0, 0, framebufferDesc.getWidth(), framebufferDesc.getHeight(), 0, (void**)&data))
			{
				QImage image(width, height, QImage::Format_RGB888);

				constexpr auto size = 16;

				for (std::uint32_t y = 0; y < height; y++)
				{
					for (std::uint32_t x = 0; x < width; x++)
					{
						auto n = (y * height + x) * 4;

						std::uint8_t u = x / size % 2;
						std::uint8_t v = y / size % 2;
						std::uint8_t bg = (u == 0 && v == 0 || u == v) ? 200u : 255u;

						auto r = octoon::math::lerp(bg, data[n], data[n + 3] / 255.f);
						auto g = octoon::math::lerp(bg, data[n + 1], data[n + 3] / 255.f);
						auto b = octoon::math::lerp(bg, data[n + 2], data[n + 3] / 255.f);

						image.setPixelColor((int)x, (int)y, QColor::fromRgb(r, g, b));
					}
				}

				colorTexture->unmap();

				pixmap.convertFromImage(image);
				pixmap = pixmap.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			}
		}
	}

	void
	MaterialComponent::initMaterialScene() noexcept(false)
	{
		auto renderer = this->getFeature<octoon::VideoFeature>()->getRenderer();
		if (renderer)
		{
			std::uint32_t width = previewWidth_ * 2;
			std::uint32_t height = previewHeight_ * 2;

			octoon::hal::GraphicsTextureDesc textureDesc;
			textureDesc.setSize(width, height);
			textureDesc.setTexDim(octoon::hal::TextureDimension::Texture2D);
			textureDesc.setTexFormat(octoon::hal::GraphicsFormat::R8G8B8A8UNorm);
			auto colorTexture = renderer->getScriptableRenderContext()->createTexture(textureDesc);
			if (!colorTexture)
				throw std::runtime_error("createTexture() failed");

			octoon::hal::GraphicsTextureDesc depthTextureDesc;
			depthTextureDesc.setSize(width, height);
			depthTextureDesc.setTexDim(octoon::hal::TextureDimension::Texture2D);
			depthTextureDesc.setTexFormat(octoon::hal::GraphicsFormat::D16UNorm);
			auto depthTexture = renderer->getScriptableRenderContext()->createTexture(depthTextureDesc);
			if (!depthTexture)
				throw std::runtime_error("createTexture() failed");

			octoon::hal::GraphicsFramebufferLayoutDesc framebufferLayoutDesc;
			framebufferLayoutDesc.addComponent(octoon::hal::GraphicsAttachmentLayout(0, octoon::hal::GraphicsImageLayout::ColorAttachmentOptimal, octoon::hal::GraphicsFormat::R8G8B8A8UNorm));
			framebufferLayoutDesc.addComponent(octoon::hal::GraphicsAttachmentLayout(1, octoon::hal::GraphicsImageLayout::DepthStencilAttachmentOptimal, octoon::hal::GraphicsFormat::D16UNorm));

			octoon::hal::GraphicsFramebufferDesc framebufferDesc;
			framebufferDesc.setWidth(width);
			framebufferDesc.setHeight(height);
			framebufferDesc.setFramebufferLayout(renderer->getScriptableRenderContext()->createFramebufferLayout(framebufferLayoutDesc));
			framebufferDesc.setDepthStencilAttachment(octoon::hal::GraphicsAttachmentBinding(depthTexture, 0, 0));
			framebufferDesc.addColorAttachment(octoon::hal::GraphicsAttachmentBinding(colorTexture, 0, 0));

			framebuffer_ = renderer->getScriptableRenderContext()->createFramebuffer(framebufferDesc);
			if (!framebuffer_)
				throw std::runtime_error("createFramebuffer() failed");

			camera_ = std::make_shared<octoon::PerspectiveCamera>(60, 1, 100);
			camera_->setClearColor(octoon::math::float4::Zero);
			camera_->setClearFlags(octoon::hal::ClearFlagBits::AllBit);
			camera_->setFramebuffer(framebuffer_);
			camera_->setTransform(octoon::math::makeLookatRH(octoon::math::float3(0, 0, 1), octoon::math::float3::Zero, octoon::math::float3::UnitY));

			geometry_ = std::make_shared<octoon::Geometry>();
			geometry_->setMesh(octoon::SphereMesh::create(0.5));

			octoon::math::Quaternion q1;
			q1.makeRotation(octoon::math::float3::UnitX, octoon::math::PI / 2.75);
			octoon::math::Quaternion q2;
			q2.makeRotation(octoon::math::float3::UnitY, octoon::math::PI / 4.6);

			directionalLight_ = std::make_shared<octoon::DirectionalLight>();
			directionalLight_->setColor(octoon::math::float3(1, 1, 1));
			directionalLight_->setTransform(octoon::math::float4x4(q1 * q2));

			environmentLight_ = std::make_shared<octoon::EnvironmentLight>();
			environmentLight_->setEnvironmentMap(octoon::PMREMLoader::load("../../system/hdri/Ditch-River_1k.hdr"));

			scene_ = std::make_unique<octoon::RenderScene>();
			scene_->addRenderObject(camera_.get());
			scene_->addRenderObject(directionalLight_.get());
			scene_->addRenderObject(environmentLight_.get());
			scene_->addRenderObject(geometry_.get());
		}
	}

	void
	MaterialComponent::initPackageIndices() noexcept(false)
	{
		std::ifstream indexStream(this->getModel()->materialPath + "/index.json");
		if (indexStream)
			this->indexList_ = nlohmann::json::parse(indexStream);

		bool needUpdateIndexFile = false;

		std::set<std::string> indexSet;

		for (auto& it : this->indexList_)
		{
			if (!std::filesystem::exists(std::filesystem::path(this->getModel()->materialPath).append(it.get<nlohmann::json::string_t>())))
				needUpdateIndexFile = true;
			else
				indexSet.insert(it.get<nlohmann::json::string_t>());
		}

		for (auto& it : std::filesystem::directory_iterator(this->getModel()->materialPath))
		{
			if (std::filesystem::is_directory(it))
			{
				auto filepath = it.path();
				auto filename = filepath.filename();

				auto index = indexSet.find(filename.string());
				if (index == indexSet.end())
				{
					if (std::filesystem::exists(filepath.append("package.json")))
					{
						needUpdateIndexFile = true;
						indexSet.insert(filename.string());
					}
				}
			}
		}

		if (needUpdateIndexFile)
		{
			nlohmann::json json;
			for (auto& it : indexSet)
				json += it;

			this->indexList_ = json;
			this->save();
		}
	}

	void
	MaterialComponent::save() const noexcept
	{
		try
		{
			if (!std::filesystem::exists(this->getModel()->materialPath))
				std::filesystem::create_directory(this->getModel()->materialPath);

			std::ofstream ifs(this->getModel()->materialPath + "/index.json", std::ios_base::binary);
			if (ifs)
			{
				auto data = indexList_.dump();
				ifs.write(data.c_str(), data.size());
			}
		}
		catch (...)
		{
		}
	}

	void
	MaterialComponent::onEnable() noexcept(false)
	{
		this->initMaterialScene();

		if (std::filesystem::exists(this->getModel()->materialPath))		
			this->initPackageIndices();
		else
			std::filesystem::create_directory(this->getModel()->materialPath);

		this->addMessageListener("editor:selected", [this](const std::any& data) {
			if (data.has_value())
			{
				auto hit = std::any_cast<octoon::RaycastHit>(data);
				if (!hit.object.lock())
					return;

				octoon::Materials materials;
				auto renderComponent = hit.object.lock()->getComponent<octoon::MeshRendererComponent>();
				if (renderComponent)
					materials = renderComponent->getMaterials();
				else
				{
					auto smr = hit.object.lock()->getComponent<octoon::SkinnedMeshRendererComponent>();
					if (smr)
						materials = smr->getMaterials();
				}

				bool dirty = false;

				for (auto& mat : materials)
				{
					dirty |= this->addMaterial(mat);
				}

				if (dirty)
					this->sendMessage("editor:material:change");

				auto selectedMaterial_ = this->materialsRemap_[materials[hit.mesh]];
				this->sendMessage("editor:material:selected", selectedMaterial_);
			}
		});
	}

	void
	MaterialComponent::onDisable() noexcept
	{
	}
}