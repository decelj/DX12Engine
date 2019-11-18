#pragma once

#include "stdafx.h"

class Window
{
public:
	Window();
	~Window();

	void Show(int32_t flags) const;
	HWND Handle() const { return m_Window; }

	uint32_t Width() const;
	uint32_t Height() const;

	static bool RegesterWindowClass(HINSTANCE instance, WNDPROC windowMessageProc);
private:
	void MakeWindow();

	HWND	m_Window;
};


inline void Window::Show(int32_t flags) const
{
	ShowWindow(m_Window, flags);
}

inline uint32_t Window::Width() const
{
	RECT rc;
	GetClientRect(m_Window, &rc);
	return rc.right - rc.left;
}

inline uint32_t Window::Height() const
{
	RECT rc;
	GetClientRect(m_Window, &rc);
	return rc.bottom - rc.top;
}
