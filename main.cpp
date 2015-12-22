#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glfw3.lib")

//Includes
#include <Windows.h>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gl/glew.h>
#include <gl/GL.h>
#include <glfw3.h>

//Custom classes
#include "camera.h"
#include "object.h"
#include "Shaders.h"

using namespace glm;

//Window
int width = 1280;
int height = 720;
extern GLFWwindow* window = glfwCreateWindow(width, height, "Test", NULL, NULL);
HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HGLRC CreateOpenGLContext(HWND wndHandle);
HWND windowReference;

//Pointer variables
GLuint program = 0;
GLuint MatrixID;	//the in-shader matrix variable (think cbuffer)
GLuint TexID;

//Matrices
glm::mat4 Model		 = glm::mat4(1.0f);
glm::mat4 View		 = glm::mat4(1.0f);
glm::mat4 Projection = glm::mat4(1.0f);
glm::mat4 MVP		 = glm::mat4(1.0f);		//model * view * projection

//Variables
int GL_FPS = 0;
float GL_SCALE = 0;
bool keyDown;

//World objects
Object* cube;
Camera camera;

//Input variables
POINT p;
float mousespeed = 0.005f;
float horizontalAngle = 3.14f;
float verticalAngle = 0.0f;
float MoveSpeed = 5.0f;


void SetFPS(int fps)
{
	GL_FPS = fps;
}

void InitializeCamera ()
{
	camera.Initialize (glm::vec3(0.0f, 1.0f, -8.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	View	   = camera.GetView ();
	Projection = camera.GetProj ();
}

void CreateShaders()
{
	//Create and load shaders (GS IS DISABLED!)
	program = LoadShaders("vertex.glsl", "geometry.glsl", "fragment.glsl");
	
	TexID	 = glGetUniformLocation (program, "anim");
	MatrixID = glGetUniformLocation (program, "MVP");
	glClearColor (0.2f, 0.2f, 0.2f, 1.0f);
}

void CompileShaderError(GLuint* _shader)
{
	GLint success = 0;
}

void CreateObjects()
{
	TextureData monkey;
	monkey.texturename = "Data/wireframe.png";
	monkey.width = 32;
	monkey.height = 32;

	cube = new Object ("Data/cube2.obj", &monkey, glm::vec3 (0.0f, 0.0f, 0.0f), program);
}

void SetViewport(HWND hwnd)
{
	windowReference = hwnd;
	ShowCursor (false);
	glViewport(0, 0, width, height);
	
}

glm::vec3 GetInputDir ()
{
	GetCursorPos (&p);
	ScreenToClient (windowReference, &p);

	horizontalAngle -= mousespeed * float (width / 2 - p.x);

	glm::vec3 dir = glm::vec3(cos (verticalAngle) * sin (horizontalAngle),
							  sin (verticalAngle),
							  cos (verticalAngle) * cos (horizontalAngle));
	
	dir = glm::normalize (dir);
	
	p.x = width / 2;
	p.y = height / 2;

	ClientToScreen (windowReference, &p);
	SetCursorPos (p.x, p.y);

	return dir;
}

void Update (double deltaTime)
{
	cube->Rotate (GetInputDir ());				//rotates to match direction
	cube->Update();								//updates model matrix (T * R * S compute)

	glm::vec3 scale = cube->GetScale();
	glm::vec3 pos = camera.GetPos ();

	if (GetAsyncKeyState(0x53) & 0x8000)
	{
		//pos.z -= MoveSpeed * deltaTime;
		if (scale.x > 1.0f && keyDown == false)
		{
			cube->Rescale(glm::vec3(-0.1f, -0.1f, -0.1f));
			keyDown = true;
		}
	}

	else if (GetAsyncKeyState(0x57) & 0x8000)
	{
		//pos.z += MoveSpeed * deltaTime;
		if (scale.x < 5.0f && keyDown == false)
		{
			cube->Rescale(glm::vec3(0.1f, 0.1f, 0.1f));
			keyDown = true;
		}
	}
	else
	{
		keyDown = false;
	}

	GL_SCALE = scale.x;

	glm::vec3 dir = pos;
	dir.z = dir.z + 1.0f;
	dir.y = dir.y - 0.5f;

	camera.SetPos (pos);
	camera.SetDir (dir);
	camera.UpdateView ();
}

void Render()
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LESS);

	View = camera.GetView ();

	// ----------- This is done for each Object* in the scene -----------
	Model = glm::mat4 (1.0f);			//Model must be reset for each object and each frame
	Model = cube->GetModel ();			//match Model with object Model

	MVP	  = Projection * View * Model;	//have to mult with PVM to get camera correct view
	glUniformMatrix4fv (MatrixID, 1, GL_FALSE, &MVP[0][0]);	//update "MVP" shader var to MVP matrix
	cube->Render();						//renders cube buffers with shader program set in Initialize()
	// ------------------------------------------------------------------
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance); //1. Skapa fönster
	
	if (wndHandle)
	{
		HDC hDC = GetDC(wndHandle);
		HGLRC hRC = CreateOpenGLContext(wndHandle); //2. Skapa och koppla OpenGL context
		
		glewInit(); //3. Initiera The OpenGL Extension Wrangler Library (GLEW)
		SetViewport(wndHandle);
		
		double dt = 0.0f;
		float timePass = 0.0f;
		int fps = 0;
		unsigned int start = clock();

		//my functions
		InitializeCamera ();
		CreateShaders();
		CreateObjects();
		
		ShowWindow(wndHandle, nCmdShow);

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				
			}
			else
			{
				Update(dt);
				Render();
				SwapBuffers(hDC);
				
				unsigned int temp = clock();
				dt = unsigned int(temp - start) / double(1000);
				timePass += dt;
				start = temp;
				fps++;
				int WindowFPS = (int)1 / dt;
				std::wstringstream wss;
				
				wss << std::setprecision(3) << "FPS: " << GL_FPS << " Scale: " << GL_SCALE;
				
				SetWindowText(wndHandle, wss.str().c_str());

				if (timePass > 1.0f)
				{
					SetFPS(fps);
					timePass = 0.0f;
					fps = 0;
				}

				if (WM_KEYDOWN)
				{
					if (GetAsyncKeyState (VK_ESCAPE))
					{
						msg.message = WM_QUIT;
						//break;	//this breaks out of the while loop (exits program)
					}		//	   |
				}			//	  /
			}				//   /
		}					//  /
							// \/
		wglMakeCurrent(NULL, NULL);
		ReleaseDC(wndHandle, hDC);
		wglDeleteContext(hRC);
		DestroyWindow(wndHandle);
	}

	return (int) msg.wParam;
}

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.hInstance      = hInstance;
	wcex.lpszClassName = L"BTH_GL_DEMO";
	if( !RegisterClassEx(&wcex) )
		return false;

	RECT rc = { 0, 0, width, height };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	
	HMONITOR monitor = MonitorFromRect (&rc, MONITOR_DEFAULTTONEAREST);	//find out which monitor is active
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo (monitor, &info);									//store monitor res info
	int monitor_width = info.rcMonitor.right - info.rcMonitor.left;	
	int monitor_height = info.rcMonitor.bottom - info.rcMonitor.top;
	
	HWND handle = CreateWindow(
		L"BTH_GL_DEMO",
		L"OpenGL Experiment",
		WS_OVERLAPPEDWINDOW,				
		(monitor_width / 2) - (width/2),	//always in middle of active monitor screen width
		(monitor_height / 2) - (height/2),	//always in middle of active monitor screen height
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);
	
	return handle;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message) 
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;		
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

HGLRC CreateOpenGLContext(HWND wndHandle)
{
	//get handle to a device context (DC) for the client area
	//of a specified window or for the entire screen
	HDC hDC = GetDC(wndHandle);
	
	//details: http://msdn.microsoft.com/en-us/library/windows/desktop/dd318286(v=vs.85).aspx
	static  PIXELFORMATDESCRIPTOR pixelFormatDesc =
	{
		sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd  
		1,                                // version number  
		PFD_DRAW_TO_WINDOW |              // support window  
		PFD_SUPPORT_OPENGL |              // support OpenGL  
		PFD_DOUBLEBUFFER |                // double buffered
		0,               // disable depth buffer <-- added by Stefan
		PFD_TYPE_RGBA,                    // RGBA type  
		32,                               // 32-bit color depth  
		0, 0, 0, 0, 0, 0,                 // color bits ignored  
		0,                                // no alpha buffer  
		0,                                // shift bit ignored  
		0,                                // no accumulation buffer  
		0, 0, 0, 0,                       // accum bits ignored  
		0,                                // 0-bits for depth buffer <-- modified by Stefan      
		0,                                // no stencil buffer  
		0,                                // no auxiliary buffer  
		PFD_MAIN_PLANE,                   // main layer  
		0,                                // reserved  
		0, 0, 0                           // layer masks ignored  
	};

	//attempt to match an appropriate pixel format supported by a
	//device context to a given pixel format specification.
	int pixelFormat = ChoosePixelFormat(hDC, &pixelFormatDesc);

	//set the pixel format of the specified device context
	//to the format specified by the iPixelFormat index.
	SetPixelFormat(hDC, pixelFormat, &pixelFormatDesc);

	//create a new OpenGL rendering context, which is suitable for drawing
	//on the device referenced by hdc. The rendering context has the same
	//pixel format as the device context.
	HGLRC hRC = wglCreateContext(hDC);
	
	//makes a specified OpenGL rendering context the calling thread's current
	//rendering context. All subsequent OpenGL calls made by the thread are
	//drawn on the device identified by hdc. 
	wglMakeCurrent(hDC, hRC);

	return hRC;
}