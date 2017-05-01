#include "Testbed\Tests\AngryBird.h"
#include <string>

AngryBirds::AngryBirds()
{
	m_world->SetGravity(b2Vec2(0.0f, -30.0f));

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

	CreateLevel();
	CreatePayload();
	CreateEnemies();
}




void AngryBirds::Step(Settings* settings)
{
	Test::Step(settings);

	// We are going to destroy some bodies according to contact
	// points. We must buffer the bodies that should be destroyed
	// because they may belong to multiple contact points.
	const int32 k_maxNuke = 10;
	b2Body* nuke[k_maxNuke];
	int32 nukeCount = 0;

	// Traverse the contact results. Destroy bodies that
	// are touching heavier bodies.
	for (int32 i = 0; i < m_pointCount; ++i)
	{
		ContactPoint* point = m_points + i;

		b2Body* body1 = point->fixtureA->GetBody();
		b2Body* body2 = point->fixtureB->GetBody();

		float32 mass1 = point->fixtureA->GetDensity();
		float32 mass2 = point->fixtureB->GetDensity();

		float32 velocity1 = body1->GetLinearVelocity().Length();
		float32 velocity2 = body2->GetLinearVelocity().Length();

		float32 velocity = velocity1 + velocity2;

		if (velocity > 10.0f)
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
			bool projectile = false;
			UserData* data = static_cast<UserData*>(b->GetUserData());
			if (data != NULL)
			{
				if (data->isEnemy)
				{
					EnemyCount[0]--;
				}
				if (data->isPayload)
				{
					projectile = true;
				}
				if (data->isDestroyable)
				{
					int massss = b->GetMass();
					int payloadmass;
					if(m_Payload != NULL)
						payloadmass = m_Payload->GetMass();
					data->isEnemy = true;
				}
			}

			if (!projectile)
				m_world->DestroyBody(b);
		}
	}

	if (m_Payload != NULL)
	{
		if (!m_Payload->IsAwake() && Fired)
		{
			m_world->DestroyBody(m_Payload);
			m_Payload = NULL;
			Birdsleft[0]--;
		}
		else
		{
			b2Vec2 pos = m_Payload->GetPosition();
			if (pos.y < 0 || pos.x < -40 || pos.x > 40)
			{
				m_world->DestroyBody(m_Payload);
				m_Payload = NULL;
				Birdsleft[0]--;
			}
		}
	}

	if (m_Payload == NULL)
	{
		if (Birdsleft[0] > '0')
		{
			CreatePayload();
		}
	}
	
	std::string text = "Pigs left = ";
	std::string ammo = "Birds left = ";
	
	g_debugDraw.DrawString(5, m_textLine, text.append(EnemyCount).c_str());
	m_textLine += 16;
	g_debugDraw.DrawString(5, m_textLine, ammo.append(Birdsleft).c_str());
}

void AngryBirds::Keyboard(int key)
{
	switch (key)
	{
	case GLFW_KEY_B:
		if (m_Payload == NULL)
			CreatePayload();
		break;
	case GLFW_KEY_1:
		m_BirdType = NORMAL;
		break;
	case GLFW_KEY_2:
		m_BirdType = SPLIT;
		break;
	case GLFW_KEY_3:
		m_BirdType = HEAVY;
		break;
	}
}


void AngryBirds::MouseUp(const b2Vec2& p)
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
		UserData* data = new UserData();
		data->isPayload = true;
		data->isDraggable = false;
		Payload->SetUserData(data);

		b2PolygonShape shape;
		shape.SetAsBox(0.75f, 0.75f);

		b2FixtureDef fd;
		fd.shape = &shape;
		fd.friction = 0.6f;
		float density = 5.0f;
		switch (m_BirdType)
		{
			case HEAVY:
				density = 7.0f;

				break;
			case SPLIT:
				density = 3.5f;

				break;

			case NORMAL:
			default:
				density = 5.0f;
				break;
		}
		fd.density = density;
		Payload->CreateFixture(&fd);

		b2Vec2 force = m_joint->GetReactionForce(150.0f);
		Payload->ApplyForce(force, force, true);

		m_world->DestroyJoint(m_joint);
		m_world->DestroyBody(m_Payload);
		m_Payload = NULL;
		m_joint = NULL;

		m_Payload = Payload;

		Fired = true;
	}

	m_mouseFirstClick = b2Vec2(p);
}


void AngryBirds::MouseMove(const b2Vec2& p)
{
	m_mouseWorld = p;

	if (m_mouseJoint)
	{
		m_mouseJoint->SetTarget(p);
	}
}

void AngryBirds::CreatePayload()
{
	//Define a body type and position, assign our user data, so the mouse ray-cast only picks up on the PayLoad
	b2BodyDef bd;
	bd.type = b2_dynamicBody;
	bd.position.Set(-20.0f, 8.0f);
	m_Payload = m_world->CreateBody(&bd);

	PayloadData.isPayload = true;
	PayloadData.isDraggable = true;

	m_Payload->SetUserData(&PayloadData);

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
	Fired = false;
}

void AngryBirds::CreateLevel()
{
	Birdsleft[0] = { '3' };
	{
		DestructibleData.isDestroyable = true;
		//Create a body type and position
		b2BodyDef bd;
		bd.type = b2_dynamicBody;
		bd.position.Set(0, 4.0f);
		
		//Define a shape
		b2PolygonShape shape;
		shape.SetAsBox(4.0f, 0.5f, b2Vec2(4.0f, 0.0f), 0.5f * b2_pi);
		
		//Give it matter
		b2FixtureDef fd;
		fd.shape = &shape;
		fd.friction = 0.6f;
		fd.density = 7.0f;
		
		//Use the above templated to create multiple bodies in the world.
		b2Body* body = m_world->CreateBody(&bd);
		body->CreateFixture(&fd);
		body->SetUserData(&DestructibleData);

		bd.position.Set(5.0f, 4.0f);
		body = m_world->CreateBody(&bd);
		body->CreateFixture(&fd);
		body->SetUserData(&DestructibleData);

		bd.position.Set(0.0f, 12.0f);
		body = m_world->CreateBody(&bd);
		body->CreateFixture(&fd);
		body->SetUserData(&DestructibleData);

		shape.SetAsBox(0.5f, 4.0f, b2Vec2(4.0f, 0.0f), 0.5f * b2_pi);
		bd.position.Set(0.0, 8.0f);
		body = m_world->CreateBody(&bd);
		body->CreateFixture(&fd);
		body->SetUserData(&DestructibleData);

		bd.position.Set(0.0f, 20.0f);
		body = m_world->CreateBody(&bd);
		body->CreateFixture(&fd);
		body->SetUserData(&DestructibleData);
	}
}

void AngryBirds::CreateEnemies()
{
	b2BodyDef bd;
	b2FixtureDef fd;
	b2CircleShape shape;
	EnemyData.isEnemy = true;

	bd.type = b2_dynamicBody;
	shape.m_radius = 1;
	fd.shape = &shape;
	fd.friction = 0.6f;
	fd.density = 1.0f;

	bd.position.Set(0, 4.0f);
	m_EnemyPigs[0] = m_world->CreateBody(&bd);
	m_EnemyPigs[0]->CreateFixture(&fd);
	m_EnemyPigs[0]->SetUserData(&EnemyData);

	bd.position.Set(6.0f, 12.0f);
	m_EnemyPigs[1] = m_world->CreateBody(&bd);
	m_EnemyPigs[1]->CreateFixture(&fd);
	m_EnemyPigs[1]->SetUserData(&EnemyData);
}
