#include "engine\utils\global\GScreen.h"
#include "engine\utils\global\event\GMouse.h"

std::string GScreen::m_tooltip = "";
float GScreen::m_fps = 0;
float GScreen::m_fov = 0;
float GScreen::m_deltaTime = 0;
Sint8 GScreen::m_exitting = 0;
bool GScreen::m_developer = false;

GLFWwindow *GScreen::m_window = 0;
Vector2<Sint32> GScreen::m_monitorSize = {};
Vector2<Sint32> GScreen::m_windowPos = {};
bool GScreen::m_maximized = false;
Vector2<Sint32> GScreen::m_screenSize = {};
Vector2<Sint32> GScreen::m_smallScreen = {};
bool GScreen::m_draggingWindow = false;
Vector2<Sint32> GScreen::m_dragStart = {};
Vector2<Sint32> GScreen::m_dragDistance = {};
Vector2<Sint32> GScreen::m_initWindowSize = {};
Vector2<Sint32> GScreen::m_resizeMousePos = {};
bool GScreen::m_resizing = false;
bool GScreen::m_finishedResize = false;

GScreen::WindowCommand GScreen::m_windowCommand = NONE;

void GScreen::initWindow(GLFWwindow *p_window)
{
	m_window = p_window;

	int *x = new int(), *y = new int();
	glfwGetWindowPos(m_window, x, y);
	m_windowPos = Vector2<Sint32>(*x, *y);

	glfwGetWindowSize(m_window, x, y);
	m_smallScreen = m_screenSize = Vector2<Sint32>(*x, *y);
}

void GScreen::startWindowDrag()
{
	m_draggingWindow = true;
	m_dragStart = GMouse::m_guiMousePos;

}
bool GScreen::isDraggingWindow()
{
	return m_draggingWindow;
}
void GScreen::endWindowDrag()
{
	if(m_draggingWindow)
	{
		m_draggingWindow = false;
		m_dragDistance = {};
	}
}

void GScreen::startResizing() {
	m_initWindowSize = m_screenSize;
	m_resizeMousePos = GMouse::m_guiMousePos;
	m_resizing = true;
}
void GScreen::stopResizing() {
	m_resizing = false;
}

void GScreen::updateWindow()
{
	if(isDraggingWindow() && !isMaximized()) {
		m_dragDistance = m_dragStart - GMouse::m_guiMousePos;
		m_windowPos = m_windowPos - m_dragDistance;
		glfwSetWindowPos(m_window, m_windowPos.x, m_windowPos.y);
		m_dragStart = GMouse::m_guiMousePos + m_dragDistance;
	}
	if(m_finishedResize) {
		m_finishedResize = false;
	}
	if(m_resizing) {
		Vector2<Sint32> _size = m_screenSize;
		m_screenSize.x = max(800, (m_initWindowSize.x - (m_resizeMousePos.x - GMouse::m_guiMousePos.x)));
		m_screenSize.y = max(700, (m_initWindowSize.y - (m_resizeMousePos.y - GMouse::m_guiMousePos.y)));
		if(!(_size == m_screenSize))
			m_finishedResize = true;
	}
}
