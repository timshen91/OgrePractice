#include <OGRE/Ogre.h>
#include <OIS/OIS.h>

class Main : public Ogre::WindowEventListener, public Ogre::FrameListener, public OIS::KeyListener, public OIS::MouseListener {
	const char * mResourcesCfg;
	Ogre::Root * mRoot;
	Ogre::RenderWindow * mWindow;
	Ogre::SceneManager * mSceneMgr;
	Ogre::Camera * mCamera;
	OIS::InputManager* mInputManager;
	OIS::Mouse*	mMouse;
	OIS::Keyboard* mKeyboard;
	bool mShutDown;
	Ogre::SceneNode * mHeroNode;
	Ogre::SceneNode * mCenterNode;
	Ogre::SceneNode * mCameraNode;
	Ogre::Entity * mHero;
	std::string mAnimationBase;
	//std::vector<std::string> mAnimation;
	float mDistance;
	int rotateY;
	bool grab;
	bool guide;
	unsigned int directionMap;
	Ogre::Real speed;

	enum KeyDirection {
		LEFT,
		RIGHT,
		UP,
		DOWN,
	};

	void updateSpeed() {
		auto dir = Ogre::Vector3{float((directionMap >> RIGHT) & 1) - float((directionMap >> LEFT) & 1), 0, float((directionMap >> DOWN) & 1) - float((directionMap >> UP) & 1)};
		if (dir.x != 0 && dir.z != 0) {
			dir /= Ogre::Math::Sqrt(2);
		}
		if (dir.length() == 0) {
			changeBase("Stand");
			speed = 0;
		} else {
			mHeroNode->setOrientation(Ogre::Quaternion{mCenterNode->getOrientation().getYaw(), Ogre::Vector3::UNIT_Y});
			mHeroNode->lookAt(dir, Ogre::Node::TS_LOCAL);
			changeBase("Run");
			speed = 15;
		}
	}

	void setDir(KeyDirection d) {
		directionMap |= 1 << d;
		updateSpeed();
	}

	void unsetDir(KeyDirection d) {
		directionMap &= ~(1 << d);
		updateSpeed();
	}

	void changeBase(std::string state) {
		mHero->getAnimationState(mAnimationBase)->setEnabled(false);
		mHero->getAnimationState(state)->setEnabled(true);
		mAnimationBase = state;
	}
public:
	Main(const char * mResourcesCfg = "resources.cfg") : mResourcesCfg{mResourcesCfg}, mShutDown{false}, grab{false}, guide{false}, mDistance{20}, rotateY{0}, directionMap{0}, mAnimationBase{"Stand"}, speed{0} {
		mRoot = new Ogre::Root();

		Ogre::ConfigFile cf;
		cf.load(mResourcesCfg);
		auto seci = cf.getSectionIterator();
		Ogre::String secName, typeName, archName;
		while (seci.hasMoreElements()) {
			secName = seci.peekNextKey();
			Ogre::ConfigFile::SettingsMultiMap * settings = seci.getNext();
			for (auto & i : *settings) {
				typeName = i.first;
				archName = i.second;
				Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
			}
		}

		//mRoot->showConfigDialog();
		mRoot->restoreConfig();
		mWindow = mRoot->initialise(true, "Main");
		mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
		Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
		initScene();

		// listeners
		OIS::ParamList pl;
		size_t windowHnd = 0;
		std::ostringstream windowHndStr;
		mWindow->getCustomAttribute("WINDOW", &windowHnd);
		windowHndStr << windowHnd;
		pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
		//pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
		//pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
		//pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("true")));
		//pl.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
		mInputManager = OIS::InputManager::createInputSystem(pl);
		mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
		mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));
		mMouse->setEventCallback(this);
		mKeyboard->setEventCallback(this);

		windowResized(mWindow);
		Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

		changeBase("Stand");

		mRoot->addFrameListener(this);
	}

	void initScene() {
		mHeroNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		auto temp = mHeroNode->createChildSceneNode();
		//mHero = mSceneMgr->createEntity("hero", "arthaslichking.mesh");
		mHero = mSceneMgr->createEntity("hero", "illidan_past.mesh");
		mHero->attachObjectToBone("55", mSceneMgr->createEntity("rightHand", "glave_1h_dualblade_d_01.mesh"));
		mHero->attachObjectToBone("56", mSceneMgr->createEntity("leftHand", "glave_1h_dualblade_d_01left.mesh"));
		mHero->setCastShadows(true);
		temp->attachObject(mHero);
		temp->yaw(Ogre::Degree{90});
		mCenterNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		mCameraNode = mCenterNode->createChildSceneNode(Ogre::Vector3(0, 0, mDistance));
		mCamera = mSceneMgr->createCamera("cam");
		mCamera->lookAt(Ogre::Vector3(0, 0, -1));
		mCameraNode->attachObject(mCamera);
		mCamera->setNearClipDistance(0.1);
		auto vp = mWindow->addViewport(mCamera);
		vp->setBackgroundColour(Ogre::ColourValue(1, 1, 1));
		mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));

		//int m = 15;
		//auto skel = mHero->getSkeleton();
		//int numAnimations = skel->getNumAnimations();
		//auto iter = mHero->getSkeleton()->getBoneIterator().begin();
		//for (int i = 1; i < m; i++) {
		//	auto bone = *iter;
		//	bone->setManuallyControlled(true);
		//	for(int j=0;j<numAnimations;j++){
		//		Ogre::Animation * anim = skel->getAnimation(j);
		//		anim->destroyNodeTrack(bone->getHandle());
		//	}
		//	bone->yaw(Ogre::Degree{90});
		//	++iter;
		//}
		//for (int i = m; i < 57; i++) {
		//	auto bone = *iter;
		//	//bone->setManuallyControlled(true);
		//	//for(int j=0;j<numAnimations;j++){
		//	//	Ogre::Animation * anim = skel->getAnimation(j);
		//	//	anim->destroyNodeTrack(bone->getHandle());
		//	//}
		//	//bone->yaw(Ogre::Degree{90});
		//	++iter;
		//}
	}

	virtual bool frameRenderingQueued(const Ogre::FrameEvent & evt) {
		if (mWindow->isClosed()) {
			return false;
		}
		if (mShutDown) {
			return false;
		}
		updateWorld(evt.timeSinceLastFrame);
		mKeyboard->capture();
		mMouse->capture();
		return true;
	}

	virtual bool keyPressed(const OIS::KeyEvent & arg) {
		switch (arg.key) {
		case OIS::KC_W:
			setDir(UP);
			break;
		case OIS::KC_A:
			setDir(LEFT);
			break;
		case OIS::KC_S:
			setDir(DOWN);
			break;
		case OIS::KC_D:
			setDir(RIGHT);
			break;
		case OIS::KC_J:
			mDistance++;
			mCameraNode->setPosition(Ogre::Vector3{0, 0, mDistance});
			break;
		case OIS::KC_K:
			mDistance--;
			mCameraNode->setPosition(Ogre::Vector3{0, 0, mDistance});
			break;
		case OIS::KC_SPACE:
			// TODO
			break;
		case OIS::KC_ESCAPE:
			mShutDown = true;
			break;
		}
	}

	virtual bool keyReleased(const OIS::KeyEvent & arg) {
		switch (arg.key) {
		case OIS::KC_W:
			unsetDir(UP);
			break;
		case OIS::KC_A:
			unsetDir(LEFT);
			break;
		case OIS::KC_S:
			unsetDir(DOWN);
			break;
		case OIS::KC_D:
			unsetDir(RIGHT);
			break;
		}
	}

	virtual bool mouseMoved(const OIS::MouseEvent & arg) {
		if (grab) {
			auto newRotateY = arg.state.Y.rel + rotateY;
			if (newRotateY < 1) {
				newRotateY = 1;
			}
			if (newRotateY > 89) {
				newRotateY = 89;
			}
			auto r = newRotateY - rotateY;
			rotateY = newRotateY;
			mCenterNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Degree{float(-arg.state.X.rel)}, Ogre::Node::TS_WORLD);
			mCenterNode->rotate(Ogre::Vector3::UNIT_X, Ogre::Degree{float(-r)});
			if (guide) {
				updateSpeed();
			}
		}
	}

	virtual bool mousePressed(const OIS::MouseEvent & arg, OIS::MouseButtonID id) {
		if (id == OIS::MB_Left) {
			grab = true;
		}
		if (id == OIS::MB_Right) {
			grab = true;
			guide = true;
		}
	}

	virtual bool mouseReleased(const OIS::MouseEvent & arg, OIS::MouseButtonID id) {
		if (id == OIS::MB_Left) {
			grab = false;
		}
		if (id == OIS::MB_Right) {
			grab = false;
			guide = false;
		}
	}

	void updateWorld(Ogre::Real deltaTime) {
		mHeroNode->translate(Ogre::Vector3::NEGATIVE_UNIT_Z * speed * deltaTime, Ogre::Node::TS_LOCAL);
		mCenterNode->setPosition(mHeroNode->getPosition());
		mHero->getAnimationState(mAnimationBase)->addTime(deltaTime);
	}

	virtual void windowResized(Ogre::RenderWindow * rw) {
		unsigned int width, height, depth;
		int left, top;
		rw->getMetrics(width, height, depth, left, top);
		const OIS::MouseState &ms = mMouse->getMouseState();
		ms.width = width;
		ms.height = height;
	}

	virtual void windowClosed(Ogre::RenderWindow * rw) {
		mInputManager->destroyInputObject(mMouse);
		mInputManager->destroyInputObject(mKeyboard);
		OIS::InputManager::destroyInputSystem(mInputManager);
	}

	void createScene() {
		// shadow
		mSceneMgr->setAmbientLight(Ogre::ColourValue{0.5, 0.5, 0.5});
		mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
		mSceneMgr->setShadowColour(Ogre::ColourValue{0.5, 0.5, 0.5});
		mSceneMgr->setShadowTextureSize(1024);
		mSceneMgr->setShadowTextureCount(1);
		mSceneMgr->setShadowFarDistance(200);

		// light
		Ogre::Light* light = mSceneMgr->createLight();
		light->setType(Ogre::Light::LT_DIRECTIONAL);
		light->setDirection(Ogre::Vector3{-1, -1, -1});
		light->setDiffuseColour(Ogre::ColourValue::White);
		light->setSpecularColour(Ogre::ColourValue::White);

		// plane
		Ogre::Plane plane{Ogre::Vector3::UNIT_Y, 0};
		Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane, 1500, 1500, 20, 20, true, 1, 100, 100, Ogre::Vector3::UNIT_Z);
		Ogre::Entity* entGround = mSceneMgr->createEntity("GroundEntity", "ground");
		mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entGround);
		entGround->setMaterialName("Examples/Rockwall");
		entGround->setCastShadows(false);

		// head
		Ogre::Entity * ogreHead2 = mSceneMgr->createEntity("head2", "ogrehead.mesh");
		Ogre::SceneNode * headNode2 = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3{0, 20, -100});
		headNode2->attachObject(ogreHead2);

		// animation
		mHero->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);
		auto iter = mHero->getAllAnimationStates()->getAnimationStateIterator();
		for (auto i = iter.begin(); i != iter.end(); ++i) {
			fprintf(stderr, "TIM: %s\n", i->first.c_str());
		}
	}

	void run() {
		mRoot->startRendering();
	}
};

int main() {
	Main a{};
	a.createScene();
	a.run();
	return 0;
}
