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

float4 main(VS_IN input) : SV_TARGET
{
    return float4(normalize(input.normal), 1.f);
}