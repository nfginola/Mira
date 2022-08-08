#pragma once
#include "ShaderInterop_Common.h"

struct ShaderInterop_Mesh
{
	uint vertex_offset_in_big_buffer;

	/*
		potentially draw args here? 
		we can use this in the DrawIndexedInstanced and Generate the Indirect Commands on the compute shader.
		Meaning we dont need to setup the draw calls on-site on the CPU, the draw calls are SAVED and accessed by the GPU
		depending on the Objects/Instances we want to draw! --> No zero sized draws?

		So on the CPU, we simply update Per Instance Data and also fill Per Draw Data

			For each object:
				SSBO[objectID].mesh = object.mesh		--> Enough to generate the geometry for a draw call
				.material = object.material				--> Enough to access shader parameters (textures, constants, etc.)
				.culling = object.culling				--> Enough to do identify the instance in compute culling (e.g sphere bounds)
				.batch_no = batch						--> Generated for each unique Mesh/Material 

				add_draw_arg(at: batch_no, for: object.mesh):		--> Fills draw args (mesh subsection args)
					if batch_no is not handled:
						DrawArgs[batch_no].args = object.mesh.draw_args

				We should batch the scene and add a Refresh only if we need. 
				In the case of streaming in resources --> We can stream in the scene in arbitrary ways! And 
				update the batch at a different frequency if we want to.

				++objectID

			At this point, the only thing missing 



		On compute (object per thread)

			if visible(object.culling_bounds):
				atomic_add(
				cmd.args = mesh_sections[object.mesh].draw_args
	*/
};
