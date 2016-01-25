#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "AntTweakBar.lib")
#pragma comment(lib, "Ws2_32.lib")

//Includes
#include <Windows.h>
#include <ctime>
#include <sstream>
#include <iomanip> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <gl/glew.h>
#include <gl/GL.h>
#include <AntTweakBar.h>
#include <glfw3.h>

#include <fstream>
#include <iostream>
#include <winsock.h>


//Additional includes
#include "structs.h"

//Custom classes
#include "camera.h"
#include "object.h"
#include "Shaders.h"
#include "particlesystem.h"
#include "Win32InputBox.h"

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

GLuint ps_program = 0;
GLuint ps_MatrixID;
GLuint ps_CamID;
GLuint ps_SizeID;

ParticleSystemData temp;

//Matrices
glm::mat4 Model		 = glm::mat4(1.0f);
glm::mat4 View		 = glm::mat4(1.0f);
glm::mat4 Projection = glm::mat4(1.0f);
glm::mat4 Ortho		 = glm::mat4(1.0f);
glm::mat4 MVP		 = glm::mat4(1.0f);		//model * view * projection

//UI Variables
glm::vec3	CameraPos	  = glm::vec3 (1.0f);
glm::vec2	CURRENT_SCALE = glm::vec2 (1.0f);

//Particle UI Variables
glm::vec2 FakeCurrentScale = glm::vec2 (1.0f, 1.0f);

int			CURRENT_FPS = 0;
float		CURRENT_ROTX = 0.0f;
float		CURRENT_ROTY = 0.0f;
float		CURRENT_ROTZ = 0.0f;
int			CURRENT_TEXTURE = 0;
float		CURRENT_GRAVITY = 1.0f;
std::string CURRENT_LABEL;
float		CURRENT_RATE = 0.1f;
bool		RENDER_DIR = true;
bool press = false;
glm::vec3	CURRENT_ROT;

float input_cooldown = 0.3f;
float input_current = 0.3f;

std::vector<std::string> texturenames;
std::vector<TextureData> texturedata;

//World objects
Object* arrow;
Object* sphere;
Object* plane;
Object* ui_particle;
ParticleSystem* ps;
Camera camera;
TwBar* BarGUI;

//Input variables
POINT p;
float mousespeed = 0.005f;
float horizontalAngle = 3.14f;
float verticalAngle = 0.0f;
float MoveSpeed = 5.0f;

bool PNGSize(const char *fileName, unsigned int &x, unsigned int &y) 
{
	std::ifstream file (fileName, std::ios_base::binary | std::ios_base::in);

	if (!file.is_open () || !file) 
	{
		file.close ();
		return false;
	}

	file.seekg (8, std::ios_base::cur);
	file.seekg (4, std::ios_base::cur);
	file.seekg (4, std::ios_base::cur);

	__int32 width, height;

	file.read ((char*) &width, 4);
	file.read ((char*) &height, 4);

	x = ntohl(width);
	y = ntohl(height);

	file.close ();

	return true;
}

void SetFPS(int fps)
{
	CURRENT_FPS = fps;
}

std::string WCHAR_TO_STRING (const wchar_t *wchar)
{
	std::string str = "";
	int index = 0;
	while (wchar[index] != 0)
	{
		str += (char) wchar[index];
		++index;
	}
	return str;
}

wchar_t* STRING_TO_WCHAR (const std::string &str)
{
	wchar_t wchar[260];
	int index = 0;
	while (index < str.size ())
	{
		wchar[index] = (wchar_t) str[index];
		++index;
	}
	wchar[index] = 0;
	return wchar;
}

std::vector<std::string> ListFiles(std::string directoryName)
{
	WIN32_FIND_DATA FindFileData;
	wchar_t * FileName = STRING_TO_WCHAR (directoryName);
	HANDLE hFind = FindFirstFile (FileName, &FindFileData);

	std::vector<std::string> listFileNames;
	listFileNames.push_back (WCHAR_TO_STRING (FindFileData.cFileName));

	while (FindNextFile (hFind, &FindFileData))
	{
		listFileNames.push_back (WCHAR_TO_STRING (FindFileData.cFileName));
	}

	//Insert "Data/Textures" at the beginning of each string
	for (int i = 0; i < listFileNames.size (); i++)
	{
		listFileNames[i].insert (0, std::string ("Data/Textures/"));
	}

	return listFileNames;
}

void SetLabel()
{
	std::string temp = texturenames.at(CURRENT_TEXTURE);
	temp.erase (0, 14);

	//Set label name to first part of the label
	CURRENT_LABEL = "label='";

	//Add texturename
	CURRENT_LABEL.append (temp);

	//Add finishing '
	CURRENT_LABEL.append ("'");

	std::string tw;

	tw = " Settings/Name: ";
	tw.append (CURRENT_LABEL);

	TwDefine (tw.c_str());
}

void TW_CALL Export(void *clientData)
{
	wchar_t buf[100] = { 0 };
	CWin32InputBox::InputBox(_T("Set filename"), _T("Set your filename.\nIt will automatically get a .ps extension."), buf, 100, false);
	std::wstring ws(buf);
	std::string filename(ws.begin(), ws.end());



	std::vector<std::string> filelist = ListFiles("Exports/*.ps");

	if (filename.size() == 0)
	{
		return;
	}
	filename.append(".ps");

	//Checks if the filename user wrote is something that already exists
	for (int i = 0; i < filelist.size(); i++)
	{
		std::string loop = filelist[i];
		loop.erase(0, 14);

		if (filename == loop)
		{
			MessageBox(NULL, L"Filename already exists! Operation canceled, agent.", L"Error!", MB_OK);
			return;
		}

	}

	filename.insert(0, std::string("Exports/"));

	//Get info
	TextureData tex_temp;
	ParticleSystemData ps_temp;

	tex_temp = ps->GetTextureData();
	ps_temp	 = ps->GetPSData();

	//Opens file
	std::ofstream file;
	file.open(filename);
	
	//Texture details
	file << "Texture: \n";
	file << tex_temp.texturename << "\n";
	file << tex_temp.height << "\n";
	file << tex_temp.width << "\n";

	//ParticleSystem details
	file << "\n";
	file << "Particle System: \n";
	file << ps_temp.width << "\n";
	file << ps_temp.height << "\n";
	file << ps_temp.maxparticles << "\n";
	file << ps_temp.lifetime << "\n";
	file << ps_temp.rate << "\n";
	file << ps_temp.force << "\n";
	file << ps_temp.gforce << "\n";
	file.close();
}

void InitializeGUI()
{
	CURRENT_LABEL = texturenames.at (CURRENT_TEXTURE);

	TwInit(TW_OPENGL, NULL);
	TwWindowSize(1280, 720);
	BarGUI = TwNewBar("Settings");
	TwDefine(" GLOBAL fontsize=3");
	TwDefine(" Settings position='1050 0'");
	TwDefine(" Settings color='0 0 0'");
	TwDefine(" Settings size='250 500'");
	TwDefine(" Settings refresh=0.1");
	TwDefine(" Settings movable=false");
	TwDefine(" Settings resizable=false");
	TwDefine(" Settings fontresizable=false");
	TwDefine(" GLOBAL buttonalign=right ");
	
	TwAddButton(BarGUI, "Export", Export, NULL, " label='Export Particle System' ");
	TwAddVarRW(BarGUI, "Rate:", TW_TYPE_FLOAT, &CURRENT_RATE, "min=-5.0f max=10.0f step=0.01f");
	TwAddVarRW(BarGUI, "Scale X:", TW_TYPE_FLOAT, &CURRENT_SCALE.x, "min=0.05f max=5.0f step=0.05f keyIncr=e keyDecr=d");
	TwAddVarRW(BarGUI, "Scale Y:", TW_TYPE_FLOAT, &CURRENT_SCALE.y, "min=0.05f max=5.0f step=0.05f keyIncr=r keyDecr=f");
	TwAddVarRW(BarGUI, "Direction X:", TW_TYPE_FLOAT, &CURRENT_ROTX, "min=-1.0f max=1.0f step=0.05f");
	TwAddVarRW(BarGUI, "Direction Y:", TW_TYPE_FLOAT, &CURRENT_ROTY, "min=-1.0f max=1.0f step=0.05f");
	TwAddVarRW(BarGUI, "Direction Z:", TW_TYPE_FLOAT, &CURRENT_ROTZ, "min=-1.0f max=1.0f step=0.05f");
	TwAddVarRW(BarGUI, "Direction", TW_TYPE_DIR3F, &CURRENT_ROT, "");
	TwAddVarRW(BarGUI, "Gravity:", TW_TYPE_FLOAT, &CURRENT_GRAVITY, "min=-5.0f max=5.0f step=0.05f");
	TwAddVarRW(BarGUI, "Show Direction", TW_TYPE_BOOL32, &RENDER_DIR, "");
	TwAddVarRO(BarGUI, "Texture:", TW_TYPE_INT16, &CURRENT_TEXTURE, "");
	TwAddButton(BarGUI, "Name:", NULL, NULL, CURRENT_LABEL.c_str ());


	SetLabel ();
}

void InitializeCamera ()
{
	camera.Initialize (glm::vec3(0.0f, 1.0f, -8.0f), glm::vec3(0.0f, 0.0f, 1.0f), width, height);
	View	   = camera.GetView ();
	Projection = camera.GetProj ();
	Ortho	   = camera.GetOrtho ();
}

void CreateShaders()
{
	//Create and load shaders (Name GS "none" to disable GS)
	program = LoadShaders("vertex.glsl", "none", "fragment.glsl");
	ps_program = LoadShaders("particle_vs.glsl", "particle_gs.glsl", "particle_fs.glsl");

	MatrixID = glGetUniformLocation (program, "MVP");
	ps_MatrixID = glGetUniformLocation(ps_program, "MVP");
	ps_CamID = glGetUniformLocation(ps_program, "cam");
	ps_SizeID = glGetUniformLocation(ps_program, "size");

	glClearColor (0.2f, 0.2f, 0.2f, 0.0f);
}

void CompileShaderError(GLuint* _shader)
{
	GLint success = 0;
}

void CreateObjects()
{
	//Lists all files inside of Data folder with the .png extension in a std::vector
	texturenames = ListFiles("Data/Textures/*.png");

	//With this power, I can use the std::vector to loop through all textures and
	//automatically set their name and dimensions in each TextureData.
	//Just remember to add "Data/" before texture name in the TextureData.

	for (int i = 0; i < texturenames.size (); i++)
	{
		//Temporary variables for this iteration
		unsigned int temp_width, temp_height;
		TextureData  temp_tex;

		//Initialize
		temp_tex.texturename = texturenames.at(i).c_str();
		temp_tex.width = 0;
		temp_tex.height = 0;

		//Determine file dimensions
		PNGSize (temp_tex.texturename, temp_width, temp_height);

		//Set TextureData values
		temp_tex.width		 = temp_width;
		temp_tex.height		 = temp_height;

		//Add info to back of std::vector texturedata
		texturedata.push_back (temp_tex);
	}

	TextureData wire_tex;
	wire_tex.texturename = "Data/Textures/wireframe.png";
	wire_tex.width = 32;
	wire_tex.height = 32;

	TextureData part_tex;
	part_tex.texturename = "Data/Textures/particle.png";
	part_tex.width = 32;
	part_tex.height = 32;


	ParticleSystemData part;
	part.width = 0.2f;
	part.height = 0.2f;
	part.lifetime = 1.0f;
	part.maxparticles = 100;
	part.rate = 0.0f;
	part.force = 2.0f;
	part.gforce = 0.5f; //1.0f = earth grav, 0.5f = half earth grav

	temp = part;

	CURRENT_SCALE.x = (float)part.width;
	CURRENT_SCALE.y = (float)part.height;
	CURRENT_GRAVITY = part.gforce;

	arrow	= new Object("Data/OBJ/arrow.obj", &wire_tex, glm::vec3 (0.0f, 0.0f, 0.0f), program, false);
	sphere	= new Object("Data/OBJ/sphere.obj", &wire_tex, glm::vec3(0.0f, 0.0f, 0.0f), program, false);
	plane	= new Object("Data/OBJ/plane.obj", &wire_tex, glm::vec3 (0.0f, 0.0f, 0.0f), program, false);

	ui_particle = new Object("Data/OBJ/particle.obj", &texturedata[CURRENT_TEXTURE], glm::vec3 (0.0f, 0.0f, 0.0f), program, true);
	ps			= new ParticleSystem(&part,			  &texturedata[CURRENT_TEXTURE], glm::vec3 (0.0f, 0.0f, 0.0f), ps_program);

	ui_particle->Rescale (glm::vec3 (0.125f, 0.2f, 1.0f));
	ui_particle->Translate (glm::vec3 (5.8f, 1.5f, 0.0f));

	//Initial rotation for arrow
	glm::vec3 dir = glm::vec3(cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle));
	dir = glm::normalize(dir);

	//Initial rot (direction) values
	CURRENT_ROTX = dir.x;
	CURRENT_ROTY = dir.y;
	CURRENT_ROTZ = dir.z;
	CURRENT_ROT = glm::vec3(CURRENT_ROTX, CURRENT_ROTY, CURRENT_ROTZ);

	//Rotate arrow once with direction
	arrow->Rotate(dir);
}

void SetViewport(HWND hwnd)
{
	windowReference = hwnd;
	glViewport(0, 0, width, height);
	
}

glm::vec3 GetInputDir (double deltaTime)
{
	//horizontalAngle -= mousespeed * float (width / 2 - p.x);
	horizontalAngle = horizontalAngle - 1.0f * deltaTime;

	glm::vec3 dir = glm::vec3(cos (verticalAngle) * sin (horizontalAngle),
							  sin (verticalAngle),
							  cos (verticalAngle) * cos (horizontalAngle));

	dir = glm::normalize (dir);

	CURRENT_ROTX = dir.x;
	CURRENT_ROTY = dir.y;
	CURRENT_ROTZ = dir.z;

	return dir;
}

void Update (double deltaTime)
{
	if (input_current > 0.0f)
	{
		input_current = input_current - (float) deltaTime;
	}


	glm::vec3 rot (CURRENT_ROTX, CURRENT_ROTY, CURRENT_ROTZ);
	arrow->Rotate (rot);

	if (GetAsyncKeyState(VK_SPACE) & 0x8000)  //the button is being held currently
	{
		rot = GetInputDir(deltaTime);
		arrow->Rotate(rot);
	}


	if (GetAsyncKeyState (VK_RIGHT) && input_current <= 0.0f)
	{
		input_current = input_cooldown;
		if (CURRENT_TEXTURE+1 >= texturedata.size ())
		{
			CURRENT_TEXTURE = 0;
		}
		else
		{
			CURRENT_TEXTURE++;
		}

		ps->Rebuild (&texturedata[CURRENT_TEXTURE]);
		ui_particle->Rebuild (&texturedata[CURRENT_TEXTURE]);
		SetLabel ();

	}
	if (GetAsyncKeyState (VK_LEFT) && input_current <= 0.0f)
	{
		input_current = input_cooldown;
		if (CURRENT_TEXTURE - 1 < 0)
		{
			CURRENT_TEXTURE = texturedata.size()-1;
		}
		else
		{
			CURRENT_TEXTURE--;
		}

		ps->Rebuild (&texturedata[CURRENT_TEXTURE]);
		ui_particle->Rebuild (&texturedata[CURRENT_TEXTURE]);
		SetLabel ();
	}


	if (GetAsyncKeyState(VK_TAB) && press == false)
	{
		bool active = arrow->IsActive();
		arrow->SetActive(!active);
		sphere->SetActive(active);
		press = true;
	}
	else if (!GetAsyncKeyState(VK_TAB))
	{
		press = false;
	}
	
	sphere->Update();
	arrow->Update();	//updates model matrix (T * R * S compute)
	ui_particle->Update ();

	glm::vec3 rotation(CURRENT_ROTX, CURRENT_ROTY, CURRENT_ROTZ);
	CURRENT_ROT = rotation;

	//Update shader variables
	CameraPos = camera.GetPos();


	//Update temp with new values
	temp.dir = rotation;
	temp.width = CURRENT_SCALE.x;
	temp.height = CURRENT_SCALE.y;
	temp.gforce = CURRENT_GRAVITY;
	temp.rate = CURRENT_RATE;

	//TODO: Instead of separate variables, send one whole ParticleInfo struct each frame
	//and change its values. This way we can easily modify it and export later :()
	ps->Update(deltaTime, CURRENT_GRAVITY, arrow->IsActive(), &temp, camera.GetPos());

	glm::vec3 pos = camera.GetPos();
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

	//	--- Objects
	if (arrow->IsActive() == true && RENDER_DIR)
	{
		glUseProgram (program);
		Model = glm::mat4(1.0f);			//Model must be reset for each object and each frame
		Model = arrow->GetModel();			//match Model with object Model
		Model = glm::transpose(Model);

		MVP = Projection * View * Model;	//have to mult with PVM to get camera correct view
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);	//update "MVP" shader var to MVP matrix


		arrow->Render();		//renders arrow buffers with shader program set in Initialize()
	}
	else if (arrow->IsActive () == false && RENDER_DIR)
	{
		glUseProgram (program);
		Model = glm::mat4(1.0f);			//Model must be reset for each object and each frame
		Model = sphere->GetModel();			//match Model with object Model

		MVP = Projection * View * Model;	//have to mult with PVM to get camera correct view
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);	//update "MVP" shader var to MVP matrix


		sphere->Render();
	}

	if (plane->IsActive ())
	{
		glUseProgram (program);
		Model = glm::mat4 (1.0f);			//Model must be reset for each object and each frame
		Model = plane->GetModel ();			//match Model with object Model

		MVP = Projection * View * Model;	//have to mult with PVM to get camera correct view
		glUniformMatrix4fv (MatrixID, 1, GL_FALSE, &MVP[0][0]);	//update "MVP" shader var to MVP matrix

		plane->Render ();
	}
	//	--- End of Objects


	//	--- PS Object
	glUseProgram (ps_program);
	glm::mat4 VP = glm::mat4 (1.0f);
	VP = Projection * View;
	glUniformMatrix4fv(ps_MatrixID, 1, GL_FALSE, &VP[0][0]);
	glUniform3fv(ps_CamID, 1, glm::value_ptr(CameraPos));
	glUniform2fv(ps_SizeID, 1, glm::value_ptr(CURRENT_SCALE));
	ps->Render();
	//	--- End of PS Object

	// ----------- Render GUI -------- 
	TwDraw ();

	glDisable (GL_DEPTH_TEST);
	glUseProgram (program);
	Model = glm::mat4 (1.0f);
	Model = ui_particle->GetModel ();

	MVP = Model;
	glUniformMatrix4fv (MatrixID, 1, GL_FALSE, &MVP[0][0]);
	ui_particle->Render ();

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


		InitializeCamera ();
		CreateShaders();
		CreateObjects();
		InitializeGUI ();
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
				
				wss << std::setprecision(3) << "FPS: " << CURRENT_FPS;
				
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
						TwTerminate();
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
	if (TwEventWin(hWnd, message, wParam, lParam))
	{
		return 0;
	}

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