#include "ShaderInterop_Renderer.h"
#include "ShaderInterop_Mesh.h"

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct PushConstantElement
{
    uint value;
    uint value2;
    uint value3;
    
    uint value4;
    uint value5;
    uint value6;
    
    uint value7;
    uint value8;
    uint value9;
    
};

ConstantBuffer<PushConstantElement> push_constant : register(b0, space0);

SamplerState point_samp : register(s1, space1);

float4 main(VS_OUT input) : SV_TARGET
{    
    //return float4(push_constant.value, 0.f, 0.f, 1.f);
    return float4(input.uv, 0.f, 1.f);
    
    //ConstantBuffer<ShaderInterop_Global_Datastructures> global_ds = ResourceDescriptorHeap[push_constant.value2];
    
    //// Get submesh mds
    //StructuredBuffer<ShaderInterop_SubmeshMD> submesh_mds = ResourceDescriptorHeap[global_ds.submesh_md_array];
    //ShaderInterop_SubmeshMD specific_submesh = submesh_mds[0];
    
    //return float4(global_ds.vert_uv_array, specific_submesh.index_count, 0.f, 1.f);
    
    
    //ConstantBuffer<TestCB> thing = ResourceDescriptorHeap[push_constant.value];
    //return float4(thing.a, thing.b, thing.c, 1.f);
    
    
    
    //return float4(push_constant.value.rr, 0.f, 1.f);
    
    //Texture2D tex = ResourceDescriptorHeap[push_constant.value];
    //float3 color = tex.Sample(point_samp, input.uv);
    
    // Gamma correction
    //color = pow(color, (1.f / 2.22f).rrr);
    
    //return float4(color, 1.f);
}