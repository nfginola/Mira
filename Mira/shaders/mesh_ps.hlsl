#include "ShaderInterop_Renderer.h"

struct VS_IN
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    uint instance_id : SV_InstanceID;
};

struct PushConstants
{
    uint mesh_table_id;
    uint submesh_id;
    uint per_frame_id;
    uint per_draw_id;
    uint tex_id;
};

ConstantBuffer<PushConstants> push_constant : register(b0, space0);

SamplerState point_samp : register(s1, space1);


float4 main(VS_IN input) : SV_TARGET
{
    Texture2D tex = ResourceDescriptorHeap[push_constant.tex_id];
    return tex.Sample(point_samp, input.uv);
    
    return float4(normalize(input.normal), 1.f);
}