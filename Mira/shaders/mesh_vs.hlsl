#include "ShaderInterop_Renderer.h"
#include "ShaderInterop_Mesh.h"

struct VS_OUT
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

// Set on Indirect Command
ConstantBuffer<PushConstant> per_draw_constant : register(b0, space0);

ConstantBuffer<PushConstant> global_constant : register(b1, space0);
ConstantBuffer<PushConstant> per_frame_constant : register(b2, space0);


VS_OUT main(uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID)
{
    VS_OUT output = (VS_OUT) 0;
    // Indirection 1
    StructuredBuffer<ShaderInterop_SceneInstanceID> per_instance = ResourceDescriptorHeap[per_draw_constant.value];

    ConstantBuffer<ShaderInterop_PerFrame> pf_data = ResourceDescriptorHeap[per_frame_constant.value];
    ConstantBuffer<ShaderInterop_BindlessGlobals> global_table = ResourceDescriptorHeap[global_constant.value];
   
    StructuredBuffer<ShaderInterop_PerInstance> scene_instances = ResourceDescriptorHeap[pf_data.scene_instance_table_id];
    StructuredBuffer<ShaderInterop_Mesh> mesh_section_table = ResourceDescriptorHeap[global_table.mesh_sections_id];
    StructuredBuffer<float3> positions = ResourceDescriptorHeap[global_table.pos_id];
    StructuredBuffer<float2> uvs = ResourceDescriptorHeap[global_table.uv_id];
    StructuredBuffer<float3> normals = ResourceDescriptorHeap[global_table.nor_id];
    
    ShaderInterop_PerInstance instance_data = scene_instances[per_instance[instance_id].object_id];
    
    ShaderInterop_Mesh mesh = mesh_section_table[instance_data.mesh];
    
    // Vertex offet to submesh in the big buffer
    vertex_id += mesh.vertex_offset_in_big_buffer;
     
    output.position = mul(pf_data.proj_mat, mul(pf_data.view_mat, mul(instance_data.world_matrix, float4(positions[vertex_id].xyz, 1.f))));
    output.uv = uvs[vertex_id];
    output.normal = normals[vertex_id];
    output.instance_id = instance_id;
        
    return output;
}