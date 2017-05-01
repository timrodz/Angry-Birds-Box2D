#pragma once

#include "Testbed\Framework\Test.h"




class AngryBirds : public Test
{
public:

	AngryBirds();

	void Keyboard(int key);
	void MouseUp(const b2Vec2& p);
	void MouseMove(const b2Vec2& p);
	void Step(Settings* settings);

	void CreatePayload();
	void CreateLevel();
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
	UserData PayloadData;
	//The joint that keeps the payload attached to the slingshot and shows direction and tention force of the pull.
	b2Joint* m_joint;


	b2Body* m_EnemyPigs[5];
	b2Body* m_Destructibles[5];
	UserData EnemyData;
	UserData DestructibleData;


	char EnemyCount[5] = { '2' };
	char Birdsleft[5] = { '3' };
};