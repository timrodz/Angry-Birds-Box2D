2D PHYSICS PROJECT - ANGRY BIRDS


CONTROLS
********

The controls are displayed on the game screen.

- Keys "1", "2" and "3" to select Normal, Seeking and Heavy birds.
- Mouse to aim and fire the bird.
- "R" to reset the level.
- "S" to seek enemies (with the seeking bird only).



FEATURE LIST
*************

- Two Levels
	- Level 1 - A simple scene with a pulley and some destructible and indestructible planks and enemies.
	- Level 2 - A more advanced scene including a piston motor with multiple revolute joints.

- Three Types of Joints
	- Player Slingshot uses a distance joint (b2DistanceJointDef).
	- Pulley in level 1 uses a pulley joint (b2PulleyJointDef).
	- Crank in level 2 uses multiple revolute joints (b2RevoluteJointDef).

- Three Bird Types 
	- Normal  - Yellow - Standard density and mass. Average projectile movement. Capable of knocking things over and destroying enemy entities. 
	- Heavy   - Black  - Heavier density and mass, slower projectile movement. Capable of destroying denser bodies and indestructible bodies (Feature)
	- Seeking - Blue   - Light density and mass, faster projectile movement. Pressing "S" makes the bird fly towards an enemy. (Feature)

- Destructible items with physics behaviour - Colour CYAN
- Indestructible items with physics behaviour - Colour PURPLE - Can be destroyed by HEAVY type bird
- Enemy Entities - Colour RED
- Indestructible Ground - Colour GREEN (b2_StaticBody default)
- Fling-able player entities - Colour (Yellow, Blue, Black) according to type.

- Gravity (-25.0f)
- Forces (Slingshot and collisions)
- Rigidbody (All dynamic entities)



CREDITS
*******

Cameron Peet
Mitchell Currie
Juan Rodriguez


