#pragma once

namespace mira
{
	/*
		Designed for device-local GPU tables that have significant data per element.
		Examples: MaterialData, InstanceData, SubmeshData, etc..

		Vertex buffers 'can' be handled using this, but it's not convenient.
		(e.g handle per Vertex? Not productive).
	
	*/
	template <typename Handle, typename DataType>
	class GPUTableDL
	{
	public:
		
		/*
			{ handle, table_idx } = allocate(init_data, size);

			free(handle)				// Free element

			u32 = get_table_view();		// For bindless binding

			// Upload all requested allocations at once 
			sync = execute_upload(sync_with, generate_sync, queue_target...)
			sync with:  guarantee read completion before copying into curr version

		*/

	private:
		struct Internal_Storage
		{

		};

	private:



	};
}


