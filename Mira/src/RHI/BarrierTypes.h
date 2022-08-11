#pragma once
#include "RenderResourceHandle.h"
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
			None,
		};

		enum class ResourceType
		{
			Buffer,
			Texture,
			None
		};

		ResourceBarrier& uav_barrier(Texture resource) { barrier.resource_or_before = resource.handle; barrier.type = Type::UnorderedAccess; barrier.res_type = ResourceType::Texture; return *this; }
		ResourceBarrier& uav_barrier(Buffer resource) { barrier.resource_or_before = resource.handle; barrier.type = Type::UnorderedAccess; barrier.res_type = ResourceType::Buffer; return *this; }

		ResourceBarrier& aliasing(Texture before, Texture after) 
		{ 
			barrier.resource_or_before = before.handle; 
			barrier.after = after.handle;
			barrier.type = Type::Aliasing; 
			barrier.res_type = ResourceType::Texture;
			return *this;  
		}
		ResourceBarrier& transition(Buffer resource, ResourceState before, ResourceState after) 
		{ 
			barrier.resource_or_before = resource.handle; 
			barrier.state_before = before; 
			barrier.state_after = after; 
			barrier.type = Type::Transition; 
			barrier.subresource = 0xffffffff;
			barrier.res_type = ResourceType::Buffer; 
			return *this; 
		}
		ResourceBarrier& transition(Texture resource, ResourceState before, ResourceState after, u32 subresource)
		{
			barrier.resource_or_before = resource.handle;
			barrier.state_before = before;
			barrier.state_after = after;
			barrier.type = Type::Transition;
			barrier.subresource = subresource;
			barrier.res_type = ResourceType::Texture;
			return *this;
		}

		struct BarrierInfo
		{
			Type type{ Type::None };					
			ResourceType res_type{ ResourceType::None };	
	
			u64 resource_or_before;
			u64 after;
			ResourceState state_before, state_after;
			u32 subresource{ 0 };
		} barrier;
	};

}
