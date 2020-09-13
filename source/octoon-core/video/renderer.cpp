#include <octoon/video/renderer.h>
#include <octoon/video/render_scene.h>

#include <octoon/video/forward_material.h>
#include <octoon/video/forward_renderer.h>

#include <octoon/light/ambient_light.h>
#include <octoon/light/directional_light.h>
#include <octoon/light/point_light.h>
#include <octoon/light/spot_light.h>
#include <octoon/light/disk_light.h>
#include <octoon/light/rectangle_light.h>
#include <octoon/light/environment_light.h>
#include <octoon/light/tube_light.h>

#include <octoon/material/mesh_depth_material.h>
#include <octoon/material/mesh_standard_material.h>

#include <octoon/runtime/except.h>

#include "rtx_manager.h"

namespace octoon::video
{
	OctoonImplementSingleton(Renderer)

	Renderer::Renderer() noexcept
		: width_(0)
		, height_(0)
		, sortObjects_(true)
		, enableGlobalIllumination_(false)
	{
	}

	Renderer::~Renderer() noexcept
	{
		this->close();
	}

	void
	Renderer::setup(const hal::GraphicsContextPtr& context, std::uint32_t w, std::uint32_t h) except
	{
		context_ = context;
		depthMaterial_ = material::MeshDepthMaterial::create();
		forwardRenderer_ = std::make_unique<ForwardRenderer>(context);

		this->setFramebufferSize(w, h);
	}

	void
	Renderer::close() noexcept
	{
		this->profile_.reset();
		this->buffers_.clear();
		this->materials_.clear();
		this->rtxManager_.reset();
		currentBuffer_.reset();
		context_.reset();
	}

	void
	Renderer::setFramebufferSize(std::uint32_t w, std::uint32_t h) noexcept
	{
		if (width_ != w || height_ != h)
		{
			width_ = w;
			height_ = h;
			this->forwardRenderer_->setFramebufferSize(w, h);
		}
	}

	void
	Renderer::getFramebufferSize(std::uint32_t& w, std::uint32_t& h) const noexcept
	{
		w = width_;
		h = height_;
	}

	const hal::GraphicsFramebufferPtr&
	Renderer::getFramebuffer() const noexcept
	{
		assert(this->forwardRenderer_);

		if (this->enableGlobalIllumination_)
			return this->rtxManager_->getFramebuffer();
		else
			return this->forwardRenderer_->getFramebuffer();
	}

	void
	Renderer::setSortObjects(bool sortObject) noexcept
	{
		this->sortObjects_ = sortObject;
	}

	bool
	Renderer::getSortObject() const noexcept
	{
		return this->sortObjects_;
	}

	void
	Renderer::setOverrideMaterial(const std::shared_ptr<material::Material>& material) noexcept
	{
		this->overrideMaterial_ = material;
	}

	const std::shared_ptr<material::Material>&
	Renderer::getOverrideMaterial() const noexcept
	{
		return this->overrideMaterial_;
	}

	hal::GraphicsInputLayoutPtr
	Renderer::createInputLayout(const hal::GraphicsInputLayoutDesc& desc) noexcept
	{
		return this->context_->getDevice()->createInputLayout(desc);
	}

	hal::GraphicsDataPtr
	Renderer::createGraphicsData(const hal::GraphicsDataDesc& desc) noexcept
	{
		return this->context_->getDevice()->createGraphicsData(desc);
	}

	hal::GraphicsTexturePtr
	Renderer::createTexture(const hal::GraphicsTextureDesc& desc) noexcept
	{
		return this->context_->getDevice()->createTexture(desc);
	}

	hal::GraphicsSamplerPtr
	Renderer::createSampler(const hal::GraphicsSamplerDesc& desc) noexcept
	{
		return this->context_->getDevice()->createSampler(desc);
	}

	hal::GraphicsFramebufferPtr
	Renderer::createFramebuffer(const hal::GraphicsFramebufferDesc& desc) noexcept
	{
		return this->context_->getDevice()->createFramebuffer(desc);
	}

	hal::GraphicsFramebufferLayoutPtr
	Renderer::createFramebufferLayout(const hal::GraphicsFramebufferLayoutDesc& desc) noexcept
	{
		return this->context_->getDevice()->createFramebufferLayout(desc);
	}

	hal::GraphicsShaderPtr
	Renderer::createShader(const hal::GraphicsShaderDesc& desc) noexcept
	{
		return this->context_->getDevice()->createShader(desc);
	}

	hal::GraphicsProgramPtr
	Renderer::createProgram(const hal::GraphicsProgramDesc& desc) noexcept
	{
		return this->context_->getDevice()->createProgram(desc);
	}

	hal::GraphicsStatePtr
	Renderer::createRenderState(const hal::GraphicsStateDesc& desc) noexcept
	{
		return this->context_->getDevice()->createRenderState(desc);
	}

	hal::GraphicsPipelinePtr
	Renderer::createRenderPipeline(const hal::GraphicsPipelineDesc& desc) noexcept
	{
		return this->context_->getDevice()->createRenderPipeline(desc);
	}

	hal::GraphicsDescriptorSetPtr
	Renderer::createDescriptorSet(const hal::GraphicsDescriptorSetDesc& desc) noexcept
	{
		return this->context_->getDevice()->createDescriptorSet(desc);
	}

	hal::GraphicsDescriptorSetLayoutPtr
	Renderer::createDescriptorSetLayout(const hal::GraphicsDescriptorSetLayoutDesc& desc) noexcept
	{
		return this->context_->getDevice()->createDescriptorSetLayout(desc);
	}

	hal::GraphicsDescriptorPoolPtr
	Renderer::createDescriptorPool(const hal::GraphicsDescriptorPoolDesc& desc) noexcept
	{
		return this->context_->getDevice()->createDescriptorPool(desc);
	}

	void
	Renderer::setViewport(const math::float4& viewport) noexcept(false)
	{
		this->context_->setViewport(0, viewport);
	}

	void
	Renderer::setFramebuffer(const hal::GraphicsFramebufferPtr& framebuffer) noexcept(false)
	{
		this->context_->setFramebuffer(framebuffer);
	}

	void
	Renderer::clearFramebuffer(std::uint32_t i, hal::GraphicsClearFlags flags, const math::float4& color, float depth, std::int32_t stencil) noexcept
	{
		this->context_->clearFramebuffer(i, flags, color, depth, stencil);
	}

	void
	Renderer::generateMipmap(const hal::GraphicsTexturePtr& texture) noexcept
	{
		this->context_->generateMipmap(texture);
	}

	void
	Renderer::renderObject(const geometry::Geometry& geometry, const camera::Camera& camera, const std::shared_ptr<material::Material>& overrideMaterial) noexcept
	{
		if (camera.getLayer() != geometry.getLayer())
			return;

		if (geometry.getVisible())
		{
			for (std::size_t i = 0; i < geometry.getMaterials().size(); i++)
			{
				if (!this->setProgram(overrideMaterial ? overrideMaterial : geometry.getMaterials()[i], camera, geometry))
					return;

				if (!this->setBuffer(geometry.getMesh(), i))
					return;

				auto indices = currentBuffer_->getNumIndices(i);
				if (indices > 0)
					this->context_->drawIndexed((std::uint32_t)indices, 1, 0, 0, 0);
				else
					this->context_->draw((std::uint32_t)currentBuffer_->getNumVertices(), 1, 0, 0);
			}
		}
	}

	void
	Renderer::renderObjects(const std::vector<geometry::Geometry*>& geometries, const camera::Camera& camera, const std::shared_ptr<material::Material>& overrideMaterial) noexcept
	{
		for (auto& geometry : geometries)
			this->renderObject(*geometry, camera, overrideMaterial);
	}

	void
	Renderer::prepareShadowMaps(const std::vector<light::Light*>& lights, const std::vector<geometry::Geometry*>& geometries) noexcept
	{
		for (auto& light : lights)
		{
			if (!light->getVisible())
				continue;

			std::uint32_t faceCount = 0;
			std::shared_ptr<camera::Camera> camera;

			if (light->isA<light::DirectionalLight>())
			{
				auto directionalLight = light->cast<light::DirectionalLight>();
				if (directionalLight->getShadowEnable())
				{
					faceCount = 1;
					camera = directionalLight->getCamera();
				}
			}
			else if (light->isA<light::SpotLight>())
			{
				auto spotLight = light->cast<light::SpotLight>();
				if (spotLight->getShadowEnable())
				{
					faceCount = 1;
					camera = spotLight->getCamera();
				}
			}
			else if (light->isA<light::PointLight>())
			{
				auto pointLight = light->cast<light::PointLight>();
				if (pointLight->getShadowEnable())
				{
					faceCount = 6;
					camera = pointLight->getCamera();
				}
			}

			for (std::uint32_t face = 0; face < faceCount; face++)
			{
				auto framebuffer = camera->getFramebuffer();

				this->context_->setFramebuffer(framebuffer);
				this->context_->clearFramebuffer(0, camera->getClearFlags(), camera->getClearColor(), 1.0f, 0);
				this->context_->setViewport(0, camera->getPixelViewport());

				for (auto& geometry : geometries)
				{
					if (geometry->getMaterial()->getPrimitiveType() == this->depthMaterial_->getPrimitiveType())
						this->renderObject(*geometry, *camera, this->depthMaterial_);
				}

				if (camera->getRenderToScreen())
				{
					auto& v = camera->getPixelViewport();
					this->context_->blitFramebuffer(framebuffer, v, nullptr, v);
				}

				auto& swapFramebuffer = camera->getSwapFramebuffer();
				if (swapFramebuffer && framebuffer)
				{
					math::float4 v1(0, 0, (float)framebuffer->getFramebufferDesc().getWidth(), (float)framebuffer->getFramebufferDesc().getHeight());
					math::float4 v2(0, 0, (float)swapFramebuffer->getFramebufferDesc().getWidth(), (float)swapFramebuffer->getFramebufferDesc().getHeight());
					this->context_->blitFramebuffer(framebuffer, v1, swapFramebuffer, v2);
				}
			}
		}
	}

	void
	Renderer::render(RenderScene& scene) noexcept
	{
		if (this->sortObjects_)
		{
			scene.sortCameras();
			scene.sortGeometries();
		}

		this->prepareShadowMaps(scene.getLights(), scene.getGeometries());

		for (auto& camera : scene.getCameras())
		{
			scene.setMainCamera(camera);

			camera->onRenderBefore(*camera);

			for (auto& it : scene.getGeometries())
			{
				if (camera->getLayer() != it->getLayer())
					continue;

				if (it->getVisible())
					it->onRenderBefore(*camera);
			}

			if (this->sortObjects_)
				scene.sortGeometries();

			if (scene.getGlobalIllumination())
			{
				if (!rtxManager_)
				{
					rtxManager_ = std::make_unique<RtxManager>();
					rtxManager_->setGraphicsContext(this->context_);
				}

				this->rtxManager_->render(&scene);
			}
			else
			{
				forwardRenderer_->render(&scene);
			}

			for (auto& it : scene.getLights())
			{
				if (camera->getLayer() != it->getLayer())
					continue;

				it->setDirty(false);
			}

			for (auto& it : scene.getGeometries())
			{
				if (camera->getLayer() != it->getLayer())
					continue;

				if (it->getVisible())
					it->onRenderAfter(*camera);

				auto mesh = it->getMesh();
				if (mesh)
					mesh->setDirty(false);

				for (auto& material : it->getMaterials())
				{
					if (material)
						material->setDirty(false);
				}

				it->setDirty(false);
			}

			camera->onRenderAfter(*camera);
			camera->setDirty(false);
		}
	}

	bool
	Renderer::setBuffer(const std::shared_ptr<mesh::Mesh>& mesh, std::size_t subset)
	{
		if (mesh)
		{
			auto& buffer = buffers_[((std::intptr_t)mesh.get())];
			if (!buffer)
				buffer = std::make_shared<ForwardBuffer>(mesh);

			this->context_->setVertexBufferData(0, buffer->getVertexBuffer(), 0);
			this->context_->setIndexBufferData(buffer->getIndexBuffer(subset), 0, hal::GraphicsIndexType::UInt32);

			this->currentBuffer_ = buffer;

			return true;
		}
		else
		{
			return false;
		}
	}

	bool
	Renderer::setProgram(const std::shared_ptr<material::Material>& material, const camera::Camera& camera, const geometry::Geometry& geometry)
	{
		if (material)
		{
			auto& pipeline = materials_[((std::intptr_t)material.get())];
			if (!pipeline)
				pipeline = std::make_shared<ForwardMaterial>(material, this->profile_);

			pipeline->update(camera, geometry, this->profile_);

			this->context_->setRenderPipeline(pipeline->getPipeline());
			this->context_->setDescriptorSet(pipeline->getDescriptorSet());

			return true;
		}
		else
		{
			return false;
		}
	}
}