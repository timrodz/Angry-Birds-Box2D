#include "Testbed\Tests\AngryBird.h"
#include <string>

int AngryBirds::Level = 0;


AngryBirds::AngryBirds()
{
	Begin();
}


void AngryBirds::Begin()
{
	for (b2Body* b = m_world->GetBodyList(); b; /*b = b->GetNext()*/)
	{
		b2Body* next = b->GetNext();  // remember next body before *b gets destroyed
		m_world->DestroyBody(b); // do I need to destroy fixture as well(and how?) or it does that for me?
		b = next;  // go to next body
	}
	EnemyCount[0] = { '2' };
	Birdsleft[0] = { '3' };

	m_world->SetGravity(b2Vec2(0.0f, -30.0f));
	DestructibleData.isDestroyable = true;
	DestructibleData.Color = b2Color(0.25f, 0.9f, 0.9f);
	IndestructibleData.isDestroyable = false;
	IndestructibleData.Color = b2Color(1.0f, 0.0f, 1.0f);
	EnemyData.isEnemy = true;
	EnemyData.Color = b2Color(1.0f, 0.0f, 0.0f);
	PayloadData.isPayload = true;
	PayloadData.Color = b2Color(1, 1, 0);

	//Create some ground so stuff doesn't fall of the screen.
	m_groundBody = NULL;
	{
		b2BodyDef bd;
		m_groundBody = m_world->CreateBody(&bd);

		b2EdgeShape shape;
		shape.Set(b2Vec2(-40.0f, 0.0f), b2Vec2(40.0f, 0.0f));
		m_groundBody->CreateFixture(&shape, 0.0f);
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

	switch (Level)
	{
	case 0:
		CreateLevel1();
		break;
	case 1:
		CreateLevel2();
		break;

	default:
		CreateLevel2();
		break;
	}
	CreatePayload();
}
void AngryBirds::Step(Settings* settings)
{
	Test::Step(settings);

	// Seeking Bird

	if (m_BirdType == SEEK && Fired && SeekPig && CanSeek)
	{
		if (m_Payload != NULL)
		{
			m_DesiredVelocity = m_EnemyPigs[rand() % 2]->GetPosition() - m_Payload->GetPosition();
			float32 multiplier = 4000.0f;
			b2Vec2 force = b2Vec2(multiplier * m_DesiredVelocity.x, multiplier * m_DesiredVelocity.y);
			m_Payload->ApplyForce(force, force, true);
			SeekPig = false;
			CanSeek = false;
		}
		

			//b2Vec2 drop = b2Vec2(0.0f, -100000.0f);
			//b2Vec2 force = m_Payload->GetLinearVelocity();
			//force.x *= -1;
			//force.y *= -1;
			//b2Vec2 force2 = force + drop;
			//m_Payload->ApplyForce(force, force, true);
		
	}



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

		float check = 10.0f;
		if (mass1 == 14.0f)
		{
			check = 1.0f;
		}

		if (velocity > check)
			if (mass1 > 0.0f && mass2 > 0.0f && mass1 != mass2)
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
			bool ShouldNuke = true;
			UserData* data = static_cast<UserData*>(b->GetUserData());
			if (data != NULL)
			{
				if (data->isEnemy)
				{
					EnemyCount[0]--;
				}
				else if (data->isPayload)
				{
					ShouldNuke = false;
				}
				else if (!data->isDestroyable)
				{
					ShouldNuke = false;
				}
			}

			if (ShouldNuke)
				m_world->DestroyBody(b);
		}
	}



	if (m_Payload != NULL)
	{

		b2Vec2 pos = m_Payload->GetPosition();
		if(Fired)
			if (pos.y < 0 || pos.x < -40 || pos.x > 40)
			{	
				m_world->DestroyBody(m_Payload);
				m_Payload = NULL;
				Birdsleft[0]--;
				return;
			}

		else if (!m_Payload->IsAwake() && Fired)
		{
			m_world->DestroyBody(m_Payload);
			m_Payload = NULL;
			Birdsleft[0]--;
		}
		else if (m_Payload->IsAwake() && Fired)
		{
			DestroyTimer += 1.0f / 60.0f;
			if (DestroyTimer > 6.0f)
			{
				DestroyTimer = 0.0f;
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

	if (Birdsleft[0] <= '0')
	{

		LevelEndBirdTimer += 1.0f / 60.0f;

		if (LevelEndBirdTimer > 4.0f)
		{
			Begin();
			LevelEndBirdTimer = 0.0f;
		}	
	}
	else if (EnemyCount[0] <= '0')
	{
		LevelEndEnemyTimer += 1.0f / 60.0f;

		if (LevelEndEnemyTimer > 1.5f)
		{
			if (Level < 1)
				Level++;
			else
				Level = 0;

			Begin();
			LevelEndEnemyTimer = 0.0f;
		}	
	}
	
	
	std::string text = "Pigs left = ";
	std::string ammo = "Birds left = ";
	std::string level = "";
	std::string reset = "Press 'R' to Reset Level";
	std::string click = "Click and drag to aim bird, release to fire";
	std::string bird1 = "Press '1' for Standard Bird";
	std::string bird2 = "Press '2' for Seeking Bird";
	std::string bird3 = "Press '3' for Heavy Bird";
	std::string seek = "Press 'S' to Seek Enemy (only with Seeking Bird)";

	if (Level == 0)
		level = "Level 1";
	else
		level = "Level 2";
	
	g_debugDraw.DrawString(5, m_textLine, text.append(EnemyCount).c_str());
	m_textLine += 16;
	g_debugDraw.DrawString(5, m_textLine, ammo.append(Birdsleft).c_str());

	m_textLine += 32;
	g_debugDraw.DrawString(5, m_textLine, click.c_str());
	m_textLine += 24;

	g_debugDraw.DrawString(5, m_textLine, bird1.c_str());
	m_textLine += 16;
	g_debugDraw.DrawString(5, m_textLine, bird2.c_str());
	m_textLine += 16;
	g_debugDraw.DrawString(5, m_textLine, bird3.c_str());
	m_textLine += 24;

	g_debugDraw.DrawString(5, m_textLine, seek.c_str());
	m_textLine += 32;

	g_debugDraw.DrawString(5, m_textLine, reset.c_str());

	g_debugDraw.DrawString(500, 16, level.c_str());

	//std::string space = "Space Pressed";

	//if (SeekPig)
	//{
	//	g_debugDraw.DrawString(500, 16, space.c_str());
	//}
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
		PayloadData.Color = b2Color(1.0f, 1.0f, 0.f);
		break;
	case GLFW_KEY_2:
		m_BirdType = SEEK;
		PayloadData.Color = b2Color(0.2f, 0.2f, 1.0f);
		break;
	case GLFW_KEY_3:
		m_BirdType = HEAVY;
		PayloadData.Color = b2Color(0.f, 0.f, 0.f);
		break;
	case GLFW_KEY_S:
		if (m_BirdType == SEEK)
			SeekPig = true;		
		break;
	}
}

/*
@ Function: MouseUp 
@ Author: Cameron Peet 
@ Param : vector2 cointaining mouse coordinate on release of mouse button
@ desc : Player releases the payload so destroy this one to remove the JointDef between the payload and the slingshot, and apply the tension force
		 to a new payload that doesn't have the joint attached to it. Disable dragging this payload and set its density according to the selected payload type.
*/
void AngryBirds::MouseUp(const b2Vec2& p)
{
	if (m_mouseJoint && m_joint)
	{
		m_world->DestroyJoint(m_mouseJoint);
		m_mouseJoint = NULL;

		if (m_Payload->GetPosition().y > 0)
		{
			b2BodyDef bd;
			bd.type = b2_dynamicBody;
			b2Vec2 pos = m_Payload->GetPosition();
			bd.position.Set(pos.x, pos.y);
			b2Body* Payload = m_world->CreateBody(&bd);
			UserData* data = new UserData();
			data->isPayload = true;
			data->isDraggable = false;
			data->Color = PayloadData.Color;
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
				fd.friction = 0.5f;
				density = 20.0f;
				break;
			case SEEK:
				density = 3.5f;
				break;
			case NORMAL:
			default:
				density = 5.0f;
				break;
			}
			fd.density = density;
			Payload->CreateFixture(&fd);

			b2Vec2 force;
			if (m_BirdType == HEAVY)
				force = m_joint->GetReactionForce(500.0f);
			else
				force = m_joint->GetReactionForce(150.0f);

			Payload->ApplyForce(force, force, true);

			m_world->DestroyJoint(m_joint);
			m_world->DestroyBody(m_Payload);
			m_Payload = NULL;
			m_joint = NULL;

			m_Payload = Payload;

			Fired = true;
		}
		else
		{

			m_world->DestroyJoint(m_joint);
			m_world->DestroyBody(m_Payload);
			m_Payload = NULL;
			m_joint = NULL;
			CreatePayload();
		}
	}

	m_mouseFirstClick = b2Vec2(p);
}

/*
@ Function: MouseMove
@ Author: Cameron Peet
@ Param : vector2 containing mouse coordinate for each frame
@ desc :
*/
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
	CanSeek = true;

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

void AngryBirds::CreateLevel1()
{
	{
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
		body->SetUserData(&IndestructibleData);

		bd.position.Set(5.0f, 4.0f);
		body = m_world->CreateBody(&bd);
		body->CreateFixture(&fd);
		body->SetUserData(&IndestructibleData);

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
	//Create enemies
	{
		b2BodyDef bd;
		b2FixtureDef fd;
		b2CircleShape shape;

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
}

void AngryBirds::CreateEnemies()
{

}

void AngryBirds::CreateLevel2()
{

	b2Body* prevBody = m_groundBody;

	// Define crank.
	{
		b2PolygonShape shape;
		shape.SetAsBox(0.5f, 2.0f);

		b2BodyDef bd;
		bd.type = b2_dynamicBody;
		bd.position.Set(0.0f, 7.0f);
		b2Body* body = m_world->CreateBody(&bd);
		body->CreateFixture(&shape, 2.0f);
		body->SetUserData(&IndestructibleData);

		b2RevoluteJointDef rjd;
		rjd.Initialize(prevBody, body, b2Vec2(0.0f, 5.0f));
		
		rjd.motorSpeed = 1.0f * b2_pi;
		rjd.maxMotorTorque = 10000.0f;
		rjd.enableMotor = true;
		m_revJoint = (b2RevoluteJoint*)m_world->CreateJoint(&rjd);

		prevBody = body;
	}

	//// Define follower.
	{
		b2PolygonShape shape;
		shape.SetAsBox(4.0f, .5f);

		b2BodyDef bd;
		bd.type = b2_dynamicBody;
		bd.position.Set(4.0f, 9.0f);
		b2Body* body = m_world->CreateBody(&bd);
		body->CreateFixture(&shape, 2.0f);
		body->SetUserData(&IndestructibleData);

		b2RevoluteJointDef rjd;
		rjd.Initialize(prevBody, body, b2Vec2(0.0f, 9.0f));
		rjd.enableMotor = false;
		m_world->CreateJoint(&rjd);

		prevBody = body;
	}


	// Define piston
	{
		b2PolygonShape shape;
		shape.SetAsBox(1.5f, 1.5f);

		b2BodyDef bd;
		bd.type = b2_dynamicBody;
		bd.fixedRotation = true;
		bd.position.Set(8.0f, 9.0f);
		b2Body* body = m_world->CreateBody(&bd);
		body->CreateFixture(&shape, 2.0f);
		body->SetUserData(&IndestructibleData);

		b2RevoluteJointDef rjd;
		rjd.Initialize(prevBody, body, b2Vec2(8.0f, 9.0f));
		m_world->CreateJoint(&rjd);

	}

	//Define wall
	{
		b2PolygonShape shape;
		shape.SetAsBox(1.5f, 10.0f);

		b2BodyDef bd;
		bd.type = b2_staticBody;
		bd.fixedRotation = true;
		bd.position.Set(14.0f, 15.0f);
		b2Body* body = m_world->CreateBody(&bd);
		body->CreateFixture(&shape, 0.0f);
	}

	//Define platform
	{
		b2PolygonShape shape;
		shape.SetAsBox(6.0f, 0.5f);

		b2BodyDef bd;
		bd.type = b2_staticBody;
		bd.fixedRotation = true;
		bd.position.Set(2.0f, 14.0f);
		b2Body* body = m_world->CreateBody(&bd);
		body->CreateFixture(&shape, 2.0f);
		body->SetUserData(&DestructibleData);
	}

	//Define boxes
	{
		b2PolygonShape shape;
		shape.SetAsBox(1.0f, 1.0f);

		b2BodyDef bd;
		bd.type = b2_dynamicBody;
		bd.position.Set(6.0f, 16.0f);
		b2Body* body = m_world->CreateBody(&bd);
		body->CreateFixture(&shape, 2.0f);
		body->SetUserData(&IndestructibleData);

		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				bd.position.Set(2.0f + (1.0f * i), 16.0f + (1.0f * j));
				b2Body* body = m_world->CreateBody(&bd);
				body->CreateFixture(&shape, 2.0f);
				body->SetUserData(&IndestructibleData);
			}
		}
	}

	//Create the stack on the other side of the wall.
	{
		//Create a body type and position
		b2BodyDef bd;
		bd.type = b2_dynamicBody;
		bd.position.Set(15.0f, 4.0f);

		//Define a shape
		b2PolygonShape shape;
		shape.SetAsBox(4.0f, 0.5f, b2Vec2(4.0f, 0.0f), 0.5f * b2_pi);

		//Give it matter
		b2FixtureDef fd;
		fd.shape = &shape;
		fd.friction = 0.6f;
		fd.density = 7.0f;

		//Use the above templated to create multiple bodies in the world.
		//Vertical slates
		b2Body* body = m_world->CreateBody(&bd);
		body->CreateFixture(&fd);
		body->SetUserData(&IndestructibleData);

		bd.position.Set(20.0f, 4.0f);
		body = m_world->CreateBody(&bd);
		body->CreateFixture(&fd);
		body->SetUserData(&IndestructibleData);

		bd.position.Set(15.0f, 13.0f);
		body = m_world->CreateBody(&bd);
		body->CreateFixture(&fd);
		body->SetUserData(&DestructibleData);

		bd.position.Set(20.0f, 13.0f);
		body = m_world->CreateBody(&bd);
		body->CreateFixture(&fd);
		body->SetUserData(&DestructibleData);

		//Horizontal slates
		shape.SetAsBox(0.5f, 4.0f, b2Vec2(4.0f, 0.0f), 0.5f * b2_pi);

		bd.position.Set(17.5f, 8.5f);
		body = m_world->CreateBody(&bd);
		body->CreateFixture(&fd);
		body->SetUserData(&DestructibleData);

		bd.position.Set(17.5f, 17.5f);
		body = m_world->CreateBody(&bd);
		body->CreateFixture(&fd);
		body->SetUserData(&DestructibleData);
	}
	// Create some enemies on the stack
	{
		b2BodyDef bd;
		b2FixtureDef fd;
		b2CircleShape shape;

		bd.type = b2_dynamicBody;
		shape.m_radius = 1;
		fd.shape = &shape;
		fd.friction = 0.6f;
		fd.density = 1.0f;

		bd.position.Set(21.5f, 10.5f);
		m_EnemyPigs[0] = m_world->CreateBody(&bd);
		m_EnemyPigs[0]->CreateFixture(&fd);
		m_EnemyPigs[0]->SetUserData(&EnemyData);

		bd.position.Set(21.5f, 19.5f);
		m_EnemyPigs[1] = m_world->CreateBody(&bd);
		m_EnemyPigs[1]->CreateFixture(&fd);
		m_EnemyPigs[1]->SetUserData(&EnemyData);
	}
}