#include "Ball.h"

Ball::Ball(GLfloat blockSize, GLfloat offsetX, GLfloat offsetY, GLfloat offsetZ)
{
	this->scale = 1.0f;

	printf("x=%f, y=%f, z=%f\n", offsetX, offsetY, offsetZ);

	this->x = 0 + offsetX;
	this->y = 0 + offsetY;
	this->z = 0 + offsetZ;

	this->radius = (blockSize * this->scale)/2;

	this->orientation = TOP;
	this->facing = FRONT;

	this->model = NULL;

	if((this->model = load_model("models/ball.md3")) == NULL)
	{
		printf("Could not load model!\n");
	}
}

Ball::~Ball()
{
}

void Ball::render()
{
	glPushMatrix();

	glTranslatef(this->x, this->y, this->z);
	if(this->model == NULL)
		draw_unit_cube();
	else
		draw_md3_file(this->model);


	glPopMatrix();
}

typedef struct
{
	int x;
	int y;
	int z;
} intXYZ;

intXYZ facing_vector[MAX_DIRS] = 
{
	{ 0, 0, 0},	// None
	{ 0, 1, 0},	// Top
	{ 0,-1, 0},	// Bottom
	{ 1, 0, 0},	// Left
	{-1, 0, 0},	// Right
	{ 0, 0,-1},	// Front
	{ 0, 0, 1},	// Back
};

void Ball::lookAtMe(Camera *c)
{
	GLfloat udist = 3, adist = 3;
	GLfloat ux=0, uy=1, uz=0;
	GLfloat ax=0, ay=0, az=0;

	ux = facing_vector[this->orientation].x;
	uy = facing_vector[this->orientation].y;
	uz = facing_vector[this->orientation].z;
	/*switch(this->orientation)
	{
		case TOP:	ux= 0; uy= 1; uz= 0; break;
		case BOTTOM:ux= 0; uy=-1; uz= 0; break;
		case LEFT:	ux= 0; uy= 0; uz= 0; break;
		case RIGHT:	ux= 0; uy= 0; uz= 0; break;
		case FRONT:	ux= 0; uy= 0; uz= 0; break;
		case BACK:	ux= 0; uy= 0; uz= 0; break;
		default: ux=0; uy=1; ux=0; break;
	}
	switch(this->facing)
	{
		case TOP:	ax= 0; ay= 0; az= 0; break;
		case BOTTOM:ax= 0; ay= 0; az= 0; break;
		case LEFT:	ax= 1; ay= 0; az= 0; break;
		case RIGHT:	ax=-1; ay= 0; az= 0; break;
		case FRONT:	ax= 0; ay= 0; az=-1; break;
		case BACK:	ax= 0; ay= 0; az= 1; break;
		default: ax=0; ay=0; ax=1; break;
	}*/
	ax = facing_vector[this->facing].x;
	ay = facing_vector[this->facing].y;
	az = facing_vector[this->facing].z;
	/*gluLookAt(	(this->x+this->radius) + (ax*dist) + (ux*dist), (this->y+this->radius) + (ay*dist) + (uy*dist), (this->z+this->radius) + (az*dist) + (uz*dist),
				this->x+this->radius, this->y+this->radius, this->z+this->radius,
				ux,uy,uz
			 );*/
	c->setEye( (this->x+this->radius) + (ax*adist) + (ux*udist), (this->y+this->radius) + (ay*adist) + (uy*udist), (this->z+this->radius) + (az*adist) + (uz*udist) );
	c->setAt( this->x+this->radius, this->y+this->radius, this->z+this->radius );
	c->setUp(ux, uy, uz);
}

Orientation left_facing[MAX_DIRS][MAX_DIRS] = {
	//	NONE	TOP,	BOTTOM,	LEFT,	RIGHT,	FRONT,	BACK 
	{	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE},	// orientation = NONE
	{	NONE,	NONE,	NONE,	FRONT,	BACK,	RIGHT,	LEFT},	// orientation = TOP
	{	NONE,	NONE,	NONE,	BACK,	FRONT,	LEFT,	RIGHT},	// orientation = BOTTOM
	{	NONE,	BACK,	FRONT,	NONE,	NONE,	TOP,	BOTTOM},// orientation = LEFT
	{	NONE,	FRONT,	BACK,	NONE,	NONE,	BOTTOM,	TOP},	// orientation = RIGHT
	{	NONE,	LEFT,	RIGHT,	BOTTOM,	TOP,	NONE,	NONE},	// orientation = FRONT
	{	NONE,	RIGHT,	LEFT,	TOP,	BOTTOM,	NONE,	NONE},	// orientation = BACK
};

void Ball::turnLeft()
{
	//printf("Turn left!\n");
	/*if(this->orientation == UP && this->facing == FRONT)
		this->facing = RIGHT;
	else if(this->orientation == UP && this->facing == RIGHT)
		this->facing = BACK;
	else if(this->orientation == UP && this->facing == BACK)
		this->facing = LEFT;
	else if(this->orientation == UP && this->facing == LEFT)
		this->facing = FRONT;*/
	Orientation no = left_facing[this->orientation][this->facing];
	if(no != NONE)
		this->facing = no;
}

Orientation right_facing[MAX_DIRS][MAX_DIRS] = {
	//	NONE	TOP,	BOTTOM,	LEFT,	RIGHT,	FRONT,	BACK 
	{	NONE,	NONE,	NONE,	NONE,	NONE,	NONE,	NONE},	// orientation = NONE
	{	NONE,	NONE,	NONE,	BACK,	FRONT,	LEFT,	RIGHT},	// orientation = TOP
	{	NONE,	NONE,	NONE,	FRONT,	BACK,	RIGHT,	LEFT},	// orientation = BOTTOM
	{	NONE,	FRONT,	BACK,	NONE,	NONE,	BOTTOM,	TOP},	// orientation = LEFT
	{	NONE,	BACK,	FRONT,	NONE,	NONE,	TOP,	BOTTOM},// orientation = RIGHT
	{	NONE,	RIGHT,	LEFT,	TOP,	BOTTOM,	NONE,	NONE},	// orientation = FRONT
	{	NONE,	LEFT,	RIGHT,	BOTTOM,	TOP,	NONE,	NONE},	// orientation = BACK
};

void Ball::turnRight()
{
	//printf("Turn right!\n");
	/*if(this->orientation == UP)
	{
		if(this->facing == FRONT)
			this->facing = LEFT;
		else if(this->facing == LEFT)
			this->facing = BACK;
		else if(this->facing == BACK)
			this->facing = RIGHT;
		else if(this->facing == RIGHT)
			this->facing = FRONT;
	}*/
	//for(int j=0
	Orientation no = right_facing[this->orientation][this->facing];
	if(no != NONE)
		this->facing = no;
}

/*Orientation Ball::perpendicular(Orientation orientation, Orientation facing)
{
	// might be left_facing?
	right_facing[orientation][facing];
}*/

void Ball::move(GLfloat byF, GLfloat byS)
{
	this->x += (byF * -facing_vector[this->facing].x) + (byS * facing_vector[right_facing[orientation][facing]].x);
	this->y += (byF * -facing_vector[this->facing].y) + (byS * facing_vector[right_facing[orientation][facing]].y);
	this->z += (byF * -facing_vector[this->facing].z) + (byS * facing_vector[right_facing[orientation][facing]].z);;
}

GLfloat Ball::getX()
{
	return this->x;
}

GLfloat Ball::getY()
{
	return this->y;
}

GLfloat Ball::getZ()
{
	return this->z;
}


GLfloat Ball::getFacingX()
{
	return facing_vector[this->facing].x;
}

GLfloat Ball::getFacingY()
{
	return facing_vector[this->facing].y;
}

GLfloat Ball::getFacingZ()
{
	return facing_vector[this->facing].z;
}

GLfloat Ball::getSideX()
{
	return facing_vector[right_facing[orientation][facing]].x;
}

GLfloat Ball::getSideY()
{
	return facing_vector[right_facing[orientation][facing]].y;
}

GLfloat Ball::getSideZ()
{
	return facing_vector[right_facing[orientation][facing]].z;
}

