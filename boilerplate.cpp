// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <time.h>
#include <fstream>

#include "glm/glm.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

// specify that we want the OpenGL core profile before including GLFW headers
//#include "middleware\glad\include\glad\glad.h"
#include "glad/glad.h"
#include <glfw/glfw3.h>
#include "camera.h"
#include "Box.h"
#include "MCNode.h"

#define STB_IMAGE_IMPLEMENTATION
    #include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
    #include "stb/stb_image_write.h"

#define PI 3.14159265359

using namespace std;
using namespace glm;

//Forward definitions
bool CheckGLErrors(string location);
void QueryGLVersion();
string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
vector<vec3> generateOffsets(vector<MCNode*>& nodevec, Box& myBox);
void createNewPoints(Box &box, vector<vec3>* vertices, vector<vec3>* normals, vector<unsigned int>* indices, vector<MCNode*>& nodevec, bool cap);
void generateBox(vector<vec3>* vertices, vector<vec3>* normals, vector<unsigned int>* indices, float sideLength, Box * box);
void createNosePoints(Box &box, vector<vec3>* vertices, vector<vec3>* normals, vector<unsigned int>* indices, vector<MCNode*>& nodevec, bool cap);

//Geometry information
vector<vec3> points, normals;
vector<unsigned int> indices;

//Cylinder buffers
vector<vec3> pointsCylinder, normalsCylinder;
vector<unsigned int> indicesCylinder;

Box *initialBox = new Box();

GLuint starTexture;

vector<MCNode*> nodePointers;
vec2 mousePos;
bool leftmousePressed = false;
bool rightmousePressed = false;
//int t = 0;
float g = 0.000981*2;
float offsetScale = 0.2f;
float scaleFactor = 1.f;

Camera* activeCamera;

//Camera cam = Camera(vec3(0, 0, -1), vec3(0, 0, 1));
Camera cam = Camera(vec3(0, 0, -1), vec3(0, 0, 20));
// Remember to start this at false for final submission and demo
bool animate = true;

GLFWwindow* window = 0;

mat4 winRatio = mat4(1.f);

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
    cout << "GLFW ERROR " << error << ":" << endl;
    cout << description << endl;
}

// handles keyboard input events
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
        
    /*if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
        createNewPoints(*initialBox, &points, &normals, &indices, nodePointers, false);
	}*/

    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        points.clear();
        normals.clear();
        indices.clear();
        delete initialBox;
        initialBox = new Box();
        generateBox(&points, &normals, &indices, 3.f, initialBox);
        for (int i = 0; i <15; i++)
        {
            createNewPoints(*initialBox, &points, &normals, &indices, nodePointers, (i==14)?true:false);
        }

        for (int i = 0; i < 10; i++)
        {
            createNosePoints(*initialBox, &points, &normals, &indices, nodePointers, (i == 9) ? true : false);
        }
    }

    if (key == GLFW_KEY_E && action == GLFW_PRESS){
    

        ofstream outputFile;
        char filepath[250];
        time_t timer;
        struct tm* tm_info;

        time(&timer);
        tm_info = localtime(&timer);

        strftime(filepath, 250, "spaceship%Y-%m-%d-%H-%M-%S.obj", tm_info);
        puts(filepath);

        outputFile.open(filepath);

        outputFile << "# Vertices" << endl;
        for (auto v : points)
        {
            memset(filepath, 0, sizeof(filepath));
            sprintf_s(filepath, "v %f %f %f", v.x, v.y, v.z);
            outputFile << filepath << endl;
        }

        outputFile << "# Vertex Normals" << endl;
        for (auto n : normals)
        {
            memset(filepath, 0, sizeof(filepath));
            sprintf_s(filepath, "vn %f %f %f", n.x, n.y, n.z);
            outputFile << filepath << endl;
        }

        outputFile << "# Faces" << endl;
        for (int i = 0; i < indices.size(); i+=3)
        {
            memset(filepath, 0, sizeof(filepath));
            sprintf_s(filepath, "f %d//%d %d//%d %d//%d", indices.at(i)+1, indices.at(i)+1, indices.at(i+1)+1, indices.at(i+1)+1, indices.at(i+2)+1, indices.at(i+2)+1);
            outputFile << filepath << endl;
        }
        outputFile.close();
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if( (action == GLFW_PRESS) || (action == GLFW_RELEASE) ){
		if(button == GLFW_MOUSE_BUTTON_LEFT)
			leftmousePressed = !leftmousePressed;
		else if(button == GLFW_MOUSE_BUTTON_RIGHT)
			rightmousePressed = !rightmousePressed;
	}
}

void mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	vec2 newPos = vec2(xpos/(double)vp[2], -ypos/(double)vp[3])*2.f - vec2(1.f);

	vec2 diff = newPos - mousePos;
	if(leftmousePressed){
		activeCamera->trackballRight(-diff.x);
		activeCamera->trackballUp(-diff.y);
	}
	else if(rightmousePressed){
		float zoomBase = (diff.y > 0) ? 1.f/2.f : 2.f;

		activeCamera->zoom(pow(zoomBase, abs(diff.y)));
	}

	mousePos = newPos;
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    scaleFactor += yoffset*(scaleFactor / (5));
}


void resizeCallback(GLFWwindow* window, int width, int height)
{
	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	glViewport(0, 0, width, height);

	float minDim = float(min(width, height));

	winRatio[0][0] = minDim/float(width);
	winRatio[1][1] = minDim/float(height);
}

//==========================================================================
// TUTORIAL STUFF


//vec2 and vec3 are part of the glm math library. 
//Include in your own project by putting the glm directory in your project, 
//and including glm/glm.hpp as I have at the top of the file.
//"using namespace glm;" will allow you to avoid writing everyting as glm::vec2

struct VertexBuffers{
	enum{ VERTICES=0, NORMALS, INDICES, COUNT};

	GLuint id[COUNT];
};


struct StarVertexBuffers {
    enum { POINTS = 0, NORMALS, UVS, INDICES, COUNT };
    GLuint id[COUNT];
};

//Describe the setup of the Vertex Array Object
bool initVAO(GLuint vao, const VertexBuffers& vbo)
{
	glBindVertexArray(vao);		//Set the active Vertex Array

	glEnableVertexAttribArray(0);		//Tell opengl you're using layout attribute 0 (For shader input)
	glBindBuffer( GL_ARRAY_BUFFER, vbo.id[VertexBuffers::VERTICES] );		//Set the active Vertex Buffer
	glVertexAttribPointer(
		0,				//Attribute
		3,				//Size # Components
		GL_FLOAT,	//Type
		GL_FALSE, 	//Normalized?
		sizeof(vec3),	//Stride
		(void*)0			//Offset
		);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo.id[VertexBuffers::NORMALS]);
	glVertexAttribPointer(
		1,				//Attribute
		3,				//Size # Components
		GL_FLOAT,	//Type
		GL_FALSE, 	//Normalized?
		sizeof(vec3),	//Stride
		(void*)0			//Offset
		);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.id[VertexBuffers::INDICES]);

	return !CheckGLErrors("initVAO");		//Check for errors in initialize
}

//Describe the setup of the Vertex Array Object
bool initVAOStars(GLuint vao, const StarVertexBuffers& vbo)
{
    glBindVertexArray(vao);		//Set the active Vertex Array

    glEnableVertexAttribArray(0);		//Tell opengl you're using layout attribute 0 (For shader input)
    glBindBuffer(GL_ARRAY_BUFFER, vbo.id[StarVertexBuffers::POINTS]);		//Set the active Vertex Buffer
    glVertexAttribPointer(
        0,				//Attribute
        3,				//Size # Components
        GL_FLOAT,	//Type
        GL_FALSE, 	//Normalized?
        sizeof(vec3),	//Stride
        (void*)0			//Offset
    );

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo.id[StarVertexBuffers::NORMALS]);
    glVertexAttribPointer(
        1,				//Attribute
        3,				//Size # Components
        GL_FLOAT,	//Type
        GL_FALSE, 	//Normalized?
        sizeof(vec3),	//Stride
        (void*)0			//Offset
    );

    glEnableVertexAttribArray(2);		//Tell opengl you're using layout attribute 1
    glBindBuffer(GL_ARRAY_BUFFER, vbo.id[StarVertexBuffers::UVS]);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(vec2),
        (void*)0
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.id[StarVertexBuffers::INDICES]);

    return !CheckGLErrors("initStarsVAO");		//Check for errors in initialize
}

//Loads buffers with data
bool loadBuffer(const VertexBuffers& vbo, 
				const vector<vec3>& points, 
				const vector<vec3> normals, 
				const vector<unsigned int>& indices)
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo.id[VertexBuffers::VERTICES]);
	glBufferData(
		GL_ARRAY_BUFFER,				//Which buffer you're loading too
		sizeof(vec3)*points.size(),		//Size of data in array (in bytes)
		&points[0],						//Start of array (&points[0] will give you pointer to start of vector)
		GL_STATIC_DRAW					//GL_DYNAMIC_DRAW if you're changing the data often
										//GL_STATIC_DRAW if you're changing seldomly
		);

	glBindBuffer(GL_ARRAY_BUFFER, vbo.id[VertexBuffers::NORMALS]);
	glBufferData(
		GL_ARRAY_BUFFER,				//Which buffer you're loading too
		sizeof(vec3)*normals.size(),	//Size of data in array (in bytes)
		&normals[0],					//Start of array (&points[0] will give you pointer to start of vector)
		GL_STATIC_DRAW					//GL_DYNAMIC_DRAW if you're changing the data often
										//GL_STATIC_DRAW if you're changing seldomly
		);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.id[VertexBuffers::INDICES]);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(unsigned int)*indices.size(),
		&indices[0],
		GL_STATIC_DRAW
		);

	return !CheckGLErrors("loadBuffer");	
}

bool loadBufferStars(const StarVertexBuffers& vbo, const vector<vec3>& points, const vector<vec3> normals,
    const vector<vec2>& uvs, const vector<unsigned int>& indices)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo.id[StarVertexBuffers::POINTS]);
    glBufferData(
        GL_ARRAY_BUFFER,				//Which buffer you're loading too
        sizeof(vec3)*points.size(),	//Size of data in array (in bytes)
        &points[0],							//Start of array (&points[0] will give you pointer to start of vector)
        GL_STATIC_DRAW						//GL_DYNAMIC_DRAW if you're changing the data often
                                            //GL_STATIC_DRAW if you're changing seldomly
    );

    glBindBuffer(GL_ARRAY_BUFFER, vbo.id[StarVertexBuffers::NORMALS]);
    glBufferData(
        GL_ARRAY_BUFFER,				//Which buffer you're loading too
        sizeof(vec3)*normals.size(),	//Size of data in array (in bytes)
        &normals[0],							//Start of array (&points[0] will give you pointer to start of vector)
        GL_STATIC_DRAW						//GL_DYNAMIC_DRAW if you're changing the data often
                                            //GL_STATIC_DRAW if you're changing seldomly
    );

    glBindBuffer(GL_ARRAY_BUFFER, vbo.id[StarVertexBuffers::UVS]);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vec2)*uvs.size(),
        &uvs[0],
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.id[StarVertexBuffers::INDICES]);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(unsigned int)*indices.size(),
        &indices[0],
        GL_STATIC_DRAW
    );

    return !CheckGLErrors("stars loadBuffer");
}

//Compile and link shaders, storing the program ID in shader array
GLuint initShader(string vertexName, string fragmentName)
{	
	string vertexSource = LoadSource(vertexName);		//Put vertex file text into string
	string fragmentSource = LoadSource(fragmentName);		//Put fragment file text into string

	GLuint vertexID = CompileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragmentID = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
	
	return LinkProgram(vertexID, fragmentID);	//Link and store program ID in shader array
}

//For reference:
//	https://open.gl/textures
GLuint createTexture(const char* filename)
{
    int components;
    GLuint texID;
    int tWidth, tHeight;

    //stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filename, &tWidth, &tHeight, &components, 0);

    if (data != NULL)
    {
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);

        if (components == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tWidth, tHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else if (components == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tWidth, tHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //Clean up
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);

        return texID;
    }

    return 0;	//Error
}

//Use program before loading texture
//	texUnit can be - GL_TEXTURE0, GL_TEXTURE1, etc...
bool loadTexture(GLuint texID, GLuint texUnit, GLuint program, const char* uniformName)
{
    glActiveTexture(texUnit);
    glBindTexture(GL_TEXTURE_2D, texID);

    GLuint uniformLocation = glGetUniformLocation(program, uniformName);
    glUniform1i(uniformLocation, 0);

    return !CheckGLErrors("loadTexture");
}

//Initialization
void initGL()
{
	glEnable(GL_DEPTH_TEST);
	glPointSize(25);
	glDepthFunc(GL_LEQUAL);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //Lines mode, uncomment to show wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClearColor(0.f, 0.f, 0.f, 0.f);		//Color to clear the screen with (R, G, B, Alpha)
}

bool loadUniforms(GLuint program, mat4 perspective, mat4 modelview, GLuint texId = NULL)
{
	glUseProgram(program);

    mat4 camMatrix = cam.getMatrix();

    glUniformMatrix4fv(glGetUniformLocation(program, "cameraMatrix"),
        1,
        false,
        &camMatrix[0][0]);


	glUniformMatrix4fv(glGetUniformLocation(program, "modelviewMatrix"),
						1,
						false,
						&modelview[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(program, "perspectiveMatrix"),
						1,
						false,
						&perspective[0][0]);

    glUniform3fv(glGetUniformLocation(program, "viewPos"), 1, &cam.pos[0]);

    if (texId != NULL) {
        loadTexture(texId, GL_TEXTURE0, program, "image");
    }

	return !CheckGLErrors("loadUniforms");
}

//Draws buffers to screen
void render(GLuint vao, int startElement, int numElements)
{
	glBindVertexArray(vao);		//Use the LINES vertex array

	glDrawElements(
			GL_TRIANGLES,		//What shape we're drawing	- GL_TRIANGLES, GL_LINES, GL_POINTS, GL_QUADS, GL_TRIANGLE_STRIP
			numElements,		//How many indices
			GL_UNSIGNED_INT,	//Type
			(void*)0			//Offset
			);

    glBindVertexArray(0);
	CheckGLErrors("render");
}

//Draws buffers to screen
void renderBox(GLuint vao, int startElement, int numElements)
{
	glBindVertexArray(vao);

	glDrawElements(
			GL_TRIANGLES,		//What shape we're drawing	- GL_TRIANGLES, GL_LINES, GL_POINTS, GL_QUADS, GL_TRIANGLE_STRIP
			numElements,		//How many indices
			GL_UNSIGNED_INT,	//Type
			(void*)0			//Offset
			);

    glBindVertexArray(0);
	CheckGLErrors("renderBox");
}

GLFWwindow* createGLFWWindow()
{
	// initialize the GLFW windowing system
    if (!glfwInit()) {
        cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
        return NULL;
    }
    glfwSetErrorCallback(ErrorCallback);

    // attempt to create a window with an OpenGL 4.1 core profile context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(512, 512, "Procedural Spaceship Generator", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return NULL;
    }

    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mousePosCallback);
    glfwSetWindowSizeCallback(window, resizeCallback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwMakeContextCurrent(window);

    return window;
}

void generateSquare(vector<vec3>& vertices, vector<vec3>& normals,
    vector<unsigned int>& indices, float width)
{
    vertices.push_back(vec3(-width*0.5f, -0.06, -width*0.5f));
    vertices.push_back(vec3(width*0.5f, -0.06, -width*0.5f));
    vertices.push_back(vec3(width*0.5f, -0.06, width*0.5f));
    vertices.push_back(vec3(-width*0.5f, -0.06, width*0.5f));

    normals.push_back(vec3(0.8f, 0.8f, 0.8f));
    normals.push_back(vec3(0.8f, 0.8f, 0.8f));
    normals.push_back(vec3(0.8f, 0.8f, 0.8f));
    normals.push_back(vec3(0.8f, 0.8f, 0.8f));

    //First triangle
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    //Second triangle
    indices.push_back(2);
    indices.push_back(3);
    indices.push_back(0);
}

void generateSphere(vector<vec3>* positions, vector<vec3>* normals, vector<vec2>* uvs, vector<unsigned int>* indices,
					float r, vec3 center, int uDivisions, int vDivisions)
{
	// udivisions will be theta
	// vdivisions will be phi	
	float uStep = 1.f/(float)(uDivisions-1);
	float vStep = 1.f/(float)(vDivisions-1);

	float u = 0.f;
	
    // Iterate through phi and theta
    for (double phi = 0.; phi < uDivisions; phi ++) // Azimuth [0, 2PI]
    {
		float v = 0.f;
        for (double theta = 0.; theta < vDivisions; theta++) // Elevation [0, PI]
        {
            vec3 point;
            point.x = r * cos(v*2*PI) * sin(u*PI) + center.x;
            point.y = r * sin(v*2*PI) * sin(u*PI) + center.y;
            point.z = r               * cos(u*PI) + center.z;
            
            vec3 normal = normalize(point - center);
            
            positions->push_back(point);
            normals->push_back(normal);
            uvs->push_back(vec2(u, v));
            
            v+=vStep;
        }
        u+=uStep;
    }
   
    for(int i=0; i<uDivisions-1; i++)
	{
		for(int j=0; j<vDivisions -1; j++)
		{
			unsigned int p00 = i*vDivisions+j;
			unsigned int p01 = i*vDivisions+j+1;
			unsigned int p10 = (i+1)*vDivisions + j;
			unsigned int p11 = (i+1)*vDivisions + j + 1;

			indices->push_back(p00);
			indices->push_back(p10);
			indices->push_back(p01);

			indices->push_back(p01);
			indices->push_back(p10);
			indices->push_back(p11);
		}
	}
    
}

void generateCylinder(vector<vec3>* positions, vector<vec3>* normals1, vector<unsigned int>* indices1,
					float r, vec3 center, int uDivisions, int vDivisions, float h)
{
	// udivisions will be theta
	// vdivisions will be phi	
	float uStep = 1.f/(float)(uDivisions-1);
	float vStep = 1.f/(float)(vDivisions-1);

	float u = 0.f;
	
    // Iterate through phi and theta
    for (int phi = 0; phi < uDivisions; phi ++) // Azimuth [0, 2PI]
    {
		float v = 0.f;
        for (int theta = 0; theta < vDivisions; theta++) // Elevation [0, PI]
        {
            vec3 point;
            point.x = r * 2 * cos(v*2*PI);
            point.y = r * 2 * sin(v*2*PI) - 4*r ;
            point.z = -(u*h) + (h/2);
            
            vec3 normal = normalize(point - center);
            //normal = vec3 ( 1, 0 ,0 );
            positions->push_back(point);
            normals1->push_back(normal);
            
            v+=vStep;
        }
        u+=uStep;
    }
	
    for(int i=0; i<(uDivisions-1); i++)
	{
		for(int j=0; j<(vDivisions-1) ; j++)
		{
			unsigned int p00 = i*vDivisions+j;
			unsigned int p01 = i*vDivisions+j+1;
			unsigned int p10 = (i+1)*vDivisions + j;
			unsigned int p11 = (i+1)*vDivisions + j + 1;

			indices1->push_back(p00);
			indices1->push_back(p10);
			indices1->push_back(p01);

			indices1->push_back(p01);
			indices1->push_back(p10);
			indices1->push_back(p11);
		}
	}
}

void generateBox(vector<vec3>* vertices, vector<vec3>* normals, vector<unsigned int>* indices, float sideLength, Box * box){

	vec3 bottomLeftFront = vec3(-sideLength, -sideLength/2, sideLength);
	vec3 bottomLeftBack = vec3(-sideLength, -sideLength/2, -sideLength);
	vec3 topLeftBack = vec3(-sideLength, sideLength/2, -sideLength);
	vec3 topLeftFront = vec3(-sideLength, sideLength/2, sideLength);
	
	vec3 bottomRightFront = vec3(sideLength, -sideLength/2, sideLength);
	vec3 bottomRightBack = vec3(sideLength, -sideLength/2, -sideLength);
	vec3 topRightBack = vec3(sideLength, sideLength/2, -sideLength);
	vec3 topRightFront = vec3(sideLength, sideLength/2, sideLength);
	
	vertices->push_back(bottomLeftFront); 
	vertices->push_back(bottomLeftBack);
	vertices->push_back(topLeftBack);
	vertices->push_back(topLeftFront);
	
	vertices->push_back(bottomRightFront);
	vertices->push_back(bottomRightBack);
	vertices->push_back(topRightBack);
	vertices->push_back(topRightFront);

    vec3 center = vec3(0, 0, 0);

    normals->push_back(bottomLeftFront - center);
    normals->push_back(bottomLeftBack - center);
    normals->push_back(topLeftBack - center);
    normals->push_back(topLeftFront - center);

    normals->push_back(bottomRightFront - center);
    normals->push_back(bottomRightBack - center);
    normals->push_back(topRightBack - center);
    normals->push_back(topRightFront-center);
	

	
	
	//TOP
	indices->push_back(3); indices->push_back(6); indices->push_back(2);
	indices->push_back(6); indices->push_back(3); indices->push_back(7);
	
	//RIGHT
	//indices->push_back(5);indices->push_back(6);indices->push_back(7);
	//indices->push_back(4);indices->push_back(5);indices->push_back(7);
	
	//BACK
	indices->push_back(1);indices->push_back(2);indices->push_back(6);
	indices->push_back(1); indices->push_back(6); indices->push_back(5);

	//LEFT
	//indices->push_back(1);indices->push_back(2);indices->push_back(3);
	//indices->push_back(0);indices->push_back(1);indices->push_back(3);

	//FRONT
	//indices->push_back(0);indices->push_back(4);indices->push_back(7);
	//indices->push_back(0);indices->push_back(7);indices->push_back(3);

	//BOTTOM
	indices->push_back(0);indices->push_back(1);indices->push_back(5);
	indices->push_back(0);indices->push_back(5);indices->push_back(4);

    box->updateLeft(2, 3, 1, 0);
    box->updateRight(6, 7, 5, 4);
    box->updateNose();

    box->sideLength = sideLength;

}

float magnitude(vec3 v){
	return sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

/*

Points naming convention

z1y1______________z2y2
|                    |              ^y
|                    |              |
|                    |>>>>>>        |
|                    |              |
|                    |              |---------->z
z3y3______________z4y4

*/
void generateNewFaces(vector<vec3>* vertices, vector<unsigned int>* indices, Box box) {



}
/*
void createNewPoints(Box &box, vector<vec3>* vertices, vector<vec3>* normals, vector<unsigned int>* indices, vector<MCNode*>& nodevec, bool cap) {
    vec3 topLeftBack = vertices->at(box.topLeftBack);
    vec3 topLeftFront = vertices->at(box.topLeftFront);
    vec3 bottomLeftBack = vertices->at(box.bottomLeftBack);
    vec3 bottomLeftFront = vertices->at(box.bottomLeftFront);

    vec3 topRightBack = vertices->at(box.topRightBack);
    vec3 topRightFront = vertices->at(box.topRightFront);
    vec3 bottomRightBack = vertices->at(box.bottomRightBack);
    vec3 bottomRightFront = vertices->at(box.bottomRightFront);

    // Generate offsets
    // Use the offsets on the current set of points to generate new points
    // Push back new points to vertices
    // Push back new colors to normals
    vector<vec3> offsets = generateOffsets(nodevec, box);

    int oldtopLeftBack = box.topLeftBack;
    int oldtopLeftFront = box.topLeftFront;
    int oldbottomLeftBack = box.bottomLeftBack;
    int oldbottomLeftFront = box.bottomLeftFront;

    int oldtopRightBack = box.topRightBack;
    int oldtopRightFront = box.topRightFront;
    int oldbottomRightBack = box.bottomRightBack;
    int oldbottomRightFront = box.bottomRightFront;

    // Check contrainst for top left back point
    if (((topLeftBack + offsets[0]).z) > -0.1f) {
        topLeftBack.z = -0.1f;
    }
    else {
        topLeftBack.z += offsets[0].z;
    }
    if (((topLeftBack + offsets[0]).y) < 0.1f) {
        topLeftBack.y = 0.1f;
    }
    else {
        topLeftBack.y += offsets[0].y;
    }

    // Check contrainst for front top left point
    if (((topLeftFront + offsets[1]).z) < 0.1f) {
        topLeftFront.z = 0.1f;
    }
    else {
        topLeftFront.z += offsets[1].z;
    }
    if (((topLeftFront + offsets[1]).y) < 0.1f) {
        topLeftFront.y = 0.1f;
    }
    else {
        topLeftFront.y += offsets[1].y;
    }

    // Check contrainst for bottom left back point
    if (((bottomLeftBack + offsets[2]).z) > -0.1f) {
        bottomLeftBack.z = -0.1f;
    }
    else {
        bottomLeftBack.z += offsets[2].z;
    }
    if (((bottomLeftBack + offsets[2]).y) > -0.1f) {
        bottomLeftBack.y = -0.1f;
    }
    else {
        bottomLeftBack.y += offsets[2].y;
    }

    // Check contrainst for front top left point
    if (((bottomLeftFront + offsets[3]).z) < 0.1f) {
        bottomLeftFront.z = 0.1f;
    }
    else {
        bottomLeftFront.z += offsets[3].z;
    }
    if (((bottomLeftFront + offsets[3]).y) > -0.1f) {
        bottomLeftFront.y = -0.1f;
    }
    else {
        bottomLeftFront.y += offsets[3].y;
    }
/*
   

    box.updateLeft((vertices->size() + 2), (vertices->size() + 3), (vertices->size() + 1), (vertices->size()));
    
    vertices->push_back(bottomLeftFront);
    vertices->push_back(bottomLeftBack);
    vertices->push_back(topLeftBack);
    vertices->push_back(topLeftFront);

    //T1
    indices->push_back(vertices->size());
    indices->push_back(box.bottomLeftFront);
    indices->push_back(box.topLeftFront);

    //T2
    indices->push_back(vertices->size());
    indices->push_back(vertices->size()+3);
    indices->push_back(box.topLeftFront);

    //T3
    indices->push_back(box.topLeftBack);
    indices->push_back(vertices->size() + 3);
    indices->push_back(box.topLeftFront);

    //T4
    indices->push_back(box.topLeftBack);
    indices->push_back(vertices->size() + 3);
    indices->push_back(vertices->size() + 2);

    //T5
    indices->push_back(box.topLeftBack);
    indices->push_back(vertices->size() + 1);
    indices->push_back(vertices->size() + 2);

    //T6
    indices->push_back(box.topLeftBack);
    indices->push_back(vertices->size() + 1);
    indices->push_back(box.bottomLeftBack);

    //T7
    indices->push_back(vertices->size());
    indices->push_back(box.bottomLeftFront);
    indices->push_back(vertices->size()+1);

    //T8
    indices->push_back(box.bottomLeftBack);
    indices->push_back(box.bottomLeftFront);
    indices->push_back(vertices->size() + 1);

   
    if (cap)
    {
        //T9
        indices->push_back(vertices->size());
        indices->push_back(vertices->size() + 3);
        indices->push_back(vertices->size() + 1);

        //10
        indices->push_back(vertices->size() + 2);
        indices->push_back(vertices->size() + 3);
        indices->push_back(vertices->size() + 1);
    }

    //T9
    indices->push_back(oldtopLeftBack);
    indices->push_back(oldbottomLeftBack);
    indices->push_back(box.topLeftBack);

    //T10
    indices->push_back(box.topLeftBack);
    indices->push_back(oldbottomLeftBack);
    indices->push_back(box.bottomLeftBack);

    //T11
    indices->push_back(oldbottomLeftBack);
    indices->push_back(oldbottomLeftFront);
    indices->push_back(box.bottomLeftBack);

    //T12
    indices->push_back(box.bottomLeftFront);
    indices->push_back(oldbottomLeftFront);
    indices->push_back(box.bottomLeftBack);

    //T13
    indices->push_back(box.bottomLeftFront);
    indices->push_back(oldbottomLeftFront);
    indices->push_back(oldtopLeftFront);

    //T14
    indices->push_back(box.bottomLeftFront);
    indices->push_back(box.topLeftFront);
    indices->push_back(oldtopLeftFront);

    //T15
    indices->push_back(oldtopLeftBack);
    indices->push_back(box.topLeftFront);
    indices->push_back(oldtopLeftFront);

    //T16
    indices->push_back(oldtopLeftBack);
    indices->push_back(box.topLeftFront);
    indices->push_back(box.topLeftBack);

    normals->push_back(bottomLeftFront - vec3(0, 0, 0));
    normals->push_back(bottomLeftBack - vec3(0, 0, 0));
    normals->push_back(topLeftBack - vec3(0, 0, 0));
    normals->push_back(topLeftFront - vec3(0, 0, 0));

    /*
    normals->push_back(vec3(1, 0, 0));
    normals->push_back(vec3(1, 0, 0));
    normals->push_back(vec3(1, 0, 0));
    normals->push_back(vec3(1, 0, 0));

    normals->push_back(vec3(1, 0, 0));
    normals->push_back(vec3(1, 0, 0));
    normals->push_back(vec3(1, 0, 0));
    normals->push_back(vec3(1, 0, 0));
    

    box.updateLeft((vertices->size() + 2), (vertices->size() + 3), (vertices->size() + 1), (vertices->size()));


    normals->push_back(bottomLeftFront - vec3(0, 0, 0));
    normals->push_back(bottomLeftBack - vec3(0, 0, 0));
    normals->push_back(topLeftBack - vec3(0, 0, 0));
    normals->push_back(topLeftFront - vec3(0, 0, 0));

    topLeftBack.x += offsets[0].x;
    topLeftFront.x += offsets[1].x;
    bottomLeftBack.x += offsets[2].x;
    bottomLeftFront.x += offsets[3].x;


    vertices->push_back(bottomLeftFront);
    vertices->push_back(bottomLeftBack);
    vertices->push_back(topLeftBack);
    vertices->push_back(topLeftFront);

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Right Side
    box.updateRight((vertices->size() + 2), (vertices->size() + 3), (vertices->size() + 1), (vertices->size()));
    
    bottomRightFront.y = bottomLeftFront.y;
    bottomRightFront.z = bottomLeftFront.z;

    bottomRightBack.y = bottomLeftBack.y;
    bottomRightBack.z = bottomLeftBack.z;

    topRightFront.y = topLeftFront.y;
    topRightFront.z = topLeftFront.z;

    topRightBack.y = topLeftBack.y;
    topRightBack.z = topLeftBack.z;

    vertices->push_back(bottomRightFront);
    vertices->push_back(bottomRightBack);
    vertices->push_back(topRightBack);
    vertices->push_back(topRightFront);

    //T1
    indices->push_back(vertices->size());
    indices->push_back(box.bottomRightFront);
    indices->push_back(box.topRightFront);

    //T2
    indices->push_back(vertices->size());
    indices->push_back(vertices->size() + 3);
    indices->push_back(box.topRightFront);

    //T3
    indices->push_back(box.topRightBack);
    indices->push_back(vertices->size() + 3);
    indices->push_back(box.topRightFront);

    //T4
    indices->push_back(box.topRightBack);
    indices->push_back(vertices->size() + 3);
    indices->push_back(vertices->size() + 2);

    //T5
    indices->push_back(box.topRightBack);
    indices->push_back(vertices->size() + 1);
    indices->push_back(vertices->size() + 2);

    //T6
    indices->push_back(box.topRightBack);
    indices->push_back(vertices->size() + 1);
    indices->push_back(box.bottomRightBack);

    //T7
    indices->push_back(vertices->size());
    indices->push_back(box.bottomRightFront);
    indices->push_back(vertices->size() + 1);

    //T8
    indices->push_back(box.bottomRightBack);
    indices->push_back(box.bottomRightFront);
    indices->push_back(vertices->size() + 1);
    
    //9
    if (cap)
    {
        indices->push_back(vertices->size());
        indices->push_back(vertices->size() + 3);
        indices->push_back(vertices->size() + 1);

        //10
        indices->push_back(vertices->size() + 2);
        indices->push_back(vertices->size() + 3);
        indices->push_back(vertices->size() + 1);
    }

    //T9
    indices->push_back(oldtopRightFront);
    indices->push_back(oldbottomRightFront);
    indices->push_back(box.topRightFront);

    //T10
    indices->push_back(box.topRightFront);
    indices->push_back(oldbottomRightFront);
    indices->push_back(box.bottomRightFront);

    //T11
    indices->push_back(oldbottomRightFront);
    indices->push_back(oldbottomRightBack);
    indices->push_back(box.bottomRightFront);

    //T12
    indices->push_back(box.bottomRightBack);
    indices->push_back(oldbottomRightBack);
    indices->push_back(box.bottomRightFront);

    //T13
    indices->push_back(box.bottomRightBack);
    indices->push_back(oldbottomRightBack);
    indices->push_back(oldtopRightBack);

    //T14
    indices->push_back(box.bottomRightBack);
    indices->push_back(box.topRightBack);
    indices->push_back(oldtopRightBack);

    //T15
    indices->push_back(oldtopRightFront);
    indices->push_back(box.topRightBack);
    indices->push_back(oldtopRightBack);

    //T16
    indices->push_back(oldtopRightFront);
    indices->push_back(box.topRightBack);
    indices->push_back(box.topRightFront);




    // Calculate center point of the new box section that has been created to set normals more accurately

    
    vec3 vertMid = (bottomRightBack + topRightBack) / 2.f;
    vec3 horiMid = (bottomRightBack + bottomRightFront) / 2.f;

    vec3 centerPoint = vec3(horiMid.z, vertMid.y, bottomRightBack.x - ((offsets[0].x) / 2.f));

/*

    normals->push_back(bottomRightFront - vec3(0, 0, 0));
    normals->push_back(bottomRightBack - vec3(0, 0, 0));
    normals->push_back(topRightBack - vec3(0, 0, 0));
    normals->push_back(topRightFront - vec3(0, 0, 0));


    normals->push_back(centerPoint - bottomRightFront);
    normals->push_back(centerPoint - bottomRightBack);
    normals->push_back(centerPoint - topRightBack);
    normals->push_back(centerPoint - topRightFront);

    box.updateRight((vertices->size() + 2), (vertices->size() + 3), (vertices->size() + 1), (vertices->size()));


    //normals->push_back(bottomRightFront - vec3(0, 0, 0));
    //normals->push_back(bottomRightBack - vec3(0, 0, 0));
    //normals->push_back(topRightBack - vec3(0, 0, 0));
    //normals->push_back(topRightFront - vec3(0, 0, 0));

    
    normals->push_back(centerPoint - bottomRightFront);
    normals->push_back(centerPoint - bottomRightBack);
    normals->push_back(centerPoint - topRightBack);
    normals->push_back(centerPoint - topRightFront);

    topRightBack.x -= offsets[0].x;
    topRightFront.x -= offsets[1].x;
    bottomRightBack.x -= offsets[2].x;
    bottomRightFront.x -= offsets[3].x;


    vertices->push_back(bottomRightFront);
    vertices->push_back(bottomRightBack);
    vertices->push_back(topRightBack);
    vertices->push_back(topRightFront);
    
}

*/
void createNewPoints(Box &box, vector<vec3>* vertices, vector<vec3>* normals, vector<unsigned int>* indices, vector<MCNode*>& nodevec, bool cap) {
vec3 topLeftBack = vertices->at(box.topLeftBack);
vec3 topLeftFront = vertices->at(box.topLeftFront);
vec3 bottomLeftBack = vertices->at(box.bottomLeftBack);
vec3 bottomLeftFront = vertices->at(box.bottomLeftFront);

vec3 topRightBack = vertices->at(box.topRightBack);
vec3 topRightFront = vertices->at(box.topRightFront);
vec3 bottomRightBack = vertices->at(box.bottomRightBack);
vec3 bottomRightFront = vertices->at(box.bottomRightFront);

// Generate offsets
// Use the offsets on the current set of points to generate new points
// Push back new points to vertices
// Push back new colors to normals

vector<vec3> offsets = generateOffsets(nodevec, box);

// Check contrainst for top left back point
if (((topLeftBack + offsets[0]).z) > -0.1f) {
topLeftBack.z = -0.1f;
topRightBack.z = -0.1f;
}
else {
topLeftBack.z += offsets[0].z;
topRightBack.z += offsets[0].z;
}
if (((topLeftBack + offsets[0]).y) < 0.1f) {
topLeftBack.y = 0.1f;
topRightBack.y = 0.1f;
}
else {
topLeftBack.y += offsets[0].y;
topRightBack.y += offsets[0].y;
}

// Check contrainst for front top left point
if (((topLeftFront + offsets[1]).z) < 0.1f) {
topLeftFront.z = 0.1f;
topRightFront.z = 0.1f;
}
else {
topLeftFront.z += offsets[1].z;
topRightFront.z += offsets[1].z;
}
if (((topLeftFront + offsets[1]).y) < 0.1f) {
topLeftFront.y = 0.1f;
topRightFront.y = 0.1f;
}
else {
topLeftFront.y += offsets[1].y;
topRightFront.y += offsets[1].y;
}

// Check contrainst for bottom left back point
if (((bottomLeftBack + offsets[2]).z) > -0.1f) {
bottomLeftBack.z = -0.1f;
bottomRightBack.z = -0.1f;
}
else {
bottomLeftBack.z += offsets[2].z;
bottomRightBack.z += offsets[2].z;
}
if (((bottomLeftBack + offsets[2]).y) > -0.1f) {
bottomLeftBack.y = -0.1f;
bottomRightBack.y = -0.1f;
}
else {
bottomLeftBack.y += offsets[2].y;
bottomRightBack.y += offsets[2].y;
}

// Check contrainst for front top left point
if (((bottomLeftFront + offsets[3]).z) < 0.1f) {
bottomLeftFront.z = 0.1f;
bottomRightFront.z = 0.1f;
}
else {
bottomLeftFront.z += offsets[3].z;
bottomRightFront.z += offsets[3].z;
}
if (((bottomLeftFront + offsets[3]).y) > -0.1f) {
bottomLeftFront.y = -0.1f;
bottomRightFront.y = -0.1f;
}
else {
bottomLeftFront.y += offsets[3].y;
bottomRightFront.y += offsets[3].y;
}

topLeftBack.x += offsets[0].x;
topLeftFront.x += offsets[1].x;
bottomLeftBack.x += offsets[2].x;
bottomLeftFront.x += offsets[3].x;

//T1
indices->push_back(box.topLeftFront);
indices->push_back(vertices->size());
indices->push_back(box.bottomLeftFront);

//T2
indices->push_back(vertices->size());
indices->push_back(box.topLeftFront);
indices->push_back(vertices->size()+3);

//T3
indices->push_back(box.topLeftBack);
indices->push_back(vertices->size() + 3);
indices->push_back(box.topLeftFront);

//T4
indices->push_back(box.topLeftBack);
indices->push_back(vertices->size() + 2);
indices->push_back(vertices->size() + 3);

//T5
indices->push_back(box.topLeftBack);
indices->push_back(vertices->size() + 1);
indices->push_back(vertices->size() + 2);

//T6
indices->push_back(box.topLeftBack);
indices->push_back(box.bottomLeftBack);
indices->push_back(vertices->size() + 1);

//T7
indices->push_back(vertices->size());
indices->push_back(vertices->size() + 1);
indices->push_back(box.bottomLeftFront);

//T8
indices->push_back(box.bottomLeftBack);
indices->push_back(box.bottomLeftFront);
indices->push_back(vertices->size() + 1);

if (cap)
{
	indices->push_back(vertices->size());
	indices->push_back(vertices->size() + 3);
	indices->push_back(vertices->size() + 1);

	//10
	indices->push_back(vertices->size() + 2);
    indices->push_back(vertices->size() + 1);
	indices->push_back(vertices->size() + 3);
}
vec3 avg = (bottomLeftFront + bottomLeftBack + topLeftBack + topLeftFront) / 4.0f;
normals->push_back(bottomLeftFront-avg);
normals->push_back(bottomLeftBack-avg);
normals->push_back(topLeftBack-avg);
normals->push_back(topLeftFront-avg);


box.updateLeft((vertices->size() + 2), (vertices->size() + 3), (vertices->size() + 1), (vertices->size()));

vertices->push_back(bottomLeftFront);
vertices->push_back(bottomLeftBack);
vertices->push_back(topLeftBack);
vertices->push_back(topLeftFront);

// RIGHT SIDE
topRightBack.x -= offsets[0].x;
topRightFront.x -= offsets[1].x;
bottomRightBack.x -= offsets[2].x;
bottomRightFront.x -= offsets[3].x;

//vertices->push_back(bottomRightFront);    0
//vertices->push_back(bottomRightBack);     1
//vertices->push_back(topRightBack);        2
//vertices->push_back(topRightFront);       3
//T1
indices->push_back(box.topRightFront);
indices->push_back(box.bottomRightFront);
indices->push_back(vertices->size());

//T2
indices->push_back(vertices->size());
indices->push_back(vertices->size() + 3);
indices->push_back(box.topRightFront);

//T3
indices->push_back(box.topRightBack);
indices->push_back(box.topRightFront);
indices->push_back(vertices->size() + 3);

//T4
indices->push_back(box.topRightBack);
indices->push_back(vertices->size() + 3);
indices->push_back(vertices->size() + 2);

//T5
indices->push_back(box.topRightBack);
indices->push_back(vertices->size() + 2);
indices->push_back(vertices->size() + 1);

//T6
indices->push_back(box.topRightBack);
indices->push_back(vertices->size() + 1);
indices->push_back(box.bottomRightBack);

//T7
indices->push_back(vertices->size());
indices->push_back(box.bottomRightFront);
indices->push_back(vertices->size() + 1);

//T8
indices->push_back(box.bottomRightBack);
indices->push_back(vertices->size() + 1);
indices->push_back(box.bottomRightFront);

if (cap)
{
	indices->push_back(vertices->size());
    indices->push_back(vertices->size() + 1);
	indices->push_back(vertices->size() + 3);

	//10
	indices->push_back(vertices->size() + 2);
    indices->push_back(vertices->size() + 3);
    indices->push_back(vertices->size() + 1);
}

avg = (bottomRightFront + bottomRightBack + topRightBack + topRightFront) / 4.0f;
normals->push_back(bottomRightFront - avg);
normals->push_back(bottomRightBack - avg);
normals->push_back(topRightBack - avg);
normals->push_back(topRightFront - avg);


box.updateRight((vertices->size() + 2), (vertices->size() + 3), (vertices->size() + 1), (vertices->size()));

vertices->push_back(bottomRightFront);
vertices->push_back(bottomRightBack);
vertices->push_back(topRightBack);
vertices->push_back(topRightFront);

// Generate the new faces -using the new 4 indices and the old 4 indices
//   This step involves creating triangles using the two sets of indices (old and new) for each side of the rectangle
//   then updating each sides indices to the next set (ie old -> new)
}

//Oldstyle Nose
void createNosePoints(Box &box, vector<vec3>* vertices, vector<vec3>* normals, vector<unsigned int>* indices, vector<MCNode*>& nodevec, bool cap) {
	vec3 topLeftNose = vertices->at(box.topLeftNose);
	vec3 topRightNose = vertices->at(box.topRightNose);
	vec3 bottomLeftNose = vertices->at(box.bottomLeftNose);
	vec3 bottomRightNose = vertices->at(box.bottomRightNose);
	// Generate offsets
	// Use the offsets on the current set of points to generate new points
	// Push back new points to vertices
	// Push back new colors to normals

	vector<vec3> offsets = generateOffsets(nodevec, box);

	int oldtopLeftNose = box.topLeftNose;
	int oldtopRightNose = box.topRightNose;
	int oldbottomLeftNose = box.bottomLeftNose;
	int oldbottomRightNose = box.bottomRightNose;


	// Check contrainst for top left point
	if (((topLeftNose + offsets[0]).z) < 0.1f) {
		topLeftNose.x = 0.1f;
	}
	else {
		topLeftNose.x += offsets[0].z;
	}
	if (((topLeftNose + offsets[0]).y) < -0.5f) {
		topLeftNose.y = -0.5f;
	}
	else {
		topLeftNose.y += offsets[0].y;
	}

	topRightNose.x = -topLeftNose.x;
	topRightNose.y = topLeftNose.y;

	// Check contrainst for bottom left back point
	if (((bottomLeftNose + offsets[2]).z) < 0.1f) {
		bottomLeftNose.x = 0.1f;
	}
	else {
		bottomLeftNose.x += offsets[2].z;
	}
	if (((bottomLeftNose + offsets[2]).y) > -1.0f) {
		bottomLeftNose.y = -1.0f;
	}
	else {
		bottomLeftNose.y += offsets[2].y;
	}

	bottomRightNose.x = -bottomLeftNose.x;
	bottomRightNose.y = bottomLeftNose.y;

	/*


	box.updateNose((vertices->size() + 2), (vertices->size() + 3), (vertices->size() + 1), (vertices->size()));

	vertices->push_back(bottomRightNose); 0
	vertices->push_back(bottomLeftNose);  1
	vertices->push_back(topLeftNose);     2
	vertices->push_back(topRightNose);    3
	*/
	//T1
	indices->push_back(vertices->size());
	indices->push_back(oldbottomRightNose);
	indices->push_back(oldtopRightNose);

	//T2
	indices->push_back(vertices->size());
    indices->push_back(oldtopRightNose);
	indices->push_back(vertices->size() + 3);

	//T3
	indices->push_back(oldtopLeftNose);
	indices->push_back(vertices->size() + 3);
	indices->push_back(oldtopRightNose);

	//T4
	indices->push_back(oldtopLeftNose);
    indices->push_back(vertices->size() + 2);
	indices->push_back(vertices->size() + 3);

	//T5
	indices->push_back(oldtopLeftNose);
	indices->push_back(vertices->size() + 1);
	indices->push_back(vertices->size() + 2);

	//T6
	indices->push_back(oldtopLeftNose);
    indices->push_back(oldbottomLeftNose);
	indices->push_back(vertices->size() + 1);

	//T7
	indices->push_back(vertices->size());
    indices->push_back(vertices->size() + 1);
	indices->push_back(oldbottomRightNose);

	//T8
	indices->push_back(oldbottomLeftNose);
	indices->push_back(oldbottomRightNose);
	indices->push_back(vertices->size() + 1);

	if (cap)
	{
	//9
	indices->push_back(vertices->size());
	indices->push_back(vertices->size() + 3);
	indices->push_back(vertices->size() + 1);

	//10
	indices->push_back(vertices->size() + 2);
    indices->push_back(vertices->size() + 1);
	indices->push_back(vertices->size() + 3);
	}

	vec3 vertMid = (bottomRightNose + topRightNose) / 2.f;
	vec3 horiMid = (bottomRightNose + bottomLeftNose) / 2.f;

	vec3 centerPoint = vec3(horiMid.x, vertMid.y, bottomLeftNose.z - ((offsets[0].x) / 2.f));

	normals->push_back(bottomRightNose - centerPoint);
	normals->push_back(bottomLeftNose - centerPoint);
	normals->push_back(topLeftNose - centerPoint);
	normals->push_back(topRightNose - centerPoint);


	box.updateNose((vertices->size() + 2), (vertices->size() + 3), (vertices->size() + 1), (vertices->size()));


	topLeftNose.z -= offsets[0].x;
	topRightNose.z -= offsets[1].x;
	bottomLeftNose.z -= offsets[2].x;
	bottomRightNose.z -= offsets[3].x;


	vertices->push_back(bottomRightNose);
	vertices->push_back(bottomLeftNose);
	vertices->push_back(topLeftNose);
	vertices->push_back(topRightNose);

	}

// Using same generateOffsets as for wings, but using generated X as the Z part of nose
/*void createNosePoints(Box &box, vector<vec3>* vertices, vector<vec3>* normals, vector<unsigned int>* indices, vector<MCNode*>& nodevec, bool cap) {
    vec3 topLeftNose = vertices->at(box.topLeftNose);
    vec3 topRightNose = vertices->at(box.topRightNose);
    vec3 bottomLeftNose = vertices->at(box.bottomLeftNose);
    vec3 bottomRightNose = vertices->at(box.bottomRightNose);
    // Generate offsets
    // Use the offsets on the current set of points to generate new points
    // Push back new points to vertices
    // Push back new colors to normals

    vector<vec3> offsets = generateOffsets(nodevec, box);

    int oldtopLeftNose = box.topLeftNose;
    int oldtopRightNose = box.topRightNose;
    int oldbottomLeftNose = box.bottomLeftNose;
    int oldbottomRightNose = box.bottomRightNose;


    // Check contrainst for top left point
    if (((topLeftNose + offsets[0]).z) < 0.1f) {
        topLeftNose.x = 0.1f;
    }
    else {
        topLeftNose.x += offsets[0].z;
    }
    if (((topLeftNose + offsets[0]).y) < -0.5f) {
        topLeftNose.y = -0.5f;
    }
    else {
        topLeftNose.y += offsets[0].y;
    }

    topRightNose.x = -topLeftNose.x;
    topRightNose.y = topLeftNose.y;

    // Check contrainst for bottom left back point
    if (((bottomLeftNose + offsets[2]).z) < 0.1f) {
        bottomLeftNose.x = 0.1f;
    }
    else {
        bottomLeftNose.x += offsets[2].z;
    }
    if (((bottomLeftNose + offsets[2]).y) > -1.0f) {
        bottomLeftNose.y = -1.0f;
    }
    else {
        bottomLeftNose.y += offsets[2].y;
    }

        bottomRightNose.x = -bottomLeftNose.x;
        bottomRightNose.y = bottomLeftNose.y;
    
    /*

    
    box.updateNose((vertices->size() + 2), (vertices->size() + 3), (vertices->size() + 1), (vertices->size()));

    vertices->push_back(bottomRightNose);
    vertices->push_back(bottomLeftNose);
    vertices->push_back(topLeftNose);
    vertices->push_back(topRightNose);

    //T1
    indices->push_back(vertices->size());
    indices->push_back(box.bottomRightNose);
    indices->push_back(box.topRightNose);

    //T2
    indices->push_back(vertices->size());
    indices->push_back(vertices->size() + 3);
    indices->push_back(box.topRightNose);

    //T3
    indices->push_back(box.topLeftNose);
    indices->push_back(vertices->size() + 3);
    indices->push_back(box.topRightNose);

    //T4
    indices->push_back(box.topLeftNose);
    indices->push_back(vertices->size() + 3);
    indices->push_back(vertices->size() + 2);

    //T5
    indices->push_back(box.topLeftNose);
    indices->push_back(vertices->size() + 1);
    indices->push_back(vertices->size() + 2);

    //T6
    indices->push_back(box.topLeftNose);
    indices->push_back(vertices->size() + 1);
    indices->push_back(box.bottomLeftNose);

    //T7
    indices->push_back(vertices->size());
    indices->push_back(box.bottomRightNose);
    indices->push_back(vertices->size() + 1);

    //T8
    indices->push_back(box.bottomLeftNose);
    indices->push_back(box.bottomRightNose);
    indices->push_back(vertices->size() + 1);

    if (cap)
    {
        //9
        indices->push_back(vertices->size());
        indices->push_back(vertices->size() + 3);
        indices->push_back(vertices->size() + 1);

        //10
        indices->push_back(vertices->size() + 2);
        indices->push_back(vertices->size() + 3);
        indices->push_back(vertices->size() + 1);
    }
    
    //T9
    indices->push_back(oldtopLeftNose);
    indices->push_back(oldbottomLeftNose);
    indices->push_back(box.topLeftNose);

    //T10
    indices->push_back(box.topLeftNose);
    indices->push_back(oldbottomLeftNose);
    indices->push_back(box.bottomLeftNose);

    //T11
    indices->push_back(oldbottomLeftNose);
    indices->push_back(oldbottomRightNose);
    indices->push_back(box.bottomLeftNose);

    //T12
    indices->push_back(box.bottomRightNose);
    indices->push_back(oldbottomRightNose);
    indices->push_back(box.bottomLeftNose);

    //T13
    indices->push_back(box.bottomRightNose);
    indices->push_back(oldbottomRightNose);
    indices->push_back(oldtopRightNose);

    //T14
    indices->push_back(box.bottomRightNose);
    indices->push_back(box.topRightNose);
    indices->push_back(oldtopRightNose);

    //T15
    indices->push_back(oldtopLeftNose);
    indices->push_back(box.topRightNose);
    indices->push_back(oldtopRightNose);

    //T16
    indices->push_back(oldtopLeftNose);
    indices->push_back(box.topRightNose);
    indices->push_back(box.topLeftNose);



    vec3 vertMid = (bottomRightNose + topRightNose) / 2.f;
    vec3 horiMid = (bottomRightNose + bottomLeftNose) / 2.f;

    vec3 centerPoint = vec3(horiMid.x, vertMid.y, bottomLeftNose.z - ((offsets[0].x) / 2.f));

    normals->push_back(bottomRightNose - vec3(0, 0, 0));
    normals->push_back(bottomLeftNose - vec3(0, 0, 0));
    normals->push_back(topLeftNose - vec3(0, 0, 0));
    normals->push_back(topRightNose - vec3(0, 0, 0));


    //normals->push_back(centerPoint - bottomRightNose);
    //normals->push_back(centerPoint - bottomLeftNose);
    //normals->push_back(centerPoint - topLeftNose);
    //normals->push_back(centerPoint - topRightNose);


    box.updateNose((vertices->size() + 2), (vertices->size() + 3), (vertices->size() + 1), (vertices->size()));


    normals->push_back(bottomRightNose - vec3(0, 0, 0));
    normals->push_back(bottomLeftNose - vec3(0, 0, 0));
    normals->push_back(topLeftNose - vec3(0, 0, 0));
    normals->push_back(topRightNose - vec3(0, 0, 0));
/*

    normals->push_back(centerPoint - bottomRightNose);
    normals->push_back(centerPoint - bottomLeftNose);
    normals->push_back(centerPoint - topLeftNose);
    normals->push_back(centerPoint - topRightNose);


    topLeftNose.z -= offsets[0].x;
    topRightNose.z -= offsets[1].x;
    bottomLeftNose.z -= offsets[2].x;
    bottomRightNose.z -= offsets[3].x;


    vertices->push_back(bottomRightNose);
    vertices->push_back(bottomLeftNose);
    vertices->push_back(topLeftNose);
    vertices->push_back(topRightNose);

}*/

vector<vec3> generateOffsets(vector<MCNode*>& nodevec, Box& myBox)
{
    
    float xoffset = -offsetScale*1.5f*myBox.sideLength*((float)rand() / (RAND_MAX))-0.3f;
    //float xoffset = -myBox.sideLength*offsetScale*3.0f;
    vector<vec3> temp;
    vec3 offsets;
    float direction, zoffset, yoffset;

    // Top Left Point 1
    zoffset = offsetScale*myBox.sideLength*((float)rand() / (RAND_MAX));
    direction = ((float)rand() / (RAND_MAX));
    if (direction < nodevec.at(0)->in)
        nodevec.at(0) = nodevec.at(0)->innode;
    else if (direction < nodevec.at(0)->in + nodevec.at(0)->out)
    {
        zoffset = -zoffset;
        nodevec.at(0) = nodevec.at(0)->outnode;
    }
    else
    {
        zoffset = 0.0f;
        nodevec.at(0) = nodevec.at(0)->levelnode;
    }

    yoffset = offsetScale*myBox.sideLength*((float)rand() / (RAND_MAX));
    direction = ((float)rand() / (RAND_MAX));
    if (direction < nodevec.at(1)->in)
    {
        yoffset = -yoffset;
        nodevec.at(1) = nodevec.at(1)->innode;
    }
    else if (direction < nodevec.at(1)->in + nodevec.at(1)->out)
        nodevec.at(1) = nodevec.at(1)->outnode;
    else
    {
        yoffset = 0.0f;
        nodevec.at(1) = nodevec.at(1)->levelnode;
    }
    offsets = vec3(xoffset, yoffset, zoffset);
    temp.push_back(offsets);

    // Top Right Point 2
    zoffset = offsetScale*myBox.sideLength*((float)rand() / (RAND_MAX));
    direction = ((float)rand() / (RAND_MAX));
    if (direction < nodevec.at(2)->in)
    {
        zoffset = -zoffset;
        nodevec.at(2) = nodevec.at(2)->innode;
    }
    else if (direction < nodevec.at(2)->in + nodevec.at(2)->out)
        nodevec.at(2) = nodevec.at(2)->outnode;
    else
    {
        zoffset = 0.0f;
        nodevec.at(2) = nodevec.at(2)->levelnode;
    }

    yoffset = offsetScale*myBox.sideLength*((float)rand() / (RAND_MAX));
    direction = ((float)rand() / (RAND_MAX));
    if (direction < nodevec.at(3)->in)
    {
        yoffset = -yoffset;
        nodevec.at(3) = nodevec.at(3)->innode;
    }
    else if (direction < nodevec.at(3)->in + nodevec.at(3)->out)
        nodevec.at(3) = nodevec.at(3)->outnode;
    else
    {
        yoffset = 0.0f;
        nodevec.at(3) = nodevec.at(3)->levelnode;
    }
    offsets = vec3(xoffset, yoffset, zoffset);
    temp.push_back(offsets);

    // Bottom Left Point 3
    zoffset = offsetScale*myBox.sideLength*((float)rand() / (RAND_MAX));
    direction = ((float)rand() / (RAND_MAX));
    if (direction < nodevec.at(4)->in)
        nodevec.at(4) = nodevec.at(4)->innode;
    else if (direction < nodevec.at(4)->in + nodevec.at(4)->out)
    {
        zoffset = -zoffset;
        nodevec.at(4) = nodevec.at(4)->outnode;
    }
    else
    {
        zoffset = 0.0f;
        nodevec.at(4) = nodevec.at(4)->levelnode;
    }

    yoffset = offsetScale*myBox.sideLength*((float)rand() / (RAND_MAX));
    direction = ((float)rand() / (RAND_MAX));
    if (direction < nodevec.at(5)->in)
        nodevec.at(5) = nodevec.at(5)->innode;
    else if (direction < nodevec.at(5)->in + nodevec.at(5)->out)
    {
        yoffset = -yoffset;
        nodevec.at(5) = nodevec.at(5)->outnode;
    }
    else
    {
        yoffset = 0.0f;
        nodevec.at(5) = nodevec.at(5)->levelnode;
    }
    offsets = vec3(xoffset, yoffset, zoffset);
    temp.push_back(offsets);

    // Bottom Right Point 4
    zoffset = offsetScale*myBox.sideLength*((float)rand() / (RAND_MAX));
    direction = ((float)rand() / (RAND_MAX));
    if (direction < nodevec.at(6)->in)
    {
        zoffset = -zoffset;
        nodevec.at(6) = nodevec.at(6)->innode;
    }
    else if (direction < nodevec.at(6)->in + nodevec.at(6)->out)
        nodevec.at(6) = nodevec.at(6)->outnode;
    else
    {
        zoffset = 0.0f;
        nodevec.at(6) = nodevec.at(6)->levelnode;
    }

    yoffset = offsetScale*myBox.sideLength*((float)rand() / (RAND_MAX));
    direction = ((float)rand() / (RAND_MAX));
    if (direction < nodevec.at(7)->in)
        nodevec.at(7) = nodevec.at(7)->innode;
    else if (direction < nodevec.at(7)->in + nodevec.at(7)->out)
    {
        yoffset = -yoffset;
        nodevec.at(7) = nodevec.at(7)->outnode;
    }
    else
    {
        yoffset = 0.0f;
        nodevec.at(7) = nodevec.at(7)->levelnode;
    }
    offsets = vec3(xoffset, yoffset, zoffset);
    temp.push_back(offsets);

    return temp;
}

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{   
    window = createGLFWWindow();
    if(window == NULL)
    	return -1;

    //Initialize glad
    if (!gladLoadGL())
	{
		cout << "GLAD init failed" << endl;
		return -1;
	}

    // query and print out information about our OpenGL environment
    QueryGLVersion();

	initGL();

   // starTexture = createTexture("stars_milkyway.jpg");
    starTexture = createTexture("stars_milkyway.jpg");

    int seed = static_cast<int>(time(0));
    srand(seed);

	//Initialize shader
	GLuint program = initShader("vertex.glsl", "fragment.glsl");
    GLuint programStars = initShader("vertexStars.glsl", "fragmentStars.glsl");
    
	GLuint vao, vaoCylinder, vaoSquare, vaoBox, vaoStars;
	VertexBuffers vbo, vboCylinder, vboSquare, vboBox;
    StarVertexBuffers vboStars;

	//Generate object ids
	glGenVertexArrays(1, &vao);
	glGenBuffers(VertexBuffers::COUNT, vbo.id);

    //Generate object ids
    glGenVertexArrays(1, &vaoCylinder);
    glGenBuffers(VertexBuffers::COUNT, vboCylinder.id);

    //Generate object ids
    glGenVertexArrays(1, &vaoStars);
    glGenBuffers(StarVertexBuffers::COUNT, vboStars.id);

	initVAO(vao, vbo);
    initVAO(vaoCylinder, vboCylinder);
    initVAOStars(vaoStars, vboStars);

    //Sphere Buffers information
    vector<vec3> starBoxPoints, starBoxNormals;
    vector<vec2> starBoxUVS;
    vector<unsigned int> starBoxIndices;

    generateSphere(&starBoxPoints, &starBoxNormals, &starBoxUVS, &starBoxIndices, 400.f, vec3(0, 0, 0), 100, 100);
    loadBufferStars(vboStars, starBoxPoints, starBoxNormals, starBoxUVS, starBoxIndices);

    generateCylinder(&pointsCylinder, &normalsCylinder, &indicesCylinder, 1.f, vec3(0, 0, 0), 20, 20, 5.f);
    loadBuffer(vboCylinder, pointsCylinder, normalsCylinder, indicesCylinder);

	generateBox(&points, &normals, &indices, 3.f, initialBox);
	loadBuffer(vbo, points, normals, indices);
	
	activeCamera = &cam;
	
	//float fovy, float aspect, float zNear, float zFar
	mat4 perspectiveMatrix = perspective(radians(80.f), 1.f, 0.1f, 1000.f);
	
    mat4 model = mat4(1.f);

    // Setup Markov Chain Tree for Generation
    MCNode* levelNode = new MCNode(0.5f, 0.1f, 0.4f);
    MCNode* inNode = new MCNode(0.4f, 0.1f, 0.5f);
    MCNode* outNode = new MCNode(0.4f, 0.0f, 0.6f, inNode, nullptr, levelNode);
    outNode->outnode = outNode;
    inNode->innode = inNode;
    inNode->outnode = outNode;
    inNode->levelnode = levelNode;
    levelNode->innode = inNode;
    levelNode->outnode = outNode;
    levelNode->levelnode = levelNode;

    
    // Pointer to current node during traversal
    // z1,y1 = top left; z2,y2 = top right
    // z3,y3 = bottom left; z4,y4 = bottom right
    MCNode* z1currentNode = levelNode;
    MCNode* y1currentNode = levelNode;
    MCNode* z2currentNode = levelNode;
    MCNode* y2currentNode = levelNode;
    MCNode* z3currentNode = levelNode;
    MCNode* y3currentNode = levelNode;
    MCNode* z4currentNode = levelNode;
    MCNode* y4currentNode = levelNode;
    nodePointers.push_back(z1currentNode);
    nodePointers.push_back(y1currentNode);
    nodePointers.push_back(z2currentNode);
    nodePointers.push_back(y2currentNode);
    nodePointers.push_back(z3currentNode);
    nodePointers.push_back(y3currentNode);
    nodePointers.push_back(z4currentNode);
    nodePointers.push_back(y4currentNode);

    for (int i = 0; i <15; i++)
    {
        createNewPoints(*initialBox, &points, &normals, &indices, nodePointers, (i == 14) ? true : false);
    }

    for (int i = 0; i < 10; i++)
    {
        createNosePoints(*initialBox, &points, &normals, &indices, nodePointers, (i==9)?true:false);
    }
    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//Clear color and depth buffers (Haven't covered yet)
       
        loadUniforms(programStars, winRatio*perspectiveMatrix*cam.getMatrix(), mat4(1.f), starTexture);
        loadBufferStars(vboStars, starBoxPoints, starBoxNormals, starBoxUVS, starBoxIndices);

        render(vaoStars, 0, starBoxIndices.size());

		loadUniforms(program, winRatio*perspectiveMatrix*cam.getMatrix(),scale(model, vec3(scaleFactor)));
        loadBuffer(vbo, points, normals, indices);

		render(vao, 0, indices.size());
       // render(vaoCylinder, 0, indicesCylinder.size());
		
        // scene is rendered to the back buffer, so swap to front for display
        glfwSwapBuffers(window);

        // sleep until next event before drawing again
        glfwPollEvents();

        //loadBuffer(vboCylinder, pointsCylinder, normalsCylinder, indicesCylinder);
	}

	// clean up allocated resources before exit
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(VertexBuffers::COUNT, vbo.id);
	glDeleteProgram(program);


	glfwDestroyWindow(window);
   glfwTerminate();

   return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
    // query opengl version and renderer information
    string version  = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    string glslver  = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

    cout << "OpenGL [ " << version << " ] "
         << "with GLSL [ " << glslver << " ] "
         << "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors(string location)
{
    bool error = false;
    for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
    {
        cout << "OpenGL ERROR:  ";
        switch (flag) {
        case GL_INVALID_ENUM:
            cout << location << ": " << "GL_INVALID_ENUM" << endl; break;
        case GL_INVALID_VALUE:
            cout << location << ": " << "GL_INVALID_VALUE" << endl; break;
        case GL_INVALID_OPERATION:
            cout << location << ": " << "GL_INVALID_OPERATION" << endl; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            cout << location << ": " << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
        case GL_OUT_OF_MEMORY:
            cout << location << ": " << "GL_OUT_OF_MEMORY" << endl; break;
        default:
            cout << "[unknown error code]" << endl;
        }
        error = true;
    }
    return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
    string source;

    ifstream input(filename.c_str());
    if (input) {
        copy(istreambuf_iterator<char>(input),
             istreambuf_iterator<char>(),
             back_inserter(source));
        input.close();
    }
    else {
        cout << "ERROR: Could not load shader source from file "
             << filename << endl;
    }

    return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
    // allocate shader object name
    GLuint shaderObject = glCreateShader(shaderType);

    // try compiling the source as a shader of the given type
    const GLchar *source_ptr = source.c_str();
    glShaderSource(shaderObject, 1, &source_ptr, 0);
    glCompileShader(shaderObject);

    // retrieve compile status
    GLint status;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
        cout << "ERROR compiling shader:" << endl << endl;
        cout << source << endl;
        cout << info << endl;
    }

    return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    // allocate program object name
    GLuint programObject = glCreateProgram();

    // attach provided shader objects to this program
    if (vertexShader)   glAttachShader(programObject, vertexShader);
    if (fragmentShader) glAttachShader(programObject, fragmentShader);

    // try linking the program with given attachments
    glLinkProgram(programObject);

    // retrieve link status
    GLint status;
    glGetProgramiv(programObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
        cout << "ERROR linking shader program:" << endl;
        cout << info << endl;
    }

    return programObject;
}
// ==========================================================================
