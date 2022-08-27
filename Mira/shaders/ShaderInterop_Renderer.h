#pragma once
#include "ShaderInterop_Common.h"

struct ShaderInterop_MeshTable
{
	uint vert_pos_array;
	uint vert_uv_array;
	uint vert_nor_array;
	uint vert_tangent_array;
	uint submesh_md_array;
};

struct ShaderInterop_PerDraw
{
	matrix world_matrix;
};

struct ShaderInterop_PerFrame
{
	matrix view_matrix;
	matrix projection_matrix;
};




