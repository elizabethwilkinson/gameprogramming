#include "Level1.h"

#define LEVEL1_ENEMY_COUNT 1

#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8

unsigned int level1_data[] =
{
 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
 3, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

void Level1::Initialize() {

	state.nextScene = -1;

	GLuint mapTextureID = Util::LoadTexture("tileset.png");
	state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, level1_data, mapTextureID, 1.0f, 4, 1);
	// Initialize Game Objects

	// Initialize Player
	state.player = new Entity();
	state.player->entityType = PLAYER;
	state.player->position = glm::vec3(2, 0, 0);
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
	state.player->width = 0.5f;

	state.player->jumpPower = 7.0f;

	//Initialize Enemies
	state.enemies = new Entity[LEVEL1_ENEMY_COUNT];

	GLuint enemyTextureID = Util::LoadTexture("polar-bear.png");

	state.enemies[0].entityType = ENEMY;
	state.enemies[0].aiType = WALKER;
	//state.enemies[0].aiState = IDLE;
	state.enemies[0].textureID = enemyTextureID;
	state.enemies[0].height = 0.75;
	state.enemies[0].speed = 1;
	state.enemies[0].acceleration = glm::vec3(0, -9.81, 0);
	state.enemies[0].position = glm::vec3(7, 0, 0);
}

void Level1::Update(float deltaTime) {
	state.player->Update(deltaTime, state.player, state.enemies, LEVEL1_ENEMY_COUNT, state.map);

	state.enemies->Update(deltaTime, state.player, state.enemies, LEVEL1_ENEMY_COUNT, state.map);

	if (state.player->position.x >= 12) {
		state.nextScene = 2;
	}
}

void Level1::Render(ShaderProgram *program) {
	state.map->Render(program);
	state.player->Render(program);
	state.enemies->Render(program);
}