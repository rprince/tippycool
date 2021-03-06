// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2000,2004 Alistair Riddoch

#include "headers.h"
#include "StatusBar.h"
#include "MenuSystem.h";
#include "MenuItem.h";
#include "Menu.h";
#include "fmod.h"

// Constants
static const int screen_width = 600;
static const int screen_height = 400;

//Menu m("Game Menu", screen_width, screen_height);
StatusBar *sb;
MenuSystem *ms;
Menu *m1, *m2, *m3;
TTF_Font *font24, *font20, *font16, *font14;

// Number of squares in the grid. The number of points is this number +1.
static const int grid_width = 12;
static const int grid_height = 12;

// Number of milliseconds between steps in the game model.
static const int step_time = 1000;

// Variables that store the game state

static int block_x = 4;
static int block_y = 11;

int score;

// Structure to hold the properties of a single square on the grid.
// If you want to add more information to the grid, add new members here.
typedef struct block_properties {
    bool block;
} BlockProperties;

// Current blocks on the grid, stored as true if there is a block at the
// given localtion.
BlockProperties properties[grid_width][grid_height];

// Flag used to inform the main loop if the program should now terminate.
// Set this to true if its done.
static bool program_finished = false;

// Calculated frames per second to display. Very useful feedback when
// debugging graphics performance problems.
int average_frames_per_second;

// Texture handles for the texture used to handle printing text on the
// screen.
GLuint textTexture;
GLuint textBase;

//FMOD tutorial: http://www.gamedev.net/reference/articles/article2098.asp
FSOUND_SAMPLE *menuclick = NULL;
FSOUND_SAMPLE *buttonblockclick = NULL;
FSOUND_SAMPLE *getstar = NULL;
FSOUND_SAMPLE *goal = NULL;
FSOUND_SAMPLE *blockmove = NULL;
FSOUND_SAMPLE *shower = NULL;
FSOUND_SAMPLE *hitspikes = NULL;

bool init_audio() {	
	return FSOUND_Init(44100, 32, 0);
}

void uninit_audio() {
	FSOUND_Sample_Free(menuclick);
	FSOUND_Sample_Free(buttonblockclick);
	FSOUND_Sample_Free(getstar);
	FSOUND_Sample_Free(goal);
	FSOUND_Sample_Free(blockmove);
	FSOUND_Sample_Free(shower);
	FSOUND_Sample_Free(hitspikes);
	FSOUND_Close();
}

void load_audio() {
	menuclick = FSOUND_Sample_Load (0, "audio//menuclick.wav", 0, 0, 0); //http://www.partnersinrhyme.com/pirsounds/WEB_DESIGN_SOUNDS_WAV/BUTTONS.shtml
	buttonblockclick = FSOUND_Sample_Load (0, "audio//buttonblockclick.wav", 0, 0, 0); //http://www.wavsource.com/sfx/sfx.htm
	getstar = FSOUND_Sample_Load (0, "audio//getstar.wav", 0, 0, 0); //http://www.wavsource.com/sfx/sfx.htm
	goal = FSOUND_Sample_Load (0, "audio//goal.wav", 0, 0, 0); //http://www.wavsource.com/sfx/sfx.htm
	blockmove = FSOUND_Sample_Load (0, "audio//blockmove.wav", 0, 0, 0); //http://www.a1freesoundeffects.com/button.html
	shower = FSOUND_Sample_Load (0, "audio//shower.wav", 0, 0, 0); //http://www.a1freesoundeffects.com/button.html
	hitspikes = FSOUND_Sample_Load (0, "audio//hitspikes.wav", 0, 0, 0); //http://www.wavsource.com/sfx/sfx2.htm
	FSOUND_SetSFXMasterVolume(50);
}

void play_audio(int channel, FSOUND_SAMPLE *sound) {
	FSOUND_PlaySound (channel, sound);
}

// Initialise the graphics subsystem. This is pretty much boiler plate
// code with very little to worry about.
bool init_graphics()
{
    // Initialise SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        return false;
	}

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Surface * screen;

    // Create the window
    screen = SDL_SetVideoMode(screen_width, screen_height, 0, SDL_OPENGL);
    if (screen == NULL) {
        // std::cerr << "Failed to set video mode" << std::endl << std::flush;
        SDL_Quit();
        return false;
    }
/**/
    // Setup the viewport transform
    glViewport(0, 0, screen_width, screen_height);

    // Enable vertex arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    // Texture coordinate arrays well need to be enabled _ONLY_ when using
    // texture coordinates from an array, and disabled afterwards.
    // glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // Set the colour the screen will be when cleared - black
    glClearColor(0.0, 0.5, 1.0, 0.0);
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	/* Required if you want alpha-blended textures (for our fonts) */
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);
    
	// Initialise the texture used for rendering text
    glGenTextures(1, &textTexture);
    glBindTexture(GL_TEXTURE_2D, textTexture);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexImage2D(GL_TEXTURE_2D, 0, texture_font_internalFormat,
                 texture_font_width, texture_font_height, 0,
                 texture_font_format, GL_UNSIGNED_BYTE, texture_font_pixels);
    if (glGetError() != 0) {
        return false;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    textBase = glGenLists(256);
    float vertices[] = { 0, 0, 16, 0, 16, 16, 0, 16 };
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    for(int loop=0; loop<256; loop++) {
        float cx=(float)(loop%16)/16.0f;      // X Position Of Current Character
        float cy=(float)(loop/16)/16.0f;      // Y Position Of Current Character

        float texcoords[] = { cx, 1-cy-0.0625f,
                              cx+0.0625f, 1-cy-0.0625f,
                              cx+0.0625f, 1-cy,
                              cx, 1-cy };

        glNewList(textBase+loop,GL_COMPILE);   // Start Building A List

        glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
        glDrawArrays(GL_QUADS, 0, 4);

        glTranslated(10,0,0);                  // Move To The Right Of The Character
        glEndList();                           // Done Building The Display List
    }
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    return true;
}

// Clear the grid state.
void clear()
{
    for(int i = 0; i < grid_width; ++i) {
        for(int j = 0; j < grid_height; ++j) {
            properties[i][j].block = false;
        }
    }
}

// Print a text string on the screen at the current position.
void gl_print(const char * str)
{
    glPushMatrix();
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, textTexture);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glListBase(textBase-32);
    glCallLists(strlen(str),GL_BYTE,str);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glPopMatrix();
}

void setup()
{
    // Clear the block store
    clear();

}

void draw_unit_cube()
{
    static const float front_vertices[] = {
        0.f, 0.f, 1.f,
        1.f, 0.f, 1.f,
        1.f, 1.f, 1.f,
        0.f, 1.f, 1.f,
    };
    glVertexPointer(3, GL_FLOAT, 0, front_vertices);
    glDrawArrays(GL_QUADS, 0, 4);

    static const float left_vertices[] = {
        0.f, 0.f, 0.f,
        0.f, 0.f, 1.f,
        0.f, 1.f, 1.f,
        0.f, 1.f, 0.f,
    };
    glVertexPointer(3, GL_FLOAT, 0, left_vertices);
    glDrawArrays(GL_QUADS, 0, 4);

    static const float right_vertices[] = {
        1.f, 0.f, 1.f,
        1.f, 0.f, 0.f,
        1.f, 1.f, 0.f,
        1.f, 1.f, 1.f,
    };
    glVertexPointer(3, GL_FLOAT, 0, right_vertices);
    glDrawArrays(GL_QUADS, 0, 4);

    static const float top_vertices[] = {
        0.f, 1.f, 1.f,
        1.f, 1.f, 1.f,
        1.f, 1.f, 0.f,
        0.f, 1.f, 0.f,
    };
    glVertexPointer(3, GL_FLOAT, 0, top_vertices);
    glDrawArrays(GL_QUADS, 0, 4);

    static const float bottom_vertices[] = {
        0.f, 0.f, 0.f,
        1.f, 0.f, 0.f,
        1.f, 0.f, 1.f,
        0.f, 0.f, 1.f,
    };
    glVertexPointer(3, GL_FLOAT, 0, bottom_vertices);
    glDrawArrays(GL_QUADS, 0, 4);
}

void draw_grid()
{
    float horizontal_line_vertices[] = {
        0.f, 0.f, 0.f,
        grid_width, 0.f, 0.f,
    };
    float vertical_line_vertices[] = {
        0.f, 0.f, 0.f,
        0.f, grid_height, 0.f,
    };

    glColor3f(0.3, 0.3, 0.3);

    // Move to the origin of the grid
    glTranslatef(-(float)grid_width/2.0f, -(float)grid_height/2.0f, 0.0f);
    // Store this position
    glPushMatrix();

    // Draw vertical lines
    glVertexPointer(3, GL_FLOAT, 0, vertical_line_vertices);
    for (int i = 0; i <= grid_width; ++i) {
        glDrawArrays(GL_LINES, 0, 2);
        glTranslatef(1.0f, 0.0f, 0.0f);
    }

    // Reset to the origin
    glPopMatrix();
    glPushMatrix();

    // Draw horizontal lines
    glVertexPointer(3, GL_FLOAT, 0, horizontal_line_vertices);
    for (int j = 0; j <= grid_height; ++j) {
        glDrawArrays(GL_LINES, 0, 2);
        glTranslatef(0.0f, 1.0f, 0.0f);
    }

    // Reset to the origin
    glPopMatrix();
    glPushMatrix();

    // Draw blocks whereever one should be placed on the grid.
    for(int i = 0; i < grid_width; ++i) {
        for(int j = 0; j < grid_height; ++j) {
            if ((properties[i][j].block)) {
                draw_unit_cube();
            }
            glTranslatef(0.0f, 1.0f, 0.0f);
        }
        glTranslatef(1.0f, -grid_height, 0.0f);
    }

    glColor3f(1.0, 1.0, 1.0);

    // Draw the user controlled block
    glPopMatrix();
    glTranslatef(block_x, block_y, 0.f);
    draw_unit_cube();
}

float camera_rotation = 0.0f;

void camera_pos()
{
    // Set up the modelview
    glMatrixMode(GL_MODELVIEW);
    // Reset the camera
    glLoadIdentity();
    // Move the camera 20 units from the objects
    glTranslatef(0.0f, 0.0f, -20.0f);
    // Add a little camera movement
    glRotatef(10, sin(camera_rotation), cos(camera_rotation), 0.0f);
}

void render_scene()
{
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable the depth test
    glEnable(GL_DEPTH_TEST);

    // Set the projection transform
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (float)screen_width/screen_height, 1.f, 100.f);

    // Set the camera position
    camera_pos();

    // Draw the scene
    draw_grid();
}

void glEnable2D()
{
	int vPort[4];
  
	glGetIntegerv(GL_VIEWPORT, vPort);
  
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
  
	glOrtho(0, vPort[2], 0, vPort[3], -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void glDisable2D()
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();	
}

// Draw any text output and other screen oriented user interface
// If you want any kind of text or other information overlayed on top
// of the 3d view, put it here.
void render_interface()
{
    char buf[256];

    // Set the projection to a transform that allows us to use pixel
    // coordinates.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screen_width, 0, screen_height, -800.0f, 800.0f);

    // Set up the modelview
    glMatrixMode(GL_MODELVIEW);
    // Reset the camera
    glLoadIdentity();

    // Disable the depth test, as its not useful when rendering text
    glDisable(GL_DEPTH_TEST);

    // Print the number of frames per second. This is essential performance
    // information when developing 3D graphics.

    // Use glTranslatef to go to the screen coordinates where we want the
    // text. The origin is the bottom left by default in OpenGL.
    glTranslatef(5.f, 5.f, 0);
    glColor3f(1.f, 1.f, 1.f);
    sprintf(buf, "FPS: %d", average_frames_per_second);
    gl_print(buf);

	/*SDL_Event event;

	int done = 0;
	while (!done)
	{*/
		//glEnable2D();
		//glDisable(GL_DEPTH_TEST);
		ms->render();
		sb->render();
		//glEnable(GL_DEPTH_TEST);
		//glDisable2D();
/*		SDL_GL_SwapBuffers( );
		SDL_WaitEvent(&event);
		switch(event.type)
		{
			case SDL_QUIT:
				done = 1;
				break;
		}
	}*/
}

// Handle a mouse click. Call this function with the screen coordinates where
// the mouse was clicked, in OpenGL format with the origin in the bottom left.
// This function exactly mimics rendering a scene, and then detects which
// grid location of the scene were under the mouse pointer when the click
// occured.
void mouse_click(unsigned int x, unsigned int y)
{
    GLuint selectBuf[512];
    GLfloat square_vertices[] = { 0.f, 0.f, 0.f, 1.f, 0.f, 0.f,
                                  1.f, 1.f, 0.f, 0.f, 1.f, 0.f };

    // Put OpenGL into Selection mode rather than Render mode. This disables
    // rendering to the window.
    glSelectBuffer(512,selectBuf);
    glRenderMode(GL_SELECT);

    
    // This is just like setting up the projection for normal rendering
    // except we use gluPickMatrix to specify the portion of the screen
    // we are interested in - the 1 pixel square under the mouse pointer.
    {
        // Set the projection transform
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT,viewport);
        gluPickMatrix(x, y, 1, 1, viewport);
        gluPerspective(45, (float)screen_width/screen_height, 1.f, 100.f);

        // Set the camera position
        camera_pos();
    }

    // Each thing we render will have a numerical name that we must later use
    // to identify which thing we rendered was under the mouse pointer.
    // The names are pushed onto a stack, so a thing can have more than
    // one. We will just have one name per thing, so we start by pushing a
    // token name onto the stack.
    glInitNames();
    glPushName(0);

    // Here we will draw the grid exactly as it was drawn when rendering to
    // the screen, except we draw a quad on each square rather than just
    // drawing lines. If you want to detect clicking on other things, you
    // will need to modify the code here.
    glTranslatef(-(float)grid_width/2.0f, -(float)grid_height/2.0f, 0.0f);

    glVertexPointer(3, GL_FLOAT, 0, square_vertices);

    for(int i = 0; i < grid_width; ++i) {
        for(int j = 0; j < grid_height; ++j) {
            // Before we render each grid square, we specify the name by
            // loading it into the top of the name stack. The name
            // is derived from the coordinates of the square, so that later
            // when we get a name back, we can tell which square it corresponds
            // to.
            glLoadName(i + j * grid_width);
            glDrawArrays(GL_QUADS, 0, 4);
            glTranslatef(0.0f, 1.0f, 0.0f);
        }
        glTranslatef(1.0f, -grid_height, 0.0f);
    }

    // We have now finished with the name stack, so we empty it.
    glPopName();

    // Put OpenGL back into rendering mode. This has the side effect of
    // returning how many hits we got in the selection code above.
    int hits = glRenderMode(GL_RENDER);

    // std::cout << "Got " << hits << " hits" << std::endl << std::flush;

    // If we got no hits, the user clicked on empty space.
    if (hits == 0) {
        return;
    }

    // This code searches through the results buffer to find what hits we
    // got.

    // ptr points to the beginning of the results
    GLuint * ptr = &selectBuf[0];
    GLuint minDepth = UINT_MAX, noNames = 0;
    GLuint * namePtr = 0;
    for (int i = 0; i < hits; i++) {
        int names = *(ptr++);
        // std::cout << "{" << *ptr << "}";
        // Check if this hit is closer to the viewer than the last one we
        // processed. If this test passes, then this is the closest hit
        // so far, so we record the necessary details.
        if (*ptr < minDepth) {
            noNames = names;
            minDepth = *ptr;
            namePtr = ptr + 2;
        }
        ptr += (names + 2);
    }
    // Once this loop is complete, we have the details of the nearest hit
    // stored, which is the thing we rendered that was closest to the viewer
    // under the mouse pointer. This should be the thing the player was aiming
    // to click on when they clicked.

    // noNames = Number of names our hit has
    // namePtr = Pointer to list of names

    // Extract the X and Y coords of the clicked square from the hit name
    GLuint hitName = *namePtr;
    int hit_x = hitName % grid_width;
    int hit_y = hitName / grid_width;

    // Place or remove a block on the square the user clicked.
    /*if (hit_x < grid_width && hit_y < grid_height) {
        properties[hit_x][hit_y].block = !properties[hit_x][hit_y].block;
    }*/

	
}

// This function is called every step_time milliseconds. In many games you
// will want to use this function to update the game state. For example in
// a Tetris game, this would be the place where the falling blocks were
// moved down the grid.
void step()
{
}

// The main program loop function. This does not return until the program
// has finished.
void loop()
{
    SDL_Event event;
    int elapsed_time = SDL_GetTicks();
    int last_step = elapsed_time;
    int frame_count = 0;

    // This is the main program loop. It will run until something sets
    // the flag to indicate we are done.
    while (!program_finished) {
        // Check for events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    // The user closed the window
                    program_finished = true;
                    break;
                case SDL_KEYDOWN:
                    // We have a keypress
                    if ( event.key.keysym.sym == SDLK_ESCAPE ) {
                        // quit
                        program_finished = true;
                    }
                    if ( event.key.keysym.sym == SDLK_UP ) {
						ms->changeSelected(-1);
                    }
                    if ( event.key.keysym.sym == SDLK_DOWN ) {
						ms->changeSelected(1);
                    }
                    if ( event.key.keysym.sym == SDLK_LEFT ) {
						ms->changeSelected(-1);
                    }
                    if ( event.key.keysym.sym == SDLK_RIGHT ) {
						ms->changeSelected(1);
                    }
                    if ( event.key.keysym.sym == SDLK_RETURN ) {
						if(ms->enter() == 4)
						{
							program_finished = true;
						}
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
						ms->mouseDown(event.button.x, event.button.y);
                    }
                    break;
				case SDL_MOUSEBUTTONUP:
					if(event.button.button == SDL_BUTTON_LEFT)
					{
						int num = ms->mouseUp(event.button.x, event.button.y);
						if(num > -1)
						{
							play_audio(0,  menuclick);
						}
						if(num == 4)
						{
							Sleep(100);
							program_finished = true;
						}
					}
					break;
				case SDL_MOUSEMOTION:
					ms->mouseMotion(event.motion.x, event.motion.y);
					break;
                default:
                    break;
            }
        }

        ++frame_count;

        // Get the time and check if a complete time step has passed.
        // For step based games like Tetris, this is used to update the
        // the game state
        const int ticks = SDL_GetTicks();
        if ((ticks - last_step) > step_time) {
            last_step = ticks;
            average_frames_per_second = frame_count;
            frame_count = 0;
            step();
        }

        // Calculate the time in seconds since the last frame
        // For a real time program this would be used to update the game state
        float delta = (ticks - elapsed_time) / 1000.0f;
        elapsed_time = ticks;

        // Update the rotation on the camera
        camera_rotation += delta;
        if (camera_rotation > (2 * M_PI)) {
            camera_rotation -= (2 * M_PI);
        }

        // Render the screen
        render_scene();
        render_interface();

        SDL_GL_SwapBuffers();
    }
}

int gotStar()
{
	score = score + 100;
	return score;
}

//load font for menus
char fontpath[] = "font//arial.ttf";

void init_fonts()
{
	if(TTF_Init()==-1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(2);
	}

	if(!(font24 = TTF_OpenFont(fontpath, 24))) {
		printf("Error loading font: %s", TTF_GetError());
		//return 1;
	}

	if(!(font20 = TTF_OpenFont(fontpath, 20))) {
		printf("Error loading font: %s", TTF_GetError());
		//return 1;
	}

	if(!(font16 = TTF_OpenFont(fontpath, 16))) {
		printf("Error loading font: %s", TTF_GetError());
		//return 1;
	}

	if(!(font14 = TTF_OpenFont(fontpath, 14))) {
		printf("Error loading font: %s", TTF_GetError());
		//return 1;
	}
}

void uninit_fonts()
{
	// free the font
	TTF_CloseFont(font14);
	TTF_CloseFont(font16);
	TTF_CloseFont(font20);
	TTF_CloseFont(font24);
	font14=NULL;
	font16=NULL;
	font20=NULL;
	font24=NULL;
	TTF_Quit();
}

void createStatusBar()
{
	init_fonts();
	sb = new StatusBar(font20, false, screen_width, screen_height);
	sb->setText(font20, "Status Bar");
}

void createMenus()
{
	init_fonts();

	//-1 - No Menu
	//0 - Game Menu
	//1 - Level Menu
	//2 - Instructions Menu
	//3 - Start Game
	//4 - Exit Game

	m1 = new Menu("Main Menu", screen_width, screen_height, font20);
	m1->add(new MenuItem("Start", font20, 3, true, true));
	m1->add(new MenuItem("Select Level", font20, 1, true, true));
	m1->add(new MenuItem("Instructions", font20, 2, true, true));
	m1->add(new MenuItem("Exit", font20, 4, true, true));
	m1->layout();

	m2 = new Menu("Level Menu", screen_width, screen_height, font20);
	m2->add(new MenuItem("Level 1", font16, 3, true, true));
	m2->add(new MenuItem("Level 2", font16, 3, true, true));
	m2->add(new MenuItem("Level 3", font16, 3, true, true));
	m2->add(new MenuItem("Level 4", font16, 3, true, true));
	m2->add(new MenuItem("Level 5", font16, 3, true, true));
	m2->add(new MenuItem("Level 6", font16, 3, true, true));
	m2->add(new MenuItem("Level 7", font16, 3, true, true));
	m2->add(new MenuItem("Level 8", font16, 3, true, true));
	m2->add(new MenuItem("Level 9", font16, 3, true, true));
	m2->add(new MenuItem("Level 10", font16, 3, true, true));
	m2->add(new MenuItem("Back to Main Menu", font20, 0, true, true));
	m2->layout();

	m3 = new Menu("Instructions Menu", screen_width, screen_height, font20);
	m3->add(new MenuItem(" ", font16, -1, false, false));
	m3->add(new MenuItem("TextTextText", font16, -1, false, false));
	m3->add(new MenuItem("TextTextText", font16, -1, false, false));
	m3->add(new MenuItem("TextTextText", font16, -1, false, false));
	m3->add(new MenuItem("TextTextText", font16, -1, false, false));
	m3->add(new MenuItem(" ", font16, -1, false, false));
	m3->add(new MenuItem(" ", font16, -1, false, false));
	m3->add(new MenuItem("Back to Main Menu", font20, 0, true, true));
	m3->setInstructions(true);
	m3->layout();

	ms = new MenuSystem();
	ms->add(m1);
	ms->add(m2);
	ms->add(m3);
}

int main()
{
    // Initialise the graphics
    if (!init_graphics()) {
        return 1;
    }
	//Initialise the audio
    if (!init_audio()) {
        return 1;
    }
	load_audio();

	createMenus();
	createStatusBar();

    // Intialise the game state
    setup();

    // Run the game
    loop();
    return 0;
}



#ifdef WIN32

int WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
)
{
    main();
}

#endif
