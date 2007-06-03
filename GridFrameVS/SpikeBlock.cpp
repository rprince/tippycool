#include "SpikeBlock.h"

#include <stdio.h>

SpikeBlock::SpikeBlock(Orientation o)
{
	printf("Constructing SpikeBlock!\n");

	this->model = NULL;

	if((this->model = load_model("models/spikes.md3")) == NULL)
	{
		printf("Could not load model!\n");
	}

	this->orientation = o;

	printf("Finished constructing\n");
}

SpikeBlock::~SpikeBlock()
{
	printf("Destructing SpikeBlock!\n");
}

void SpikeBlock::render()
{
	glPushMatrix();
	//printf("Hello\n");

	glColor3f(0.435294f, 0.192f, 0.596f);

	glTranslatef(0.5, 0.5, 0.5);
	if(this->orientation == TOP)
		glRotatef(-90, 1.0, 0.0, 0.0);
	else if(this->orientation == BOTTOM)
		glRotatef(90, 1.0, 0.0, 0.0);
	else if(this->orientation == LEFT)
		glRotatef(-90, 0.0, 1.0, 0.0);
	else if(this->orientation == RIGHT)
		glRotatef(90, 0.0, 1.0, 0.0);
	else if(this->orientation == FRONT)
		glRotatef(0, 1.0, 0.0, 0.0);
	else if(this->orientation == BACK)
		glRotatef(-180, 1.0, 0.0, 0.0);
	glTranslatef(-0.5, -0.5, -0.5);
	
	if(this->model == NULL)
		draw_unit_cube();
	else
		draw_md3_file(this->model);

	glPopMatrix();
}

void SpikeBlock::print(int i)
{
	printf("Hello\n");
}
