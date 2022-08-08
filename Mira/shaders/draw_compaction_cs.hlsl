#include "ShaderInterop_Renderer.h"

struct PushConstant
{
    uint value;
};
ConstantBuffer<PushConstant> draw_compaction_target : register(b1, space0);
ConstantBuffer<PushConstant> target_command_stream : register(b2, space0);
ConstantBuffer<PushConstant> max_draws : register(b3, space0);

struct CompactionData
{
    uint num;
    
    uint pad0;
    uint pad1;
    uint pad2;
};


[numthreads(128, 1, 1)]
void main(uint3 globalId : SV_DispatchThreadID, uint3 threadId : SV_GroupThreadID)
{
    const uint draw_arg_id = globalId.x;
    const uint max_count = max_draws.value;
    
    RWStructuredBuffer<ShaderInterop_IndirectCommand> target_draws = ResourceDescriptorHeap[target_command_stream.value];
    RWStructuredBuffer<CompactionData> compactions = ResourceDescriptorHeap[draw_compaction_target.value];
    
    // Rest
    if (draw_arg_id == 0)
        compactions[0].num = 0;
    
    // Wait
    AllMemoryBarrierWithGroupSync();
   
    if (draw_arg_id >= max_count)
        return;
    
    // Load draw arg
    ShaderInterop_IndirectCommand draw_arg = target_draws[draw_arg_id];
    
    
    /*
        Strategy:
           
            1) Use Ballot to find surviving Draw calls within the Wave
            2) Mask with all lanes under this current lane 
            3) Count the 1s --> Local offset to place the Draw Argument
                Global offset is per wave
    */
    
    const uint lane_count = WaveGetLaneCount();
    const uint lane_idx = WaveGetLaneIndex();       // Current lane WITHIN this wave [0, lane_count]
    
    const bool lane_active = draw_arg.instance_count > 0;  
    
    // Sets bit = 1 if instance_count > 0 for each lane on this wave
    // Max 128-bits (128 lane waves)
    uint4 valid_ballot = WaveActiveBallot(lane_active);        // Get remaining active lanes (instance count > 0)
    
    // LSB is furthest right bit! 
    // valid_ballot has index 0 (lowest idx) at right-most (LSB)
    
    // Create thread mask (lane 0 up until this threads lane should have bit set 1)
    uint4 compact_mask = 0.rrrr;
    for (uint i = 0; i < lane_idx; ++i)
    {
        // uint is 32-bit 
        uint mask_idx = i / 32;     
        compact_mask[mask_idx] |= 1 << (i % 32);
    }
    
    uint local_slot = 0;     
    for (uint x = 0; x < 4; ++x)
        local_slot += countbits(valid_ballot[x] & compact_mask[x]);
    
    // Get total surviving for whole wavefromt
    uint wavefront_surviving = WaveActiveCountBits(lane_active);
     
    uint global_slot;       // wavefront offset
    if (WaveIsFirstLane())
    {
        InterlockedAdd(compactions[0].num, wavefront_surviving, global_slot);       // atomically reserve space for all survivors this wavefront
        // Final value is the number of draws we want to do! (counter to pass to ExecuteIndirect)
    }
    
    global_slot = WaveReadLaneFirst(global_slot);
    
    if (lane_active)
    {
        // Move draw
        target_draws[global_slot + local_slot] = draw_arg;
    }
    
    
    
    
}