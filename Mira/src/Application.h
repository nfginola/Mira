#pragma once
#include "Window/Window.h"
#include "Common.h"

class Application
{
public:
	Application();

	void run();

private:
	LRESULT window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	std::unique_ptr<Window> m_window;


};

