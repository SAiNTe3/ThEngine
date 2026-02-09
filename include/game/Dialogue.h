#pragma once
#include <Message.h>

class Dialogue_Illustration
{
public:
	void init(const std::string& path,const std::string& index,bool reverse);
	std::unique_ptr<esl::Texture> full_illustration = nullptr;
	std::unique_ptr<esl::Sprite> full_sprite = nullptr;
	std::vector<std::unique_ptr<esl::Texture>> face_textures;
	int face_index=-1; // 正在显示的脸部表情贴图索引
	std::vector<std::unique_ptr<esl::Sprite>> face_sprites;
	std::unique_ptr<esl::Texture> name_flavor = nullptr;
	std::unique_ptr<esl::Sprite> name_flavor_sprite = nullptr;
	glm::vec2 face_pos{ 0,0 };
	float offsetY = 0;
	float cutScale = 1.0f;
	glm::vec2 ori_pos{ 0,0 };
	glm::vec2 dir_pos{ 0,0 };
	void moveToDir(float deltaTime) {
		glm::vec2 pos = full_sprite->getPosition();
		if (glm::distance(pos, dir_pos) > 8) {
			glm::vec2 dir = glm::normalize(dir_pos - pos);
			full_sprite->setPosition(pos + dir * deltaTime * 400.0f);
		}
		else full_sprite->setPosition(dir_pos);
	}
	void moveToOri(float deltaTime) {
		glm::vec2 pos = full_sprite->getPosition();
		if (glm::distance(pos, ori_pos) > 8) {
			glm::vec2 dir = glm::normalize(ori_pos - pos);
			full_sprite->setPosition(pos + dir * deltaTime * 400.0f);
		}
		else full_sprite->setPosition(ori_pos);
	}
};

class Dialogue
{
	Dialogue_Illustration player;
	Dialogue_Illustration boss;
	bool player_visible = false;
	bool boss_visible = false;
	std::string dialogue_file;
	esl::Window* renderer;
	// test
	esl::Font* font;
	// 退出状态跟踪
	bool mIsExiting = false;
	double mExitDuration = 0.6;  // 退出动画时长
	double mExitTimer = 0.0;
public:
	void init(esl::Window& renderer);
	void hide_player() { player_visible = false; }
	void hide_boss() { boss_visible = false; }
	void show_player() { player_visible = true; }
	void show_boss() { boss_visible = true; }
	void render();
	void update(double deltaTime);
	void process_input(esl::Event& e);
	void reset();
	void startExit();
	bool isExiting() const { return mIsExiting; }
	friend class ScriptSystem;
};