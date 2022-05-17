#include "drag_component.h"
#include "flower_behaviour.h"
#include <octoon/mesh/cube_wireframe_mesh.h>

namespace flower
{
	DragComponent::DragComponent() noexcept
	{
	}

	DragComponent::~DragComponent() noexcept
	{
	}

	std::optional<octoon::RaycastHit>
	DragComponent::intersectObjects(float x, float y) noexcept
	{
		auto preofile = this->getContext()->profile;
		if (preofile->entitiesModule->camera)
		{
			auto cameraComponent = preofile->entitiesModule->camera->getComponent<octoon::CameraComponent>();
			if (cameraComponent)
			{
				octoon::Raycaster raycaster(cameraComponent->screenToRay(octoon::math::float2(x, y)));
				auto& intersects = raycaster.intersectObjects(preofile->entitiesModule->objects);
				if (!intersects.empty())
					return intersects[0];
			}
		}

		return std::nullopt;
	}

	void
	DragComponent::handleMouseDown(const octoon::input::InputEvent& event) noexcept
	{
		auto& model = this->getModel();
		auto& profile = this->getContext()->profile;

		auto selected = this->intersectObjects(event.button.x, event.button.y);
		if (model->selectedItem_ != selected)
		{
			model->selectedItem_ = selected;

			if (selected)
			{
				this->captureEvent();
				this->sendMessage("editor:selected", selected.value());
			}
			else
			{
				this->sendMessage("editor:selected");
			}
		}
	}

	void
	DragComponent::handleMouseMove(const octoon::input::InputEvent& event) noexcept
	{
	}

	void
	DragComponent::handleMouseHover(const octoon::input::InputEvent& event) noexcept
	{
		auto& model = this->getModel();
		auto& profile = this->getContext()->profile;

		auto hover = this->intersectObjects(event.motion.x, event.motion.y);
		if (model->selectedItemHover_ != hover)
		{
			model->selectedItemHover_ = hover;

			if (hover)
				this->sendMessage("editor:hover", hover.value());
			else
				this->sendMessage("editor:hover");
		}
	}

	void
	DragComponent::onEnable() noexcept
	{
		this->gizmoHoverMtl_ = std::make_shared<octoon::LineBasicMaterial>(octoon::math::float3(0, 1, 0));
		this->gizmoSelectedMtl_ = std::make_shared<octoon::LineBasicMaterial>(octoon::math::float3(0, 0, 1));

		gizmoHover_ = octoon::GameObject::create("GizmoHover");
		gizmoHover_->addComponent<octoon::MeshFilterComponent>(octoon::CubeWireframeMesh::create(1.0f, 1.0f, 1.0f));
		auto meshRenderHover = gizmoHover_->addComponent<octoon::MeshRendererComponent>(this->gizmoHoverMtl_);
		meshRenderHover->setVisible(false);
		meshRenderHover->setRenderOrder(1);

		gizmoSelected_ = octoon::GameObject::create("GizmoSelect");
		gizmoSelected_->addComponent<octoon::MeshFilterComponent>(octoon::CubeWireframeMesh::create(1.0f, 1.0f, 1.0f));
		auto meshRenderSelected = gizmoSelected_->addComponent<octoon::MeshRendererComponent>(this->gizmoSelectedMtl_);
		meshRenderSelected->setVisible(false);
		meshRenderSelected->setRenderOrder(1);
	}

	void
	DragComponent::onDisable() noexcept
	{
		this->gizmoHover_ = nullptr;
		this->gizmoSelected_ = nullptr;
	}

	void
	DragComponent::onMouseDown(const octoon::input::InputEvent& event) noexcept
	{
		if (event.button.button == octoon::input::InputButton::Left)
			this->handleMouseDown(event);
	}

	void
	DragComponent::onMouseUp(const octoon::input::InputEvent& event) noexcept
	{
		this->releaseEvent();
	}

	void
	DragComponent::onMouseMotion(const octoon::input::InputEvent& event) noexcept
	{
		auto inputFeature = this->getFeature<octoon::InputFeature>();
		if (inputFeature)
		{
			auto input = inputFeature->getInput();
			if (input)
			{
				if (this->isCapture())
					this->handleMouseMove(event);
				else if (!input->isButtonPressed(octoon::input::InputButton::Left))
					this->handleMouseHover(event);
			}
		}
	}

	void
	DragComponent::onUpdate() noexcept
	{
		auto& model = this->getModel();
		auto& profile = this->getContext()->profile;

		if (model->selectedItem_ && !profile->playerModule->isPlaying)
		{
			auto hit = model->selectedItem_.value();
			auto hitObject = hit.object.lock();
			if (hitObject)
			{
				octoon::MeshPtr mesh;
				auto skinnedMesh = hitObject->getComponent<octoon::SkinnedMeshRendererComponent>();
				if (skinnedMesh)
					mesh = skinnedMesh->getSkinnedMesh();
				else
				{
					auto meshFilter = hitObject->getComponent<octoon::MeshFilterComponent>();
					if (meshFilter)
						mesh = meshFilter->getMesh();
				}

				if (mesh)
				{
					auto& box = mesh->getBoundingBox(hit.mesh).box();

					auto gizmoTransform = this->gizmoSelected_->getComponent<octoon::TransformComponent>();
					gizmoTransform->setLocalScale(box.size());
					gizmoTransform->setLocalTranslate(hitObject->getComponent<octoon::TransformComponent>()->getTransform() * box.center());

					this->gizmoSelected_->getComponent<octoon::MeshRendererComponent>()->setVisible(true);
				}
			}
			else
			{
				model->selectedItem_.emplace();
				this->gizmoSelected_->getComponent<octoon::MeshRendererComponent>()->setVisible(false);
			}
		}
		else
		{
			gizmoSelected_->getComponent<octoon::MeshRendererComponent>()->setVisible(false);
		}

		if (model->selectedItemHover_ && model->selectedItem_ != model->selectedItemHover_ && !profile->playerModule->isPlaying)
		{
			auto hit = model->selectedItemHover_.value();
			auto hitObject = hit.object.lock();
			if (hitObject)
			{
				octoon::MeshPtr mesh;
				auto skinnedMesh = hit.object.lock()->getComponent<octoon::SkinnedMeshRendererComponent>();
				if (skinnedMesh)
					mesh = skinnedMesh->getSkinnedMesh();
				else
				{
					auto meshFilter = hit.object.lock()->getComponent<octoon::MeshFilterComponent>();
					if (meshFilter)
						mesh = meshFilter->getMesh();
				}

				if (mesh)
				{
					auto& box = mesh->getBoundingBox(hit.mesh).box();

					auto gizmoTransform = this->gizmoHover_->getComponent<octoon::TransformComponent>();
					gizmoTransform->setLocalScale(box.size());
					gizmoTransform->setLocalTranslate(hitObject->getComponent<octoon::TransformComponent>()->getTransform() * box.center());

					this->gizmoHover_->getComponent<octoon::MeshRendererComponent>()->setVisible(true);
				}
			}
			else
			{
				model->selectedItemHover_.emplace();
				this->gizmoHover_->getComponent<octoon::MeshRendererComponent>()->setVisible(false);
			}
		}
		else
		{
			gizmoHover_->getComponent<octoon::MeshRendererComponent>()->setVisible(false);
		}
	}
}