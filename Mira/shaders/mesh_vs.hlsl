#include "ShaderInterop_Renderer.h"
#include "ShaderInterop_Mesh.h"

struct VS_OUT
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

ConstantBuffer<PushConstants> g_push_constants : register(b0, space0);


VS_OUT main(uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID)
{
    VS_OUT output = (VS_OUT) 0;

    // Grab global table
    ConstantBuffer<ShaderInterop_MeshTable> mesh_table = ResourceDescriptorHeap[g_push_constants.mesh_table_id];
    
    // Grab per frame data
    ConstantBuffer<ShaderInterop_PerFrame> per_frame_data = ResourceDescriptorHeap[g_push_constants.per_frame_id];
    
    // Grab per draw data
    ConstantBuffer<ShaderInterop_PerDraw> per_draw_data = ResourceDescriptorHeap[g_push_constants.per_draw_id];
    
    // Grab submesh
    StructuredBuffer<ShaderInterop_SubmeshMD> submeshes = ResourceDescriptorHeap[mesh_table.submesh_md_array];
    ShaderInterop_SubmeshMD submesh = submeshes[g_push_constants.submesh_id];
    
    vertex_id += submesh.vert_start;    // Add vertex offset to submesh within mesh
    
    // Grab vertex tables and vertex data
    StructuredBuffer<float3> positions = ResourceDescriptorHeap[mesh_table.vert_pos_array];
    StructuredBuffer<float2> uvs = ResourceDescriptorHeap[mesh_table.vert_uv_array];
    StructuredBuffer<float3> normals = ResourceDescriptorHeap[mesh_table.vert_nor_array];
    StructuredBuffer<float3> tangents = ResourceDescriptorHeap[mesh_table.vert_tangent_array];
    output.position = mul(per_frame_data.projection_matrix, mul(per_frame_data.view_matrix, mul(per_draw_data.world_matrix, float4(positions[vertex_id], 1.f))));
    output.uv = uvs[vertex_id];
    output.normal = mul(per_draw_data.world_matrix, float4(normals[vertex_id], 1.f));
        
    return output;
}