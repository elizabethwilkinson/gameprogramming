#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
ShaderProgram program;
bool gameIsRunning = true;

float player_x = 0;
float fish2_x = 0;
float fish3_x = 0;
float player_rotate = 0;
float rotation_flag = 1;

glm::mat4 viewMatrix;
glm::mat4 modelMatrix; //first fish
glm::mat4 fish2Matrix;
glm::mat4 fish3Matrix;
glm::mat4 anotherModelMatrix; //bubbles
glm::mat4 projectionMatrix;

GLuint LoadTexture(const char* filePath) {
	int w, h, n;
	unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(image);
	return textureID;
}

GLuint playerTextureID; //fish
GLuint anotherTextureID; //penguin

void Initialize() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Simple 2D Scene: Very Excited Penguin!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 480);

	program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

	viewMatrix = glm::mat4(1.0f);
	modelMatrix = glm::mat4(1.0f);
	fish2Matrix = glm::mat4(1.0f);
	fish3Matrix = glm::mat4(1.0f);
	anotherModelMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetColor(1.0f, 1.0f, 0.0f, 1.0f);

	glUseProgram(program.programID);

	glClearColor(0.53f, 0.8f, 0.92f, 1.0f);

	glEnable(GL_BLEND);
	// Good setting for transparency
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	playerTextureID = LoadTexture("fish.png");
	anotherTextureID = LoadTexture("penguin.png");
}

void ProcessInput() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			gameIsRunning = false;
		}
	}
}

float lastTicks = 0.0f;

void Update() {
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - lastTicks;
	lastTicks = ticks;

	player_x += 0.75f*deltaTime;
	fish2_x -= 0.5f*deltaTime;
	fish3_x += 0.4f*deltaTime;
	player_rotate += rotation_flag * -0.01f * deltaTime;

	if (rotation_flag > 0 && player_rotate > 0.2) {
		rotation_flag = -1;
	}
	else if (rotation_flag < 0 && player_rotate < -0.75) {
		rotation_flag = 1;
	}

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 1.5f, 0.0f));	
	modelMatrix = glm::translate(modelMatrix, glm::vec3(player_x, 0.0f, 0.0f));
	
	fish2Matrix = glm::mat4(1.0f);
	fish2Matrix = glm::translate(fish2Matrix, glm::vec3(0.0f, -1.5f, 0.0f));
	fish2Matrix = glm::translate(fish2Matrix, glm::vec3(fish2_x, 0.0f, 0.0f));

	fish3Matrix = glm::mat4(1.0f);
	fish3Matrix = glm::translate(fish3Matrix, glm::vec3(0.0f, -2.0f, 0.0f));
	fish3Matrix = glm::translate(fish3Matrix, glm::vec3(fish3_x, 0.0f, 0.0f));
	
	anotherModelMatrix = glm::rotate(anotherModelMatrix, glm::radians(player_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
	std::cout << "rotation angle: " << player_rotate << "\n";
}

void Render() {
	float vertices[] = { -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,-0.5f,-0.5f,0.5f,0.5f,-0.5f,0.5f };
	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

	glClear(GL_COLOR_BUFFER_BIT);

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);
	
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);
	
	program.SetModelMatrix(modelMatrix); //draw first fish
	glBindTexture(GL_TEXTURE_2D, playerTextureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	program.SetModelMatrix(fish2Matrix); //draw second fish
	glBindTexture(GL_TEXTURE_2D, playerTextureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	program.SetModelMatrix(fish3Matrix); //draw third fish
	glBindTexture(GL_TEXTURE_2D, playerTextureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	program.SetModelMatrix(anotherModelMatrix); //draw penguin
	glBindTexture(GL_TEXTURE_2D, anotherTextureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

	SDL_GL_SwapWindow(displayWindow);

}

void Shutdown() {
	SDL_Quit();
}

int main(int argc, char* argv[]) {
	Initialize();
	//SDL_Init(SDL_INIT_VIDEO);
	//displayWindow = SDL_CreateWindow("Project 1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
	//SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	//SDL_GL_MakeCurrent(displayWindow, context);

//#ifdef _WINDOWS
//	glewInit();
//#endif

	//glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

	while (gameIsRunning) {
		ProcessInput();
		Update();
		Render();
	}

	/*SDL_Event event;
	while (gameIsRunning) {

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				gameIsRunning = false;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(displayWindow);
	}*/

	Shutdown();
	return 0;
}