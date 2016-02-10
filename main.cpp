#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "AntTweakBar.lib")
#pragma comment(lib, "Ws2_32.lib")

//Includes
#include <Windows.h>
#include <ctime>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <gl/glew.h>
#include <gl/GL.h>
#include <AntTweakBar.h>
#include <glfw3.h>
#include <iostream>
#include <fstream>
#include <shobjidl.h> 



//Additional includes
#include "structs.h"
#include "functions.h"

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

GLuint ps_lprogram = 0;
GLuint ps_lMatrixID;

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
int			CURRENT_TEXTURE = 0;
int			CURRENT_VTXCOUNT = 0;
int			CURRENT_VTXCOUNT_DIFF = 0;
int			CURRENT_ACTIVE = 0;
float		CURRENT_FORCE = 0.0f;
float		CURRENT_DRAG = 0.0f;
float		CURRENT_GRAVITY = 0.0f;
std::string CURRENT_LABEL;
float		CURRENT_EMISSION = 0.0f;
float		CURRENT_EMISSION_DIFF = 0.0f;
float		CURRENT_LIFETIME = 1.0f;
bool		CURRENT_REPEAT = true;
bool		RENDER_DIR = true;
bool press = false;
glm::vec3	CURRENT_ROT;

double dt = 0.0f;

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
TwBar* BarControls;

//Input variables
POINT p;
float mousespeed = 0.005f;
float horizontalAngle = 3.14f;
float verticalAngle = 0.0f;
float MoveSpeed = 5.0f;

void SetFPS(int fps)
{
	CURRENT_FPS = fps;
}

void SetLabel()
{
	std::string temp = texturenames.at(CURRENT_TEXTURE);
	temp.erase (0, 14);

	//Set label name to first part of the label
	CURRENT_LABEL = "label='";
	CURRENT_LABEL.append (temp);
	CURRENT_LABEL.append ("'");

	std::string tw;

	tw = " Settings/Name: ";
	tw.append (CURRENT_LABEL);

	TwDefine (tw.c_str());
}

void TW_CALL Export(void *clientData)
{
	//Get texture name (minus directories)
	std::string temp_tex = texturenames.at(CURRENT_TEXTURE);
	temp_tex.erase(0, 14);
	const char* texture = temp_tex.c_str();

	//Get particle system
	ParticleSystemData* ps_temp = ps->GetPSData();

	//Buffer to write input to
	wchar_t buf[100] = { 0 };
	CWin32InputBox::InputBox(_T("Set filename"), _T("Set your filename.\nNOTE: It will automatically get a .ps extension."), buf, 100, false);
	std::wstring ws(buf);
	std::string filename(ws.begin(), ws.end());
	std::vector<std::string> filelist = ListFiles("Exports/*", ".ps");

	//If the user was stupid and didn't enter a filename, return and cancel
	if (filename.size() == 0) {	return;	}

	//Add .ps extension to the file automatically
	filename.append(".ps");

	//If the active particles is way different from maxparticles, give user a warning
	//and return from filesaving
	float amount;
	float result;

	amount = ps_temp->maxparticles - ps->GetActiveParticles();
	result = (float)amount / (float)ps_temp->maxparticles;

	//30% or more than the used particles
	if (result >= 0.3f)
	{
		int msg = MessageBox(
			NULL,
			L"Your active particle count is way lower than your allocated amount of particles. Do you want to continue exporting?",
			L"Warning",
			MB_ICONWARNING | MB_YESNO);

		if (msg == IDNO)
		{
			return;
		}
	}
	


	//Checks if the filename user wrote is something that already exists
	for (int i = 0; i < filelist.size(); i++)
	{
		std::string loop = filelist[i];
		loop.erase(0, 8);

		if (filename == loop)
		{
			int msg = MessageBox(
				NULL, 
				L"Filename already exists. Do you want to replace it?", 
				L"Warning", 
				MB_ICONWARNING | MB_YESNO);
			
			if (msg == IDNO)
			{
				return;
			}
		}
	}


	//Add folder to the filename to export it to the correct location
	filename.insert(0, std::string("Exports/"));

	//Header size and texture size
	int texturesize;
	int totalsize;

	//Texturesize is how long the string is * bytes
	texturesize = strlen(texture) * sizeof(const char);
	totalsize = texturesize;				//texturename
	totalsize += sizeof(float) * 3;			//glm::vec3 dir
	totalsize += sizeof(float);				//float width
	totalsize += sizeof(float);				//float height
	totalsize += sizeof(int);				//int maxparticles
	totalsize += sizeof(float);				//float lifetime
	totalsize += sizeof(float);				//float emission
	totalsize += sizeof(float);				//float force
	totalsize += sizeof(float);				//float drag
	totalsize += sizeof(float);				//float gravity
	totalsize += sizeof(bool);				//bool continuous
	totalsize += sizeof(bool);				//bool omni

	//Opens file
	std::ofstream file;
	file.open(filename, std::ios::binary | std::ios::out);
	
	//Header
	file.write(reinterpret_cast<char*>(&totalsize), sizeof(int));
	file.write(reinterpret_cast<char*>(&texturesize), sizeof(int));
	file.write(texture, sizeof(const char) * strlen(texture));
	
	//Particle System variables
	file.write(reinterpret_cast<char*>(&ps_temp->dir), sizeof(glm::vec3));
	file.write(reinterpret_cast<char*>(&ps_temp->width), sizeof(float));
	file.write(reinterpret_cast<char*>(&ps_temp->height), sizeof(float));
	file.write(reinterpret_cast<char*>(&ps_temp->maxparticles), sizeof(int));
	file.write(reinterpret_cast<char*>(&ps_temp->lifetime), sizeof(float));
	file.write(reinterpret_cast<char*>(&ps_temp->emission), sizeof(float));
	file.write(reinterpret_cast<char*>(&ps_temp->force), sizeof(float));
	file.write(reinterpret_cast<char*>(&ps_temp->drag), sizeof(float));
	file.write(reinterpret_cast<char*>(&ps_temp->gravity), sizeof(float));
	file.write(reinterpret_cast<char*>(&ps_temp->continuous), sizeof(bool));
	file.write(reinterpret_cast<char*>(&ps_temp->omni), sizeof(float));
	//file.write(seed sizeof(int))
	file.close();
}

void TW_CALL Import(void *clientData)
{
	std::string fResult;

	HRESULT hr = CoInitializeEx(NULL, 
	COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if (SUCCEEDED(hr))
	{
		//Windows File opening dialog system
		IFileOpenDialog* pFile;

		//What files to display
		LPCWSTR ps = L".ps";
		COMDLG_FILTERSPEC rgSpec = { ps, L"*.ps" };

		//Create instance
		hr = CoCreateInstance(
			CLSID_FileOpenDialog, 
			NULL, 
			CLSCTX_ALL,
			IID_IFileOpenDialog, 
			reinterpret_cast<void**>(&pFile));

		if (SUCCEEDED(hr))
		{
			//Set attributes
			pFile->SetDefaultExtension(L"ps");
			pFile->SetFileTypes(1, &rgSpec);

			//Display dialog system
			hr = pFile->Show(NULL);

			if (SUCCEEDED(hr))
			{
				//File chosen
				IShellItem* pItem;
				hr = pFile->GetResult(&pItem);

				if (SUCCEEDED(hr))
				{
					//Filepath from file
					LPWSTR fPath;
					hr = pItem->GetDisplayName(SIGDN_NORMALDISPLAY, &fPath);

					if (SUCCEEDED(hr))
					{
						fResult = WCHAR_TO_STRING(fPath);
						CoTaskMemFree(fPath);
					}
					pItem->Release();
				}
				pFile->Release();
			}
			CoUninitialize();
		}
	}

	if (fResult.size() <= 0)
	{
		//No file was selected
		Beep(1000, 50);
		Beep(900, 50);
		Beep(800, 50);
		Beep(700, 50);
		Beep(600, 50);
		Beep(500, 50);
		Beep(400, 50);
		int msg = MessageBox(
			NULL,
			L"No file selected! Import canceled.",
			L"Error",
			MB_ICONERROR | MB_OK);
		return;
	}

	//Add folder to the filename to import it from the correct location
	fResult.insert(0, std::string("Exports/"));
	
	//Test reading file
	std::ifstream file;
	file.open(fResult, std::ios::binary | std::ios::in);
	if (!file.is_open())
	{
		//Error in opening the file
		Beep(1000, 50);
		Beep(900, 50);
		Beep(800, 50);
		Beep(700, 50);
		Beep(600, 50);
		Beep(500, 50);
		Beep(400, 50);
		int msg = MessageBox(
			NULL,
			L"File was damaged or corrupt!",
			L"Error",
			MB_ICONERROR | MB_OK);
		file.close();
		return;
	}

	//Read header
	ExportHeader exHeader;
	file.read((char*)&exHeader, sizeof(exHeader));

	//Read texture name
	char* f = (char*)malloc(exHeader.texturesize + 1);
	file.read(f, sizeof(char) * exHeader.texturesize);
	f[exHeader.texturesize] = 0;

	//Read Particle System
	ParticleSystemData exPS;
	file.read((char*)&exPS, sizeof(exPS));

	file.close();

	//File found and filename is fResult
	Beep(400, 50);
	Beep(500, 50);
	Beep(600, 50);
	Beep(700, 50);
	Beep(800, 50);
	Beep(900, 50);
	Beep(1000, 50);

	CURRENT_ROT = exPS.dir;
	CURRENT_SCALE.x = exPS.width;
	CURRENT_SCALE.y = exPS.height;
	CURRENT_FORCE = exPS.force;
	CURRENT_DRAG = exPS.drag;
	CURRENT_GRAVITY = exPS.gravity;
	CURRENT_VTXCOUNT = exPS.maxparticles;
	CURRENT_VTXCOUNT_DIFF = 0;
	CURRENT_EMISSION = exPS.emission;
	CURRENT_EMISSION_DIFF = exPS.emission;
	CURRENT_LIFETIME = exPS.lifetime;

	std::string name = "Data/Textures/";
	name.append(f);

	unsigned int x = 0;
	unsigned int y = 0;
	bool result = PNGSize(name.c_str(), x, y);
	if (result == false)
	{
		//Texture wasn't recognized
		Beep(1000, 50);
		Beep(900, 50);
		Beep(800, 50);
		Beep(700, 50);
		Beep(600, 50);
		Beep(500, 50);
		Beep(400, 50);
		int msg = MessageBox(
			NULL,
			L"Texture not found!",
			L"Error",
			MB_ICONERROR | MB_OK);
		return;
	}



	TextureData exTD;
	exTD.width = x;
	exTD.height = y;
	exTD.texturename = name.c_str();

	arrow->SetActive(!exPS.omni);
	ui_particle->Rebuild(&exTD);
	ps->Retexture(&exTD);
	ps->Rebuild(&exPS);
}

void TW_CALL Rebuild(void *clientData)
{
	temp.maxparticles = CURRENT_VTXCOUNT;
	ps->Rebuild(&temp);
}

void TW_CALL PausePlay(void* clientData)
{
	if (ps->IsPlaying() == true)
	{
		ps->Pause();
	}
	else
	{
		ps->Play();
	}
}

void InitializeGUI()
{
	CURRENT_LABEL = texturenames.at (CURRENT_TEXTURE);

	TwInit(TW_OPENGL, NULL);
	TwWindowSize(1280, 720);
	BarGUI		= TwNewBar("Settings");
	BarControls = TwNewBar("Controls");
	TwDefine(" GLOBAL fontsize=3");
	TwDefine(" GLOBAL buttonalign=right ");

	TwDefine(" Settings position='1050 0'");
	TwDefine(" Settings color='0 0 0'");
	TwDefine(" Settings size='250 500'");
	TwDefine(" Settings refresh=0.1");
	TwDefine(" Settings movable=false");
	TwDefine(" Settings resizable=false");
	TwDefine(" Settings fontresizable=false");

	TwDefine(" Controls position='0 0'");
	TwDefine(" Controls color='0 0 0'");
	TwDefine(" Controls size='250 250'");
	TwDefine(" Controls refresh=0.1");
	TwDefine(" Controls movable=false");
	TwDefine(" Controls resizable=false");
	TwDefine(" Controls fontresizable=false");

	TwAddVarRW(BarGUI, "Max Particles:", TW_TYPE_INT16, &CURRENT_VTXCOUNT, "label='Max Particles:' min=1 max=2000 ");
	TwAddVarRO(BarGUI, "Unused", TW_TYPE_INT16, &CURRENT_ACTIVE, "label='Active Particles:' ");
	TwAddVarRW(BarGUI, "Emission Delay:", TW_TYPE_FLOAT, &CURRENT_EMISSION, "min=0.0f max=10.0f step=0.01f");
	TwAddVarRW(BarGUI, "Lifetime:", TW_TYPE_FLOAT, &CURRENT_LIFETIME, "min=0.0f max=5.0f step=0.01f");
	TwAddVarRW(BarGUI, "Repeat:", TW_TYPE_BOOLCPP, &CURRENT_REPEAT, "");
	TwAddVarRW(BarGUI, "Scale X:", TW_TYPE_FLOAT, &CURRENT_SCALE.x, "min=0.05f max=5.0f step=0.05f");
	TwAddVarRW(BarGUI, "Scale Y:", TW_TYPE_FLOAT, &CURRENT_SCALE.y, "min=0.05f max=5.0f step=0.05f");
	TwAddVarRW(BarGUI, "Direction X:", TW_TYPE_FLOAT, &CURRENT_ROT.x, "min=-1.0f max=1.0f step=0.05f");
	TwAddVarRW(BarGUI, "Direction Y:", TW_TYPE_FLOAT, &CURRENT_ROT.y, "min=-1.0f max=1.0f step=0.05f");
	TwAddVarRW(BarGUI, "Direction Z:", TW_TYPE_FLOAT, &CURRENT_ROT.z, "min=-1.0f max=1.0f step=0.05f");
	TwAddVarRW(BarGUI, "Force:", TW_TYPE_FLOAT, &CURRENT_FORCE, "min=-10.0f max=10.0f step=0.01");
	TwAddVarRW(BarGUI, "Drag:", TW_TYPE_FLOAT, &CURRENT_DRAG, "min=0.0f max=10.0f step=0.01");
	//TwAddVarRW(BarGUI, "Direction", TW_TYPE_DIR3F, &CURRENT_ROT, "min=-1.0f max=1.0f step=0.05f");
	TwAddVarRW(BarGUI, "Gravity:", TW_TYPE_FLOAT, &CURRENT_GRAVITY, "min=-100.0f max=100.0f step=0.05f");
	TwAddVarRW(BarGUI, "Show Direction", TW_TYPE_BOOLCPP, &RENDER_DIR, "");
	TwAddVarRO(BarGUI, "Texture:", TW_TYPE_INT16, &CURRENT_TEXTURE, "");
	TwAddButton(BarGUI, "Name:", NULL, NULL, CURRENT_LABEL.c_str ());
	
	
	TwAddButton(BarControls, "Export", Export, NULL, " label='Export Particle System' ");
	TwAddButton(BarControls, "Import", Import, NULL, " label='Import Particle System' ");
	TwAddButton(BarControls, "Rebuild", Rebuild, NULL, " label='Rebuild Particle System' key=r");
	TwAddButton(BarControls, "Pause/Play", PausePlay, NULL, " label='Pause/Play' key=space");

	SetLabel ();
}

void InitializeCamera ()
{
	camera.Initialize (glm::vec3(0.0f, 4.0f, -8.0f), glm::vec3(0.0f, 0.0f, -1.0f), width, height);
	View	   = camera.GetView ();
	Projection = camera.GetProj ();
	Ortho	   = camera.GetOrtho ();
}

void CreateShaders()
{
	//Create and load shaders (Name GS "none" to disable GS)
	program = LoadShaders("vertex.glsl", "none", "fragment.glsl");
	ps_program = LoadShaders("particle_vs.glsl", "particle_gs.glsl", "particle_fs.glsl");
	ps_lprogram = LoadShaders("particle_lvs.glsl", "none", "particle_lfs.glsl");

	MatrixID = glGetUniformLocation (program, "MVP");
	ps_MatrixID = glGetUniformLocation(ps_program, "MVP");
	ps_CamID = glGetUniformLocation(ps_program, "cam");
	ps_SizeID = glGetUniformLocation(ps_program, "size");
	ps_lMatrixID = glGetUniformLocation(ps_lprogram, "MVP");

	glClearColor (0.2f, 0.2f, 0.2f, 0.0f);
}

void CreateObjects()
{
	//Lists all files inside of Data folder with the .png extension in a std::vector
	texturenames = ListFiles("Data/Textures/*", ".png");

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

	temp.width = 0.20000f;
	temp.height = 0.200000f;
	temp.lifetime = 1.0f;
	temp.maxparticles = 100;
	temp.emission = 0.1f;
	temp.force = 5.0f;
	temp.drag = 0.0f;
	temp.gravity = 1.0f; //1.0f = earth grav, 0.5f = half earth grav
	temp.continuous = true;
	temp.omni = false;

	CURRENT_SCALE.x = temp.width;
	CURRENT_SCALE.y = temp.height;
	CURRENT_FORCE = temp.force;
	CURRENT_DRAG = temp.drag;
	CURRENT_GRAVITY = temp.gravity;
	CURRENT_VTXCOUNT = temp.maxparticles;
	CURRENT_VTXCOUNT_DIFF = temp.maxparticles;
	CURRENT_EMISSION = temp.emission;
	CURRENT_EMISSION_DIFF = temp.emission;
	CURRENT_LIFETIME = temp.lifetime;

	arrow	= new Object("Data/OBJ/arrow.obj", &wire_tex, glm::vec3 (0.0f, 0.0f, 0.0f), program, false);
	sphere	= new Object("Data/OBJ/sphere.obj", &wire_tex, glm::vec3(0.0f, 0.0f, 0.0f), program, false);
	plane	= new Object("Data/OBJ/plane.obj", &wire_tex, glm::vec3 (0.0f, 0.0f, 0.0f), program, false);

	ui_particle = new Object("Data/OBJ/particle.obj", &texturedata[CURRENT_TEXTURE], glm::vec3 (0.0f, 0.0f, 0.0f), program, true);
	ps			= new ParticleSystem(&temp,			  &texturedata[CURRENT_TEXTURE], glm::vec3 (0.0f, 0.0f, 0.0f), ps_program, ps_lprogram);

	ui_particle->Rescale (glm::vec3 (0.125f, 0.2f, 1.0f));
	ui_particle->Translate (glm::vec3 (5.8f, 0.3f, 0.0f));

	//Initial rot (direction) values
	CURRENT_ROT = glm::vec3(1.0f, 0.0f, 0.0f);

	//Rotate arrow once with direction
	arrow->Rotate(CURRENT_ROT);
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

	CURRENT_ROT = { dir.x, dir.y, dir.z };

	return dir;
}

void Update (double deltaTime)
{
	if (input_current > 0.0f)
	{
		input_current = input_current - (float) deltaTime;
	}

/*	if (GetAsyncKeyState(VK_SPACE) & 0x8000)  //the button is being held currently
	{
		rot = GetInputDir(deltaTime);
		arrow->Rotate(rot);
	}
	*/

	if (GetAsyncKeyState (0x54) & 0x8000 && input_current <= 0.0f )
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

		ps->Retexture (&texturedata[CURRENT_TEXTURE]);
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

	if (CURRENT_VTXCOUNT != CURRENT_VTXCOUNT_DIFF)
	{
		Rebuild((void*)0);
	}

	if (CURRENT_EMISSION != CURRENT_EMISSION_DIFF)
	{
		Rebuild((void*)0);
	}

	CURRENT_VTXCOUNT_DIFF = CURRENT_VTXCOUNT;
	CURRENT_EMISSION_DIFF = CURRENT_EMISSION;

	CURRENT_ROT = glm::clamp(CURRENT_ROT, -1.0f, 1.0f);

	arrow->Rotate(glm::vec3(CURRENT_ROT.x, -CURRENT_ROT.y, CURRENT_ROT.z));
	
	sphere		->Update();
	arrow		->Update();	//updates model matrix (T * R * S compute)
	ui_particle	->Update ();

	//Update shader variables
	CameraPos = camera.GetPos();

	//Update temp with new values
	temp.dir		= CURRENT_ROT;
	temp.width		= CURRENT_SCALE.x;
	temp.height		= CURRENT_SCALE.y;
	temp.force		= CURRENT_FORCE;
	temp.drag		= CURRENT_DRAG;
	temp.gravity	= CURRENT_GRAVITY;
	temp.emission	= CURRENT_EMISSION;
	temp.lifetime	= CURRENT_LIFETIME;
	temp.continuous = CURRENT_REPEAT;
	temp.omni		= !arrow->IsActive();

	ps->Update(deltaTime, &temp, camera.GetPos());
	CURRENT_ACTIVE = ps->GetActiveParticles();

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

	//Enable this and comment out ps->Render() to render lightning between points.
	//glUseProgram(ps_lprogram);
	//glUniformMatrix4fv(ps_lMatrixID, 1, GL_FALSE, &VP[0][0]);
	//ps->RenderLightning();
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
				
				wss << "FPS: " << CURRENT_FPS;
				
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
		0,								  // disable depth buffer
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