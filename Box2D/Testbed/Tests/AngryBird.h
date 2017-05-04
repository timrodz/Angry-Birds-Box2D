//
// Bachelor of Software Engineering
// Media Design School
// Auckland
// New Zealand
//
// (c) 2005 - 2017 Media Design School
//
// File Name : AngryBird.h
// Description : Game header file 
// Authors: Cameron Peet & Mitchell Currie
// Mail : Cameron.Peet@mediadesignschool.com
//		  Mitchell.Currie@mediadesignschool.com
//

#pragma once

#include "Testbed\Framework\Test.h"


enum BirdType
{
	NORMAL, 
	SEEK, 
	HEAVY
};

class AngryBirds : public Test
{
public:

	AngryBirds();

	void Keyboard(int key); // for keyboard input
	void MouseUp(const b2Vec2& p);
	void MouseMove(const b2Vec2& p);
	void Step(Settings* settings); // update function

	void Begin();

	void CreatePayload(); // creating each bird
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
	bool SeekPig = false;
	bool CanSeek = true;

	// Timers
	float DestroyTimer = 0.0f;
	float LevelEndBirdTimer = 0.0f;
	float LevelEndEnemyTimer = 0.0f;

	UserData PayloadData;

	//The joint that keeps the payload attached to the slingshot and shows direction and tention force of the pull.
	b2Joint* m_joint;
	//Slider Crank Joints
	b2RevoluteJoint* m_revJoint;
	b2PrismaticJoint* m_prismJoint;

	// For seek calculation
	b2Vec2 m_SteeringForce;
	b2Vec2 m_DesiredVelocity;


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