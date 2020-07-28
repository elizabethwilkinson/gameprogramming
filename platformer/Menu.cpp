#include "Menu.h"

void Menu::Initialize() {

	state.nextScene = -1;

	//music = Mix_LoadMUS("music.mp3");
	//Mix_PlayMusic(music,-1); //loop forever (use 0 to play track just 1 time)
	//Mix_VolumeMusic(MIX_MAX_VOLUME/2); //ranges 0 (mute) to 128 (max volume)
	//Mix_HaltMusic();
}

void Menu::Update(float deltaTime) {
	state.nextScene = 1;
}

void Menu::Render(ShaderProgram *program) {
	//DrawText(&program, fontTextureID, "Press Enter to Start", 0.5f, -0.1f, glm::vec3(0, 0, 0));
}