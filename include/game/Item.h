#pragma once
#include <iostream>
#include <deque>
#include <Sprite.hpp>
#include <Window.hpp>
#include <functional>
using pTexture = std::unique_ptr<esl::Texture>;
using pSprite = std::unique_ptr<esl::Sprite>;

class Player;

class Item {

protected:
	static esl::Window* mRenderer;
	static pTexture itemTexture;
	pSprite mSprite;
	float mSpeed = -100;
	unsigned int mBonus = 0;
	static const float mAcc;
	static const float mMaxSpeed;
	static std::deque<Item*> mItems;
	static Player* mPlayer;
	static struct Data* mData;
	void update(double delta);
	bool checkAvailable();
	void render();
	bool isCollected = false;
	bool mMoveToCenter = false;
	static float collectLineY;
	static glm::vec2 centerPos;
	glm::vec2 mTargetPos = {0,0};
public:
	enum class Type {
		PowerUp,
		Power,
		LifeUp,
		Life,
		SpellCard,
		FullPower,
		Point
	}mType;
	Item(Type type);
	~Item();
	static void init(esl::Window* renderer, Player* player, struct Data& data);
	static void generate_item(Type type, glm::vec2 pos);
	static void generate_at_player_death(glm::vec2 pos, glm::vec2 centerPos);
	static void generate_items(int PowerUp, int Power, int LifeUp, int Life, int SpellCard, int FullPower, int Point, glm::vec2 pos, int radius);
	static void RenderAll();
	static void UpdateAll(double delta, float playerPosY);
	static void SetCollectLine(float y);
	static void cleanup();
};