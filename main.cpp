#define GLEW_STATIC
#include <GL/glew.h>


#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"

#include <iostream>


int retina_width, retina_height;
GLFWwindow* glWindow = NULL;
int glWindowWidth = 1920;
int glWindowHeight = 1080;

const unsigned int SHADOW_WIDTH = 8192;
const unsigned int SHADOW_HEIGHT = 8192;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 lightRotation;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::vec3 lightPunctiform;
glm::vec3 lightPunctiformColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint afisCeataLoc;
GLint afisLuminaLoc;
GLuint shadowMapFBO;
GLuint depthMapTexture;
GLint lightPunctiformLoc;
GLint lightPunctiformColorLoc;

//constants for fog
int enable = 0, button = 0;
float density = 1.2f;

gps::Camera myCamera(
	glm::vec3(3.0f, 3.0f, 5.5f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.9f;

bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

//models
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D plane;
gps::Model3D stuff;

//shader
gps::Shader depthMapShader;
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;


bool showDepthMap;

bool firstMouse = true;
float lastX = glWindowWidth;
float lastY = glWindowHeight;
float initFog = 0.2f;

GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:
			error = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error = "INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			error = "INVALID_OPERATION";
			break;
		case GL_OUT_OF_MEMORY:
			error = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "Window resized to width: %d, and height: %d\n", width, height);

	// Update the global window dimensions
	glWindowWidth = width;
	glWindowHeight = height;

	// Get framebuffer size for high-DPI displays
	int retina_width, retina_height;
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	// Update the viewport
	glViewport(0, 0, retina_width, retina_height);

	// Recalculate the projection matrix
	glm::mat4 projection = glm::perspective(
		glm::radians(45.0f),            // Field of view
		(float)width / (float)height,   // Aspect ratio
		0.1f,                           // Near plane
		1000.0f                         // Far plane
	);

	// Pass the updated projection matrix to the shader
	GLuint projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	fprintf(stdout, "Projection matrix updated and sent to the shader.\n");
}


void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

double initX = 400, initY = 300;
double newX = 0, newY = 0;
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	xpos = (initX - xpos) / 10;
	ypos = -(initY - ypos) / 10;
	myCamera.rotate(ypos, xpos);
	glfwSetCursorPos(window, initX, initY);
}



void processMovement()

{
	if (pressedKeys[GLFW_KEY_Q])
	{
		angleY -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_E])
	{
		angleY += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_J])
	{
		lightAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_L])
	{
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W])
	{
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S])
	{
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A])
	{
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D])
	{
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_F]) {
		myCustomShader.useShaderProgram();
		glUniform1i(afisCeataLoc, 1);
	}

	if (pressedKeys[GLFW_KEY_G]) {
		myCustomShader.useShaderProgram();
		glUniform1i(afisCeataLoc, 0);
	}

	if (pressedKeys[GLFW_KEY_V]) {
		myCustomShader.useShaderProgram();
		glUniform1i(afisLuminaLoc, 1);
	}


	if (pressedKeys[GLFW_KEY_B]) {
		myCustomShader.useShaderProgram();
		glUniform1i(afisLuminaLoc, 0);
	}

	if (pressedKeys[GLFW_KEY_1]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_2]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glPointSize(2.0f);
	}

	if (pressedKeys[GLFW_KEY_3]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_SPACE]) {
		myCamera.afisCamera();
	}

	cameraSpeed = pressedKeys[GLFW_KEY_LEFT_SHIFT] ? 0.5f : 0.01f;
}

bool initOpenGLWindow()
{
	if (!glfwInit())
	{
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// window scaling for HiDPI displays
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	// for sRBG framebuffer
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	// for antialising
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow)
	{
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined(__APPLE__)
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION);	// version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	// for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS);	 // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE);	 // cull face
	glCullFace(GL_BACK);	 // cull back face
	glFrontFace(GL_CCW);	 // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects()
{
	lightCube.LoadModel("objects/cube/cube.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
	plane.LoadModel("objects/Plane/scena.obj");
	stuff.LoadModel("objects/stuff/scena.obj");
}

void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depth.vert", "shaders/depth.frag");
	depthMapShader.useShaderProgram();
}

void initUniforms()
{
	myCustomShader.useShaderProgram();
	
	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// set the light direction (direction towards the light)
	lightDir = glm::vec3(-40.0f, 60.0f, 60.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	// set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); // white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	myCustomShader.useShaderProgram();
	afisCeataLoc = glGetUniformLocation(myCustomShader.shaderProgram, "afisCeata");

	myCustomShader.useShaderProgram();
	//lumina punctiforma
	lightPunctiform = glm::vec3(10.842, -2.5834, 1.8286);

	lightPunctiformLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPunctiform");
	glUniform3fv(lightPunctiformLoc, 1, glm::value_ptr(lightPunctiform));
	lightPunctiformColor = glm::vec3(10.842, -2.5834, 1.8286);
	lightPunctiformColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, " lightPunctiformColor");
	glUniform3fv(lightPunctiformColorLoc, 1, glm::value_ptr(lightPunctiformColor));

	afisLuminaLoc = glGetUniformLocation(myCustomShader.shaderProgram, "afisLuminaDoi");
}

void initFBO()
{
	// TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	glGenFramebuffers(1, &shadowMapFBO);
	// create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix()
{
	// TODO - Return the light-space transformation matrix
	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	const GLfloat near_plane = 0.1f, far_plane = 500.0f;
	glm::mat4 lightProjection = glm::ortho(-75.0f, 75.0f, -75.0f, 75.0f, near_plane, far_plane);

	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView *glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, -1.0f, 0.0f));

	return lightSpaceTrMatrix;
}




void drawObjects(gps::Shader shader, bool depthPass)
{

	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass)
	{
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	stuff.Draw(shader);

	/*model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));*/

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass)
	{
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}


	plane.Draw(shader);


}

void renderShadow() {
	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, true);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderScene()
{
	// Render depth map first
	renderShadow();

	// Render depth map on-screen if toggled
	if (showDepthMap)
	{
		glViewport(0, 0, retina_width, retina_height);
		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		// Final scene rendering pass
		glViewport(0, 0, retina_width, retina_height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		// Draw a white cube around the light
		lightShader.useShaderProgram();
		glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), lightDir); // Light's position
		lightModel = glm::scale(lightModel, glm::vec3(0.5f));            // Scale the cube
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		lightCube.Draw(lightShader);
	}
}


void cleanup()
{
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	// close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char* argv[])
{

	if (!initOpenGLWindow())
	{
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();

	

	glCheckError();

	while (!glfwWindowShouldClose(glWindow))
	{
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}