#include <iostream>
#include "Application.h"

int main()
{
	Application app;
	app.run();

	/*
		
		WIP,
		setup window,
		setup swapchain
		setup renderer (frame buffer)

		notes:
			we technically break LSP when converting RenderCommandList to RenderCommandList_DX12 in execute command list,
			but we can enforce by asserting that the command list is of the correct API type on submission
			(future notes, if we want to support Vk and multiple simultaneous RenderDevices).

			Probably better solution is to enforce that command lists submitted to a device HAS to be allocated from that specific
			device.

			hazard:
				auto cmdl = vk_rd->allocate_cmdl(..)
				dx_rd->submit_cmdl(cmdl)

				breaks at DX impl., 
	
	*/




	return 0;
}