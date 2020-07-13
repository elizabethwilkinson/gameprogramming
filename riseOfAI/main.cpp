/*
Project 4: Rise of the AI
Author: Elizabeth Wilkinson
Course: CS-UY 3113 Introduction to Game Programming
Due: July 12, 2020
Criteria: (1) 3 AI with unique behaviors
		  (2) Ability to defeat enemies
		  (3) Defeat screen
		  (4) Win screen
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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Entity.h"

#define PLATFORM_COUNT 11
#define ENEMY_COUNT 3

struct GameState {
	Entity *player;
	Entity *platforms;
	Entity *walls;
	Entity *enemies;
};

GameState state;

//Mix_Music *music;

SDL_Window* displayWindow;

bool gameIsRunning = true;

GLuint fontTextureID;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

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


void Initialize() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	displayWindow = SDL_CreateWindow("Project 4: Rise of the AI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

	//int Mix_OpenAudtio(int frequency, Uint16 format, int channels, int chunksize);
	//Start Audio
	//Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

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

	fontTextureID = LoadTexture("pixel_font.png");

	// Initialize Game Objects

	// Initialize Player
	state.player = new Entity();
	state.player->entityType = PLAYER;
	state.player->position = glm::vec3(-4, -1, 0);
	state.player->movement = glm::vec3(0);
	state.player->acceleration = glm::vec3(0, -9.81f, 0); //free fall gravity
	state.player->speed = 1.5f;
	state.player->textureID = LoadTexture("penguin.png");

	state.player->animRight = new int[4]{ 24, 25, 26, 25 };
	state.player->animLeft = new int[4]{ 12, 13, 14, 13 };
	state.player->animUp = new int[4]{ 36, 37, 38, 37 };
	state.player->animDown = new int[4]{ 0, 1, 2, 1 };

	state.player->animIndices = state.player->animRight;
	state.player->animFrames = 4;
	state.player->animIndex = 0;
	state.player->animTime = 0;
	state.player->animCols = 12;
	state.player->animRows = 8;

	state.player->height = 1.0f;
	state.player->width = 1.0f;

	state.player->jumpPower = 5.0f;

	//Initialize Platform Tiles
	state.platforms = new Entity[PLATFORM_COUNT];

	GLuint platformTextureID = LoadTexture("platformPack_tile033.png");

	for (int i = 0; i < PLATFORM_COUNT; i++) {
		state.platforms[i].entityType = PLATFORM;
		state.platforms[i].textureID = platformTextureID;
		state.platforms[i].position = glm::vec3(-5 + i, -3.25f, 0);
	}

	//state.platforms[1].isActive = false;

	for (int i = 0; i < PLATFORM_COUNT; i++) {
		state.platforms[i].Update(0, NULL, NULL, 0, NULL, 0);
	}

	//music = Mix_LoadMUS("music.mp3");
	//Mix_PlayMusic(music,-1); //loop forever (use 0 to play track just 1 time)
	//Mix_VolumeMusic(MIX_MAX_VOLUME/2); //ranges 0 (mute) to 128 (max volume)
	//Mix_HaltMusic();

	//Initialize Enemies
	state.enemies = new Entity[ENEMY_COUNT];

	GLuint enemyTextureID = LoadTexture("polar-bear.png");

	for (int i = 0; i < ENEMY_COUNT; i++) {
		state.enemies[i].entityType = ENEMY;
		state.enemies[i].aiState = IDLE;
		state.enemies[i].textureID = enemyTextureID;
		state.enemies[i].height = 0.75;
		state.enemies[i].speed = 1;
		state.enemies[i].acceleration = glm::vec3(0, -9.81, 0);
	}

	state.enemies[0].position = glm::vec3(4, -1.5, 0);
	state.enemies[0].aiType = WAITANDGO;

	state.enemies[1].position = glm::vec3(2.5, -1.5, 0);
	state.enemies[1].aiType = JUMPER;
	state.enemies[1].jumpPower = 3.0;

	state.enemies[2].position = glm::vec3(1, -1.5, 0);
	state.enemies[2].aiType = WALKER;
}

void ProcessInput() {

	state.player->movement = glm::vec3(0);

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

			case SDLK_SPACE:
				// jump
				if (state.player->collidedBottom) {
					state.player->jump = true;
				}
				break;
			}
			break; // SDL_KEYDOWN
		}
	}

	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	if (keys[SDL_SCANCODE_LEFT]) {
		state.player->movement.x = -1.0f;
		state.player->animIndices = state.player->animLeft;
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {
		state.player->movement.x = 1.0f;
		state.player->animIndices = state.player->animRight;
	}

	if (glm::length(state.player->movement) > 1.0f) {
		state.player->movement = glm::normalize(state.player->movement);
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
		state.player->Update(FIXED_TIMESTEP, state.player, state.platforms, PLATFORM_COUNT, state.enemies, ENEMY_COUNT);

		for (int i = 0; i < ENEMY_COUNT; i++) {
			state.enemies[i].Update(FIXED_TIMESTEP, state.player, state.platforms, PLATFORM_COUNT, state.enemies, ENEMY_COUNT);
		}

		deltaTime -= FIXED_TIMESTEP;
	}

	accumulator = deltaTime;
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

	for (int i = 0; i < PLATFORM_COUNT; i++) {
		state.platforms[i].Render(&program);
	}

	for (int i = 0; i < ENEMY_COUNT; i++) {
		state.enemies[i].Render(&program);
	}

	state.player->Render(&program);

	if (!state.player->isActive) {
		DrawText(&program, fontTextureID, "Mission Failed", 0.5f, -0.1f, glm::vec3(-2.5, 1.0, 0));
	}

	int enemiesLeft = ENEMY_COUNT;

	for (int i = 0; i < ENEMY_COUNT; i++) {
		if (!state.enemies[i].isActive) {
			enemiesLeft--;
		}
	}

	if (enemiesLeft == 0) {
		DrawText(&program, fontTextureID, "Mission Success!", 0.5f, -0.1f, glm::vec3(-3.0, 1.0, 0));
	}

	/*if (state.player->isActive && enemyCount == 0) {
		DrawText(&program, fontTextureID, "Mission Success!", 0.5f, -0.1f, glm::vec3(-3.0, 1.0, 0));
	}*/
	//else if (state.player->lastCollision == WALL || state.player->lastCollision == ENEMY) {
	//	DrawText(&program, fontTextureID, "Mission Failed", 0.5f, -0.1f, glm::vec3(-2.5, 1.0, 0));
	//}
	//else if (state.player->position.y <= -3.5f) { //player hits bottom of window
	//	DrawText(&program, fontTextureID, "Mission Failed", 0.5f, -0.1f, glm::vec3(-2.5, 1.0, 0));
	//}

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
		Render();
	}

	Shutdown();
	return 0;
}