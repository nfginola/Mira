#include "GPUGarbageBin.h"

namespace mira
{
	GPUGarbageBin::GPUGarbageBin(u8 max_frames_in_flight) : 
		m_max_frames_in_flight(max_frames_in_flight)
	{
	}

	void GPUGarbageBin::push_deferred_deletion(const std::function<void()>& deletion_func)
	{
		Deletion_Storage storage{};
		storage.frame_idx_on_request = m_curr_frame_idx;
		storage.func = deletion_func;

		std::lock_guard<std::mutex> guard(m_mutex);
		m_deletes.push(storage);
	}

	void GPUGarbageBin::begin_frame()
	{
		/*
			Assuming deletes are always grouped contiguously:

			[ 0, 0, 0, 1, 1, 1, 2, 2, 2, 0, 0, 0, ...]
		*/	
		while (!m_deletes.empty())
		{
			auto& storage = m_deletes.front();
			if (storage.frame_idx_on_request == m_curr_frame_idx)
			{
				storage.func();	// delete
				m_deletes.pop();
			}
			else
			{
				break;
			}
		}
	}

	void GPUGarbageBin::end_frame()
	{
		m_curr_frame_idx = (m_curr_frame_idx + 1) % m_max_frames_in_flight;
	}
}
