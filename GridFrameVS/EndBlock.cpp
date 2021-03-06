#include "EndBlock.h"

#include <stdio.h>

EndBlock::EndBlock(Orientation o)
{
	printf("Constructing EndBlock!\n");

	this->model = NULL;

	if((this->model = load_model("models/endflag2.md3")) == NULL)
	{
		printf("Could not load model!\n");
	}

	this->orientation = o;
	this->facing = none;

	this->moveable = false;

	this->moved = false;

	printf("Finished constructing\n");
}

EndBlock::EndBlock(Orientation o, bool m)
{
	printf("Constructing EndBlock!\n");

	this->model = NULL;

	if((this->model = load_model("models/endflag2.md3")) == NULL)
	{
		printf("Could not load model!\n");
	}

	this->orientation = o;
	this->facing = none;

	this->moveable = m;

	this->moved = false;

	printf("Finished constructing\n");
}

EndBlock::~EndBlock()
{
	printf("Destructing EndBlock!\n");
}

/*void EndBlock::render()
{
}*/

void EndBlock::print(int i)
{
	printf("Hello\n");
}

CollisionResult EndBlock::collision()
{
	return COMPLETE_LEVEL;
}
