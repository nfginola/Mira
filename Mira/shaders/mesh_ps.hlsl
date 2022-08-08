#include "ShaderInterop_Renderer.h"

struct VS_IN
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    uint instance_id : SV_InstanceID;
};

struct PushConstant
{
    uint value;
};
ConstantBuffer<PushConstant> push_constant : register(b0, space0);
ConstantBuffer<PushConstant> per_frame_constant : register(b2, space0);


SamplerState samp : register(s0, space1);


float4 main(VS_IN input) : SV_TARGET
{
    StructuredBuffer<ShaderInterop_SceneInstanceID> per_instance = ResourceDescriptorHeap[push_constant.value];
    ConstantBuffer<ShaderInterop_PerFrame> pf_data = ResourceDescriptorHeap[per_frame_constant.value];
  
    StructuredBuffer<ShaderInterop_PerInstance> scene_instances = ResourceDescriptorHeap[pf_data.scene_instance_table_id];
    
    // Get instance data
    ShaderInterop_PerInstance instance_data = scene_instances[per_instance[input.instance_id].object_id];
   
    // ... Lighting code
    
    Texture2D diffuse = ResourceDescriptorHeap[instance_data.material];
    float3 final_col = diffuse.Sample(samp, input.uv).rgb;
    
    return float4(final_col, 1.f);
}