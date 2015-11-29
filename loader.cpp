//#include "loader.h"

/*#include <glm/glm.hpp>

using namespace std;

class Loader
{
public:
	static bool loadOBJ (const char* filepath,
	std::vector<glm::vec3> &vertices,
	std::vector<glm::vec2> &uvs,
	std::vector<glm::vec3> &normals,
	std::vector<glm::vec3> &colors)
	{
		std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
		std::vector<glm::vec3> temp_vertices;
		std::vector<glm::vec2> temp_uvs;
		std::vector<glm::vec3> temp_normals;

		printf ("Loading OBJ: \n", filepath);

		FILE* file = fopen (filepath, "r");
		if (file == NULL)
		{
			printf ("Couldn't load file! \n");
			return false;
		}

		while (1)
		{
			char lineHeader[128];
			int res = fscanf (file, "%s", lineHeader);
			if (res == EOF)
			{
				break;
			}

			if (strcmp (lineHeader, "v") == 0)
			{
				glm::vec3 vertex;
				fscanf (file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				temp_vertices.push_back (vertex);
			}
			else if (strcmp (lineHeader, "vt") == 0)
			{
				glm::vec2 uv;
				fscanf (file, "%f %f\n", &uv.x, &uv.y);
				temp_uvs.push_back (uv);
			}
			else if (strcmp (lineHeader, "vn") == 0)
			{
				glm::vec3 normal;
				fscanf (file, "%f %f %f", &normal.x, &normal.y, &normal.z);
				temp_normals.push_back (normal);
			}
			else if (strcmp (lineHeader, "f") == 0)
			{
				std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				int matches = fscanf (file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
					&vertexIndex[0], &uvIndex[0], &normalIndex[0],
					&vertexIndex[1], &uvIndex[1], &normalIndex[1],
					&vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				if (matches != 9)
				{
					printf ("File can't be read\n");
					return false;
				}
				vertexIndices.push_back (vertexIndex[0]);
				vertexIndices.push_back (vertexIndex[1]);
				vertexIndices.push_back (vertexIndex[2]);
				uvIndices.push_back (uvIndex[0]);
				uvIndices.push_back (uvIndex[1]);
				uvIndices.push_back (uvIndex[2]);
				normalIndices.push_back (normalIndex[0]);
				normalIndices.push_back (normalIndex[1]);
				normalIndices.push_back (normalIndex[2]);
			}
			else
			{
				//If it's something unknown it's probably commented
				char buffer[1000];
				fgets (buffer, 1000, file);
			}
		}

		for (unsigned int i = 0; i < vertexIndices.size (); i += 3)
		{
			for (unsigned int j = 0; j < 3; j++)
			{
				unsigned int vertexIndex = vertexIndices[i + j];
				unsigned int uvIndex = uvIndices[i + j];
				unsigned int normalIndex = normalIndices[i + j];

				glm::vec3 vertex = temp_vertices[vertexIndex - 1];
				glm::vec2 uv = temp_uvs[uvIndex - 1];
				glm::vec3 normal = temp_normals[normalIndex - 1];

				vertices.push_back (vertex);
				uvs.push_back (uv);
				normals.push_back (normal);
				colors.push_back (vertex);
			}
		}

		return true;
	}
};
*/