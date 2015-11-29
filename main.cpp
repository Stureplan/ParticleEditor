#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glfw3.lib")

//Includes
#include <Windows.h>
#include <ctime>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gl/glew.h>
#include <gl/GL.h>
#include <glfw3.h>

//Custom classes
#include "camera.h"
#include "object.h"

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
GLuint shader = 0;
GLuint MatrixID;	//the in-shader matrix variable (think cbuffer)
GLuint TexID;

//Matrices
glm::mat4 Model		 = glm::mat4(1.0f);
glm::mat4 View		 = glm::mat4 (1.0f);
glm::mat4 Projection = glm::mat4(1.0f);
glm::mat4 MVP		 = glm::mat4(1.0f);		//model * view * projection

//Variables
int GL_FPS = 0;
float TEXANIM = 0.0f;

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

void InitializeMatrices ()
{
	camera.Initialize ();
	Projection = camera.GetProj ();
}

void CreateShaders()
{
	const char* vertex_shader = R"(
		#version 400
		layout(location = 0) in vec3 vertex_position;
		layout(location = 1) in vec3 vertex_color;
		layout(location = 2) in vec2 vertex_uv;
		
		out vec3 color;
		out vec2 uv;
		out vec2 uv2;

		uniform mat4 MVP;
		uniform float anim;
		
		void main () 
		{
			gl_Position = MVP * vec4(vertex_position, 1);
			color = vertex_color;

			//Create two UV sets to be able to animate the same texture twice
			//in different directions
			uv = vertex_uv;
			uv2 = vertex_uv;

			//Animate both textures based on a uniform
			uv.x = uv.x + anim/10;
			uv.y = uv.y + anim/2;
			uv2.x = uv2.x - anim/2;
		}
	)";

	const char* fragment_shader = R"(
		#version 400
		in vec3 color;
		in vec2 uv;
		in vec2 uv2;

		uniform sampler2D tex;

		out vec4 fragment_color;
		void main () 
		{
			fragment_color = texture(tex, uv2);

			float distance = gl_FragCoord.z / gl_FragCoord.w;
			float newdistance;
			float near = 7.0f;
			float far = 10.0f;

			//Make the distance a manageable value (colors range from 0 to 1, so we want this close to that range)
			newdistance = distance / 25;

			//If the color value is  66% or more bright
			if ((fragment_color.x + fragment_color.y + fragment_color.z) > 2.0f)
			{				
				//fragment_color = fragment_color + newdistance;

				//Only a certain depth range gets fake specular. 0.4 -> 0.6
				if (newdistance < 0.5f && newdistance > 0.3f)
				{
					//Add fake specular
					fragment_color += newdistance * 1.3f;
				}
			}


			//Sample the same texture again with opposite animated UV coords,
			//this creates a "double layer" effect
			fragment_color = fragment_color * texture(tex, uv);

			//Restrict darkness and light. More "newdistance" = more darkness.			
			newdistance = clamp(newdistance, 0.0f, 0.45f);
			
			//Darken based on distance. Further back = darker
			fragment_color -= newdistance * 1.5f;


		}
	)";

	//create vertex shader
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, nullptr);
	glCompileShader(vs);

	//create fragment shader
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, nullptr);
	glCompileShader(fs);

	//link shader program (connect vs and ps)
	shader = glCreateProgram();
	glAttachShader(shader, fs);
	glAttachShader(shader, vs);
	glLinkProgram(shader);
	
	TexID	 = glGetUniformLocation (shader, "anim");
	MatrixID = glGetUniformLocation (shader, "MVP");
	glClearColor (0.2f, 0.2f, 0.2f, 1.0f);
}

void CompileShaderError(GLuint* shader)
{
	GLint success = 0;
}

void CreateObjects()
{
	TextureData monkey;
	monkey.texturename = "Data/water.png";
	monkey.width = 1024;
	monkey.height = 1024;

	cube = new Object ("Data/water.obj", &monkey, glm::vec3 (0.0f, 0.0f, 0.0f), shader);
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
	TEXANIM = TEXANIM + 0.05f * deltaTime;		//update what i add to the UVs in shader

	glm::vec3 pos = camera.GetPos ();
	if (WM_KEYDOWN && GetAsyncKeyState(0x53))
	{
		pos.z -= MoveSpeed * deltaTime;
	}

	if (WM_KEYDOWN && GetAsyncKeyState(0x57))
	{
		pos.z += MoveSpeed * deltaTime;
	}

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

	// ----------- This is done for each Object* in the scene -----------
	Model = glm::mat4 (1.0f);			//Model must be reset for each object and each frame
	Model = cube->GetModel ();			//match Model with object Model
	View = camera.GetView ();
	

	MVP	  = Projection * View * Model;	//have to mult with PVM to get camera correct view
	glUniform1f (TexID, TEXANIM);
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
		InitializeMatrices ();
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
				wss << "FPS: " << GL_FPS;
				
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