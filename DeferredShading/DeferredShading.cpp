#include <Windows.h>
#include <sstream>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl31.h>
#pragma comment (lib, "libEGL.lib")
#pragma comment (lib, "libGLESv2.lib")
#pragma comment (lib, "libMaliEmulator.lib")

float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

const char* vs  = R"(#version 310 es
layout (location = 0) in vec3 position;
void main()
{
   gl_Position = vec4(position, 1.0f);
}
)";

const char* fs = R"(#version 310 es
#extension GL_ARM_shader_framebuffer_fetch : enable
precision mediump float;
layout(location = 0) uniform vec4 uBlend0;
layout(location = 1) uniform vec4 uBlend1;
layout(location = 0) out vec4 fragColor;
void main ( void )
{
#ifdef GL_ARM_shader_framebuffer_fetch
	vec4 Color = gl_LastFragColorARM;
	Color = mix(Color, uBlend0, Color.w*uBlend0.w);
	Color *= uBlend1;
#else
	vec4 Color = vec4(1,0,0,0) + 0.001*uBlend0*uBlend1;
#endif
	fragColor = Color;
}
)";

HWND hWnd;
EGLDisplay eglDisplay;
EGLSurface eglSurface;
GLuint vao, vbo, program;

void Init()
{
	// vertex and index buffer
	// create and bind vertex array object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	// create vertex buffer object copy our vertices array in a buffer for OpenGL to use
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// set vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	
	// shader
	GLint compileResult = GL_FALSE;
	//compile vertex shader
	GLuint vertexShader(glCreateShader(GL_VERTEX_SHADER));
	glShaderSource(vertexShader, 1, &vs, nullptr);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileResult);
	if(compileResult != GL_TRUE)
	{
		int logLength = 0;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0) {
			GLchar* strInfoLog = new GLchar[logLength + 1];
			int actualLogLength = 0;
			glGetShaderInfoLog(vertexShader, logLength, &actualLogLength, strInfoLog);
			if(actualLogLength > 0)//Otherwise this is a false error.
			{
				char buffer[512];
				sprintf_s(buffer, "vertex shader error log:\n %s\n", strInfoLog);
				OutputDebugStringA(buffer);
				delete[] strInfoLog;
			}
		}
	}

	//compile fragment shader
	GLuint fragmentShader(glCreateShader(GL_FRAGMENT_SHADER));
	glShaderSource(fragmentShader, 1, &fs, nullptr);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileResult);
	if(compileResult != GL_TRUE)
	{
		int logLength = 0;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0) {
			GLchar* strInfoLog = new GLchar[logLength + 1];
			int actualLogLength = 0;
			glGetShaderInfoLog(fragmentShader, logLength, &actualLogLength, strInfoLog);
			if(actualLogLength > 0)//Otherwise this is a false error.
			{
				char buffer[512];
				sprintf_s(buffer, "fragment shader error log:\n %s\n", strInfoLog);
				OutputDebugStringA(buffer);
				delete[] strInfoLog;
			}
		}
	}

	//link vertex and fragment shader together
	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	//delete shaders objects
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);


	//misc
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

RECT clientRect = {};
void Render()
{
	//fetch client rect and adjust GL viewport
	RECT newClientRect;
	// Set viewport according to the client rect
	if (!GetClientRect(hWnd, &newClientRect))
	{
		OutputDebugStringA("GetClientRect Failed!\n");
	}
	if (newClientRect.left != clientRect.left
		|| newClientRect.top != clientRect.top
		|| newClientRect.right != clientRect.right
		|| newClientRect.bottom != clientRect.bottom)
	{
		clientRect = newClientRect;
		glViewport(0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glUseProgram(program);
	float blend0[] = {0.1f, 0.5f, 0.1f, 0.5f};
	float blend1[] = {0.5f, 0.1f, 0.1f, 0.5f};
	glUniform4fv(0, 1, blend0);
	glUniform4fv(1, 1, blend1);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	eglSwapBuffers(eglDisplay, eglSurface);
}

void Destroy()
{
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	//TODO destroy EGL
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
	hWnd = CreateWindowW(wc.lpszClassName, L"OpenGLES FrameBufferFetch Demo: Deferred Shading",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 640, 480, 0, 0, hInstance, 0);
	HDC hdc = GetDC(hWnd);
	
	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	// Get EGL display
	EGLNativeWindowType nativeWindowType = hWnd;
	EGLNativeDisplayType nativeDisplayType = hdc;
	eglDisplay = eglGetDisplay(nativeDisplayType);

	// Initialize EGL
	EGLint major;
	EGLint minor;
	if (!eglInitialize(eglDisplay, &major, &minor))
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
	if (!eglChooseConfig(eglDisplay, attributes, &config, 1, &num_configs))
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
	eglSurface = eglCreateWindowSurface(eglDisplay, config, nativeWindowType, nullptr);
	CheckEGLError

	// Create EGL context
	EGLint contextAttribs[] = {
		EGL_CONTEXT_MAJOR_VERSION, 3,
		EGL_CONTEXT_MINOR_VERSION, 1,
		EGL_NONE
	};
	EGLContext context = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
	if (context == EGL_NO_CONTEXT)
	{
		CheckEGLError
		return -1;
	}

	// Make the EGL context current
	if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, context))
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
