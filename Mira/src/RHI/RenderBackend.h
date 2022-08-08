#pragma once


namespace mira
{
	class RenderDevice;

	class RenderBackend
	{
	public:
		virtual RenderDevice* create_device() = 0;

		virtual ~RenderBackend() {}
	};
}

