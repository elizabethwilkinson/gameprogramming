/*
Project 2: Pong
Author: Elizabeth Wilkinson
Due: 6/14/2020
Criteria: (1) Paddles/Players - 2 independent paddles within window; controlled independently 
          (2) Ball bounces off the top and bottom window borders
		  (3) Game ends when the ball hits left or right window borders
*/
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

float paddle_speed = 2.5f;
float ball_speed = 2.0f;

glm::vec3 ball_pos = glm::vec3(0, 0, 0); //ball starts at origin
glm::vec3 ball_path = glm::vec3(0, 0, 0); //stationary
glm::vec3 right_pos = glm::vec3(4.0, 0, 0);
glm::vec3 right_path = glm::vec3(0, 0, 0);
glm::vec3 left_pos = glm::vec3(-4.0, 0, 0);
glm::vec3 left_path = glm::vec3(0, 0, 0);

glm::mat4 viewMatrix;
glm::mat4 modelMatrix; //right paddle
glm::mat4 anotherModelMatrix; //left paddle
glm::mat4 ballMatrix;
glm::mat4 projectionMatrix;

//GLuint LoadTexture(const char* filePath) {
//	int w, h, n;
//	unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
//
//	if (image == NULL) {
//		std::cout << "Unable to load image. Make sure the path is correct\n";
//		assert(false);
//	}
//
//	GLuint textureID;
//	glGenTextures(1, &textureID);
//	glBindTexture(GL_TEXTURE_2D, textureID);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
//
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//
//	stbi_image_free(image);
//	return textureID;
//}
//
//GLuint playerTextureID; 
//GLuint anotherTextureID; 

void Initialize() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 480);

	program.Load("shaders/vertex.glsl", "shaders/fragment.glsl");

	viewMatrix = glm::mat4(1.0f);
	modelMatrix = glm::mat4(1.0f);
	anotherModelMatrix = glm::mat4(1.0f);
	ballMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetColor(1.0f, 1.0f, 1.0f, 1.0f); //paddle color, white

	glUseProgram(program.programID);

	glClearColor(0.53f, 0.8f, 0.92f, 1.0f); //background color, sky blue

	glEnable(GL_BLEND);
	// Good setting for transparency
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ProcessInput() {
	right_path = glm::vec3(0, 0, 0); //stationary
	left_path = glm::vec3(0, 0, 0);

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			gameIsRunning = false;
			break;
		case SDL_KEYDOWN: //any key is pressed
			switch (event.key.keysym.sym) {
			case SDLK_UP:
				//move right paddle up
				right_path.y = 1.0f;
				break;
			case SDLK_DOWN:
				//move right paddle down
				right_path.y = -1.0f;
				break;
			case SDLK_SPACE:
				//ball begins moving
				ball_path = glm::vec3(1.0, 1.0, 0.0);
				break;
			case SDLK_w:
				//move left paddle up
				left_path.y = 1.0f;
				break;
			case SDLK_s:
				//move left paddle down
				left_path.y = -1.0f;
				break;
			}
			break;
		}
	}

	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	//check if certain keys are currently depressed
	if (keys[SDL_SCANCODE_UP]) {
		//move right paddle up
		right_path.y = 1.0f;
	}
	else if (keys[SDL_SCANCODE_DOWN]) {
		//move right paddle down
		right_path.y = -1.0f;
	}
	if (keys[SDL_SCANCODE_W]) {
		//move left paddle up
		left_path.y = 1.0f;
	}
	else if (keys[SDL_SCANCODE_S]) {
		//move left paddle down
		left_path.y = -1.0f;
	}

	//normalization
	if (glm::length(right_path) > 1.0f) {
		right_path = glm::normalize(right_path);
	}
	if (glm::length(left_path) > 1.0f) {
		right_path = glm::normalize(left_path);
	}
}

float lastTicks = 0.0f;

void Update() {
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - lastTicks;
	lastTicks = ticks;

	//Add (direction * units per second * elapsed time)
	right_pos += right_path * paddle_speed * deltaTime;
	left_pos += left_path * paddle_speed * deltaTime;

	//box to box collision detection calculations
	float x_diff_right = fabs(right_pos.x - ball_pos.x);
	float y_diff_right = fabs(right_pos.y - ball_pos.y);
	float x_dist_right = x_diff_right - (0.5 + 0.5) / 2;
	float y_dist_right = y_diff_right - (0.5 + 2) / 2;

	float x_diff_left = fabs(left_pos.x - ball_pos.x);
	float y_diff_left = fabs(left_pos.y - ball_pos.y);
	float x_dist_left = x_diff_left - (0.5 + 0.5) / 2;
	float y_dist_left = y_diff_left - (0.5 + 2) / 2;

	if (ball_pos.y >= 3.75f || ball_pos.y <= -3.75f) { //ball hits top or bottom of window
		//toggle y direction
		ball_path.y *= -1;
	}
	else if (x_dist_right < 0 && y_dist_right < 0) { //ball hits paddle on the right
		//toggle x direction
		ball_path.x *= -1;
	}
	else if (x_dist_left < 0 && y_dist_left < 0) { //ball hits paddle on the left
		//toggle x direction
		ball_path.x *= -1;
	}
	else if (ball_pos.x >= 5.0f || ball_pos.x <= -5.0f) { //ball hits left or right wall
		//game over
		SDL_Quit();
	}
	
	//std::cout << right_pos.x << " " << right_pos.y << "\n";
	ball_pos += ball_path * ball_speed * deltaTime;

	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, right_pos);

	anotherModelMatrix = glm::mat4(1.0f);
	anotherModelMatrix = glm::translate(anotherModelMatrix, left_pos);

	ballMatrix = glm::mat4(1.0f);
	ballMatrix = glm::translate(ballMatrix, ball_pos);
}

void Render() {
	float vertices[] = { -0.25f, -1.0f, 0.25f, -1.0f, 0.25f, 1.0f, -0.25f, -1.0f, 0.25f, 1.0f, -0.25f, 1.0f };
	float ballCoords[] = { -0.25f, -0.25f, 0.25f, -0.25f, 0.25f, 0.25f, -0.25f, -0.25f, 0.25f, 0.25f, -0.25f, 0.25f };

	glClear(GL_COLOR_BUFFER_BIT);

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	//glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	//glEnableVertexAttribArray(program.texCoordAttribute);

	program.SetModelMatrix(modelMatrix); //draw right paddle
	//glBindTexture(GL_TEXTURE_2D, playerTextureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	program.SetModelMatrix(anotherModelMatrix); //draw left paddle
	//glBindTexture(GL_TEXTURE_2D, anotherTextureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	//glDisableVertexAttribArray(program.texCoordAttribute);

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballCoords);
	glEnableVertexAttribArray(program.positionAttribute);

	program.SetModelMatrix(ballMatrix); //draw ball
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);

	SDL_GL_SwapWindow(displayWindow);

}

void Shutdown() {
	SDL_Quit();
}

int main(int argc, char* argv[]) {
	Initialize();

//#ifdef _WINDOWS
//	glewInit();
//#endif

	while (gameIsRunning) {
		ProcessInput(); //store player's intent to move, jump, etc.
		Update(); //test then apply movements
		Render(); //draw the current state of the game 
	}

	Shutdown();
	return 0;
}