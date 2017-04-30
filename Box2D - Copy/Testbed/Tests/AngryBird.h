#pragma once

#include "Testbed\Framework\Test.h"




class AngryBirds : public Test
{
public:

/*
* Constructor for the summative
*/
	AngryBirds()
	{
		m_world->SetGravity(b2Vec2(0.0f, -10.0f));

		//Create some ground so stuff doesn't fall of the screen.
		b2Body* ground = NULL;
		{
			b2BodyDef bd;
			ground = m_world->CreateBody(&bd);

			b2EdgeShape shape;
			shape.Set(b2Vec2(-40.0f, 0.0f), b2Vec2(40.0f, 0.0f));
			ground->CreateFixture(&shape, 0.0f);
		}

		//Define the slingshot - as uncollidable
		{
			b2BodyDef bd;
			bd.type = b2_staticBody;
			bd.position.Set(-25.0f, 4.0f);
			m_Slingshot = m_world->CreateBody(&bd);

			b2PolygonShape shape;
			shape.SetAsBox(4.0f, 0.5f, b2Vec2(4.0f, 0.0f), 0.5f * b2_pi);

			b2FixtureDef fd;
			
			//Uncollidable flag 
			fd.filter.maskBits = 0x0000;

			fd.shape = &shape;
			fd.friction = 0.6f;
			fd.density = 2.0f;
			m_Slingshot->CreateFixture(&fd);
		}

		CreatePayload();
	};


/*
* Create a new angry-bird (Payload) 
*/
	void CreatePayload()
	{
		// Create a payload
		{

			//Define a body type and position, assign our user data, so the mouse ray-cast only picks up on the PayLoad
			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			bd.position.Set(-20.0f, 8.0f);
			m_Payload = m_world->CreateBody(&bd);
			UserData data;
			data.isPayload = true;
			m_Payload->SetUserData(&data);

			//Define a preset shape
			b2PolygonShape shape;
			shape.SetAsBox(0.75f, 0.75f);

			//Define the fixture
			b2FixtureDef fd;
			fd.shape = &shape;
			fd.friction = 0.6f;
			fd.density = 5.0f;
		
			//Create the joint between the slingshot and the payload
			b2DistanceJointDef jd;
			b2Vec2 p1, p2, d;

			//Max Distace, and tug resistance damping
			jd.frequencyHz = 2.0f;
			jd.dampingRatio = 0.0f;

			jd.bodyA = m_Slingshot;
			jd.bodyB = m_Payload;
			jd.localAnchorA.Set(5.0f, 4.0f); //Relative to bodyA's position
			jd.localAnchorB.Set(0.0f, 0.0f); //Relative to bodyB's position
			p1 = jd.bodyA->GetWorldPoint(jd.localAnchorA); //World Position - returns a point unrelative to its body attachment. (Returns world coord)
			p2 = jd.bodyB->GetWorldPoint(jd.localAnchorB);
			//Get the distance between the joint
			d = p2 - p1;
			jd.length = d.Length();

			m_joint = m_world->CreateJoint(&jd);
			m_Payload->CreateFixture(&fd);
		}
	}

/*
* Handle Keyboard Input
*/
	void Keyboard(int key)
	{
		switch (key)
		{
		case GLFW_KEY_B:
			if (m_Payload == NULL)
				CreatePayload();
			break;


		}
	}

/*
* Handle the mouse release - Firing the payload by delete the payload we have, applying the tension created from the blue line (The Joint added to the payload and slingshot)
    to a newly created payload in the same position as the release position, but without the joint.
*/
	void MouseUp(const b2Vec2& p)
	{
		if (m_mouseJoint && m_joint)
		{
			m_world->DestroyJoint(m_mouseJoint);
			m_mouseJoint = NULL;


			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			b2Vec2 pos = m_Payload->GetPosition();
			bd.position.Set(pos.x, pos.y);
			b2Body* Payload = m_world->CreateBody(&bd);

			b2PolygonShape shape;
			shape.SetAsBox(0.75f, 0.75f);

			b2FixtureDef fd;
			fd.shape = &shape;
			fd.friction = 0.6f;
			fd.density = 5.0f;

			Payload->CreateFixture(&fd);

			b2Vec2 force = m_joint->GetReactionForce(150.0f);
			Payload->ApplyForce(force, force, true);

			m_world->DestroyJoint(m_joint);
			m_world->DestroyBody(m_Payload);
			m_Payload = NULL;
			m_joint = NULL;

		}

		m_mouseFirstClick = b2Vec2(p);
	}

/*
* Update the payloads position with the mouse joint and mouse pos
*/

	void MouseMove(const b2Vec2& p)
	{
		m_mouseWorld = p;

		if (m_mouseJoint)
		{
			m_mouseJoint->SetTarget(p);
		}
	}


	static Test* Create()
	{
		return new AngryBirds();
	}

/*
*  Update function, destroy certian bodies based on mass and velocity
     The lower mass object gets destroyed
	 Only destroyed if the combined velocity of the two bodies are greater than a number defined below

*/
	void Step(Settings* settings)
	{
		Test::Step(settings);

		// We are going to destroy some bodies according to contact
		// points. We must buffer the bodies that should be destroyed
		// because they may belong to multiple contact points.
		const int32 k_maxNuke = 6;
		b2Body* nuke[k_maxNuke];
		int32 nukeCount = 0;

		// Traverse the contact results. Destroy bodies that
		// are touching heavier bodies.
		for (int32 i = 0; i < m_pointCount; ++i)
		{
			ContactPoint* point = m_points + i;

			b2Body* body1 = point->fixtureA->GetBody();
			b2Body* body2 = point->fixtureB->GetBody();

			float32 mass1 = body1->GetMass();
			float32 mass2 = body2->GetMass();

			float32 velocity1 = body1->GetLinearVelocity().Length();
			float32 velocity2 = body2->GetLinearVelocity().Length();
			
			float32 velocity = velocity1 + velocity2;

			if(velocity > 6.0f)
				if (mass1 > 0.0f && mass2 > 0.0f)
				{
					if (mass2 > mass1)
					{
						nuke[nukeCount++] = body1;
					}
					else
					{
						nuke[nukeCount++] = body2;
					}

					if (nukeCount == k_maxNuke)
					{
						break;
					}
				}
		}

		// Sort the nuke array to group duplicates.
		std::sort(nuke, nuke + nukeCount);

		// Destroy the bodies, skipping duplicates.
		int32 i = 0;
		while (i < nukeCount)
		{
			b2Body* b = nuke[i++];
			while (i < nukeCount && nuke[i] == b)
			{
				++i;
			}

			if (b != m_bomb)
			{
				m_world->DestroyBody(b);
			}
		}
	}


private:

	//An un-collidable green box to show the slingshot stand
	b2Body* m_Slingshot;
	//A rigidbody collidable object for drawing back and firing the payload
	b2Body* m_Payload;
	//The joint that keeps the payload attached to the slingshot and shows direction and tention force of the pull.
	b2Joint* m_joint;
};