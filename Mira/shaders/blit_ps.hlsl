struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct PushConstantElement
{
    uint value;
};
ConstantBuffer<PushConstantElement> push_constant : register(b0, space0);

SamplerState point_samp : register(s1, space1);

float4 main(VS_OUT input) : SV_TARGET
{    
    //return float4(input.uv, 0.f, 1.f);
    //return float4(push_constant.value.rr, 0.f, 1.f);
    
    Texture2D tex = ResourceDescriptorHeap[push_constant.value];
    
    float3 color = tex.Sample(point_samp, input.uv);
    
    // Gamma correction
    color = pow(color, (1.f / 2.22f).rrr);
    
    return float4(color, 1.f);
}