#pragma once
#include "../Common.h"
#include <queue>
#include <mutex>


namespace mira
{
	class GPUGarbageBin
	{
	public:
		GPUGarbageBin(u8 max_frames_in_flight);

		void push_deferred_deletion(const std::function<void()>& deletion_func);

		void begin_frame();
		void end_frame();

	private:
		struct Deletion_Storage
		{
			u32 frame_idx_on_request{ 0 };
			std::function<void()> func;
		};

	private:
		u8 m_max_frames_in_flight{ 0 };
		u8 m_curr_frame_idx{ 0 };
		std::queue<Deletion_Storage> m_deletes;

		std::mutex m_mutex;

	};
}


