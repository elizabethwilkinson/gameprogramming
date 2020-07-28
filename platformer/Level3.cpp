#include "Level3.h"

#define LEVEL3_ENEMY_COUNT 1

#define LEVEL3_WIDTH 24
#define LEVEL3_HEIGHT 8

unsigned int level3_data[] =
{
 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 3, 2, 1, 1, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0,
 3, 2, 2, 2, 1, 1, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 1, 1,
 3, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2
};

void Level3::Initialize() {

	state.nextScene = -1;

	GLuint mapTextureID = Util::LoadTexture("tileset.png");
	state.map = new Map(LEVEL3_WIDTH, LEVEL3_HEIGHT, level3_data, mapTextureID, 1.0f, 4, 1);
	// Initialize Game Objects

	// Initialize Player
	state.player = new Entity();
	state.player->entityType = PLAYER;
	state.player->position = glm::vec3(5, 0, 0);
	state.player->movement = glm::vec3(0);
	state.player->acceleration = glm::vec3(0, -9.81f, 0); //free fall gravity
	state.player->speed = 1.5f;
	state.player->textureID = Util::LoadTexture("penguin.png");

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

	state.player->jumpPower = 6.0f;

	//music = Mix_LoadMUS("music.mp3");
	//Mix_PlayMusic(music,-1); //loop forever (use 0 to play track just 1 time)
	//Mix_VolumeMusic(MIX_MAX_VOLUME/2); //ranges 0 (mute) to 128 (max volume)
	//Mix_HaltMusic();

	//Initialize Enemies
	state.enemies = new Entity[LEVEL3_ENEMY_COUNT];

	GLuint enemyTextureID = Util::LoadTexture("polar-bear.png");

	for (int i = 0; i < LEVEL3_ENEMY_COUNT; i++) {
		state.enemies[i].entityType = ENEMY;
		state.enemies[i].aiState = IDLE;
		state.enemies[i].textureID = enemyTextureID;
		state.enemies[i].height = 0.75;
		state.enemies[i].speed = 1;
		state.enemies[i].acceleration = glm::vec3(0, -9.81, 0);
		state.enemies[i].isActive = false;
	}

	state.enemies[0].position = glm::vec3(4, -1.5, 0);
	state.enemies[0].aiType = WAITANDGO;

	/*state.enemies[1].position = glm::vec3(2.5, -1.5, 0);
	state.enemies[1].aiType = JUMPER;
	state.enemies[1].jumpPower = 3.0;

	state.enemies[2].position = glm::vec3(1, -1.5, 0);
	state.enemies[2].aiType = WALKER;*/
}

void Level3::Update(float deltaTime) {
	state.player->Update(deltaTime, state.player, state.enemies, LEVEL3_ENEMY_COUNT, state.map);

	/*if (state.player->position.x >= 20) {
		state.player->winFlag = true;
	}*/
}

void Level3::Render(ShaderProgram *program) {
	state.map->Render(program);
	state.player->Render(program);
}