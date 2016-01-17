#include "object.h"

Object::Object () { }

Object::Object (const char* filename, TextureData* textureinfo, glm::vec3 position, GLuint shader, bool textured)
{
	vertexbuffer = 0;
	colorbuffer = 0;

	vtxpos = 0;
	vtxcol = 0;
	vtxuv  = 0;

	this->m_filename = filename;
	this->m_textureinfo = textureinfo;
	this->m_position = position;
	this->m_shader	 = shader;
	this->m_textured = textured;

	Initialize ();
}

Object::~Object () { }

void Object::Initialize ()
{
	//Load .obj and initialize std::vectors
	bool result = LoadOBJ (m_filename);

	//Load texture
	glGenTextures (1, &texture);
	glBindTexture (GL_TEXTURE_2D, texture);
	unsigned char* image = SOIL_load_image(
		m_textureinfo->texturename,
		&m_textureinfo->width, 
		&m_textureinfo->height, 
		0, SOIL_LOAD_RGBA);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, m_textureinfo->width, m_textureinfo->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data (image);

	//Shader
	glUniform1i (glGetUniformLocation (m_shader, "tex"), 0);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	//Initialize model matrix
	Model = glm::mat4 (1.0f);


	//Generate buffers
	glGenBuffers (1, &vertexbuffer);
	glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData (GL_ARRAY_BUFFER, m_vertices.size () * sizeof(glm::vec3), &m_vertices[0], GL_STATIC_DRAW);

	glGenBuffers (1, &colorbuffer);
	glBindBuffer (GL_ARRAY_BUFFER, colorbuffer);
	glBufferData (GL_ARRAY_BUFFER, m_colors.size () * sizeof(glm::vec3), &m_colors[0], GL_STATIC_DRAW);

	glGenBuffers (1, &uvbuffer);
	glBindBuffer (GL_ARRAY_BUFFER, uvbuffer);
	glBufferData (GL_ARRAY_BUFFER, m_uvs.size () * sizeof(glm::vec2), &m_uvs[0], GL_STATIC_DRAW);

	vtxpos = glGetAttribLocation (m_shader, "vertex_position");
	vtxcol = glGetAttribLocation (m_shader, "vertex_color");
	vtxuv  = glGetAttribLocation (m_shader, "vertex_uv");

	m_active = true;
}

void Object::Rebuild (TextureData* textureinfo)
{
	texture = 0;

	this->m_textureinfo = textureinfo;

	glGenTextures (1, &texture);
	glBindTexture (GL_TEXTURE_2D, texture);
	unsigned char* image = SOIL_load_image (
		m_textureinfo->texturename,
		&m_textureinfo->width,
		&m_textureinfo->height,
		0, SOIL_LOAD_RGBA);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, m_textureinfo->width, m_textureinfo->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data (image);
	glUniform1i (glGetUniformLocation (m_shader, "tex"), 0);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


void Object::Update ()
{
	Model = glm::mat4 (1.0f);

	glm::mat4 Translation	= glm::translate (Model, m_position);
	glm::mat4 Rotation		= glm::lookAt	 (m_position, m_rotation, m_up);
	glm::mat4 Scale			= glm::scale	 (Model, m_scale);
	
	//"Correct" for objects
	if (m_textured)
	{
		Model = Scale * Translation;
	}

	//"Correct" for UI objects only
	else
	{
		Model = Translation * Rotation * Scale;
	}
	return;
}

void Object::Render ()
{
	glUseProgram (m_shader);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Texture
	glActiveTexture (GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i (glGetUniformLocation (m_shader, "tex"), 0);

	glEnableVertexAttribArray (vtxpos);
	glBindBuffer (GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer (vtxpos, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const GLvoid*) 0);

	glEnableVertexAttribArray (vtxcol);
	glBindBuffer (GL_ARRAY_BUFFER, colorbuffer);
	glVertexAttribPointer (vtxcol, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const GLvoid*) 0);

	glEnableVertexAttribArray (vtxuv);
	glBindBuffer (GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer (vtxuv, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (const GLvoid*) 0);

	//each individual object renders itself
	if (m_textured)
	{	//If we want it to show a texture, render with triangle strip
		glDrawArrays (GL_TRIANGLES, 0, m_vertices.size ());

	}
	else
	{	//Else, render lines
		glDrawArrays (GL_LINE_STRIP, 0, m_vertices.size ());
	}

	glDisableVertexAttribArray (vtxpos);
	glDisableVertexAttribArray (vtxcol);
	glDisableVertexAttribArray (vtxuv);
}

void Object::Translate (glm::vec3 position)
{
	this->m_position = position;
}

void Object::Rotate (glm::vec3 rotation)
{
	this->m_rotation = rotation;
}

void Object::Rescale (glm::vec3 scale)
{
	this->m_scale = scale;
}

glm::mat4 Object::GetModel ()
{
	return this->Model;
}

glm::vec3 Object::GetScale()
{
	return this->m_scale;
}

bool Object::IsActive()
{
	return this->m_active;
}

void Object::SetActive(bool active)
{
	//glDeleteBuffers(1, &vertexbuffer);
	//glDeleteBuffers(1, &colorbuffer);
	//glDeleteBuffers(1, &uvbuffer);

	m_active = active;
}

bool Object::LoadOBJ (const char* filepath)
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
		getchar ();
        return false;
    }

	GLuint index = 0;
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
            glm::vec2 uv	 = temp_uvs[uvIndex - 1];
			uv.y = -uv.y;
            glm::vec3 normal = temp_normals[normalIndex - 1];

            m_vertices.push_back (vertex);
            m_uvs.push_back (uv);
            m_normals.push_back (normal);
			m_colors.push_back (glm::vec3(1.0f, 0.0f, 1.0f));
        }
    }

    return true;
}