#include <Front.h>
#include <Enemy.h>
void Front::setDifficultyMode(int difficulty,glm::vec2 pos)
{
	glm::vec2 base_pos(512,256);
	glm::vec2 size(160,32);
	mDifficultyIcons->setTextureRect(glm::vec2{ base_pos.x, base_pos.y + size.y * difficulty }, size);
	mDifficultyIcons->setPosition(pos);
}

void Front::setItemGetBorderLine(glm::vec2 pos)
{
	mItemGetBorderLine->setPosition(pos);
	mItemGetBorderLine->setAlpha(0.0f);
	mItemGetBorderText->setPosition(pos + glm::vec2{ 0,32 });
	mItemGetBorderText->setAlpha(0.0f);
}

void Front::showItemGetAnimation()
{
	mItemGetAnimationStarted = true;
	mItemGetAnimationTime = 0.0;
}

void Front::showBossXPosIndicator(Boss* boss)
{
	mBossXPosIndicatorEnabled = true;
	mSharedBoss = boss;
}

void Front::hideBossXPosIndicator()
{
	mSharedBoss = nullptr;
	mBossXPosIndicatorEnabled = false;
	mBossXPosIndicator->setAlpha(0);
	mBossXPosIndicatorAlphaReverseTime = 0;
}

Front::Front(esl::Window& window) : renderer(window)
{
	const std::string texture_path = "Assets/front/front00.png";
	mTexture = std::make_unique<esl::Texture>(texture_path, esl::Texture::Wrap::CLAMP_TO_EDGE, esl::Texture::Filter::NEAREST);
	mLeftSprite = std::make_unique<esl::Sprite>(mTexture.get());
	mRightSprite = std::make_unique<esl::Sprite>(mTexture.get());
	mTopSprite = std::make_unique<esl::Sprite>(mTexture.get());
	mBottomSprite = std::make_unique<esl::Sprite>(mTexture.get());

	mLeftSprite->setTextureRect({ 0,64 }, { 64,960 });
	mRightSprite->setTextureRect({ 64,64 }, { 448,960 });
	mTopSprite->setTextureRect({ 0,32 }, { 768,32 });
	mBottomSprite->setTextureRect({ 0,0 }, { 768,32 });

	glm::vec2 size = mLeftSprite->getLocalSize();
	mLeftSprite->setPosition({ 0.f+size.x/2,0.f+size.y/2 });
	size = mRightSprite->getLocalSize();
	mRightSprite->setPosition({ 768+64+size.x/2,0.f+size.y/2 });
	size = mTopSprite->getLocalSize();
	mTopSprite->setPosition({ 64.f+size.x/2, 960-32+size.y/2 });
	size = mBottomSprite->getLocalSize();
	mBottomSprite->setPosition({ 64.f+size.x/2,0.f+size.y/2 });

	glm::vec2 pos = { 64 + 768 + 64,960 - 128 };
	// 最高得分
	mHighScore = std::make_unique<esl::Sprite>(mTexture.get());
	mHighScore->setTextureRect({ 512,1024-36 }, { 150,36 });
	mHighScore->setPosition({ pos.x+32, pos.y });
	// 得分
	mScore = std::make_unique<esl::Sprite>(mTexture.get());
	mScore->setTextureRect({ 512,1024-72 }, { 150,36 });
	mScore->setPosition({ pos.x + 32, pos.y-64 });
	// 剩余人数
	mRemainLife = std::make_unique<esl::Sprite>(mTexture.get());
	mRemainLife->setTextureRect({ 512,1024 - 108 }, { 150,36 });
	mRemainLife->setPosition({ pos.x + 32, pos.y - 64*2 });
	// SpellCard
	mSpellCard = std::make_unique<esl::Sprite>(mTexture.get());
	mSpellCard->setTextureRect({ 512+150,1024-36 }, { 164,36 });
	mSpellCard->setPosition({ pos.x + 32+14, pos.y - 64 * 3 });
	// 灵力
	mPower = std::make_unique<esl::Sprite>(mTexture.get());
	mPower->setTextureRect({ 512 + 150,1024 - 4*36 }, { 150,36 });
	mPower->setPosition({ pos.x + 64, pos.y - 64 * 4 });
	// 金钱
	mMoney = std::make_unique<esl::Sprite>(mTexture.get());
	mMoney->setTextureRect({ 512 + 150,1024 - 3*36 }, { 150,36 });
	mMoney->setPosition({ pos.x + 64, pos.y - 64 * 5 });

	mRect = { 32,36 };
	pos = { 320 + 512,1024 - 36 };
	for (int i = 0; i < 7; i++) {
		mLifes[i] = std::make_unique<esl::Sprite>(mTexture.get());
		mLifes[i]->setTextureRect(pos, mRect);
		mLifes[i]->setPosition({ mRemainLife->getPosition().x + 128 + 32 * i, mRemainLife->getPosition().y});
		mSpellCards[i] = std::make_unique<esl::Sprite>(mTexture.get());
		mSpellCards[i]->setTextureRect({ pos.x, pos.y-39 }, mRect);
		mSpellCards[i]->setPosition({ mRemainLife->getPosition().x + 128 + 32 * i, mSpellCard->getPosition().y });
	}

	mDifficultyIcons = std::make_unique<esl::Sprite>(mTexture.get());
	mItemGetBorderLine = std::make_unique<esl::Sprite>(mTexture.get());
	mItemGetBorderLine->setTextureRect({ 512,416 }, { 334,48 });
	mItemGetBorderLine->setScale({ 2,1 });
	mItemGetBorderText = std::make_unique<esl::Sprite>(mTexture.get());
	mItemGetBorderText->setTextureRect({ 512,448 }, { 334,64 });
	
	mBossXPosIndicator = std::make_unique<esl::Sprite>(mTexture.get());
	mBossXPosIndicator->setTextureRect({ 928,0 }, { 96,32 });
	char_map_init();
}
void Front::bindData(unsigned int& score, unsigned int& highscore, unsigned int& life, unsigned int& spellcard, unsigned int& power, unsigned int& money)
{
	mData.score = &score;
	mData.highscore = &highscore;
	mData.life = &life;
	mData.spellcard = &spellcard;
	mData.power = &power;
	mData.money = &money;
}
Front::~Front()
{
	delete[] mHighScoreText;
	delete[] mScoreText;
	delete[] mPowerText;
	delete[] mMoneyText;
}
void Front::char_map_init()
{
	const std::string ascii_texture_path = "Assets/ascii/ascii_big.png";
	const std::string asciif_texture_path = "Assets/ascii/ascii_bigf.png";
	mCharMap[0].loadFromFile(ascii_texture_path);
	mCharMap[1].loadFromFile(asciif_texture_path);
	for (int i = 0; i < 2; i++) {
		int y = 9;
		mCharMap[i].bindCharacter(U'终', { 0,y }, { 32,39 });
		mCharMap[i].bindCharacter(U'框', { 32,y }, { 32,39 });
		y += 40;
		mCharMap[i].bindCharacter(U'p', { 0,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'q', { 32,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'r', { 32 * 2,y }, { 17,39 });
		mCharMap[i].bindCharacter(U's', { 32 * 3,y }, { 21,39 });
		mCharMap[i].bindCharacter(U't', { 32 * 4,y }, { 19,39 });
		mCharMap[i].bindCharacter(U'u', { 32 * 5,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'v', { 32 * 6,y }, { 23,39 });
		mCharMap[i].bindCharacter(U'w', { 32 * 7,y }, { 26,39 });
		mCharMap[i].bindCharacter(U'x', { 32 * 8,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'y', { 32 * 9,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'z', { 32 * 10,y }, { 20,39 });
		mCharMap[i].bindCharacter(U'{', { 32 * 11,y }, { 16,39 });
		mCharMap[i].bindCharacter(U'|', { 32 * 12,y }, { 14,39 });
		mCharMap[i].bindCharacter(U'}', { 32 * 13,y }, { 16,39 });
		mCharMap[i].bindCharacter(U'~', { 32 * 14,y }, { 24,39 });
		mCharMap[i].bindCharacter(U'回', { 32 * 15,y }, { 30,39 });
		y += 40;
		mCharMap[i].bindCharacter(U'‘', { 0,y }, { 15,39 });
		mCharMap[i].bindCharacter(U'a', { 32,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'b', { 32 * 2,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'c', { 32 * 3,y }, { 20,39 });
		mCharMap[i].bindCharacter(U'd', { 32 * 4,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'e', { 32 * 5,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'f', { 32 * 6,y }, { 20,39 });
		mCharMap[i].bindCharacter(U'g', { 32 * 7,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'h', { 32 * 8,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'i', { 32 * 9,y }, { 13,39 });
		mCharMap[i].bindCharacter(U'j', { 32 * 10,y }, { 19,39 });
		mCharMap[i].bindCharacter(U'k', { 32 * 11,y }, { 23,39 });
		mCharMap[i].bindCharacter(U'l', { 32 * 12,y }, { 13,39 });
		mCharMap[i].bindCharacter(U'm', { 32 * 13,y }, { 31,39 });
		mCharMap[i].bindCharacter(U'n', { 32 * 14,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'o', { 32 * 15,y }, { 22,39 });
		y += 40;
		mCharMap[i].bindCharacter(U'P', { 0,y }, { 24,39 });
		mCharMap[i].bindCharacter(U'Q', { 32,y }, { 24,39 });
		mCharMap[i].bindCharacter(U'R', { 32 * 2,y }, { 25,39 });
		mCharMap[i].bindCharacter(U'S', { 32 * 3,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'T', { 32 * 4,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'U', { 32 * 5,y }, { 24,39 });
		mCharMap[i].bindCharacter(U'V', { 32 * 6,y }, { 24,39 });
		mCharMap[i].bindCharacter(U'W', { 32 * 7,y }, { 32,39 });
		mCharMap[i].bindCharacter(U'X', { 32 * 8,y }, { 26,39 });
		mCharMap[i].bindCharacter(U'Y', { 32 * 9,y }, { 24,39 });
		mCharMap[i].bindCharacter(U'Z', { 32 * 10,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'[', { 32 * 11,y }, { 16,39 });
		mCharMap[i].bindCharacter(U'\\', { 32 * 12,y }, { 17,39 });
		mCharMap[i].bindCharacter(U']', { 32 * 13,y }, { 14,39 });
		mCharMap[i].bindCharacter(U'^', { 32 * 14,y }, { 19,39 });
		mCharMap[i].bindCharacter(U'_', { 32 * 15,y }, { 21,39 });
		y += 40;
		mCharMap[i].bindCharacter(U'@', { 0,y }, { 30,39 });
		mCharMap[i].bindCharacter(U'A', { 32,y }, { 27,39 });
		mCharMap[i].bindCharacter(U'B', { 32 * 2,y }, { 24,39 });
		mCharMap[i].bindCharacter(U'C', { 32 * 3,y }, { 21,39 });
		mCharMap[i].bindCharacter(U'D', { 32 * 4,y }, { 24,39 });
		mCharMap[i].bindCharacter(U'E', { 32 * 5,y }, { 21,39 });
		mCharMap[i].bindCharacter(U'F', { 32 * 6,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'G', { 32 * 7,y }, { 24,39 });
		mCharMap[i].bindCharacter(U'H', { 32 * 8,y }, { 24,39 });
		mCharMap[i].bindCharacter(U'I', { 32 * 9,y }, { 15,39 });
		mCharMap[i].bindCharacter(U'J', { 32 * 10,y }, { 23,39 });
		mCharMap[i].bindCharacter(U'K', { 32 * 11,y }, { 25,39 });
		mCharMap[i].bindCharacter(U'L', { 32 * 12,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'M', { 32 * 13,y }, { 27,39 });
		mCharMap[i].bindCharacter(U'N', { 32 * 14,y }, { 23,39 });
		mCharMap[i].bindCharacter(U'O', { 32 * 15,y }, { 24,39 });
		y += 40;
		mCharMap[i].bindCharacter(U'0', { 0,y }, { 24,39 });
		mCharMap[i].bindCharacter(U'1', { 32,y }, { 15,39 });
		mCharMap[i].bindCharacter(U'2', { 32 * 2,y }, { 24,39 });
		mCharMap[i].bindCharacter(U'3', { 32 * 3,y }, { 24,39 });
		mCharMap[i].bindCharacter(U'4', { 32 * 4,y }, { 25,39 });
		mCharMap[i].bindCharacter(U'5', { 32 * 5,y }, { 23,39 });
		mCharMap[i].bindCharacter(U'6', { 32 * 6,y }, { 23,39 });
		mCharMap[i].bindCharacter(U'7', { 32 * 7,y }, { 23,39 });
		mCharMap[i].bindCharacter(U'8', { 32 * 8,y }, { 23,39 });
		mCharMap[i].bindCharacter(U'9', { 32 * 9,y }, { 23,39 });
		mCharMap[i].bindCharacter(U':', { 32 * 10,y }, { 17,39 });
		mCharMap[i].bindCharacter(U';', { 32 * 11,y }, { 17,39 });
		mCharMap[i].bindCharacter(U'<', { 32 * 12,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'=', { 32 * 13,y }, { 21,39 });
		mCharMap[i].bindCharacter(U'>', { 32 * 14,y }, { 20,39 });
		mCharMap[i].bindCharacter(U'?', { 32 * 15,y }, { 23,39 });
		y += 40;
		mCharMap[i].bindCharacter(U' ', { 0,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'!', { 32,y }, { 16,39 });
		mCharMap[i].bindCharacter(U'“', { 32 * 2,y }, { 18,39 });
		mCharMap[i].bindCharacter(U'#', { 32 * 3,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'$', { 32 * 4,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'%', { 32 * 5,y }, { 29,39 });
		mCharMap[i].bindCharacter(U'&', { 32 * 6,y }, { 25,39 });
		mCharMap[i].bindCharacter(U'\'', { 32 * 7,y }, { 13,39 });
		mCharMap[i].bindCharacter(U'(', { 32 * 8,y }, { 17,39 });
		mCharMap[i].bindCharacter(U')', { 32 * 9,y }, { 16,39 });
		mCharMap[i].bindCharacter(U'*', { 32 * 10,y }, { 23,39 });
		mCharMap[i].bindCharacter(U'+', { 32 * 11,y }, { 25,39 });
		mCharMap[i].bindCharacter(U',', { 32 * 12,y }, { 13,39 });
		mCharMap[i].bindCharacter(U'-', { 32 * 13,y }, { 22,39 });
		mCharMap[i].bindCharacter(U'.', { 32 * 14,y }, { 15,39 });
		mCharMap[i].bindCharacter(U'/', { 32 * 15,y }, { 26,39 });
	}
	
	for (int i = 0; i < 2; i++) {
		mHighScoreText[i] = new esl::Text(U"1,000,000");
		mHighScoreText[i]->bindMap(mCharMap[i]);
		mHighScoreText[i]->setCharacterSpace(20);
		mHighScoreText[i]->setPosition(mHighScore->getPosition() + glm::vec2{ 324,0 });
		mHighScoreText[i]->setScale(glm::vec2{ 1 + i * 0.1,1 + i * 0.1 });
		mHighScoreText[i]->setHorizontalAlign(esl::Text::HorizontalAlign::Right);

		mScoreText[i] = new esl::Text(U"0");
		mScoreText[i]->bindMap(mCharMap[i]);
		mScoreText[i]->setPosition(mScore->getPosition() + glm::vec2{ 324,0 });
		mScoreText[i]->setCharacterSpace(20);
		mScoreText[i]->setScale(glm::vec2{ 1 + i * 0.1,1 + i * 0.1 });
		mScoreText[i]->setHorizontalAlign(esl::Text::HorizontalAlign::Right);

		glm::vec2 pos = { mScore->getPosition().x,mPower->getPosition().y };
		mPowerText[i] = new esl::Text(U"1.00/4.00");
		mPowerText[i]->bindMap(mCharMap[i]);
		mPowerText[i]->setPosition(pos + glm::vec2{ 324,0 });
		mPowerText[i]->setCharacterSpace(20);
		mPowerText[i]->setScale(glm::vec2{ 1 + i * 0.1,1 + i * 0.1 });
		mPowerText[i]->setHorizontalAlign(esl::Text::HorizontalAlign::Right);
		pos.y = mMoney->getPosition().y;
		mMoneyText[i] = new esl::Text(U"0");
		mMoneyText[i]->bindMap(mCharMap[i]);
		mMoneyText[i]->setCharacterSpace(20);
		mMoneyText[i]->setPosition(pos + glm::vec2{ 324,0 });
		mMoneyText[i]->setScale(glm::vec2{ 1 + i * 0.1,1 + i * 0.1 });
		mMoneyText[i]->setHorizontalAlign(esl::Text::HorizontalAlign::Right);
	}
	mHighScoreText[0]->setColor(glm::uvec4{ 112,112,112,255 });
	mScoreText[0]->setColor(glm::uvec4{ 6,22,131,255 });
	mPowerText[0]->setColor(glm::uvec4{ 133,9,9,255 });
	mPowerText[1]->setColor(glm::uvec4{ 255,208,208,255 });
	mMoneyText[0]->setColor(glm::uvec4{ 129,129,2,255 });
	mMoneyText[1]->setColor(glm::uvec4{ 255,255,208,255 });
}
void Front::text_update()
{
	for (int i = 0; i < 2; i++) {
		std::string str = std::to_string(*mData.score);
		if (str.size() > 3) {
			for (int j = static_cast<int>(str.size()) - 3; j > 0; j -= 3) {
				str.insert(j, ",");
			}
		}
		mScoreText[i]->setText(str);
		str = std::to_string(*mData.highscore);
		if (str.size() > 3) {

			for (int j = static_cast<int>(str.size()) - 3; j > 0; j -= 3) {
				str.insert(j, ",");
			}
		}
		mHighScoreText[i]->setText(str);
		str = std::to_string(*mData.power);
		str.insert(str.size() - 2, ".");
		str += "/4.00";
		mPowerText[i]->setText(str);
		str = std::to_string(*mData.money);
		mMoneyText[i]->setText(str);
	}
}
void Front::render()
{
	renderer.draw(*mTopSprite);
	renderer.draw(*mBottomSprite);
	if (mBossXPosIndicatorEnabled) {
		renderer.draw(*mBossXPosIndicator);
	}
	renderer.draw(*mLeftSprite);
	renderer.draw(*mRightSprite);
	renderer.draw(*mHighScore);
	renderer.draw(*mScore);
	renderer.draw(*mRemainLife);
	renderer.draw(*mSpellCard);
	renderer.draw(*mPower);
	renderer.draw(*mMoney);
	for (int i = 0; i < 7; i++) {
		renderer.draw(*mLifes[i]);
		renderer.draw(*mSpellCards[i]);
	}
	for (int i = 1; i >= 0; i--) {
		renderer.draw(*mHighScoreText[i]);
		renderer.draw(*mScoreText[i]);
		renderer.draw(*mPowerText[i]);
		renderer.draw(*mMoneyText[i]);
	}
	if(mDifficultyIcons)
		renderer.draw(*mDifficultyIcons);
	if(mItemGetBorderLine)
		renderer.draw(*mItemGetBorderLine);
	if(mItemGetBorderText)
		renderer.draw(*mItemGetBorderText);
	
}

void Front::update(double delta)
{
	glm::vec2 pos = { 320 + 512,1024 - 36 };
	for (unsigned int i = 0; i < *mData.life; i++) {
		mLifes[i]->setTextureRect(pos + glm::vec2{ mRect.x * 3,0 }, mRect);
	}
	for (int i = *mData.life; i < 7; i++){
		mLifes[i]->setTextureRect(pos + glm::vec2{ 0,0 }, mRect);
	}
	for (unsigned int i = 0; i < *mData.spellcard; i++) {
		mSpellCards[i]->setTextureRect(pos + glm::vec2{ mRect.x * 3,-39 }, mRect);
	}
	for (int i = *mData.spellcard; i < 7; i++) {
		mSpellCards[i]->setTextureRect(pos + glm::vec2{ 0,-39 }, mRect);
	}
	text_update();
	// 收取线显示动画
	if (mItemGetAnimationStarted) {
		mItemGetAnimationTime += delta;
		mItemGetBorderLine->setAlpha(static_cast<float>(sin((mItemGetAnimationTime - 0.125) * 4 * glm::pi<double>())/2+0.5));
		mItemGetBorderText->setAlpha(static_cast<float>(sin((mItemGetAnimationTime - 0.125) * 4 * glm::pi<double>()) / 2 + 0.5));
		if (mItemGetAnimationTime > 4) {
			mItemGetAnimationStarted = false;
			mItemGetBorderLine->setAlpha(0);
			mItemGetBorderText->setAlpha(0);
			
		}
	}
	
	// 敌机横坐标位置指示
	if (mBossXPosIndicatorEnabled) {
		mBossXPosIndicatorAnimationTime += delta;
		
		if (mBossXPosIndicatorAnimationTime >= 0.05 && mBossXPosIndicatorAlphaReverseTime <10) {
			mBossXPosIndicator->setAlpha(static_cast<float>(mBossXPosIndicatorAlphaReverseTime %2 ? 0.0 : 1.f));
			mBossXPosIndicatorAlphaReverseTime++;
			mBossXPosIndicatorAnimationTime = 0.f;
		}
		if (mBossXPosIndicatorAlphaReverseTime >= 10) {
			mBossXPosIndicator->setAlpha(1.f);
		}
		float bossPosX = mSharedBoss->getPosition().x;
		if (bossPosX <= LEFT) bossPosX = LEFT;
		else if (bossPosX >= RIGHT) bossPosX = RIGHT;
		mBossXPosIndicator->setPosition({ bossPosX,16 });
	}
}
