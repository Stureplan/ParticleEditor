#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "AntTweakBar.lib")
#pragma comment(lib, "Ws2_32.lib")

#define GLFW_CDECL

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
#include "events.h"

//Custom classes
#include "camera.h"
#include "object.h"
#include "Shaders.h"
#include "particlesystem.h"
#include "Win32InputBox.h"


#define GLEW_STATIC

using namespace glm;

//Window
int width = 1280;
int height = 720;
GLFWwindow* window;

//Window 2
int view = 1;
GLFWwindow* preview;

//Pointer variables
GLuint program = 0;
GLuint MatrixID;	//the in-shader matrix variable (think cbuffer)

GLuint ps_program = 0;
GLuint ps_MatrixID;
GLuint ps_CamID;
GLuint ps_SizeID;
GLuint ps_GlowID;

GLuint ps_lprogram = 0;
GLuint ps_lMatrixID;

//Matrices
glm::mat4 Model		 = glm::mat4(1.0f);
glm::mat4 View		 = glm::mat4(1.0f);
glm::mat4 Projection = glm::mat4(1.0f);
glm::mat4 Ortho		 = glm::mat4(1.0f);
glm::mat4 MVP		 = glm::mat4(1.0f);		//model * view * projection

//Player variables
unsigned int CURRENT_TEXTURE = 0;
int			 CURRENT_ACTIVE = 0;
std::string  CURRENT_LABEL;
bool		 RENDER_DIR = true;

//Particle System Data
ParticleSystemData CURRENT_PS;

int			CURRENT_VTXCOUNT_DIFF = 0;
float		CURRENT_EMISSION_DIFF = 0.0f;

std::vector<std::string> texturenames;
std::vector<TextureData> texturedata;

//World objects
Object* arrow;
Object* sphere;
Object* plane;
Object* ui_particle;
Object* ui_keys;
ParticleSystem* ps;
Camera camera;
TwBar* BarGUI;
TwBar* BarControls;

//Input variables
double mX, mY;

int CheckTexture(const char* texturename)
{
	int number = 0;

	for (unsigned int i = 0; i < texturenames.size(); i++)
	{
		if (texturenames.at(i) == texturename)
		{
			number = i;
		}
	}

	return number;
}

void InitializeCamera()
{
	camera.Initialize(glm::vec3(0.0f, 4.0f, -8.0f), glm::vec3(0.0f, 0.0f, 0.0f), width, height);
	View = camera.GetView();
	Projection = camera.GetProj();
	Ortho = camera.GetOrtho();
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

void CreateShaders()
{
	//Create and load shaders (Name GS "none" to disable GS)
	program = LoadShaders("vertex.glsl", "none", "fragment.glsl");
	ps_program = LoadShaders("particle_vs.glsl", "particle_gs.glsl", "particle_fs.glsl");
	ps_lprogram = LoadShaders("particle_lvs.glsl", "none", "particle_lfs.glsl");

	MatrixID = glGetUniformLocation(program, "MVP");
	ps_MatrixID = glGetUniformLocation(ps_program, "MVP");
	ps_CamID = glGetUniformLocation(ps_program, "cam");
	ps_SizeID = glGetUniformLocation(ps_program, "size");
	ps_GlowID = glGetUniformLocation(ps_program, "glow");
	ps_lMatrixID = glGetUniformLocation(ps_lprogram, "MVP");

	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
}

void StopPreview()
{
	view = 0;
}

void RetexturePreview(std::string PSysName)
{
	view = 1;

	glfwMakeContextCurrent(preview);
	glViewport(0, 0, 240, 240);


	//Read PS from PSysName
	std::ifstream file;
	file.open(PSysName, std::ios::binary | std::ios::in);
	if (!file.is_open())
	{
		//Error in opening the file
		BeepNoise(FAILURE);
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

	std::string name = "Data/Textures/";
	name.append(f);

	unsigned int x = 0;
	unsigned int y = 0;
	bool result = PNGSize(name.c_str(), x, y);
	if (result == false)
	{
		//Texture wasn't recognized
		BeepNoise(FAILURE);
		return;
	}

	TextureData exTD;
	exTD.width = x;
	exTD.height = y;
	exTD.texturename = name.c_str();

	ParticleSystem* ps2 = new ParticleSystem(&exPS, &exTD, glm::vec3(0, 0, 0), ps_program, ps_program);
	camera.Initialize(glm::vec3(0.0f, 4.0f, -8.0f), glm::vec3(0.0f, 0.0f, 0.0f), 240, 240);

	View = camera.GetView();
	Projection = camera.GetProj();
	Ortho = camera.GetOrtho();

	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	POINT pt;
	GetCursorPos(&pt);
	glfwSetWindowPos(preview, pt.x + 20, pt.y + 20);
	glfwShowWindow(preview);

	double deltaTime = 0;
	double lastTime = 0;
	double timer = 0;

	while (view != 0)
	{
		deltaTime = glfwGetTime() - lastTime;
		lastTime = glfwGetTime();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDepthMask(GL_FALSE);


		//Update
		ps2->Update(deltaTime, &exPS, camera.GetPos());

		View = camera.GetView();

		glUseProgram(ps_program);
		glm::mat4 VP = glm::mat4(1.0f);
		VP = Projection * View;
		glUniformMatrix4fv(ps_MatrixID, 1, GL_FALSE, &VP[0][0]);
		glUniform3fv(ps_CamID, 1, glm::value_ptr(camera.GetPos()));
		glUniform2fv(ps_SizeID, 1, glm::value_ptr(glm::vec2(exPS.width, exPS.height)));
		glUniform1i(ps_GlowID, exPS.glow);
		ps2->Render();
		
			
		// Swap the screen buffers
		glfwMakeContextCurrent(preview);

		glfwSwapBuffers(preview);
		glfwPollEvents();

		POINT pt;
		GetCursorPos(&pt);
		glfwSetWindowPos(preview, pt.x + 20, pt.y + 20);
	}
	
	delete ps2;
	glfwHideWindow(preview);
	glfwMakeContextCurrent(window);
}

void TW_CALL Export(void *clientData)
{
	std::string fResult;

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if (SUCCEEDED(hr))
	{
		//Windows File opening dialog system
		IFileSaveDialog* pFile;
		
		//What files to display
		LPCWSTR ps = L".ps";
		COMDLG_FILTERSPEC rgSpec = { ps, L"*.ps" };

		//Create instance
		hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&pFile));
		
		if (SUCCEEDED(hr))
		{
			TCHAR NPath[MAX_PATH];
			TCHAR AddedFolder[MAX_PATH] = L"\\Exports";
			TCHAR TotalPath[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, NPath);


			_stprintf(TotalPath, _T("%s%s"), NPath, AddedFolder);

			IShellItem* pFolder;
			hr = SHCreateItemFromParsingName(TotalPath, NULL, IID_PPV_ARGS(&pFolder));

			//Set attributes
			pFile->SetDefaultExtension(L"ps");
			pFile->SetFolder(pFolder);
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
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &fPath);

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
		BeepNoise(FAILURE);
		int msg = MessageBox(
			NULL,
			L"No file specified! Export canceled.",
			L"Error",
			MB_ICONERROR | MB_OK);
		return;
	}

	//Get texture name (minus directories)
	std::string temp_tex = texturenames.at(CURRENT_TEXTURE);
	temp_tex.erase(0, 14);
	const char* texture = temp_tex.c_str();

	//Get particle system
	ParticleSystemData* ps_temp = ps->GetPSData();

	//If the active particles is way different from maxparticles, give user a warning
	//and return from filesaving
	float amount;
	float result;

	amount = (float)ps_temp->maxparticles - (float)ps->GetActiveParticles();
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
	
	//Header size and texture size
	int texturesize;
	int totalsize;

	//Texturesize is how long the string is * bytes
	texturesize = strlen(texture) * sizeof(const char);
	totalsize = texturesize;				//texturename
	totalsize += sizeof(ParticleSystemData);

	//Opens file
	std::ofstream file;
	file.open(fResult, std::ios::binary | std::ios::out);
	
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
	file.write(reinterpret_cast<char*>(&ps_temp->continuous), sizeof(int));
	file.write(reinterpret_cast<char*>(&ps_temp->omni), sizeof(int));
	file.write(reinterpret_cast<char*>(&ps_temp->seed), sizeof(int));
	file.write(reinterpret_cast<char*>(&ps_temp->spread), sizeof(float));
	file.write(reinterpret_cast<char*>(&ps_temp->glow), sizeof(int));
	file.write(reinterpret_cast<char*>(&ps_temp->scaleDir), sizeof(int));
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
			//Event handling
			IFileDialogEvents* pfde = NULL;
			DWORD dwCookie = 0;
			CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));


			hr = pFile->Advise(pfde, &dwCookie);



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
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &fPath);
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
		BeepNoise(FAILURE);
		int msg = MessageBox(
			NULL,
			L"No file selected! Import canceled.",
			L"Error",
			MB_ICONERROR | MB_OK);
		return;
	}
	view = 0;
	InitializeCamera();

	//Test reading file
	std::ifstream file;
	file.open(fResult, std::ios::binary | std::ios::in);
	if (!file.is_open())
	{
		//Error in opening the file
		BeepNoise(FAILURE);
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
	//file.read((char*)&variable, sizeof(vartype));
	file.close();


	//File found and filename is fResult
	BeepNoise(SUCCESS);

	CURRENT_PS = exPS;
	CURRENT_VTXCOUNT_DIFF = 0;
	CURRENT_EMISSION_DIFF = exPS.emission;

	std::string name = "Data/Textures/";
	name.append(f);

	unsigned int x = 0;
	unsigned int y = 0;
	bool result = PNGSize(name.c_str(), x, y);
	if (result == false)
	{
		//Texture wasn't recognized
		BeepNoise(FAILURE);
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
	CURRENT_TEXTURE = CheckTexture(exTD.texturename);
	SetLabel();

	arrow->SetActive(!exPS.omni);
	ui_particle->Rebuild(&exTD);
	ps->Retexture(&exTD);
	ps->Rebuild(&exPS);
}

void TW_CALL Rebuild(void *clientData)
{
	ps->Rebuild(&CURRENT_PS);
}

void TW_CALL Randomize(void *clientData)
{
	//Randomize emission delay
	CURRENT_PS.emission = RandFloat(0.0f, 0.2f);
	int explosion = RandInt(0, 1);
	if (explosion == 1)
	{
		CURRENT_PS.emission = 0.0f;

		//Randomize maxparticles
		CURRENT_PS.maxparticles = RandInt(1, 2000);
	}
	else
	{
		//Randomize maxparticles
		CURRENT_PS.maxparticles = RandInt(1, 250);
	}


	//Randomize lifetime
	CURRENT_PS.lifetime = RandFloat(0.1f, 2.0f);

	//Randomize scale
	CURRENT_PS.width = RandFloat(0.1f, 0.5f);
	CURRENT_PS.height = RandFloat(0.1f, 0.5f);

	//Randomize direction
	CURRENT_PS.dir.x = RandFloat(-1.0f, 1.0f);
	CURRENT_PS.dir.y = RandFloat(-1.0f, 1.0f);
	CURRENT_PS.dir.z = RandFloat(-1.0f, 1.0f);

	//Randomize force
	CURRENT_PS.force = RandFloat(0.0f, 10.0f);

	//Randomize gravity
	CURRENT_PS.gravity = RandFloat(-1.0f, 1.0f);

	//Randomize texture
	CURRENT_TEXTURE = RandInt(0, texturenames.size()-1);

	//Randomize seed
	CURRENT_PS.seed = RandInt(0, 1000);

	//Randomize spread
	CURRENT_PS.spread = RandFloat(0.0f, 1.0f);
	
	//Glow false
	CURRENT_PS.glow = 0;

	//Don't scale
	CURRENT_PS.scaleDir = 0;


	ps->Retexture(&texturedata[CURRENT_TEXTURE]);
	ui_particle->Rebuild(&texturedata[CURRENT_TEXTURE]);
	ps->Rebuild(&CURRENT_PS);
}

void TW_CALL RandomizeSeed(void* clientData)
{
	CURRENT_PS.seed = RandInt(0, 1000);
	ps->Rebuild(&CURRENT_PS);
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

	TwInit(TW_OPENGL_CORE, NULL);
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
	TwDefine(" Controls size='250 320'");
	TwDefine(" Controls refresh=0.1");
	TwDefine(" Controls movable=false");
	TwDefine(" Controls resizable=false");
	TwDefine(" Controls fontresizable=false");

	TwAddVarRW(BarGUI, "Max Particles:", TW_TYPE_INT16, &CURRENT_PS.maxparticles, "label='Max Particles:' min=1 max=2000 ");
	TwAddVarRO(BarGUI, "Unused", TW_TYPE_INT16, &CURRENT_ACTIVE, "label='Active Particles:' ");
	TwAddVarRW(BarGUI, "Emission Delay:", TW_TYPE_FLOAT, &CURRENT_PS.emission, "min=0.0f max=10.0f step=0.01f");
	TwAddVarRW(BarGUI, "Lifetime:", TW_TYPE_FLOAT, &CURRENT_PS.lifetime, "min=0.0f max=50.0f step=0.01f");
	TwAddVarRW(BarGUI, "Repeat:", TW_TYPE_BOOLCPP, &CURRENT_PS.continuous, "");
	TwAddVarRW(BarGUI, "Scale X:", TW_TYPE_FLOAT, &CURRENT_PS.width, "min=0.05f max=20.0f step=0.01f");
	TwAddVarRW(BarGUI, "Scale Y:", TW_TYPE_FLOAT, &CURRENT_PS.height, "min=0.05f max=10.0f step=0.01f");
	TwAddVarRW(BarGUI, "Direction X:", TW_TYPE_FLOAT, &CURRENT_PS.dir.x, "min=-1.0f max=1.0f step=0.05f");
	TwAddVarRW(BarGUI, "Direction Y:", TW_TYPE_FLOAT, &CURRENT_PS.dir.y, "min=-1.0f max=1.0f step=0.05f");
	TwAddVarRW(BarGUI, "Direction Z:", TW_TYPE_FLOAT, &CURRENT_PS.dir.z, "min=-1.0f max=1.0f step=0.05f");
	TwAddVarRW(BarGUI, "Force:", TW_TYPE_FLOAT, &CURRENT_PS.force, "min=-10.0f max=10.0f step=0.01");
	TwAddVarRW(BarGUI, "Drag:", TW_TYPE_FLOAT, &CURRENT_PS.drag, "min=0.0f max=10.0f step=0.01");
	TwAddVarRW(BarGUI, "Gravity:", TW_TYPE_FLOAT, &CURRENT_PS.gravity, "min=-100.0f max=100.0f step=0.05f");
	TwAddVarRW(BarGUI, "Show Direction", TW_TYPE_BOOLCPP, &RENDER_DIR, "");	
	TwAddVarRW(BarGUI, "Seed Number:", TW_TYPE_INT16, &CURRENT_PS.seed, "min=0 max=1000");
	TwAddButton(BarGUI, "Randomize Seed", RandomizeSeed, NULL, " label='Randomize Seed' ");
	TwAddVarRW(BarGUI, "Spread:", TW_TYPE_FLOAT, &CURRENT_PS.spread, "min=0.0f max=1.0f step=0.01f");
	TwAddVarRW(BarGUI, "Use Glow:", TW_TYPE_BOOLCPP, &CURRENT_PS.glow, "");
	TwAddVarRW(BarGUI, "Scale Dir:", TW_TYPE_INT32, &CURRENT_PS.scaleDir, "min=-1 max=1");
	TwAddVarRO(BarGUI, "Texture:", TW_TYPE_INT16, &CURRENT_TEXTURE, "");
	TwAddButton(BarGUI, "Name:", NULL, NULL, CURRENT_LABEL.c_str ());
	
	
	TwAddButton(BarControls, "Export", Export, NULL, " label='Export Particle System' ");
	TwAddButton(BarControls, "Import", Import, NULL, " label='Import Particle System' ");
	TwAddButton(BarControls, "Rebuild", Rebuild, NULL, " label='Rebuild Particle System' key=r");
	TwAddButton(BarControls, "Randomize", Randomize, NULL, " label='Randomize Particle System' key=e");
	TwAddButton(BarControls, "Pause/Play", PausePlay, NULL, " label='Pause/Play' key=space");

	
	SetLabel ();
}




void CreateObjects()
{
	//Lists all files inside of Data folder with the .png extension in a std::vector
	texturenames = ListFiles("Data/Textures/*", ".png");

	for (unsigned int i = 0; i < texturenames.size (); i++)
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

	TextureData keys_tex;
	keys_tex.texturename = "Data/OBJ/Keybindings.png";
	unsigned int x, y;
	PNGSize(keys_tex.texturename, x, y);

	//Default Particle System
	CURRENT_PS.width = 0.20000f;
	CURRENT_PS.height = 0.200000f;
	CURRENT_PS.lifetime = 1.0f;
	CURRENT_PS.maxparticles = 100;
	CURRENT_PS.emission = 0.1f;
	CURRENT_PS.force = 5.0f;
	CURRENT_PS.drag = 0.0f;
	CURRENT_PS.gravity = 1.0f; //1.0f = earth grav, 0.5f = half earth grav
	CURRENT_PS.seed = 0;
	CURRENT_PS.continuous = true;
	CURRENT_PS.omni = false;
	CURRENT_PS.spread = 0.0f;
	CURRENT_PS.glow = false;
	CURRENT_PS.scaleDir = 0;

	CURRENT_VTXCOUNT_DIFF = CURRENT_PS.maxparticles;
	CURRENT_EMISSION_DIFF = CURRENT_PS.emission;

	arrow	= new Object("Data/OBJ/arrow.obj", &wire_tex, glm::vec3 (0.0f, 0.0f, 0.0f), program, false);
	sphere	= new Object("Data/OBJ/sphere.obj", &wire_tex, glm::vec3(0.0f, 0.0f, 0.0f), program, false);
	plane	= new Object("Data/OBJ/plane.obj", &wire_tex, glm::vec3 (0.0f, 0.0f, 0.0f), program, false);

	ui_particle = new Object("Data/OBJ/particle.obj", &texturedata[CURRENT_TEXTURE], glm::vec3 (0.0f, 0.0f, 0.0f), program, true);
	ui_keys		= new Object("Data/OBJ/particle.obj", &keys_tex, glm::vec3(0, 0, 0), program, true);
	ps			= new ParticleSystem(&CURRENT_PS,	  &texturedata[CURRENT_TEXTURE], glm::vec3 (0.0f, 0.0f, 0.0f), ps_program, ps_lprogram);

	ui_particle->Rescale (glm::vec3 (0.125f, 0.2f, 1.0f));
	ui_particle->Translate (glm::vec3 (5.7f, -0.8f, 0.0f));

	ui_keys->Rescale(glm::vec3(0.258f, 0.466f, 1.0f));
	ui_keys->Translate(glm::vec3(-3.3f, 0.9f, 0.0f));

	//Initial rot (direction) values
	CURRENT_PS.dir = glm::vec3(1.0f, 0.0f, 0.0f);

	//Rotate arrow once with direction
	arrow->Rotate(CURRENT_PS.dir);
}

void Update (double deltaTime)
{
	if (CURRENT_PS.maxparticles != CURRENT_VTXCOUNT_DIFF)
	{
		Rebuild((void*)0);
	}

	if (CURRENT_PS.emission != CURRENT_EMISSION_DIFF)
	{
		Rebuild((void*)0);
	}

	CURRENT_VTXCOUNT_DIFF = CURRENT_PS.maxparticles;
	CURRENT_EMISSION_DIFF = CURRENT_PS.emission;

	CURRENT_PS.dir = glm::clamp(CURRENT_PS.dir, -1.0f, 1.0f);

	arrow->Rotate(glm::vec3(CURRENT_PS.dir.x, -CURRENT_PS.dir.y, CURRENT_PS.dir.z));
	
	sphere		->Update();
	arrow		->Update();	//updates model matrix (T * R * S compute)
	ui_particle	->Update();
	ui_keys		->Update();

	//Update temp with new values
	CURRENT_PS.omni = !arrow->IsActive();

	ps->Update(deltaTime, &CURRENT_PS, camera.GetPos());
	CURRENT_ACTIVE = ps->GetActiveParticles();

	glm::vec3 pos = camera.GetPos();
	glm::vec3 dir = pos;
	dir.z = dir.z + 1.0f;
	dir.y = dir.y - 0.5f;

	camera.SetPos (pos);
	camera.SetDir (dir);
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
	glUniform3fv(ps_CamID, 1, glm::value_ptr(camera.GetPos()));
	glUniform2fv(ps_SizeID, 1, glm::value_ptr(glm::vec2(CURRENT_PS.width, CURRENT_PS.height)));
	glUniform1i(ps_GlowID, CURRENT_PS.glow);
	ps->Render();

	//Enable this and comment out ps->Render() to render lightning between points.
	//glUseProgram(ps_lprogram);
	//glUniformMatrix4fv(ps_lMatrixID, 1, GL_FALSE, &VP[0][0]);
	//ps->RenderLightning();
	//	--- End of PS Object

	// ----------- Render GUI -------- 
	TwDraw();
	glDisable (GL_DEPTH_TEST);
	glUseProgram (program);
	Model = glm::mat4 (1.0f);
	Model = ui_particle->GetModel ();

	MVP = Model;
	glUniformMatrix4fv (MatrixID, 1, GL_FALSE, &MVP[0][0]);
	ui_particle->Render ();

	//Render keybindings
	Model = glm::mat4(1.0f);
	Model = ui_keys->GetModel();

	MVP = Model;
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	ui_keys->Render();



}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//TW Key codes
	switch (key)
	{
	case GLFW_KEY_E:
		key = TW_KEY_E;
		break;
	case GLFW_KEY_R:
		key = TW_KEY_R;
		break;
	case GLFW_KEY_ENTER:
		key = TW_KEY_RETURN;
		break;
	case GLFW_KEY_KP_ENTER:
		key = TW_KEY_RETURN;
		break;
	case GLFW_KEY_KP_0:
		key = 0x30;
		break;
	case GLFW_KEY_KP_1:
		key = 0x31;
		break;
	case GLFW_KEY_KP_2:
		key = 0x32;
		break;
	case GLFW_KEY_KP_3:
		key = 0x33;
		break;
	case GLFW_KEY_KP_4:
		key = 0x34;
		break;
	case GLFW_KEY_KP_5:
		key = 0x35;
		break;
	case GLFW_KEY_KP_6:
		key = 0x36;
		break;
	case GLFW_KEY_KP_7:
		key = 0x37;
		break;
	case GLFW_KEY_KP_8:
		key = 0x38;
		break;
	case GLFW_KEY_KP_9:
		key = 0x39;
		break;
	case GLFW_KEY_MINUS:
		key = '\-';
		break;
	case GLFW_KEY_KP_SUBTRACT:
		key = '\-';
		break;
	case GLFW_KEY_SLASH:
		key = '\-';
		break;
	case GLFW_KEY_BACKSPACE:
		key = TW_KEY_BACKSPACE;
	case GLFW_KEY_COMMA:
		key = TW_KEY_PERIOD;
	case GLFW_KEY_KP_DECIMAL:
		key = TW_KEY_PERIOD;
		break;
	}

	if (action == GLFW_PRESS) TwKeyPressed(key, TW_KMOD_NONE);

	//Regular key codes (non-TW related)
	if (key == GLFW_KEY_T && action == GLFW_PRESS)
	{
		if (CURRENT_TEXTURE + 1 >= texturedata.size())
		{
			CURRENT_TEXTURE = 0;
		}
		else
		{
			CURRENT_TEXTURE++;
		}

		ps->Retexture(&texturedata[CURRENT_TEXTURE]);
		ui_particle->Rebuild(&texturedata[CURRENT_TEXTURE]);
		SetLabel();
	}

	if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
	{
		bool active = arrow->IsActive();
		arrow->SetActive(!active);
		sphere->SetActive(active);
	}

}

void mouse_pos(GLFWwindow* window, double x, double y)
{
	TwMouseMotion(x, y);
}

void mouse_click(GLFWwindow* window, int button, int action, int mods)
{
	TwMouseAction ac;
	TwMouseButtonID id;

	switch (button)
	{
	case GLFW_MOUSE_BUTTON_1:
		id = TW_MOUSE_LEFT;
		break;
	case GLFW_MOUSE_BUTTON_2:
		id = TW_MOUSE_RIGHT;
		break;
	case GLFW_MOUSE_BUTTON_3:
		id = TW_MOUSE_MIDDLE;
		break;
	case GLFW_MOUSE_BUTTON_4:
		id = (TwMouseButtonID)0;
		break;
	case GLFW_MOUSE_BUTTON_5:
		id = (TwMouseButtonID)0;
		break;
	default:
		break;
	}
	
	switch (action)
	{
	case GLFW_PRESS:
		ac = TW_MOUSE_PRESSED;
		break;
	case GLFW_RELEASE:
		ac = TW_MOUSE_RELEASED;
		break;
	default:
		break;
	}

	TwMouseButton(ac, id);
	

}

int main(void)
{
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 
	glfwInit();
	window = glfwCreateWindow(1280, 720, "Simple example", NULL, NULL);
	glViewport(0, 0, width, height);

	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();
	glfwSwapInterval(0);

	// Window 2---
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_FOCUSED, GL_TRUE);
	glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	glfwWindowHint(GLFW_FLOATING, GL_TRUE);
	preview = glfwCreateWindow(240, 240, "Preview", nullptr, window);
	glfwHideWindow(preview);
	// -----------


	InitializeCamera();
	CreateShaders();
	CreateObjects();
	InitializeGUI();

	// Set GLFW event callbacks
	// - Directly redirect GLFW mouse button events to AntTweakBar
	glfwSetMouseButtonCallback(window, mouse_click);
	glfwSetCursorPosCallback(window, mouse_pos);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCharCallback(window, (GLFWcharfun)TwEventCharGLFW);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	double deltaTime = 0;
	double lastTime = 0;
	double timer = 0;
	int frames = 0;
	int FPS = 0;

	while (!glfwGetKey(window, GLFW_KEY_ESCAPE) && !glfwWindowShouldClose(window))
	{
		deltaTime = glfwGetTime() - lastTime;
		lastTime = glfwGetTime();

		timer += deltaTime;

		frames++;

		if (timer > 1.0)
		{
			FPS = frames;
			timer = 0;
			frames = 0;
		}

		glfwSetWindowTitle(window, std::to_string(FPS).c_str());

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

			glm::mat4 cam = camera.GetView();

			//relative to upper-left corner (0, 0)
			glfwGetCursorPos(window, &mX, &mY);
			mX = mX - width / 2;
			
			double angleX = 0.0f;
			angleX -= mX;
			
			cam = glm::rotate(cam, (float)angleX * (float)deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));

			glfwSetCursorPos(window, width / 2, height / 2);

			camera.SetView(cam);
		}

		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		Update(deltaTime);
		Render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}