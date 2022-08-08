#pragma once
#include "../Common.h"
#include "RenderResourceHandle.h"

namespace mira
{
	enum class RenderPassBeginAccessType
	{
		Discard,
		Preserve,
		Clear
	};

	enum class RenderPassEndingAccessType
	{
		Discard,
		Preserve,
		Resolve
	};

	struct RenderPassTargetDesc
	{
		Texture texture;
		u32 subresource{ 0 };
		RenderPassBeginAccessType begin_access;
		RenderPassEndingAccessType end_access;

		// Resolve not supported yet
	};

	struct RenderPassDepthStencilTargetDesc
	{
		Texture texture;
		u32 subresource{ 0 };
		RenderPassBeginAccessType begin_depth_access;
		RenderPassEndingAccessType end_depth_access;
		RenderPassBeginAccessType begin_stencil_access;
		RenderPassEndingAccessType end_stencil_access;
	};

	enum class RenderPassFlag : u8
	{
		None,
		AllowUnorderedAccessWrites,
	};

	struct RenderPassDesc
	{
		std::vector<RenderPassTargetDesc> render_target_descs{};
		std::optional<RenderPassDepthStencilTargetDesc> depth_stencil_desc{};				
		RenderPassFlag flags{ RenderPassFlag::None };
	};

	class RenderPassBuilder
	{
	public:

		RenderPassBuilder& append_rt(
			Texture texture,
			u32 subresource,
			RenderPassBeginAccessType begin_access, 
			RenderPassEndingAccessType ending_access)
		{
			RenderPassTargetDesc desc{};
			desc.texture = texture;
			desc.begin_access = begin_access;
			desc.end_access = ending_access;
			desc.subresource = subresource;
			m_rp_desc.render_target_descs.push_back(desc);
			return *this;
		}

		// Conveniece API if the user only cares about using depth
		RenderPassBuilder& add_depth(
			Texture texture,
			u32 subresource,
			RenderPassBeginAccessType depth_begin,
			RenderPassEndingAccessType depth_end
		)
		{
			add_depth_stencil(texture, subresource, depth_begin, depth_end, RenderPassBeginAccessType::Discard, RenderPassEndingAccessType::Discard);
			return *this;
		}

		RenderPassBuilder& add_depth_stencil(
			Texture texture,
			u32 subresource,
			RenderPassBeginAccessType depth_begin,
			RenderPassEndingAccessType depth_end,
			RenderPassBeginAccessType stencil_begin,
			RenderPassEndingAccessType stencil_end)
		{
			m_rp_desc.depth_stencil_desc = RenderPassDepthStencilTargetDesc{};
			m_rp_desc.depth_stencil_desc->texture = texture;
			m_rp_desc.depth_stencil_desc->subresource = subresource;
			m_rp_desc.depth_stencil_desc->begin_depth_access = depth_begin;
			m_rp_desc.depth_stencil_desc->end_depth_access = depth_end;
			m_rp_desc.depth_stencil_desc->begin_stencil_access = stencil_begin;
			m_rp_desc.depth_stencil_desc->end_stencil_access = stencil_end;

			return *this;
		}

		const RenderPassDesc& build() { return m_rp_desc; }

	private:
		RenderPassDesc m_rp_desc{};
	};

	inline RenderPassFlag operator|(RenderPassFlag a, RenderPassFlag b)
	{
		return static_cast<RenderPassFlag>(static_cast<u8>(a) | static_cast<u8>(b));
	}

	inline u16 operator&(RenderPassFlag a, RenderPassFlag b)
	{
		return static_cast<u8>(a) & static_cast<u8>(b);
	}
}
