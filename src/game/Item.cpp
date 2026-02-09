#include <Item.h>
#include <cmath>
#include <Player.h>
#include <Scene.h>
// 静态成员变量定义
pTexture Item::itemTexture = nullptr;
esl::Window* Item::mRenderer = nullptr;
std::deque<Item*> Item::mItems;
const float Item::mAcc = 100.f;
const float Item::mMaxSpeed = 150.0f;
Player* Item::mPlayer = nullptr;
Data* Item::mData = nullptr;
float Item::collectLineY = 800;
glm::vec2 Item::centerPos = { 0,0 };

Item::Item(Type type) :mType(type)
{
	mSprite = std::make_unique<esl::Sprite>(itemTexture.get());
	glm::vec2 pos = { 0,0 }, size = { 0,0 };
	switch (type) {
		case Type::PowerUp:
			pos = { 0,32 }, size = { 32,32 }, mBonus = 10;
			break;
		case Type::Power:
			pos = { 192,48 }, size = { 16,16 }, mBonus = 5;
			break;
		case Type::LifeUp:
			pos = { 32*2,32 }, size = { 32,32 }, mBonus = 10;
			break;
		case Type::Life:
			pos = { 32,32 }, size = { 32,32 }, mBonus = 5;
			break;
		case Type::SpellCard:
			pos = { 32*4,32 }, size = { 32,32 }, mBonus = 5;
			break;
		case Type::FullPower:
			pos = { 32*5,32 }, size = { 32,32 }, mBonus = 10;
			break;
		case Type::Point:
			pos = { 192+16,48 }, size = { 16,16 }, mBonus = 20;
			break;
	}
	mSprite->setTextureRect(pos,size);
	mSprite->setScale({ 1.5f,1.5f });
}

Item::~Item()
{
}

void Item::init(esl::Window* renderer, Player* player, Data& data)
{
	mRenderer = renderer;
	mPlayer = player;
	mData = &data;
	if (!itemTexture) {
		itemTexture = std::make_unique<esl::Texture>(".\\Assets\\bullet\\item.png", esl::Texture::Wrap::CLAMP_TO_EDGE, esl::Texture::Filter::NEAREST);
	}
}

void Item::generate_item(Type type, glm::vec2 pos)
{
	Item* item = new Item(type);
	item->mSprite->setPosition(pos);
	mItems.push_back(item);
}

void Item::generate_at_player_death(glm::vec2 pos, glm::vec2 centerPos)
{
	const glm::vec2 offsets[6] = {
		{-64,24}, {-48,48}, {-12, 64}, {12,64}, {48,48}, {64,24}
	};
	Item::centerPos = centerPos;
	for (int i = 0; i < 6; i++) {
		Item* item = new Item(Type::Power);
		item->mSprite->setPosition(pos + offsets[i]);
		item->mMoveToCenter = true;
		item->mTargetPos = centerPos + offsets[i]; 
		mItems.push_back(item);
	}
}

void Item::generate_items(int num1, int num2, int num3, int num4, int num5, int num6, int num7, glm::vec2 pos, int radius)
{
	for(int i=0; i<num1; i++)
		generate_item(Type::PowerUp, MainGame::RandomPos(radius, pos));
	for (int i = 0; i < num2; i++)
		generate_item(Type::Power, MainGame::RandomPos(radius, pos));
	for (int i = 0; i < num3; i++)
		generate_item(Type::LifeUp, MainGame::RandomPos(radius, pos));
	for (int i = 0; i < num4; i++)
		generate_item(Type::Life, MainGame::RandomPos(radius, pos));
	for (int i = 0; i < num5; i++)
		generate_item(Type::SpellCard, MainGame::RandomPos(radius, pos));
	for (int i = 0; i < num6; i++)
		generate_item(Type::FullPower, MainGame::RandomPos(radius, pos));
	for (int i = 0; i < num7; i++)
		generate_item(Type::Point, MainGame::RandomPos(radius,pos));
}

void Item::RenderAll()
{
	for(auto& item : mItems)
	{
		item->render();
	}
}

void Item::UpdateAll(double delta,float playerPosY)
{
	bool allCollect = playerPosY>=collectLineY;
	for (auto it = mItems.begin(); it != mItems.end(); ) {
		// 如果触发了全收取，则强制收取
		if (allCollect) {
			(*it)->isCollected = true;
		}
		(*it)->update(delta);
		if (!(*it)->checkAvailable()) {
			it = mItems.erase(it);
		}
		else {
			++it;
		}
	}
	
}

void Item::SetCollectLine(float y)
{
	collectLineY = y;
}

void Item::cleanup()
{
	// 清理所有未收集的道具
	for (auto* item : mItems) {
		if (item) {
			delete item;
		}
	}
	mItems.clear();
	
	// 清理静态纹理资源
	itemTexture.reset();
	
	// 重置静态指针
	mRenderer = nullptr;
	mPlayer = nullptr;
	mData = nullptr;
}

void Item::update(double delta)
{

	float dist = glm::distance(mPlayer->get_position(), mSprite->getPosition());
	if (isCollected) {
		// 靠近玩家
		glm::vec2 direction = glm::normalize(mPlayer->get_position() - mSprite->getPosition());
		mSprite->move(direction * 600.f * static_cast<float>(delta));
		if (dist < 8) {
			// 被玩家吃掉
			mSprite->setAvailable(false);
			Player::getItemSoundEffect();
			// 根据物品类型增加相应的属性
			switch (mType) {
			case Type::PowerUp:
				mData->mPlayerPower < 300 ? mData->mPlayerPower += 100 : mData->mPlayerPower = 400;
				break;
			case Type::Power:
				mData->mPlayerPower < 400 ? mData->mPlayerPower += 1 : mData->mPlayerPower = 400;
				break;
			case Type::LifeUp:
				mData->mPlayerLife < 7 ? mData->mPlayerLife++ : mData->mPlayerLife = 7;
				break;
			case Type::Life:
				mData->mPlayerLife < 7 ? mData->mPlayerLife++ : mData->mPlayerLife = 7;
				break;
			case Type::SpellCard:
				mData->mPlayerSpellCard < 7 ? mData->mPlayerSpellCard++ : mData->mPlayerSpellCard = 7;
				break;
			case Type::FullPower:
				mData->mPlayerPower = 400;
				break;
			case Type::Point:
				break;
			}
			mData->mPlayerScore += mBonus;
		}

	}
	else if (dist < mPlayer->mCollectRadius) {
		isCollected = true;
	}
	else {
		if (this->mMoveToCenter) {
			// 向中心点移动
			glm::vec2 direction = glm::normalize(mTargetPos - mSprite->getPosition());
			mSprite->move(direction * 300.f * static_cast<float>(delta));
			float distanceToTarget = glm::distance(mTargetPos, mSprite->getPosition());
			if (distanceToTarget < 4.f) {
				mMoveToCenter = false;
			}
		}
		else {
			// 如果速度小于最大速度，则加速
			if (mSpeed < mMaxSpeed)
				mSpeed += mAcc * static_cast<float>(delta);
			// 否则匀速移动
			else mSpeed = mMaxSpeed;
			mSprite->move({ 0,-mSpeed * static_cast<float>(delta) });
		}
	}
}

bool Item::checkAvailable()
{
	if (!mSprite->getAvailable()) return false;
	if(mSprite->getPosition().x<-64|| mSprite->getPosition().x>64+768||
		mSprite->getPosition().y<-32|| mSprite->getPosition().y>960+32)
	{
		mSprite->setAvailable(false);
		return false;
	}
	return true;
}

void Item::render()
{
	if (mRenderer && mSprite) {
		mRenderer->draw(*mSprite);
	}
}
