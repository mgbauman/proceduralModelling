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
#define PI 3.14159265359

using namespace std;
using namespace glm;

//Forward definitions
bool CheckGLErrors(string location);
void QueryGLVersion();
string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

vec2 mousePos;
bool leftmousePressed = false;
bool rightmousePressed = false;
//int t = 0;
float g = 0.000981*2;

float scaleFactor = 1.f;

Camera* activeCamera;

Camera cam = Camera(vec3(0, 0, -1), vec3(0, 0, 1));
// Remember to start this at false for final submission and demo
bool animate = true;
bool drawTrack = false;
bool firstPerson = false;
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
        
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
		animate = !animate;
	}
	if (key == GLFW_KEY_T && action == GLFW_PRESS){
		drawTrack = !drawTrack;
	}
	if (key == GLFW_KEY_F && action == GLFW_PRESS){
		firstPerson = !firstPerson;
		if ( !firstPerson){
			activeCamera->pos = vec3(0,0,7);
			activeCamera->dir = vec3(0,0,-1);
			activeCamera->up = vec3(0,1,0);
			activeCamera->right = vec3(1,0,0);
		} 
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

//Compile and link shaders, storing the program ID in shader array
GLuint initShader(string vertexName, string fragmentName)
{	
	string vertexSource = LoadSource(vertexName);		//Put vertex file text into string
	string fragmentSource = LoadSource(fragmentName);		//Put fragment file text into string

	GLuint vertexID = CompileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragmentID = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
	
	return LinkProgram(vertexID, fragmentID);	//Link and store program ID in shader array
}

//Initialization
void initGL()
{
	glEnable(GL_DEPTH_TEST);
	glPointSize(25);
	glDepthFunc(GL_LEQUAL);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(0.f, 0.f, 0.f, 0.f);		//Color to clear the screen with (R, G, B, Alpha)
}

bool loadUniforms(GLuint program, mat4 perspective, mat4 modelview)
{
	glUseProgram(program);

	glUniformMatrix4fv(glGetUniformLocation(program, "modelviewMatrix"),
						1,
						false,
						&modelview[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(program, "perspectiveMatrix"),
						1,
						false,
						&perspective[0][0]);

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
    window = glfwCreateWindow(512, 512, "OpenGL Example", 0, 0);
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

void generateSphere(vector<vec3>* positions, vector<vec3>* normals, vector<unsigned int>* indices,
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

void generateCylinder(vector<vec3>* positions, vector<vec3>* normals, vector<unsigned int>* indices,
					float r, vec3 center, int uDivisions, int vDivisions, float h)
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
            point.x = r * 2 * cos(v*2*PI) - 0.05;
            point.y = r * 2 * sin(v*2*PI) - 4*r ;
            point.z = -(u*h) + (h/2);
            
            vec3 normal = normalize(point - center);
            normal = vec3 ( 1, 0 ,0 );
            positions->push_back(point);
            normals->push_back(normal);
            
            v+=vStep;
        }
        u+=uStep;
    }
	u = 0.f;
    // Iterate through phi and theta
    for (double phi = 0.; phi < uDivisions; phi ++) // Azimuth [0, 2PI]
    {
		float v = 0.f;
        for (double theta = 0.; theta < vDivisions; theta++) // Elevation [0, PI]
        {
            vec3 point;
            point.x = r * 2 * cos(v*2*PI) + 0.05;
            point.y = r * 2 * sin(v*2*PI) - 4*r ;
            point.z = -(u*h) + (h/2);
            
            vec3 normal = normalize(point - center);
            normal = vec3 ( 1, 0 ,0 );
            positions->push_back(point);
            normals->push_back(normal);
            
            v+=vStep;
        }
        u+=uStep;
    }
   
    for(int i=0; i<(uDivisions*2)-2; i++)
	{
		for(int j=0; j<(vDivisions*2) -2; j++)
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
	
	for( int i =0; i<1; i++){
		normals->push_back(vec3(1.f,1.f,0.f));
	}
	for( int i =0; i<2; i++){
		normals->push_back(vec3(0,0,1));
	}for( int i =0; i<2; i++){
		normals->push_back(vec3(1.f,1.f,0.f));
	}
	for( int i =0; i<2; i++){
		normals->push_back(vec3(0,0,1));
	}
	
	for( int i =0; i<1; i++){
		normals->push_back(vec3(1.f,1.f,0.f));
	}
	
	//TOP
	indices->push_back(3);indices->push_back(2);indices->push_back(6);
	indices->push_back(6);indices->push_back(7);indices->push_back(3);
	
	//RIGHT
	indices->push_back(5);indices->push_back(6);indices->push_back(7);
	indices->push_back(4);indices->push_back(5);indices->push_back(7);
	
	//BACK
	indices->push_back(1);indices->push_back(2);indices->push_back(6);
	indices->push_back(1);indices->push_back(5);indices->push_back(6);

	//LEFT
	indices->push_back(1);indices->push_back(2);indices->push_back(3);
	indices->push_back(0);indices->push_back(1);indices->push_back(3);

	//FRONT
	indices->push_back(0);indices->push_back(4);indices->push_back(7);
	indices->push_back(0);indices->push_back(7);indices->push_back(3);

	//BOTTOM
	indices->push_back(0);indices->push_back(1);indices->push_back(5);
	indices->push_back(0);indices->push_back(5);indices->push_back(4);

    box->updateLeft(2, 3, 1, 0);
    box->updateRight(6, 7, 5, 4);
    box->sideLength = sideLength;

}

float magnitude(vec3 v){
	return sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

/*

Points naming convention

z1y1______________z2y2
|                    |
|                    |
|                    |>>>>>>
|                    |
|                    |
z3y3______________z4y4

*/
void generateNewFaces(vector<vec3>* vertices, vector<vec3>* normals, vector<unsigned int>* indices, Box box) {

}

void createNewPoints(Box box, vector<vec3>* vertices, vector<unsigned int>* indices) {
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
    // Generate the new faces -using the new 4 indices and the old 4 indices
    //   This step involves creating triangles using the two sets of indices (old and new) for each side of the rectangle
    //   then updating each sides indices to the next set (ie old -> new)
}
vector<vec3> generateOffsets(vector<MCNode*>& nodevec)
{
    //double decisionx = ((double)rand() / (RAND_MAX));
    vector<vec3> temp;
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

	//Initialize shader
	GLuint program = initShader("vertex.glsl", "fragment.glsl");
	GLuint vao, vaoSphere, vaoSquare, vaoBox;
	VertexBuffers vbo, vboSphere, vboSquare, vboBox;

	//Generate object ids
	glGenVertexArrays(1, &vao);
	glGenBuffers(VertexBuffers::COUNT, vbo.id);

	initVAO(vao, vbo);
	
	//Geometry information
	vector<vec3> points, normals;
	vector<unsigned int> indices;
	
    Box initialBox = Box();

	generateBox(&points, &normals, &indices, 3.f, &initialBox);
	loadBuffer(vbo, points, normals, indices);
	
	Camera cam = Camera(vec3(0, 0, -1), vec3(0, 0, 10));
	
	activeCamera = &cam;
	
	//float fovy, float aspect, float zNear, float zFar
	mat4 perspectiveMatrix = perspective(radians(80.f), 1.f, 0.1f, 20.f);
	
    mat4 model = mat4(1.f);

    // Setup Markov Chain Tree for Generation
    MCNode* levelNode = new MCNode(0.4f, 0.2f, 0.4f);
    MCNode* inNode = new MCNode(0.4f, 0.2f, 0.4f);
    MCNode* outNode = new MCNode(0.2f, 0.4f, 0.4f, inNode, nullptr, levelNode);
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
    vector<MCNode*> nodePointers;
    nodePointers.push_back(z1currentNode);
    nodePointers.push_back(y1currentNode);
    nodePointers.push_back(z2currentNode);
    nodePointers.push_back(y2currentNode);
    nodePointers.push_back(z3currentNode);
    nodePointers.push_back(y3currentNode);
    nodePointers.push_back(z4currentNode);
    nodePointers.push_back(y4currentNode);

    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//Clear color and depth buffers (Haven't covered yet)
		
		loadUniforms(program, winRatio*perspectiveMatrix*cam.getMatrix(),scale(model, vec3(scaleFactor)));
		
		render(vao, 0, indices.size());
		
        // scene is rendered to the back buffer, so swap to front for display
        glfwSwapBuffers(window);

        // sleep until next event before drawing again
        glfwPollEvents();
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
