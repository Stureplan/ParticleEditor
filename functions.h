#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <fstream>
#include <vector>
#include <winsock.h>
#include <random>

enum NOISE
{
	SUCCESS,
	FAILURE
};


void BeepNoise(NOISE type)
{
	switch (type)
	{
	case NOISE::SUCCESS:
		//Positive noise
		Beep(400, 50);
		Beep(500, 50);
		Beep(600, 50);
		Beep(700, 50);
		Beep(800, 50);
		Beep(900, 50);
		Beep(1000, 50);
		break;

	case NOISE::FAILURE:
		//Negative noise
		Beep(1000, 50);
		Beep(900, 50);
		Beep(800, 50);
		Beep(700, 50);
		Beep(600, 50);
		Beep(500, 50);
		Beep(400, 50);
		break;
	}
}

int RandInt(int a, int b)
{
	//Initiate random gen
	std::random_device rd;
	std::mt19937 mt(rd());

	std::uniform_int_distribution<int32_t> temp(a, b);

	return temp(mt);
}

float RandFloat(float a, float b)
{
	//Initiate random gen
	std::random_device rd;
	std::mt19937 mt(rd());

	std::uniform_real_distribution<float> temp(a, b);

	return temp(mt);
}

bool PNGSize(const char* fileName, unsigned int &x, unsigned int &y)
{
	std::ifstream file(fileName, std::ios_base::binary | std::ios_base::in);

	if (!file.is_open() || !file)
	{
		file.close();
		return false;
	}

	file.seekg(8, std::ios_base::cur);
	file.seekg(4, std::ios_base::cur);
	file.seekg(4, std::ios_base::cur);

	__int32 width, height;

	file.read((char*)&width, 4);
	file.read((char*)&height, 4);

	x = ntohl(width);
	y = ntohl(height);

	file.close();

	return true;
}

std::string WCHAR_TO_STRING(const wchar_t *wchar)
{
	std::string str = "";
	int index = 0;
	while (wchar[index] != 0)
	{
		str += (char)wchar[index];
		++index;
	}
	return str;
}

wchar_t* STRING_TO_WCHAR(const std::string &str)
{
	wchar_t wchar[260];
	unsigned int index = 0;
	while (index < str.size())
	{
		wchar[index] = (wchar_t)str[index];
		++index;
	}
	wchar[index] = 0;
	return wchar;
}

std::vector<std::string> ListFiles(std::string directoryName, std::string extensionName)
{
	directoryName.append(extensionName);
	
	WIN32_FIND_DATA FindFileData;
	wchar_t* FileName = STRING_TO_WCHAR(directoryName);
	HANDLE hFind = FindFirstFile(FileName, &FindFileData);

	std::vector<std::string> listFileNames;
	listFileNames.push_back(WCHAR_TO_STRING(FindFileData.cFileName));

	while (FindNextFile(hFind, &FindFileData))
	{
		listFileNames.push_back(WCHAR_TO_STRING(FindFileData.cFileName));
	}

	if (extensionName == ".png")
	{
		//Insert "Data/Textures" at the beginning of each string
		for (unsigned int i = 0; i < listFileNames.size(); i++)
		{
			listFileNames[i].insert(0, std::string("Data/Textures/"));
		}
	}
	else if (extensionName == ".ps")
	{
		//Insert "Exports/" at the beginning of each string
		for (unsigned int i = 0; i < listFileNames.size(); i++)
		{
			listFileNames[i].insert(0, std::string("Exports/"));
		}
	}


	return listFileNames;
}



#endif

//helper functions