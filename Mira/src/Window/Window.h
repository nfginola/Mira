#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <functional>

class Window
{
public:
	Window(HINSTANCE hInstance, std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> winProc = nullptr, int client_width = 1600, int client_height = 900, const std::string& title = "Application");
	~Window();

	Window& operator=(const Window&) = delete;
	Window(const Window&) = delete;
	
	void pump_messages() const;
	bool is_alive() const { return m_is_alive; };
	void set_fullscreen(bool fullscreenState);
	bool is_fullscreen() const { return m_is_fullscreen; };
	HWND& get_hwnd() { return m_hwnd; };
	int client_width() const { return m_client_width; };
	int client_height() const { return m_client_height; };
	void resize_client(UINT width, UINT height);
	
private:
	void register_window();
	void make_window();
	void fit_to_client_dim();

	LRESULT handle_proc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	static std::string get_next_id();
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	HINSTANCE m_hInstance;
	HWND m_hwnd;
	std::string m_title;
	std::string m_id;
	int m_client_width;
	int m_client_height;
	bool m_is_alive;
	bool m_is_fullscreen;
	DWORD m_style;
	DWORD m_ex_style;
	std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> m_custom_win_proc;

	struct ScreenTransitionDetails
	{
		int prev_x;
		int prev_y;
		int prev_width;
		int prev_height;
	} m_screen_transition;


};

