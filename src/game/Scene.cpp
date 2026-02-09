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

TitleScene::TitleScene(esl::Window& renderer, ScriptSystem& system) :Scene(system), mRenderer(renderer)
{

	glm::ivec2 renderPos = renderer.getWindowSize();
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
	mRenderer.clear();
	mRenderer.draw(*mTitleBackgroundSprite.get());
	for (auto& element : mMenuSelections) {
		mRenderer.draw(*element.get());
	}
	mRenderer.display();
}

void TitleScene::update(double deltaTime)
{
	// 每一帧重置颜色防止内存修改造成index与实际绘制不同
	for (int i = 0; i < 4; i++) {
		mMenuSelections[i]->setColor(glm::vec4{ 0.6,0.6,0.6,1 });
	}
	mMenuSelections[mSelectIndex]->setColor(glm::vec4{ 1,1,1,1 });
}

MainGame::MainGame(esl::Window& render, ScriptSystem& system) :Scene(system), mRenderer(render)
{
	mFront = new Front(mRenderer);
	// 步骤1：初始化静态资源（不依赖实例的）
	Background3D::init(&mRenderer, mCenterPos);
	Enemy::init(&mRenderer);
	Enemy::setSystem(&mScriptSystem);
	Bullet_1::init();  // 在预分配之前
	Bullet::initEtBreak();
	Player::setSystem(&mScriptSystem);

	// 步骤2：预分配对象池
	BulletPoolHelper::preallocateBullets(2500, render);
	mAllEnemyBullets.reserve(2500);

	// 步骤3：创建 Player 实例
	mPlayer = std::make_unique<Reimu>(mRenderer, mData.mPlayerPower);
	mPlayer->set_position(Position());
	mPlayer->setEnemyList(&mEnemys);

	// 步骤4：初始化依赖 Player 的系统（现在 mPlayer 已经存在）
	Item::init(&mRenderer, mPlayer.get(), mData);

	// 步骤5：其他初始化
	mScriptSystem.setCenterPos(mCenterPos);
	
	mBackground = new Stage01_Background();
	mScriptSystem.preloadDialogueScript("./Assets/scripts/level1.txt");
	mFront->bindData(mData.mPlayerScore, mData.mHighScore, mData.mPlayerLife,
		mData.mPlayerSpellCard, mData.mPlayerPower, mData.mMoney);
	mFront->setDifficultyMode(4, Position({608,768}));
	mFront->setItemGetBorderLine(Position({ 0,543 }));
	Item::SetCollectLine(Position().y + 543);

	mBlurEffect.resize(glm::vec2(768, 896));
	//mBlurEffect.setColor(glm::vec4{0,0,0,0});
	
	
	// 设置碰撞管理器的回调函数
	mCollisionManager.setEnemyBulletHitPlayerCallback([this]() {
		if (mPlayer->isInvincible()) return;
		if (mData.mPlayerLife > 0) {
			mData.mPlayerLife--;
			Item::generate_at_player_death(mPlayer->get_position(), Position({0,450}));
			mPlayer->hitPlayer(Position({0,-128}));
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
	handleGlobalInput(e);

	if (mPause) {
		// 处理Pause时的输入
		handlePauseInput(e);
	}
	else {
		// 处理非Pause时的输入
		handleGameplayInput(e);
	}
}

void MainGame::render()
{
	
	mRenderer.clear();
	if (mSwitchEffectEnabled) {
		mSwitchScreenAnimation.draw(mRenderer);
		
	}
	else {
		mBackground->render();
		for (auto& enemy : mEnemys) {
			enemy->render();
		}
		Bullet::drawEtBreaks(mRenderer);
		mPlayer->render();
		Item::RenderAll();

		mDeathCircle.draw(mRenderer);

		mScriptSystem.render();

		mPlayer->slowEffectRender();

		mFront->renderRemaining();
		if (mPause) {
			if (!mBlurredScreenReady) {
				mBlurEffect.setIterations(2);
				mBlurEffect.setSpread(5.f);
				mBlurEffect.captureScreen(mRenderer, { 64, 32 }, { 768, 896 });
				mBlurEffect.setAlpha(0.0f);
				mBlurEffect.process();
				mBlurredScreenReady = true;
			}

			mRenderer.draw(mBlurEffect);
			mPauseMenu.draw(mRenderer);
		}
		mFront->render();
	}
	
	mRenderer.display();

}

void MainGame::setupStage() {
	mScriptSystem.nextAudio();
	// 等待 3 秒
	mStage.addWait(3.0);
	// Wave 1: 生成10Enemy,移动，间隔1秒发射环形弹幕
	mStage.addAction([this](MainGame* game) {
		for (int i = 0; i < 10; i++) {
			EnemyUnit* enemy = new EnemyUnit(EnemyUnit::NormalType::TYPE3, { LEFT - 48,600 }, 50);
			enemy->mClearBulletAfterDeath = false;
			enemy->setBonus(0, 1, 0, 0, 0, 0, 2, 32);
			enemy->addAwait(i * 0.2f);
			enemy->addAction(
				LinearMovement()
				.to({ RIGHT + 48,600 })
				.speed(200)
				.easeOut()
				.build(),
				Enemy::ActionType::MOVEMENT
			);
			enemy->addAction(
				enemy->shoot()
				.bullet(14, 1)
				.pattern(DanmakuPattern::FAN)
				.count(4)
				.colors({0,2,4,6})
				.rounds(10)
				.angleStep(10)
				.toTarget([this]() {
					return mPlayer->get_position();
					})
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
	// Wave 2: 生成10Enemy,移动，间隔1秒发射环形弹幕
	mStage.addAction([this](MainGame* game) {
		for (int i = 0; i < 10; i++) {
			EnemyUnit* enemy = new EnemyUnit(EnemyUnit::NormalType::TYPE3, { RIGHT + 48,600 }, 50);
			enemy->mClearBulletAfterDeath = false;
			enemy->setBonus(0, 1, 0, 0, 0, 0, 2, 32);
			enemy->addAwait(i * 0.2f);
			enemy->addAction(
				LinearMovement()
				.to({ LEFT - 48,600 })
				.speed(200)
				.easeOut()
				.build(),
				Enemy::ActionType::MOVEMENT
			);
			enemy->addAction(
				enemy->shoot()
				.bullet(12, 1)
				.pattern(DanmakuPattern::FAN)
				.count(4)
				.rounds(10)
				.colors({ 0,2,4,6 })
				.angleStep(10)
				.toTarget([this]() {
					return mPlayer->get_position();
					})
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
	// Wave 3
	mStage.addAction([this](MainGame* game) {
		EnemyUnit* enemy = new EnemyUnit(EnemyUnit::NormalType::TYPE1, Position({0,800}),200);
		enemy->setBonus(0, 4, 0, 0, 0, 0, 8, 48);
		enemy->addAction(
			LinearMovement()
			.to(Position({ 0,500 }))
			.speed(100)
			.easeOut()
			.build(),
			Enemy::ActionType::MOVEMENT
		);
		enemy->addAction(
			std::make_unique<Await>(2),
			Enemy::ActionType::DANMAKU
		);
		enemy->addAction(
			enemy->shoot()
			.bullet(13, 3)
			.pattern(DanmakuPattern::CIRCLE)
			.count(12)
			.rounds(15)
			.rotatePerRound(1)
			.colors({ 1,2,3,4,5,6 })
			.syncRotation()
			.interval(0.2)
			.build(),
			Enemy::ActionType::DANMAKU
		);
		enemy->addAction(
			LinearMovement()
			.to(Position({ 0,800 }))
			.speed(100)
			.easeIn()
			.build(),
			Enemy::ActionType::MOVEMENT
		);
		game->mEnemys.push_back(enemy);
		}
	);

	
	mStage.addWait(5.0);
	// 开场对话
	mStage.addAction(
		[this](MainGame* game) {
			mScriptSystem.activateDialogueSection("opening");
		}
	);
	// 等待对话结束
	mStage.addWaitUntil([this]() {
		return !mScriptSystem.mDialogueActived;  
		});
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
		boss->addAction(
			LinearMovement()
			.to(Position({ 200,700 }))
			.speed(100)
			.easeOut()
			.build(),
			Enemy::ActionType::DEATH
		);
		
		boss->addAction(
			boss->shoot()
			.bullet(21, 5)
			.pattern(DanmakuPattern::CIRCLE)
			.count(20)
			.rounds(20)
			.rotatePerRound(-1.5)
			.colors({ 1,3,5,7 })
			.syncRotation()
			.interval(0.1)
			.build(),
			Enemy::ActionType::DANMAKU
		);
		boss->addAction(
			boss->shoot()
			.bullet(19, 5)
			.pattern(DanmakuPattern::CIRCLE)
			.count(12)
			.rounds(5)
			.direction(0)
			.rotatePerRound(1.0)
			.colors({ 1,2,3,4,5,7 })
			.syncRotation()
			.interval(0.4)
			.build(),
			Enemy::ActionType::DANMAKU
		);
			
		boss->addAction(
			std::make_unique<Await>(3.0),
			Enemy::ActionType::MOVEMENT
		);
		boss->addAction(
			LinearMovement()
			.to(RandomPos(64, Position({ 200,700 })))
			.speed(150)
			.easeInOut()
			.build(),
			Enemy::ActionType::MOVEMENT
		);
		

		game->mEnemys.push_back(boss);
		boss->addAwait(DBL_MAX);
		
		mFront->showBossXPosIndicator(boss);
		}
	);
	mStage.addAction(
		[this](MainGame* game) {
			mScriptSystem.nextAudio();
		}
	);
	mStage.addWaitUntil([this]() {
		return !mScriptSystem.mDialogueActived;  // 等待对话结束
	});
	
	
	mStage.addWaitUntil([this,boss]() {
		return boss->mFinished;
	});
	
	mStage.addAction(
		[this](MainGame* game) {
			mScriptSystem.activateDialogueSection("midway");
			
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

	mStage.start(this);
}
void MainGame::update(double deltaTime)
{
	mDeltaTime += deltaTime;
	if (mPause) {
		if (mSwitchEffectEnabled) {
			mSwitchScreenAnimation.update(deltaTime);
			if (mSwitchScreenAnimation.isFinished()) {
				switchToScene(-1);
			}
			return;
		}
		if (mPauseMenu.state == PauseMenu::PauseMenuState::FINISHED) {
			resume();
			mScriptSystem.resumeAudio();
			mPause = false;
		}

		if (mPauseMenu.state == PauseMenu::PauseMenuState::FADE_OUT) {
			glm::vec4 color = mBlurEffect.getColor();
			color.a -= 4.0f * static_cast<float>(deltaTime);
			color.a = glm::clamp(color.a, 0.0f, 1.0f);
			color.r = color.g = 1.5 - color.a;
			color.b = 1.7 - color.a;
			mBlurEffect.setColor(color);
		}
		else if (mPauseMenu.state == PauseMenu::PauseMenuState::FADE_IN) {
			glm::vec4 color = mBlurEffect.getColor();
			color.a += 4.0f * static_cast<float>(deltaTime);
			color.a = glm::clamp(color.a, 0.0f, 1.0f);
			color.r = color.g = 1.5 - color.a;
			color.b = 1.7 - color.a;
			mBlurEffect.setColor(color);
		}
		mPauseMenu.update(deltaTime);
		return;
	}
	
	mBackground->update(deltaTime);
	
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

		if (auto* reimu = dynamic_cast<Reimu*>(mPlayer.get())) {
			mCollisionManager.checkPlayerBulletsVsEnemy(reimu->mTraceBullets, *enemy);
		}

		mCollisionManager.checkPlayerVsEnemy(*mPlayer, *enemy);
	}

	// DeathCircle 更新
	mDeathCircle.update(deltaTime);
	// 批量清理死亡敌人
	mEnemys.erase(
		std::remove_if(mEnemys.begin(), mEnemys.end(), [this](Enemy* enemy) {
			if (!enemy->mSpriteAvailable && !enemy->mBulletsAvailable) {
				// 如果是 Boss,先通知 Front
				if (auto* boss = dynamic_cast<Boss*>(enemy)) {
					mFront->hideBossXPosIndicator();
					// 在此处调用deathcircle
					mDeathCircle.start(boss->getPosition());
					mScriptSystem.playSoundEffect("se_enep01.wav");
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

void MainGame::handleGlobalInput(esl::Event& e)
{
	static bool escLatch = false;
	// 按下 ESC 键且未锁定时切换暂停状态
	if (e.isKeyPressed(Keyboard::KEY_ESCAPE) && !escLatch) {
		// 上锁
		escLatch = true;

		if (!mPause) {
			enterPause();
		}
		else {
			// 请求退出暂停
			requestExitPause();
		}
	}
	// 若释放则重置锁
	else if (e.isKeyReleased(Keyboard::KEY_ESCAPE)) {
		escLatch = false;
	}
}
void MainGame::enterPause()
{
	if (mPauseMenu.getState() == PauseMenu::PauseMenuState::FINISHED) {
		mPause = true;
		pause();
		mScriptSystem.pauseAudio();
		mScriptSystem.playSoundEffect("se_pause.wav");
		mBlurredScreenReady = false;
		
		mPauseMenu.start(Position());
	}
}
void MainGame::returnTitleScreen()
{
	mSwitchEffectEnabled = true;
	mSwitchScreenAnimation.start();
}
void MainGame::requestExitPause()
{
	if (mPauseMenu.getState() == PauseMenu::PauseMenuState::WAITING) {
		mPauseMenu.state = PauseMenu::PauseMenuState::FADE_OUT; // 或直接触发 FADE_OUT
		mPauseMenu.restartTimer();
	}
}
void MainGame::handlePauseInput(esl::Event& e)
{
	// 菜单选项移动锁
	static bool navLatch = false;

	if (e.isKeyPressed(Keyboard::KEY_UP) && !navLatch) {
		mPauseMenu.previousOption();
		mScriptSystem.playSoundEffect("se_select00.wav");
		navLatch = true;
	}
	else if (e.isKeyPressed(Keyboard::KEY_DOWN) && !navLatch) {
		mPauseMenu.nextOption();
		mScriptSystem.playSoundEffect("se_select00.wav");
		navLatch = true;
	}
	else if(e.isKeyPressed(Keyboard::KEY_Z) && !navLatch) {
		switch (mPauseMenu.confirmSelection()) {
			case 0: // 继续游戏
				requestExitPause();
				break;
			case 1:
				returnTitleScreen(); // 返回标题
				break;
		default: break;
		}
		mScriptSystem.playSoundEffect("se_ok00.wav");
		navLatch = true;
	}
	else if (e.isKeyReleased(Keyboard::KEY_UP) &&
		e.isKeyReleased(Keyboard::KEY_DOWN) && e.isKeyReleased(Keyboard::KEY_Z)) {
		navLatch = false;
	}
}
void MainGame::handleGameplayInput(esl::Event& e)
{
	static bool inputSuppressed = true;

	// 解除输入抑制：Z 完全释放
	if (inputSuppressed && e.isKeyReleased(Keyboard::KEY_Z)) {
		inputSuppressed = false;
	}

	if (inputSuppressed) {
		stopPlayerInput();
		return;
	}

	handlePlayerMovement(e);
	handlePlayerShooting(e);
	mScriptSystem.processInput(e);
}
void MainGame::stopPlayerInput()
{
	mPlayer->mDirection.h = 0;
	mPlayer->mDirection.v = 0;
	mPlayer->mEnableShoot = false;
}
void MainGame::handlePlayerMovement(esl::Event& e)
{
	glm::vec2 pos = mPlayer->get_position();

	float v = 0.f;
	float h = 0.f;

	if (e.isKeyPressed(Keyboard::KEY_UP) && pos.y < 992)  v = 1.f;
	if (e.isKeyPressed(Keyboard::KEY_DOWN) && pos.y > 32) v = -1.f;
	if (e.isKeyPressed(Keyboard::KEY_LEFT) && pos.x > 64) h = -1.f;
	if (e.isKeyPressed(Keyboard::KEY_RIGHT) && pos.x < 832) h = 1.f;

	bool slow = e.isKeyPressed(Keyboard::KEY_LEFT_SHIFT);
	if (slow) {
		v *= 0.5f;
		h *= 0.5f;
		mPlayer->mHyperMode = false;
	}
	else {
		mPlayer->mHyperMode = true;
	}

	mPlayer->mDirection.v = v;
	mPlayer->mDirection.h = h;
}
void MainGame::handlePlayerShooting(esl::Event& e)
{
	mPlayer->mEnableShoot =
		e.isKeyPressed(Keyboard::KEY_Z) &&
		!mScriptSystem.mDialogueActived;
}
