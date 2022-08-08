#include "ShaderInterop_Renderer.h"


struct PushConstant
{
    uint value;
};

// Set on Indirect Command
ConstantBuffer<PushConstant> scene_instances_id : register(b0, space0);
ConstantBuffer<PushConstant> scene_draws_per_instance : register(b1, space0);
ConstantBuffer<PushConstant> target_command_stream : register(b2, space0);

// Max pre-draws to handle
ConstantBuffer<PushConstant> max_pre_draws : register(b3, space0);

[numthreads(256, 1, 1)]
void main( uint3 globalId : SV_DispatchThreadID, uint3 threadId : SV_GroupThreadID)
{    
    const uint draw_arg_id = globalId.x;
    const uint max_count = max_pre_draws.value; 
    
    // Testing draw compaction
    //if (draw_arg_id > 20)
    //    return;
    
    // Access only within bounds
    if (draw_arg_id < max_count)
    {
        StructuredBuffer<ShaderInterop_PerInstance> scene_instance_data = ResourceDescriptorHeap[scene_instances_id.value]; // Read
        StructuredBuffer<ShaderInterop_PreCommand> pre_draws = ResourceDescriptorHeap[scene_draws_per_instance.value]; // Read
        RWStructuredBuffer<ShaderInterop_IndirectCommand> target_draws = ResourceDescriptorHeap[target_command_stream.value]; // Target
    
        // Each thread handles each pre-draw (per object draw)
        ShaderInterop_PreCommand cmd_to_handle = pre_draws[draw_arg_id];
    
        // Target draw to manipulate
        ShaderInterop_IndirectCommand target = target_draws[cmd_to_handle.batch_id];
    
        // Add instance
        uint local_instance_slot = 0;
        InterlockedAdd(target_draws[cmd_to_handle.batch_id].instance_count, 1, local_instance_slot);
            
        // Get target instance data
        RWStructuredBuffer<ShaderInterop_SceneInstanceID> target_instance_data = ResourceDescriptorHeap[target.root_constant + 1];
        target_instance_data[local_instance_slot].object_id = cmd_to_handle.object_id;
                
    }
}