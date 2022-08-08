#pragma once
#include "ShaderInterop_Common.h"

struct ShaderInterop_PreCommand
{
    uint32_t batch_id;              // Maps to draw call to fill
    uint32_t object_id;             // ID to Scene Per Instance Data table 

    // Geometry information for culling
    uint32_t cull_data;
};

struct ShaderInterop_IndirectCommand
{
    // Set Root Constant command
    uint root_constant;

    // Draw command
    uint index_count_per_instance;
    uint instance_count;
    uint start_index_loc;
    int base_vert_loc;
    uint start_instance_loc;
};

struct CONSTANT_ALIGN ShaderInterop_BindlessPerDraw
{
    uint mesh_id;               // Mesh Section ID
    uint material_idx;          // Material ID

    uint object_id;             // Per Object ID
};

struct STRUCTURED_ALIGN ShaderInterop_PerInstance
{
    matrix world_matrix;

    uint mesh;
    uint material;

    uint object_id;

    uint pad1;
 };

// IDs to various tables
struct CONSTANT_ALIGN ShaderInterop_BindlessGlobals
{
    // Mesh section table
    uint mesh_sections_id;

    // Vertex data tables
    uint pos_id;
    uint uv_id;
    uint nor_id;

};

struct CONSTANT_ALIGN ShaderInterop_PerFrame
{
    // Misc
    matrix view_mat;
    matrix proj_mat;

    // ID to Per Object table
    uint scene_instance_table_id;
};

struct STRUCTURED_ALIGN ShaderInterop_SceneInstanceID
{
    uint object_id;

    uint pad0;
    uint pad1;
    uint pad2;
};




