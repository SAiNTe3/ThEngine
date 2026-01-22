#pragma once
#include <iostream>
#include <deque>
#include <Sprite.hpp>
#include <Window.hpp>

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
	
	static float collectLineY;
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
	static void generate_items(int num1, int num2, int num3, int num4, int num5, int num6, int num7, glm::vec2 pos, int radius);
	static void RenderAll();
	static void UpdateAll(double delta, float playerPosY);
	static void SetCollectLine(float y);
};