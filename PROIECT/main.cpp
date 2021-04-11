#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 lightRotation;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;

//punctiform lightning
glm::vec3 lightDirP;
GLuint lightDirLocP;
glm::vec3 lightColorP;
GLuint lightColorLocP;
glm::mat3 normalMatrixP;
GLuint normalMatrixLocP;

// red lighting
glm::vec3 lightColorR;
GLuint lightColorLocR;

// pentru lumina punctiforma
GLuint punctiformMode = 0;
GLuint punctiformModeLoc;
glm::vec3 lightPos;

// pentru lumina directionala rosie
GLuint redLightMode = 0;
GLuint redLightModeLoc;

GLuint fogMode = 0;
GLuint fogModeLoc;

// camera
gps::Camera myCamera(
	glm::vec3(0.0f, 6.0f, 100.0f),
	glm::vec3(0.0f, 3.0f, -10.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D scena;
gps::Model3D dome;
gps::Model3D boat;
gps::Model3D bear;
gps::Model3D fish1;
gps::Model3D fish2;
gps::Model3D lightCube;

int boatFlag = 0;
float boatDeplacement = 0.0f;
int fish1Flag = 0;
float fish1Deplacement = 0.0f;
int fish2Flag = 0;
float fish2Deplacement = 0.0f;
int bearFlag = 0;
float bearDeplacement = 0.0f;

bool presentation_mode = false;
bool firstSetup = true;
int movement = 0;

GLfloat angle;
GLfloat lightAngle;

// shaders
gps::Shader myBasicShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap = false;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;


GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
		case GL_INVALID_ENUM:
			error = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error = "INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			error = "INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW:
			error = "STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			error = "STACK_UNDERFLOW";
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

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) {
			pressedKeys[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			pressedKeys[key] = false;
		}
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	//TODO
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix 
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_Q]) { // rotate right
		angle -= 1.0f;
		// update model matrix
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
		// update normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	}

	if (pressedKeys[GLFW_KEY_E]) { // rotate left
		angle += 1.0f;
		// update model matrix
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
		// update normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_R]) { // rotate down
		angle -= 1.0f;
		// update model matrix
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1, 0, 0));
		// update normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_F]) { // rotate down
		angle += 1.0f;
		// update model matrix 
		model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1, 0, 0));
		// update normal matrix
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_B]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_N]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_M]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}


	if (pressedKeys[GLFW_KEY_P]) { // presentation mode

		if (!presentation_mode) {
			presentation_mode = true;
		}
		else {
			presentation_mode = false;
		}

	}

	if (pressedKeys[GLFW_KEY_K]) { // lumina punctiforma activata
		punctiformMode = 1;
	}

	if (pressedKeys[GLFW_KEY_L]) { // lumina punctiforma dezactivata
		punctiformMode = 0;
	}


	if (pressedKeys[GLFW_KEY_C]) { // lumina rosie activata
		redLightMode = 1;
	}

	if (pressedKeys[GLFW_KEY_V]) { // lumina rosie dezactivata
		redLightMode = 0;
	}

	if (pressedKeys[GLFW_KEY_U]) { // ceata activata
		fogMode = 1;
	}

	if (pressedKeys[GLFW_KEY_I]) { // ceata dezactivata
		fogMode = 0;
	}

	if (pressedKeys[GLFW_KEY_RIGHT]) { // miscare urs
		if (bearDeplacement <= 8.0f) {
			bearDeplacement += 0.1f;
		} 
	}

	if (pressedKeys[GLFW_KEY_LEFT]) { // miscare urs
		if (bearDeplacement <= 10.0f) {
			bearDeplacement -= 0.1f;
		}
	}
}

void initOpenGLWindow() {
	myWindow.Create(1024, 768, "OpenGL Project Core");

}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
	glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
	glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {

	dome.LoadModel("models/scene/my_scene/skydome.obj");
	scena.LoadModel("models/scene/my_scene/myscene1.obj");
	boat.LoadModel("models/scene/my_scene/boat.obj");
	bear.LoadModel("models/scene/my_scene/bear.obj");
	fish1.LoadModel("models/scene/my_scene/fish1.obj");
	fish2.LoadModel("models/scene/my_scene/fish2.obj");

}

void initShaders() {
	myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
	myBasicShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
	depthMapShader.useShaderProgram();
}

glm::mat4 computeLightSpaceTrMatrix() {

	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 0.1f, far_plane = 100.0f;
	glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}

void initUniforms() {
	myBasicShader.useShaderProgram();

	// create model matrix
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");


	//send lightSpace matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");
	punctiformModeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "punctiformMode");
	redLightModeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "redLightMode");
	fogModeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogMode");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
		(float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
		0.1f, 2000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	//set light color
	lightColorR = glm::vec3(1.0f, 0.0f, 0.0f); //red light
	lightColorLocR = glGetUniformLocation(myBasicShader.shaderProgram, "lightColorR");
	glUniform3fv(lightColorLocR, 1, glm::value_ptr(lightColorR));


	// lumina punctiforma
	lightDirP = glm::vec3(-0.12f, 3.6f, 34.2f);
	lightDirLocP = glGetUniformLocation(myBasicShader.shaderProgram, "lightDirP");
	glUniform3fv(lightDirLocP, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDirP));


	lightPos = glm::vec3(-3.12f, 3.6f, 35.8f);
	glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));

	// lumina galbena
	lightColorP = glm::vec3(1.0f, 1.0f, 1.0f);
	glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightColorP"), 1, glm::value_ptr(lightColorP));


}

void initFBO() {

	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
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

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


void renderMyScene(gps::Shader shader, bool depthPass) {
	
	// select active shader program
	shader.useShaderProgram();

	if (depthPass) {
		//send model matrix to shader
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"),
			1,
			GL_FALSE,
			glm::value_ptr(model));
	}

	if (boatFlag == 0) {
		boatDeplacement += 0.008f;
	}
	else {
		boatDeplacement -= 0.008f;
	}
	if (boatDeplacement > 25.0f) {
		boatFlag = 1;
	}
	else if (boatDeplacement < 0.0f) {
		boatFlag = 0;
	}

	model = glm::translate(model, glm::vec3(boatDeplacement, 0.0f, 0.0f));

	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	if (!depthPass) {
		shader.useShaderProgram();
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform1i(punctiformModeLoc, punctiformMode);
		glUniform1i(redLightModeLoc, redLightMode);
		glUniform1i(fogModeLoc, fogMode);
	}

	boat.Draw(shader);

	model = glm::translate(model, glm::vec3(-boatDeplacement, 0.0f, 0.0f));
	
	// BEAR

	if (depthPass) {
		//send model matrix to shader
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"),
			1,
			GL_FALSE,
			glm::value_ptr(model));
	}

	
	model = glm::translate(model, glm::vec3(bearDeplacement, 0.0f, 0.0f));

	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	if (!depthPass) {
		shader.useShaderProgram();
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform1i(punctiformModeLoc, punctiformMode);
		glUniform1i(redLightModeLoc, redLightMode);
		glUniform1i(fogModeLoc, fogMode);
	}

	bear.Draw(shader);

	model = glm::translate(model, glm::vec3(-bearDeplacement, 0.0f, 0.0f));
	

	// FISH1

	if (depthPass) {
		//send model matrix to shader
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"),
			1,
			GL_FALSE,
			glm::value_ptr(model));
	}

	if (fish1Flag == 0) {
		fish1Deplacement += 0.02f;
	}
	else {
		fish1Deplacement -= 0.02f;
	}
	if (fish1Deplacement > 20.0f) {
		fish1Flag = 1;
	}
	else if (fish1Deplacement < 0.0f) {
		fish1Flag = 0;
	}

	model = glm::translate(model, glm::vec3(0.0f,-fish1Deplacement, 0.0f));

	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	if (!depthPass) {
		shader.useShaderProgram();
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform1i(punctiformModeLoc, punctiformMode);
		glUniform1i(redLightModeLoc, redLightMode);
		glUniform1i(fogModeLoc, fogMode);
	}

	fish1.Draw(shader);

	model = glm::translate(model, glm::vec3(0.0f, fish1Deplacement, 0.0f));

	//FISH2

	if (depthPass) {
		//send model matrix to shader
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"),
			1,
			GL_FALSE,
			glm::value_ptr(model));
	}

	if (fish2Flag == 0) {
		fish2Deplacement += 0.05f;
	}
	else {
		fish2Deplacement -= 0.05f;
	}
	if (fish2Deplacement > 20.0f) {
		fish2Flag = 1;
	}
	else if (fish2Deplacement < 0.0f) {
		fish2Flag = 0;
	}

	model = glm::translate(model, glm::vec3(0.0f, -fish2Deplacement, 0.0f));

	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	if (!depthPass) {
		shader.useShaderProgram();
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform1i(punctiformModeLoc, punctiformMode);
		glUniform1i(redLightModeLoc, redLightMode);
		glUniform1i(fogModeLoc, fogMode);
	}

	fish2.Draw(shader);

	model = glm::translate(model, glm::vec3(0.0f, fish2Deplacement, 0.0f));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	//send model matrix data to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	//send normal matrix data to shader

	if (!depthPass) {
		shader.useShaderProgram();
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform1i(punctiformModeLoc, punctiformMode);
		glUniform1i(redLightModeLoc, redLightMode);
		glUniform1i(fogModeLoc, fogMode);
	}

	// draw
	scena.Draw(shader);

	if (!depthPass) {
		if (presentation_mode) {
			if (firstSetup) {
				myCamera = gps::Camera(glm::vec3(0.0f, 6.0f, 100.0f),
					glm::vec3(0.0f, 3.0f, -10.0f),
					glm::vec3(0.0f, 1.0f, 0.0f));

				view = myCamera.getViewMatrix();
				GLint viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
				glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

				firstSetup = false;
			}

			if (movement < 670) {
				myCamera.move(gps::MOVE_RIGHT, 0.03f);
				view = myCamera.getViewMatrix();
				myBasicShader.useShaderProgram();
				glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				// compute normal matrix 
				normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
				movement++;
			}
			else {
				if (movement < 820) {
					myCamera.move(gps::MOVE_FORWARD, 0.003f);
					//update view matrix
					view = myCamera.getViewMatrix();
					myBasicShader.useShaderProgram();
					glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
					// compute normal matrix 
					normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
					movement++;
				}
				else {
					if (movement < 1350) {
						myCamera.move(gps::MOVE_LEFT, 0.03f);
						//update view matrix
						view = myCamera.getViewMatrix();
						myBasicShader.useShaderProgram();
						glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
						// compute normal matrix
						normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
						movement++;
					}
				}
			}

		}
		else {
			firstSetup = true;
			movement = 0;
		}
	}

}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	//render the scene
	renderMyScene(depthMapShader, true);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	myBasicShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	// send view matrix to shader
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//bind the shadow map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

	glViewport(0, 0, (float)myWindow.getWindowDimensions().width, (float)myWindow.getWindowDimensions().height);
	myBasicShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	// render scene without sending the normals
	renderMyScene(myBasicShader, false);
	glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	//send normal matrix data to shader


	myBasicShader.useShaderProgram();
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	glUniform1i(punctiformModeLoc, punctiformMode);
	glUniform1i(redLightModeLoc, redLightMode);
	glUniform1i(fogModeLoc, fogMode);

	dome.Draw(myBasicShader);

}

void cleanup() {
	myWindow.Delete();
	//cleanup code for your own data
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char* argv[]) {

	try {
		initOpenGLWindow();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
	setWindowCallbacks();
	initFBO();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

	return EXIT_SUCCESS;
}
