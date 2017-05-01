#pragma once

#include "Testbed\Framework\Test.h"


enum BirdType
{
	NORMAL, 
	SPLIT, 
	HEAVY
};

class AngryBirds : public Test
{
public:

	AngryBirds();

	void Keyboard(int key);
	void MouseUp(const b2Vec2& p);
	void MouseMove(const b2Vec2& p);
	void Step(Settings* settings);

	void Begin();

	void CreatePayload();
	void CreateLevel1();
	void CreateLevel2();
	void CreateEnemies();

	static Test* Create()
	{
		return new AngryBirds();
	}




private:

	
	//An un-collidable green box to show the slingshot stand
	b2Body* m_Slingshot;
	//A rigidbody collidable object for drawing back and firing the payload
	b2Body* m_Payload;
	bool Destroyed = false;
	bool Fired = false;
	float DestroyTimer = 0.0f;
	UserData PayloadData;
	//The joint that keeps the payload attached to the slingshot and shows direction and tention force of the pull.
	b2Joint* m_joint;
	//Slider Crank Joints
	b2RevoluteJoint* m_revJoint;
	b2PrismaticJoint* m_prismJoint;


	b2Body* m_EnemyPigs[5];
	b2Body* m_Destructibles[5];
	UserData EnemyData;
	UserData DestructibleData;
	UserData IndestructibleData;
	BirdType m_BirdType;


	char EnemyCount[5] = { '2' };
	char Birdsleft[5] = { '3' };

private:

	static int Level;
};