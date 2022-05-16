#include "gl20_state.h"

namespace octoon
{
	namespace hal
	{
		OctoonImplementSubClass(GL20GraphicsState, GraphicsState, "GL20GraphicsState")

		GL20GraphicsState::GL20GraphicsState() noexcept
		{
		}

		GL20GraphicsState::~GL20GraphicsState() noexcept
		{
		}

		bool
		GL20GraphicsState::setup(const GraphicsStateDesc& desc) noexcept
		{
			_stateDesc = desc;
			return true;
		}

		void
		GL20GraphicsState::close() noexcept
		{
		}

		void
		GL20GraphicsState::apply(GraphicsStateDesc& lastStateDesc) noexcept
		{
			auto& srcBlends = _stateDesc.getColorBlends();
			auto& destBlends = lastStateDesc.getColorBlends();

			std::size_t srcBlendCount = srcBlends.size();
			std::size_t destBlendCount = destBlends.size();
			for (std::size_t i = srcBlendCount; i < destBlendCount; i++)
			{
				auto& destBlend = destBlends[i];
				if (destBlend.getBlendEnable())
				{
					glDisable(GL_BLEND);
					destBlend.setBlendEnable(false);
				}
			}

			for (std::size_t i = 0; i < srcBlendCount; i++)
			{
				auto& srcBlend = srcBlends[i];
				auto& destBlend = destBlends[i];

				if (srcBlends[i].getBlendEnable())
				{
					if (!destBlend.getBlendEnable())
					{
						glEnable(GL_BLEND);
						destBlend.setBlendEnable(true);
					}

					if (destBlend.getBlendSrc() != srcBlend.getBlendSrc() ||
						destBlend.getBlendDest() != srcBlend.getBlendDest() ||
						destBlend.getBlendAlphaSrc() != srcBlend.getBlendAlphaSrc() ||
						destBlend.getBlendAlphaDest() != srcBlend.getBlendAlphaDest())
					{
						GLenum sfactorRGB = GL20Types::asBlendFactor(srcBlend.getBlendSrc());
						GLenum dfactorRGB = GL20Types::asBlendFactor(srcBlend.getBlendDest());
						GLenum sfactorAlpha = GL20Types::asBlendFactor(srcBlend.getBlendAlphaSrc());
						GLenum dfactorAlpha = GL20Types::asBlendFactor(srcBlend.getBlendAlphaDest());

						glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);

						destBlend.setBlendSrc(srcBlend.getBlendSrc());
						destBlend.setBlendDest(srcBlend.getBlendDest());
						destBlend.setBlendAlphaSrc(srcBlend.getBlendAlphaSrc());
						destBlend.setBlendAlphaDest(srcBlend.getBlendAlphaDest());
					}

					if (destBlend.getBlendOp() != srcBlend.getBlendOp() ||
						destBlend.getBlendAlphaOp() != srcBlend.getBlendAlphaOp())
					{
						GLenum modeRGB = GL20Types::asBlendOperation(srcBlend.getBlendOp());
						GLenum modeAlpha = GL20Types::asBlendOperation(srcBlend.getBlendAlphaOp());

						glBlendEquationSeparate(modeRGB, modeAlpha);

						destBlend.setBlendOp(srcBlend.getBlendOp());
						destBlend.setBlendAlphaOp(srcBlend.getBlendAlphaOp());
					}
				}
				else
				{
					if (destBlend.getBlendEnable())
					{
						glDisable(GL_BLEND);
						destBlend.setBlendEnable(false);
					}
				}

				if (destBlend.getColorWriteMask() != srcBlend.getColorWriteMask())
				{
					auto flags = srcBlend.getColorWriteMask();

					GLboolean r = flags & ColorWriteMask::RedBit ? GL_TRUE : GL_FALSE;
					GLboolean g = flags & ColorWriteMask::GreendBit ? GL_TRUE : GL_FALSE;
					GLboolean b = flags & ColorWriteMask::BlurBit ? GL_TRUE : GL_FALSE;
					GLboolean a = flags & ColorWriteMask::AlphaBit ? GL_TRUE : GL_FALSE;

					glColorMask(r, g, b, a);

					destBlend.setColorWriteMask(srcBlend.getColorWriteMask());
				}
			}

			if (lastStateDesc.getCullMode() != _stateDesc.getCullMode())
			{
				if (_stateDesc.getCullMode() != CullMode::Off)
				{
					GLenum mode = GL20Types::asCullMode(_stateDesc.getCullMode());
					glEnable(GL_CULL_FACE);
					glCullFace(mode);

					lastStateDesc.setCullMode(_stateDesc.getCullMode());
				}
				else
				{
					glDisable(GL_CULL_FACE);
					lastStateDesc.setCullMode(CullMode::Off);
				}
			}

			if (lastStateDesc.getFrontFace() != _stateDesc.getFrontFace())
			{
				GLenum face = GL20Types::asFrontFace(_stateDesc.getFrontFace());
				glFrontFace(face);
			}

			if (lastStateDesc.getScissorTestEnable() != _stateDesc.getScissorTestEnable())
			{
				if (_stateDesc.getScissorTestEnable())
					glEnable(GL_SCISSOR_TEST);
				else
					glDisable(GL_SCISSOR_TEST);
				lastStateDesc.setScissorTestEnable(_stateDesc.getScissorTestEnable());
			}

			if (_stateDesc.getDepthEnable())
			{
				if (!lastStateDesc.getDepthEnable())
				{
					glEnable(GL_DEPTH_TEST);
					lastStateDesc.setDepthEnable(true);
				}

				if (lastStateDesc.getDepthFunc() != _stateDesc.getDepthFunc())
				{
					GLenum func = GL20Types::asCompareFunction(_stateDesc.getDepthFunc());
					glDepthFunc(func);
					lastStateDesc.setDepthFunc(_stateDesc.getDepthFunc());
				}
			}
			else
			{
				if (lastStateDesc.getDepthEnable())
				{
					glDisable(GL_DEPTH_TEST);
					lastStateDesc.setDepthEnable(false);
				}
			}

			if (lastStateDesc.getDepthWriteEnable() != _stateDesc.getDepthWriteEnable())
			{
				GLboolean enable = _stateDesc.getDepthWriteEnable() ? GL_TRUE : GL_FALSE;
				glDepthMask(enable);
				lastStateDesc.setDepthWriteEnable(_stateDesc.getDepthWriteEnable());
			}

			if (_stateDesc.getDepthBiasEnable())
			{
				if (!lastStateDesc.getDepthBiasEnable())
				{
					glEnable(GL_POLYGON_OFFSET_FILL);
					lastStateDesc.setDepthBiasEnable(true);
				}

				if (lastStateDesc.getDepthBias() != _stateDesc.getDepthBias() ||
					lastStateDesc.getDepthSlopeScaleBias() != _stateDesc.getDepthSlopeScaleBias())
				{
					glPolygonOffset(_stateDesc.getDepthSlopeScaleBias(), _stateDesc.getDepthBias());

					lastStateDesc.setDepthBias(_stateDesc.getDepthBias());
					lastStateDesc.setDepthSlopeScaleBias(_stateDesc.getDepthSlopeScaleBias());
				}
			}
			else
			{
				if (lastStateDesc.getDepthBiasEnable())
				{
					glDisable(GL_POLYGON_OFFSET_FILL);
					lastStateDesc.setDepthBiasEnable(false);
				}
			}

			if (_stateDesc.getStencilEnable())
			{
				if (!lastStateDesc.getStencilEnable())
				{
					glEnable(GL_STENCIL_TEST);
					lastStateDesc.setStencilEnable(true);
				}

				if (lastStateDesc.getStencilFrontFunc() != _stateDesc.getStencilFrontFunc() ||
					lastStateDesc.getStencilFrontRef() != _stateDesc.getStencilFrontRef() ||
					lastStateDesc.getStencilFrontReadMask() != _stateDesc.getStencilFrontReadMask())
				{
					GLenum frontfunc = GL20Types::asCompareFunction(_stateDesc.getStencilFrontFunc());
					glStencilFuncSeparate(GL_FRONT, frontfunc, _stateDesc.getStencilFrontRef(), _stateDesc.getStencilFrontReadMask());

					lastStateDesc.setStencilFrontFunc(_stateDesc.getStencilFrontFunc());
					lastStateDesc.setStencilFrontRef(_stateDesc.getStencilFrontRef());
					lastStateDesc.setStencilFrontReadMask(_stateDesc.getStencilFrontReadMask());
				}

				if (lastStateDesc.getStencilBackFunc() != _stateDesc.getStencilBackFunc() ||
					lastStateDesc.getStencilBackRef() != _stateDesc.getStencilBackRef() ||
					lastStateDesc.getStencilBackReadMask() != _stateDesc.getStencilBackReadMask())
				{
					GLenum backfunc = GL20Types::asCompareFunction(_stateDesc.getStencilBackFunc());
					glStencilFuncSeparate(GL_BACK, backfunc, _stateDesc.getStencilBackRef(), _stateDesc.getStencilBackReadMask());

					lastStateDesc.setStencilBackFunc(_stateDesc.getStencilBackFunc());
					lastStateDesc.setStencilBackRef(_stateDesc.getStencilBackRef());
					lastStateDesc.setStencilBackReadMask(_stateDesc.getStencilBackReadMask());
				}

				if (lastStateDesc.getStencilFrontFail() != _stateDesc.getStencilFrontFail() ||
					lastStateDesc.getStencilFrontZFail() != _stateDesc.getStencilFrontZFail() ||
					lastStateDesc.getStencilFrontPass() != _stateDesc.getStencilFrontPass())
				{
					GLenum frontfail = GL20Types::asStencilOperation(_stateDesc.getStencilFrontFail());
					GLenum frontzfail = GL20Types::asStencilOperation(_stateDesc.getStencilFrontZFail());
					GLenum frontpass = GL20Types::asStencilOperation(_stateDesc.getStencilFrontPass());

					glStencilOpSeparate(GL_FRONT, frontfail, frontzfail, frontpass);

					lastStateDesc.setStencilFrontFail(_stateDesc.getStencilFrontFail());
					lastStateDesc.setStencilFrontZFail(_stateDesc.getStencilFrontZFail());
					lastStateDesc.setStencilFrontPass(_stateDesc.getStencilFrontPass());
				}

				if (lastStateDesc.getStencilBackFail() != _stateDesc.getStencilBackFail() ||
					lastStateDesc.getStencilBackZFail() != _stateDesc.getStencilBackZFail() ||
					lastStateDesc.getStencilBackPass() != _stateDesc.getStencilBackPass())
				{
					GLenum backfail = GL20Types::asStencilOperation(_stateDesc.getStencilBackFail());
					GLenum backzfail = GL20Types::asStencilOperation(_stateDesc.getStencilBackZFail());
					GLenum backpass = GL20Types::asStencilOperation(_stateDesc.getStencilBackPass());
					glStencilOpSeparate(GL_BACK, backfail, backzfail, backpass);

					lastStateDesc.setStencilBackFail(_stateDesc.getStencilBackFail());
					lastStateDesc.setStencilBackZFail(_stateDesc.getStencilBackZFail());
					lastStateDesc.setStencilBackPass(_stateDesc.getStencilBackPass());
				}

				if (lastStateDesc.getStencilFrontWriteMask() != _stateDesc.getStencilFrontWriteMask())
				{
					glStencilMaskSeparate(GL_FRONT, _stateDesc.getStencilFrontWriteMask());
					lastStateDesc.setStencilFrontWriteMask(_stateDesc.getStencilFrontWriteMask());
				}

				if (lastStateDesc.getStencilBackWriteMask() != _stateDesc.getStencilBackWriteMask())
				{
					glStencilMaskSeparate(GL_BACK, _stateDesc.getStencilBackWriteMask());
					lastStateDesc.setStencilBackWriteMask(_stateDesc.getStencilBackWriteMask());
				}
			}
			else
			{
				if (lastStateDesc.getStencilEnable())
				{
					glDisable(GL_STENCIL_TEST);
					lastStateDesc.setStencilEnable(false);
				}
			}

			if (lastStateDesc.getLineWidth() != _stateDesc.getLineWidth())
			{
				glLineWidth(_stateDesc.getLineWidth());
				lastStateDesc.setLineWidth(_stateDesc.getLineWidth());
			}

			lastStateDesc.setPrimitiveType(_stateDesc.getPrimitiveType());
		}

		const GraphicsStateDesc&
		GL20GraphicsState::getStateDesc() const noexcept
		{
			return _stateDesc;
		}

		void
		GL20GraphicsState::setDevice(GraphicsDevicePtr device) noexcept
		{
			_device = device;
		}

		GraphicsDevicePtr
		GL20GraphicsState::getDevice() noexcept
		{
			return _device.lock();
		}
	}
}