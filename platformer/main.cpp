/*
Project 5: Platformer!
Author: Elizabeth Wilkinson
Course: CS-UY 3113 Introduction to Game Programming
Due: July 26, 2020
Criteria: (1) Menu screen: title, press enter to start
		  (2) 3 unique scrolling levels
		  (3) 3 lives dictating "You Win" and "You Lose" screens 
		  (4) At least 1 type of moving AI
		  (5) Audio: 1 looping background track, at least 1 sound effect
*/
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#include "Entity.h"
#include "Map.h"
#include "Util.h"
#include "Scene.h"
#include "Menu.h"
#include "Level1.h"
#include "Level2.h"
#include "Level3.h"

Mix_Music *music;

SDL_Window* displayWindow;

bool gameIsRunning = true; //default to false?

GLuint fontTextureID;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

Scene *currentScene;
Scene *sceneList[3];

void SwitchToScene(Scene *scene) {
	currentScene = scene;
	currentScene->Initialize();
}

void Initialize() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	displayWindow = SDL_CreateWindow("Project 5: Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 480);

	program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

	viewMatrix = glm::mat4(1.0f);
	modelMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);

	glUseProgram(program.programID);

	glClearColor(0.56f, 0.56f, 0.74f, 1.0f);
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	fontTextureID = Util::LoadTexture("pixel_font.png");

	//music = Mix_LoadMUS("wallpaper.mp3");
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	//Mix_PlayMusic(music,-1); //loop forever (use 0 to play track just 1 time)
	//Mix_VolumeMusic(MIX_MAX_VOLUME/2); //ranges 0 (mute) to 128 (max volume)
	//Mix_HaltMusic();

	//sceneList[0] = new Menu();
	sceneList[0] = new Level1();
	sceneList[1] = new Level2();
	sceneList[2] = new Level3();
	SwitchToScene(sceneList[0]);
}

void ProcessInput() {
	
	currentScene->state.player -> movement = glm::vec3(0);

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			gameIsRunning = false;
			break;

		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_LEFT:
				// Move the player left
				break;

			case SDLK_RIGHT:
				// Move the player right
				break;

			//case SDLK_RETURN:
			//	//Start the game if on menu screen
			//	gameIsRunning = true;
			//	break;
			case SDLK_SPACE:
				// jump
				if (currentScene->state.player->collidedBottom) {
					currentScene->state.player->jump = true;
				}
				break;
			}
			break; // SDL_KEYDOWN
		}
	}

	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_LEFT]) {
		currentScene->state.player->movement.x = -1.0f;
		currentScene->state.player->animIndices = currentScene->state.player->animLeft;
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {
		currentScene->state.player->movement.x = 1.0f;
		currentScene->state.player->animIndices = currentScene->state.player->animRight;
	}

	if (glm::length(currentScene->state.player->movement) > 1.0f) {
		currentScene->state.player->movement = glm::normalize(currentScene->state.player->movement);
	}

}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void Update() {
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - lastTicks;
	lastTicks = ticks;

	deltaTime += accumulator;
	if (deltaTime < FIXED_TIMESTEP) {
		accumulator = deltaTime;
		return;
	}

	while (deltaTime >= FIXED_TIMESTEP) {
		// Update. Notice it's FIXED_TIMESTEP. Not deltaTime
		currentScene->Update(FIXED_TIMESTEP);
		deltaTime -= FIXED_TIMESTEP;
	}

	accumulator = deltaTime;

	viewMatrix = glm::mat4(1.0f);
	if (currentScene->state.player->position.x > 5) {
		viewMatrix = glm::translate(viewMatrix,
			glm::vec3(-currentScene->state.player->position.x, 3.75, 0));
	}
	else {
		viewMatrix = glm::translate(viewMatrix, glm::vec3(-5, 3.75, 0));
	}
}

void DrawText(ShaderProgram *program, GLuint fontTextureID, std::string text, float size, float spacing, glm::vec3 position) {
	float width = 1.0f / 16.0f;
	float height = 1.0f / 16.0f;

	std::vector<float> vertices;
	std::vector<float> texCoords;

	for (int i = 0; i < text.size(); i++) {
		int index = (int)text[i];
		float offset = (size + spacing) * i;
		float u = (float)(index % 16) / 16.0f;
		float v = (float)(index / 16) / 16.0f;

		vertices.insert(vertices.end(), {
			offset + (-0.5f * size), 0.5f * size,
			offset + (-0.5f * size), -0.5f * size,
			offset + (0.5f * size), 0.5f * size,
			offset + (0.5f * size), -0.5f * size,
			offset + (0.5f * size), 0.5f * size,
			offset + (-0.5f * size), -0.5f * size, });

		texCoords.insert(texCoords.end(), {
			u, v,
			u, v + height,
			u + width, v,
			u + width, v + height,
			u + width, v,
			u, v + height,
			});

	}

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	program->SetModelMatrix(modelMatrix);

	glUseProgram(program->programID);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
	glEnableVertexAttribArray(program->texCoordAttribute);

	glBindTexture(GL_TEXTURE_2D, fontTextureID);
	glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void Render() {
	glClear(GL_COLOR_BUFFER_BIT);

	program.SetViewMatrix(viewMatrix);

	currentScene->Render(&program);

	DrawText(&program, fontTextureID, "Lives: " + std::to_string(currentScene ->state.player->lives), 0.5f, -0.1f, glm::vec3(2.0, 1.0, 0));

	/*if (currentScene == sceneList[0]) {
		DrawText(&program, fontTextureID, "Press Enter to Start", 0.5f, -0.1f, glm::vec3(0, 0, 0));
	}

	if (currentScene->state.player->lives == 0) {
		DrawText(&program, fontTextureID, "Mission Failed", 0.5f, -0.1f, glm::vec3(-2.5, 1.0, 0));
	}

	if (currentScene->state.player->winFlag) {
		DrawText(&program, fontTextureID, "Mission Success!", 0.5f, -0.1f, glm::vec3(0));
	}*/

	SDL_GL_SwapWindow(displayWindow);
}


void Shutdown() {
	SDL_Quit();
}

int main(int argc, char* argv[]) {
	Initialize();

	while (gameIsRunning) {
		ProcessInput();
		Update();

		if (currentScene->state.nextScene >= 0) SwitchToScene(sceneList[currentScene->state.nextScene]);

		Render();
	}

	Shutdown();
	return 0;
}