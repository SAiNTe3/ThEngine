#include <iostream>
#include <Window.hpp>
#include <Sprite.hpp>
#include <CharacterMap.hpp>
#include <Text.hpp>


class Boss;
class Front {
	const float LEFT = 64.0f;
	const float RIGHT = 64 + 768.0f;
	const float TOP = 896 + 32;
	const float BOTTOM = 32.0f;
	using pSprite = std::unique_ptr<esl::Sprite>;
	using pTexture = std::unique_ptr<esl::Texture>;
	struct Data {
		unsigned int* score;
		unsigned int* highscore;
		unsigned int* life;
		unsigned int* spellcard;
		unsigned int* power;
		unsigned int* money;
	}mData;
	esl::Window& renderer;
	pTexture mTexture;

	pSprite mLeftSprite;
	pSprite mRightSprite;
	pSprite mTopSprite;
	pSprite mBottomSprite;

	pSprite mHighScore, mScore, mRemainLife, mSpellCard, mPower, mMoney;
	pSprite mLifes[7];
	glm::vec2 mRect;
	pSprite mSpellCards[7];

	// 难度图标
	pSprite mDifficultyIcons = nullptr;
	// 收取线
	pSprite mItemGetBorderLine = nullptr;
	pSprite mItemGetBorderText = nullptr;
	bool mItemGetAnimationStarted = false;
	double mItemGetAnimationTime = 0.0;
	// 敌机横坐标位置指示
	pSprite mBossXPosIndicator = nullptr;
	double mBossXPosIndicatorAnimationTime = 0.0;
	bool mBossXPosIndicatorEnabled = false;
	Boss* mSharedBoss = nullptr;
	int mBossXPosIndicatorAlphaReverseTime = 0;

	esl::CharacterMap mCharMap[2];
	esl::CharacterMap mCharMapF;
	esl::Text* mHighScoreText[2], * mScoreText[2], * mPowerText[2], * mMoneyText[2];
	void char_map_init();
	void text_update();
public:
	/// <param name="difficulty">4:Easy,3:Normal,2:Hard,1:Lunatic,0:Extra</param>
	void setDifficultyMode(int difficulty, glm::vec2 pos);
	void setItemGetBorderLine(glm::vec2 pos);
	void showItemGetAnimation();
	void showBossXPosIndicator(Boss* boss);
	void hideBossXPosIndicator();
	Front(esl::Window& window);
	void bindData(unsigned int& score, unsigned int& highscore, unsigned int& life, unsigned int& spellcard, unsigned int& power, unsigned int& money);
	~Front();
	void render();
	void update(double delta);
};