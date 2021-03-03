#include <Windows.h>
#include <sstream>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#pragma comment (lib, "libEGL.lib")
#pragma comment (lib, "libGLESv2.lib")

void Init()
{
	//TODO
}

void Render()
{
	//TODO
}

void Destroy()
{
	//TODO
}

void fnCheckEGLError(const char* szFile, int nLine)
{
	const EGLint error = eglGetError();

	if (error != EGL_SUCCESS)
	{
		const char* errorStr = nullptr;
		switch (error) {
		case EGL_BAD_MATCH: {
			errorStr = "EGL_BAD_MATCH";
			break;
		}
		case EGL_BAD_DISPLAY: {
			errorStr = "EGL_BAD_DISPLAY";
			break;
		}
		case EGL_NOT_INITIALIZED: {
			errorStr = "EGL_NOT_INITIALIZED";
			break;
		}
		case EGL_BAD_CONFIG: {
			errorStr = "EGL_BAD_CONFIG";
			break;
		}
		case EGL_BAD_CONTEXT: {
			errorStr = "EGL_BAD_CONTEXT";
			break;
		}
		case EGL_BAD_ATTRIBUTE: {
			errorStr = "EGL_BAD_ATTRIBUTE";
			break;
		}
		case EGL_BAD_ALLOC: {
			errorStr = "EGL_BAD_ALLOC";
			break;
		}
		case EGL_BAD_SURFACE: {
			errorStr = "EGL_BAD_SURFACE";
			break;
		}
		case EGL_BAD_ACCESS: {
			errorStr = "EGL_BAD_ACCESS";
			break;
		}
		case EGL_BAD_NATIVE_PIXMAP: {
			errorStr = "EGL_BAD_NATIVE_PIXMAP";
			break;
		}
		case EGL_BAD_NATIVE_WINDOW: {
			errorStr = "EGL_BAD_NATIVE_WINDOW";
			break;
		}
		case EGL_BAD_CURRENT_SURFACE: {
			errorStr = "EGL_BAD_CURRENT_SURFACE";
			break;
		}
		case EGL_CONTEXT_LOST: {
			errorStr = "EGL_CONTEXT_LOST";
			break;
		}
		default: {
			errorStr = "NONE";
			break;
		}
		}
		char buffer[512];
		sprintf_s(buffer, "%s(%d):glError %s\n", szFile, nLine, errorStr);
		OutputDebugStringA(buffer);

	}
}
#define CheckEGLError fnCheckEGLError(__FILE__,__LINE__);

HINSTANCE hInst;                                // current instance
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int __stdcall WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance,
	__in_opt LPSTR lpCmdLine, __in int nShowCmd)
{
	MSG msg = { 0 };
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = L"OpenGLES3WindowClass";
	wc.style = CS_OWNDC;
	if (!RegisterClass(&wc))
	{
		OutputDebugStringA("RegisterClass failed!\n");
		return -1;
	}
	HWND hWnd = CreateWindowW(wc.lpszClassName, L"OpenGLES FrameBufferFetch Demo: Deferred Shading",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 640, 480, 0, 0, hInstance, 0);
	HDC hdc = GetDC(hWnd);

	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	// Get EGL display
	EGLNativeWindowType nativeWindowType = hWnd;
	EGLNativeDisplayType nativeDisplayType = hdc;
	EGLDisplay display = eglGetDisplay(nativeDisplayType);

	// Initialize EGL
	EGLint major;
	EGLint minor;
	if (!eglInitialize(display, &major, &minor))
	{
		CheckEGLError
		return -1;
	}

	// Choose EGL config
	EGLint attributes[] =
	{
		// Request OpenGL ES 3.1 configs
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_DEPTH_SIZE, 8,
		EGL_STENCIL_SIZE, 8,
		EGL_NONE,
	};
	EGLConfig config;
	EGLint num_configs;
	if (!eglChooseConfig(display, attributes, &config, 1, &num_configs))
	{
		CheckEGLError
		return -1;
	}
	if (num_configs < 1)
	{
		CheckEGLError
		return -1;
	}

	// Create EGL surface
	EGLSurface surface = eglCreateWindowSurface(display, config, nativeWindowType, nullptr);
	CheckEGLError

	// Create EGL context
	EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
	EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
	if (context == EGL_NO_CONTEXT)
	{
		CheckEGLError
		return -1;
	}

	// Make the EGL context current
	if (!eglMakeCurrent(display, surface, surface, context))
	{
		CheckEGLError
		return -1;
	}

	Init();

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0)
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		Destroy();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
