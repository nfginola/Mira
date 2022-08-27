#pragma once

namespace mira
{
	/*
		Works with host-visible device memory.
	
	*/
	template <typename DataType>
	class GPUTableHost
	{
	public:
		/*
			
			// Gives user immediately CPU-writable address
			// Works on single view (extra indirection on GPU, access array first, then use table_idx to index into array)
			{ void*, table_idx } = allocate_for_table();

			u32 = get_table_view();

			// Works on temporary views
			{ void*, view_idx } = allocate_for_direct_view();
			
			// Free can verify that the associated address is in range (actually belongs to this table)
			free(Handle)

			

		*/

	private:

	};
}
