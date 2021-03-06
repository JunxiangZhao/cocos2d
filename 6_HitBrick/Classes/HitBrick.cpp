#pragma execution_character_set("utf-8")
#include "HitBrick.h"
#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#define database UserDefault::getInstance()

USING_NS_CC;
using namespace CocosDenshion;

void HitBrick::setPhysicsWorld(PhysicsWorld* world) { m_world = world; }

Scene* HitBrick::createScene() {
  srand((unsigned)time(NULL));
  auto scene = Scene::createWithPhysics();

  scene->getPhysicsWorld()->setAutoStep(true);
  

  // Debug 模式
  // scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
  scene->getPhysicsWorld()->setGravity(Vec2(0, -300.0f));
  auto layer = HitBrick::create();
  layer->setPhysicsWorld(scene->getPhysicsWorld());
  layer->setJoint();
  scene->addChild(layer);
  return scene;
}

// on "init" you need to initialize your instance
bool HitBrick::init() {
  //////////////////////////////
  // 1. super init first
  if (!Layer::init()) {
    return false;
  }
  visibleSize = Director::getInstance()->getVisibleSize();

  auto edgeSp = Sprite::create();  //创建一个精灵
  auto boundBody = PhysicsBody::createEdgeBox(visibleSize, PhysicsMaterial(0.0f, 1.0f, 0.0f), 3);  //edgebox是不受刚体碰撞影响的一种刚体，我们用它来设置物理世界的边界
  
  edgeSp->setPosition(visibleSize.width / 2, visibleSize.height / 2);  //位置设置在屏幕中央
  edgeSp->setPhysicsBody(boundBody);
  addChild(edgeSp);


  preloadMusic(); // 预加载音效

  addSprite();    // 添加背景和各种精灵
  addListener();  // 添加监听器 
  addPlayer();    // 添加球与板
  BrickGeneraetd();  // 生成砖块


  schedule(schedule_selector(HitBrick::update), 0.01f, kRepeatForever, 0.1f);

  onBall = true;
  spHolded = false;
  spFactor = 0;
  return true;
}

// 关节连接，固定球与板子
// Todo
void HitBrick::setJoint() {

	joint1 = PhysicsJointPin::construct(player->getPhysicsBody(), ball->getPhysicsBody(), Vec2(0,30), Vec2(0,0));
	m_world->addJoint(joint1);
}



// 预加载音效
void HitBrick::preloadMusic() {
  auto sae = SimpleAudioEngine::getInstance();
  sae->preloadEffect("gameover.mp3");
  sae->preloadBackgroundMusic("bgm.mp3");
  sae->playBackgroundMusic("bgm.mp3", true);
}

// 添加背景和各种精灵
void HitBrick::addSprite() {
  // add background
  auto bgSprite = Sprite::create("bg.png");
  bgSprite->setPosition(visibleSize / 2);
  bgSprite->setScale(visibleSize.width / bgSprite->getContentSize().width, visibleSize.height / bgSprite->getContentSize().height);
  this->addChild(bgSprite, 0);


  // add ship
  ship = Sprite::create("ship.png");
  ship->setScale(visibleSize.width / ship->getContentSize().width * 0.97, 1.2f);
  ship->setPosition(visibleSize.width / 2, 0);
  ship->setTag(2);
  auto shipbody = PhysicsBody::createBox(ship->getContentSize(), PhysicsMaterial(100.0f, 0.0f, 1.0f));
  shipbody->setCategoryBitmask(0xFFFFFFFF);
  shipbody->setCollisionBitmask(0xFFFFFFFF);
  shipbody->setContactTestBitmask(0xFFFFFFFF);
  
  shipbody->setDynamic(false);  
  ship->setPhysicsBody(shipbody);
  this->addChild(ship, 1);

  // add sun and cloud
  auto sunSprite = Sprite::create("sun.png");
  sunSprite->setPosition(rand() % (int)(visibleSize.width - 200) + 100, 550);
  this->addChild(sunSprite);
  auto cloudSprite1 = Sprite::create("cloud.png");
  cloudSprite1->setPosition(rand() % (int)(visibleSize.width - 200) + 100, rand() % 100 + 450);
  this->addChild(cloudSprite1);
  auto cloudSprite2 = Sprite::create("cloud.png");
  cloudSprite2->setPosition(rand() % (int)(visibleSize.width - 200) + 100, rand() % 100 + 450);
  this->addChild(cloudSprite2);
}

// 添加监听器
void HitBrick::addListener() {
  auto keyboardListener = EventListenerKeyboard::create();
  keyboardListener->onKeyPressed = CC_CALLBACK_2(HitBrick::onKeyPressed, this);
  keyboardListener->onKeyReleased = CC_CALLBACK_2(HitBrick::onKeyReleased, this);
  _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

  auto contactListener = EventListenerPhysicsContact::create();
  contactListener->onContactBegin = CC_CALLBACK_1(HitBrick::onConcactBegin, this);
  _eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);
}

// 创建角色
void HitBrick::addPlayer() {
	//添加板
	player = Sprite::create("bar.png");
	int xpos = visibleSize.width / 2;
	player->setScale(0.1f, 0.1f);
	player->setPosition(Vec2(xpos, ship->getContentSize().height - player->getContentSize().height*0.1f));
	// 设置板的刚体属性
	auto barBody = PhysicsBody::createBox(player->getContentSize(), PhysicsMaterial(100.0f, 1.0f, 0.1f)); //完全弹性
	barBody->setCategoryBitmask(0xFFFFFFFF);
	barBody->setCollisionBitmask(0xFFFFFFFF);
	barBody->setContactTestBitmask(0xFFFFFFFF);
	barBody->setDynamic(false);  
	player->setPhysicsBody(barBody);
	this->addChild(player, 2);
  
	//添加球
	ball = Sprite::create("ball.png");
	ball->setPosition(Vec2(xpos, player->getPosition().y + ball->getContentSize().height*0.1f));
	ball->setScale(0.1f, 0.1f);
	ball->setTag(1);
	//设置粒子效果
	ParticleMeteor* firework = ParticleMeteor::create();
	firework->setStartSize(200);
	firework->setDuration(-1);
	ball->addChild(firework);

	//设置球的刚体属性
	auto ballBody = PhysicsBody::createBox(ball->getContentSize(), PhysicsMaterial(1.0f, 1.0f, 0.1f));
	ballBody->setCategoryBitmask(0xFFFFFFFF);
	ballBody->setCollisionBitmask(0xFFFFFFFF);
	ballBody->setContactTestBitmask(0xFFFFFFFF);
	ballBody->setRotationEnable(false);
	ballBody->setDynamic(true);
	ballBody->setGravityEnable(false);
	ball->setPhysicsBody(ballBody);
	this->addChild(ball, 3);
  
}

// 实现简单的蓄力效果
// Todo
void HitBrick::update(float dt) {
	
	//左右移动
	if (isMove) {
		moveBar(moveDir);
	}
	else {
		player->getPhysicsBody()->setVelocity(Vec2(0, 0));
	}
		

	//蓄力
	if (spHolded) {
		spFactor++;
		return;
	}
	//发射
	if (onBall && !spHolded && spFactor != 0) {
		m_world->removeJoint(joint1);
		ball->getPhysicsBody()->setVelocity(Vec2(100, spFactor + 200));
		onBall = false;
	}
	
}

// Todo
void HitBrick::BrickGeneraetd() {

	for (int i = 0; i < 3; i++) {
		int cw = 0;
		while (cw <= visibleSize.width) {
			//在顶部生成砖块
			auto box = Sprite::create("box.png");
			box->setPosition(Vec2(box->getContentSize().width/2 + cw, visibleSize.height - box->getContentSize().height * (0.5+i)));
			box->setTag(10);
			this->addChild(box);
			cw += box->getContentSize().width;
		
			// 为砖块设置刚体属性
			auto brickBody = PhysicsBody::createBox(box->getContentSize(), PhysicsMaterial(1000.0f, 1.0f, 0.0f)); //完全弹性
			brickBody->setCategoryBitmask(0x00000001);
			brickBody->setCollisionBitmask(0x00000010);
			brickBody->setContactTestBitmask(0xFFFFFFFF);
			brickBody->setGroup(-1);
			brickBody->setDynamic(false);
			box->setPhysicsBody(brickBody);
		}

	 }
}

void HitBrick::moveBar(char c) {
	if (c == 'a') {
		if (player->getBoundingBox().getMinX() > 40) {
			player->getPhysicsBody()->setVelocity(Vec2(-300, 0));

		}
	}
	else {
		if (player->getBoundingBox().getMaxX() < visibleSize.width - 40) {
			player->getPhysicsBody()->setVelocity(Vec2(300, 0));

		}
	}
	
}

// 左右
void HitBrick::onKeyPressed(EventKeyboard::KeyCode code, Event* event) {

  switch (code) {
  case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
	  isMove = true;
	  moveDir = 'a';
	  break;
  case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
	  isMove = true;
	  moveDir = 'd';
      break;
  case cocos2d::EventKeyboard::KeyCode::KEY_SPACE: // 开始蓄力
	  spHolded = true;
	  break;
  default:
    break;           
  }
}

// 释放按键
void HitBrick::onKeyReleased(EventKeyboard::KeyCode code, Event* event) {
  switch (code) {
  case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
  case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
	  isMove = false;
	  break;
	 
    break;
  case cocos2d::EventKeyboard::KeyCode::KEY_SPACE:   // 蓄力结束，小球发射
	  spHolded = false;
	  break;

  default:
	  break;
  }
}

// 碰撞检测
// Todo
bool HitBrick::onConcactBegin(PhysicsContact & contact) {
	auto nodeA = contact.getShapeA()->getBody()->getNode();
	auto nodeB = contact.getShapeB()->getBody()->getNode();

	if (nodeA && nodeB) {
		if (nodeA->getTag() == 10) {
			nodeA->removeFromParentAndCleanup(true);
			//打掉砖块就加速
			ball->getPhysicsBody()->setVelocity(1.05 * ball->getPhysicsBody()->getVelocity());
		}
		else if (nodeB->getTag() == 10) {
			nodeB->removeFromParentAndCleanup(true);
			//打掉砖块就加速
			ball->getPhysicsBody()->setVelocity(1.05 * ball->getPhysicsBody()->getVelocity());
		}
		else if (nodeA->getTag() == 1 && nodeB->getTag() == 2 || nodeA->getTag() == 2 && nodeB->getTag() == 1) {
			GameOver();
		}
	}
	
	return true;
}


void HitBrick::GameOver() {
	schedule(schedule_selector(HitBrick::update));
	_eventDispatcher->removeAllEventListeners();
	ball->getPhysicsBody()->setVelocity(Vec2(0, 0));
	player->getPhysicsBody()->setVelocity(Vec2(0, 0));
  SimpleAudioEngine::getInstance()->stopBackgroundMusic("bgm.mp3");
  SimpleAudioEngine::getInstance()->playEffect("gameover.mp3", false);

  auto label1 = Label::createWithTTF("Game Over~", "fonts/STXINWEI.TTF", 60);
  label1->setColor(Color3B(0, 0, 0));
  label1->setPosition(visibleSize.width / 2, visibleSize.height / 2);
  this->addChild(label1);

  auto label2 = Label::createWithTTF("重玩", "fonts/STXINWEI.TTF", 40);
  label2->setColor(Color3B(0, 0, 0));
  auto replayBtn = MenuItemLabel::create(label2, CC_CALLBACK_1(HitBrick::replayCallback, this));
  Menu* replay = Menu::create(replayBtn, NULL);
  replay->setPosition(visibleSize.width / 2 - 80, visibleSize.height / 2 - 100);
  this->addChild(replay);

  auto label3 = Label::createWithTTF("退出", "fonts/STXINWEI.TTF", 40);
  label3->setColor(Color3B(0, 0, 0));
  auto exitBtn = MenuItemLabel::create(label3, CC_CALLBACK_1(HitBrick::exitCallback, this));
  Menu* exit = Menu::create(exitBtn, NULL);
  exit->setPosition(visibleSize.width / 2 + 90, visibleSize.height / 2 - 100);
  this->addChild(exit);
}

// 继续或重玩按钮响应函数
void HitBrick::replayCallback(Ref * pSender) {
  Director::getInstance()->replaceScene(HitBrick::createScene());
}

// 退出
void HitBrick::exitCallback(Ref * pSender) {
  Director::getInstance()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}
