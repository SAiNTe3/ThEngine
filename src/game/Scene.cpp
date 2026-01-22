#include <Scene.h>
#include <BulletPool.h>  // 添加 BulletPool 头文件
#include <Item.h>

void Scene::process_input(esl::Event& e)
{
}

void Scene::render()
{
}

void Scene::update(double deltaTime)
{
}

TitleScene::TitleScene(esl::Window& render, ScriptSystem& system) :Scene(system), mRender(render)
{

	glm::ivec2 renderPos = render.getWindowSize();
	mTitleBackTexture = std::make_unique<esl::Texture>(".\\Assets\\title\\title_bk01.png");
	mTitleBackgroundSprite = std::make_unique<esl::Sprite>(mTitleBackTexture.get());
	mTitleBackgroundSprite->setPosition(renderPos / 2);
	std::string selections[4] = { "Game_Start.png","MusicRoom.png","Option.png","Quit.png" };
	// 选项初始化
	for (int i = 0; i < 4; i++) {
		std::string path = ".\\Assets\\title\\selecttitle\\" + selections[i];
		
		mMenuSelectionTextures.push_back(std::make_unique<esl::Texture>(path));
		mMenuSelections.push_back(std::make_unique<esl::Sprite>(mMenuSelectionTextures[i].get()));
		mMenuSelections[i]->setScale({ 0.25,0.25 });
		mMenuSelections[i]->setColor(glm::vec4{ 0.6,0.6,0.6,1 });
		mMenuSelections[i]->setPosition({ renderPos.x/2,renderPos.y/2-i*100});
	}
	mScriptSystem.setAudio(0);
	mScriptSystem.playAudio();
}

void TitleScene::process_input(esl::Event& e)
{
	// 是否允许处理键盘事件
	if (mSelectRespond) {
		// Event 1: 先处理上下键和 mSelectIndex
		// 该布尔值用于记录上一次帧是否已经按下了上下键，防止长按时多次触发
		static bool key_pressed_last_frame = false;
		if (e.isKeyPressed(Keyboard::KEY_UP) && !key_pressed_last_frame) {
			mSelectIndex+=3;
			key_pressed_last_frame = true;
			mScriptSystem.playSoundEffect("se_select00.wav");
		}
		else if (e.isKeyPressed(Keyboard::KEY_DOWN) && !key_pressed_last_frame) {
			mSelectIndex++;
			key_pressed_last_frame = true;
			mScriptSystem.playSoundEffect("se_select00.wav");
		}
		mSelectIndex = mSelectIndex % 4;
		if (e.isKeyReleased(Keyboard::KEY_UP) && e.isKeyReleased(Keyboard::KEY_DOWN)) {
			key_pressed_last_frame = false;
		}
		// Event 2: Z键选择
		if (e.isKeyPressed(Keyboard::KEY_Z)) {
			// 选中后禁止处理键盘事件
			mSelectRespond = false;
			// 设置场景切换信息
			mSceneInfo.mSwitchToNextScene = true;
			mSceneInfo.mSwitchToSceneIndex = mSelectIndex;
		}
	}
	
}

void TitleScene::render()
{
	mRender.clear();
	mRender.draw(*mTitleBackgroundSprite.get());
	for (auto& element : mMenuSelections) {
		mRender.draw(*element.get());
	}
	mRender.display();
}

void TitleScene::update(double deltaTime)
{
	// 每一帧重置颜色防止内存修改造成index与实际绘制不同
	for (int i = 0; i < 4; i++) {
		mMenuSelections[i]->setColor(glm::vec4{ 0.6,0.6,0.6,1 });
	}
	mMenuSelections[mSelectIndex]->setColor(glm::vec4{ 1,1,1,1 });
}

MainGame::MainGame(esl::Window& render, ScriptSystem& system) :Scene(system), mRender(render)
{
	mFront = new Front(mRender);
	// 步骤1：初始化静态资源（不依赖实例的）
	Background3D::init(&mRender, mCenterPos);
	Enemy::init(&mRender);
	Enemy::setSystem(&mScriptSystem);
	Bullet_1::init();  // ← 在预分配之前
	Bullet::initEtBreak();
	Player::setSystem(&mScriptSystem);

	// 步骤2：预分配对象池
	BulletPoolHelper::preallocateBullets(2500, render);
	mAllEnemyBullets.reserve(2500);

	// 步骤3：创建 Player 实例
	mPlayer = new Reimu(mRender, mData.mPlayerPower);
	mPlayer->set_position(Position());
	mPlayer->setEnemyList(&mEnemys);

	// 步骤4：初始化依赖 Player 的系统（现在 mPlayer 已经存在）
	Item::init(&mRender, mPlayer, mData);

	// 步骤5：其他初始化
	mScriptSystem.setCenterPos(mCenterPos);
	
	mBackground = new Stage01_Background();
	mScriptSystem.preloadDialogueScript("./Assets/scripts/level1.txt");
	mFront->bindData(mData.mPlayerScore, mData.mHighScore, mData.mPlayerLife,
		mData.mPlayerSpellCard, mData.mPlayerPower, mData.mMoney);
	mFront->setDifficultyMode(4, Position({608,768}));
	mFront->setItemGetBorderLine(Position({ 0,543 }));
	Item::SetCollectLine(Position().y + 543);
	// 设置碰撞管理器的回调函数
	mCollisionManager.setEnemyBulletHitPlayerCallback([this]() {
		if (mData.mPlayerLife > 0) {
			mData.mPlayerLife--;
			mPlayer->hitPlayer(Position({0,-128}));
			mPlayer->mPower -= 100;
			if (mPlayer->mPower < 100) mPlayer->mPower = 100;
			// 清除当前屏幕所有敌人子弹
			for (auto& enemy : mEnemys) {
				enemy->clearBullets();
			}
		}
	});
	// 擦弹回调
	mCollisionManager.setPlayerGrazeEnemyBulletCallback([this]() {
		mData.mPlayerScore += 1;
		mScriptSystem.playSoundEffect("se_graze.wav");
	});
	// 命中回调
	mCollisionManager.setPlayerBulletHitEnemyCallback([this]() {
		mData.mPlayerScore += 10;
	});
	mCollisionManager.setPlayerHitEnemyCallback([this]() {
		
	});
	setupStage();
}

void MainGame::process_input(esl::Event& e)
{
	static bool key_pressed_last_frame = false; 
	static bool input_suppression = true;  // 添加输入抑制标志
	
	// 检查是否需要解除输入抑制：当Z键完全释放时
	if (input_suppression && e.isKeyReleased(Keyboard::KEY_Z)) {
		input_suppression = false;
	}

	if (e.isKeyPressed(Keyboard::KEY_ESCAPE) && !key_pressed_last_frame) {
		key_pressed_last_frame = true;
		mPause = !mPause;
		if (mPause) {
			pause();
			mScriptSystem.pauseAudio();
			mScriptSystem.playSoundEffect("se_pause.wav");
		}
		else {
			resume();
			mScriptSystem.resumeAudio();
		}
	}
	else if(e.isKeyReleased(Keyboard::KEY_ESCAPE)){
		key_pressed_last_frame = false;
	}
	if (!mPause && !input_suppression) {  // 添加输入抑制检查
		bool shiftPressed = e.isKeyPressed(Keyboard::KEY_LEFT_SHIFT);

		// 方向处理
		glm::vec2 pos = mPlayer->get_position();
	
		mPlayer->mDirection.v = e.isKeyPressed(Keyboard::KEY_UP)&&(pos.y<960+32) ? 1 :
			(e.isKeyPressed(Keyboard::KEY_DOWN)&&(pos.y>32) ? -1 : 0);
		mPlayer->mDirection.h = e.isKeyPressed(Keyboard::KEY_LEFT)&&(pos.x>64) ? -1 :
			(e.isKeyPressed(Keyboard::KEY_RIGHT)&&(pos.x<64+768) ? 1 : 0);
		// Shift键效果
		if (shiftPressed) {
			mPlayer->mDirection.h /= 2;
			mPlayer->mDirection.v /= 2;
			mPlayer->mHyperMode = false;
		}
		else {
			mPlayer->mHyperMode = true;
		}

		// 射击
		mPlayer->mEnableShoot = e.isKeyPressed(Keyboard::KEY_Z) && !mScriptSystem.mDialogueActived;
		mScriptSystem.processInput(e);
	}
	else if (!mPause && input_suppression) {
		// 在输入抑制期间，确保玩家不会移动或射击
		mPlayer->mDirection.v = 0;
		mPlayer->mDirection.h = 0;
		mPlayer->mEnableShoot = false;
	}
}
void MainGame::render()
{
	if (mPause) return;	

	mRender.clear();
	mBackground->render();
	for (auto& enemy : mEnemys) {
		enemy->render();
	}
	Bullet::drawEtBreaks(mRender);
	mPlayer->render();
	Item::RenderAll();

	mScriptSystem.render();
	mFront->render();
	//mPlayer->slowEffectRender();
	
	mRender.display();

}

void MainGame::setupStage() {
	mScriptSystem.nextAudio();
	// 等待 3 秒
	mStage.addWait(3.0);
	mStage.addAction([this](MainGame* game) {
		for (int i = 0; i < 10; i++) {
			EnemyUnit* enemy = new EnemyUnit(EnemyUnit::NormalType::TYPE3, { LEFT - 48 - i * 48,600 }, 50);
			enemy->mClearBulletAfterDeath = false;
			enemy->setBonus(0, 1, 0, 0, 0, 0, 2, 32);
			enemy->addAction(
				LinearMovement()
				.to({ RIGHT + 48,600 })
				.speed(200)
				.easeOut()
				.build(),
				Enemy::ActionType::MOVEMENT
			);
			enemy->addAction(
				std::make_unique<Await>(1),
				Enemy::ActionType::DANMAKU
			);
			enemy->addAction(
				enemy->shoot()
				.bullet(12, 1)
				.pattern(DanmakuPattern::CIRCLE)
				.count(5)
				.rotatePerRound(20)
				.rounds(10)
				.syncRotation()
				.interval(0.2)
				.build(),
				Enemy::ActionType::DANMAKU
			);
			game->mEnemys.push_back(enemy);
		}
		mFront->showItemGetAnimation();
	}
	);
	mStage.addWait(5.0);
	Boss* boss = new Boss(1, 10000, Position({ 700,900 }));
	boss->setBonus(0, 16, 0, 0, 0, 0, 64, 64);
	// 生成 Boss
	mStage.addAction([this,boss](MainGame* game) {
		boss->addAction(
			LinearMovement()
			.to(Position({ 0,600 }))
			.speed(500)
			.easeOut()
			.build(),
			Enemy::ActionType::SPAWN
		);
		game->mEnemys.push_back(boss);
		boss->addAwait(DBL_MAX);
		mScriptSystem.activateDialogueSection("opening");
		mScriptSystem.stageClear();
		mFront->showBossXPosIndicator(boss);
		}
	);
	mStage.addWaitUntil([this]() {
		return !mScriptSystem.mDialogueActived;  // 等待对话结束
	});
	
	mStage.addAction(
		[this](MainGame* game) {
			mScriptSystem.nextAudio();
		}
	);
	mStage.addWaitUntil([this,boss]() {
		return boss->mFinished;  // 等待对话结束
	});
	mStage.addAction(
		[this](MainGame* game) {
			mScriptSystem.activateDialogueSection("midway");
			mScriptSystem.nextAudio();
		}
	);
	mStage.start(this);
	
}
void MainGame::update(double deltaTime)
{
	if (mPause) return;

	mDeltaTime += deltaTime;
	mBackground->update(deltaTime);
	//printf("%d\n", mAllEnemyBullets.size());
	
	mAllEnemyBullets.clear();
	
	
	// 更新敌人并收集子弹
	for (auto& enemy : mEnemys) {
		enemy->update(deltaTime);

		// 批量收集所有子弹指针
		for (auto& bullet : enemy->mBullets) {
			if (bullet && bullet->getSprite()) {
				mAllEnemyBullets.push_back(bullet.get());
			}
		}
	}

	// 更新玩家
	glm::vec2 movement = {
		mPlayer->mDirection.h * deltaTime * mPlayer->mSpeed,
		mPlayer->mDirection.v * deltaTime * mPlayer->mSpeed
	};
	mPlayer->move(movement);
	mPlayer->update(deltaTime);

	mStage.update(deltaTime, this);

	// 优化：批量碰撞检测（一次性检测所有子弹）
	if (mCollisionManager.checkEnemyBulletsVsPlayer(mAllEnemyBullets, *mPlayer)) {
		mScriptSystem.playSoundEffect("se_pldead00.wav");
	}

	// 玩家子弹 vs 敌人
	for (auto& enemy : mEnemys) {
		mCollisionManager.checkPlayerBulletsVsEnemy(mPlayer->mBullets, *enemy);

		if (auto* reimu = dynamic_cast<Reimu*>(mPlayer)) {
			mCollisionManager.checkPlayerBulletsVsEnemy(reimu->mTraceBullets, *enemy);
		}

		mCollisionManager.checkPlayerVsEnemy(*mPlayer, *enemy);
	}

	// 批量清理死亡敌人
	mEnemys.erase(
		std::remove_if(mEnemys.begin(), mEnemys.end(), [this](Enemy* enemy) {
			if (!enemy->mSpriteAvailable && !enemy->mBulletsAvailable) {
				// 如果是 Boss,先通知 Front
				if (auto* boss = dynamic_cast<Boss*>(enemy)) {
					mFront->hideBossXPosIndicator();
				}
				delete enemy;
				return true;
			}
			return false;
			}),
		mEnemys.end()
	);

	mScriptSystem.update(deltaTime);
	Item::UpdateAll(deltaTime,mPlayer->get_position().y);
	Bullet::updateEtBreaks(deltaTime);
	mFront->update(deltaTime);
}
// 数据维护，防止数据溢出和作弊，但不调用
void MainGame::data_maintain()
{
	if (mData.mPlayerLife > 7 || mData.mPlayerLife < 0) {
		mData.mPlayerLife = 3;
	}
	if(mData.mPlayerSpellCard>7||mData.mPlayerSpellCard<0){
		mData.mPlayerSpellCard = 3;
	}
}

void MainGame::pause()
{
	mSceneClock.pause();
}

void MainGame::resume()
{
	mSceneClock.resume();
}