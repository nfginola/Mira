#pragma once
#include "../RenderResourceHandle.h"
#include "ResourceTypes.h"

namespace mira
{
	struct ResourceBarrier
	{
		enum class Type
		{
			Transition,
			Aliasing,
			UnorderedAccess,
		};

		enum class ResourceType
		{
			Buffer,
			Texture,
		};

		static ResourceBarrier uav_barrier(Texture resource) 
		{ 
			ResourceBarrier barr{};
			barr.info.resource_or_before = resource.handle;
			barr.info.type = Type::UnorderedAccess;
			barr.info.res_type = ResourceType::Texture;
			return barr;
		}
		
		static ResourceBarrier uav_barrier(Buffer resource)
		{ 
			ResourceBarrier barr{};
			barr.info.resource_or_before = resource.handle;
			barr.info.type = Type::UnorderedAccess;
			barr.info.res_type = ResourceType::Buffer;
			return barr;
		}

		static ResourceBarrier transition(Buffer resource, ResourceState before, ResourceState after)
		{ 
			ResourceBarrier barr{};
			barr.info.resource_or_before = resource.handle;
			barr.info.state_before = before;
			barr.info.state_after = after;
			barr.info.type = Type::Transition;
			barr.info.subresource = 0xffffffff;
			barr.info.res_type = ResourceType::Buffer;
			return barr; 
		}

		static ResourceBarrier transition(Texture resource, ResourceState before, ResourceState after, u32 api_subresource)
		{
			ResourceBarrier barr{};
			barr.info.resource_or_before = resource.handle;
			barr.info.state_before = before;
			barr.info.state_after = after;
			barr.info.type = Type::Transition;
			barr.info.subresource = api_subresource;
			barr.info.res_type = ResourceType::Texture;
			return barr;
		}

		struct BarrierInfo
		{
			Type type{ Type::Transition };					
			ResourceType res_type{ ResourceType::Buffer };	
	
			u64 resource_or_before;
			u64 after;
			ResourceState state_before, state_after;
			u32 subresource{ 0 };
		} info;
	};


}
