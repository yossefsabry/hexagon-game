#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

// global constant
const int  WINDOW_WIDTH = 800;
const int  WINDOW_HEIGHT = 600;
const int  TIMER_PERIOD = 25; 
const double  SCALE_SPEED = 1.020; //the scale amount applied to the hexagons in every fame
const float  HEXAGON_DELAY = 1250.0; //the amount of miliseconds passed between hexaons;
const bool COLOR_CHANGE = true; //true for repeatedly color change, false for no color chnge;
// ---- global constant -----

// new struct 
struct color_t {
  float r, g, b;
};

struct hexagon_t {
  color_t color;	 //randomly generated r,g,b color
  float scale;	 //scale of the hexagon, which increases in time
  int missingPart; //the empty part of the hexagon so that player can pass through
};

struct game_control_t{
  bool isStarted; //checks if the game is started or is over
  bool pause;		//checks if the game is paused
  bool animate;
};

struct score_t {
  int current;
  int max = -1;
};
// --- new struct ---

score_t score; //keeps current and the maximum score
int width, height;
game_control_t game;
hexagon_t hexagons[4];
int input = 0;	   //the screen is splitted into 6 parts and as players uses arrow keys, the value of this variable changes accordingly
int timerCount = 0;//counts how many times timer function runned
color_t background;//background color

float maxScale; //after this scale, the scale of the hexagons will be initialized to initialScale
float initialScale; //

float rotation;	   //rotation of all objects, hexagons and players, which changes in time
float rotateSpeed;//rotation speed, which randomly switch between 1 and 3
float scale;	  //scale of the whole screen, used in the initial animation and in game in general

/*
* inital some variable for program
*/
void initializeGlobals()
{
  input = 0;
  scale = 1;
  score.current = 0;
  game.isStarted = false;
  game.pause = false;
  game.animate = false;
  rotateSpeed = (rand() % 100) / 50.0 + 1; // rand() % 100 => genrate random int between 0 99
  rotation = 0;
  float frameNeeded = HEXAGON_DELAY / TIMER_PERIOD; // the number for frame needed (fbs) => 50
  float scaleMultiplier = powf(SCALE_SPEED, frameNeeded); // for zome 
  float hexagonScales[4];
  hexagonScales[3] = 0.2;
  for (int i = 2; i >= 0; i--)
    hexagonScales[i] = hexagonScales[i + 1] / scaleMultiplier;
  initialScale = hexagonScales[2];// get the maxscale from hexagoneScales
  maxScale = hexagonScales[3] * pow(scaleMultiplier,3); // get the max value for scale window
  for (int i = 0; i < 4; i++)
  // this loop ensures that each hexagon in the hexagons array is initialized with a random color, scale, and a random missing part index
  {
    hexagons[i].color = { rand() % 100 / 200.0f + 0.5f, rand() % 100 / 200.0f + 0.5f,rand() % 100 / 200.0f + 0.5f }; // get random color for R, G, B
    hexagons[i].scale = hexagonScales[i]; // the scale 
    hexagons[i].missingPart = rand() % 6; // genrate random int between 0 5
  }
  background = { rand() % 100 / 300.0f, rand() % 100 / 300.0f ,rand() % 100 / 300.0f }; // random background rgb
}

//
// to draw circle, center at (x,y)
//  radius r
//
void circle(int x, int y, int r)
{
  const float PI = 3.1415;
  float angle;

  glBegin(GL_POLYGON);
  for (int i = 0; i < 100; i++)
  {
    angle = 2 * PI*i / 100;
    glVertex2f(x + r * cos(angle), y + r * sin(angle));
  }
  glEnd();
}

void drawString(const char* string)
{
  glPushMatrix();
  while (*string)
    glutStrokeCharacter(GLUT_STROKE_ROMAN, *string++);
  glPopMatrix();
}

void displayBackground()
{
  glColor4f(0, 0, 0, 0.1);
  for (int i = 0; i < 3; i++)
  {
    glPushMatrix(); // saves a copy of the current matrix (the top matrix) on the matrix stack.
    glRotatef(i * 120.0 + rotation, 0, 0, 1);
    glBegin(GL_TRIANGLES);
    glVertex2f(0, 0);
    glVertex2f(-1500 / sqrt(3), -1500);
    glVertex2f(1500 / sqrt(3), -1500);
    glEnd();
    glPopMatrix();
    // After each triangle is drawn, the transformation state is restored to its original state for the next iteration.
  }
}

void displayHexagons()
{
  for (int i = 0; i < 4; i++) // for drawing 4 hexagon every time
  {
    for (int j = 0; j < 6; j++) // for drawing the hexagon 
    {
      if (j == hexagons[i].missingPart)
        continue;
      glColor3f(hexagons[i].color.r, hexagons[i].color.g, hexagons[i].color.b); // set color
      glPushMatrix();
      glScalef(hexagons[i].scale, hexagons[i].scale, 0);
      glRotatef(j * 60.0 + rotation, 0, 0, 1);
      glTranslatef(0, -100 * sqrt(3), 0);
      glRectf(-100, 0, 100, -5); // this function stop distore the hexagon and must drawing a shape for hexagon to not distoray 
      glPopMatrix();
    }
  }
}

void displayPlayer()
{
  glPushMatrix();
  glColor3f(1, 1, 1);
  glRotatef(input * 60.0 + rotation, 0, 0, 1);
  glTranslatef(0, -200, 0);
  circle(0, 0, 5);
  glPopMatrix();
}

void displayUI()
{
  glPushMatrix();
  if (!game.isStarted)
  {
    glColor4f(0, 0, 0, 0.7);
    glRectf(-300, -100, 300, 100);
    glColor3f(1, 1, 1);
    glTranslatef(-100, 0, 0);
    glScalef(0.3, 0.3, 0);
    drawString("F1 to Start");
    glTranslatef(-40, -100, 0);
    glScalef(0.5, 0.5, 0);
    drawString("Arrow keys to move < >");

    if (score.max != -1)
    {
      char str[100];
      glTranslatef(150, -200, 0);
      sprintf(str, "Max Score: %d", score.max);
      drawString(str);
      glTranslatef(0, -150, 0);
      sprintf(str, "Last Score: %d", score.current);
      drawString(str);
    }

  }
  else
  {
    glTranslatef(-290, 280, 0);
    glScalef(0.1, 0.1, 0);
    if (!game.pause)
      drawString("F2 to Pause");
    else
      drawString("F2 to Continue");
    char str[100];
    glPopMatrix();
    glPushMatrix();
    sprintf(str, "Score: %d", score.current);
    glTranslatef(230, 280, 0);
    glScalef(0.1, 0.1, 0);
    drawString(str);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(230, -280, 0);
    glScalef(0.1, 0.1, 0);
    drawString("F1 to reset");
  }
  glPopMatrix();

}

//
// To display onto window using OpenGL commands
//
void display()
{
  glClearColor(background.r, background.g, background.b, 0); // set background
  glClear(GL_COLOR_BUFFER_BIT); // for clear the buffer every time

  glLoadIdentity();
  glScalef(scale, scale, 0);
  if (game.animate)
    glRotatef(rotation, 0, 0, 1); 

  displayBackground();
  displayHexagons();
  displayPlayer();
  displayUI();

  glutSwapBuffers(); //making the back buffer (which now contains fully rendered scene) become the front buffer, and vice versa.
}

//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//
void ASCIIKeyDown(unsigned char key, int x, int y)
{
  if (key == 27)
    exit(0);
}

//
// Special Key like F1, F2, F3, Arrow Keys, Page UP, ...
//
void SpecialKeyDown(int key, int x, int y)
{
  if (game.isStarted && !game.pause)
  {
    switch (key) {
      case GLUT_KEY_LEFT:
        input = (input + 5) % 6;
        break;
      case GLUT_KEY_RIGHT:
        input = (input + 1) % 6;
        break;
    }
  }
  if (key == GLUT_KEY_F1)
  {
    initializeGlobals();
    game.animate = true;
    scale = 9;
  }
  else if (key == GLUT_KEY_F2)
    game.pause = !game.pause;
}

//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void reshape(int w, int h)
{
  width = w;
  height = h;
  glViewport(0, 0, w, h); // viewport to cover the entire window. 
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(float(-w) / 2, float(w) / 2, float(-h) / 2, float(h) / 2, -1, 1); // orthographic projection maintains the size of objects regardless of their distance from the viewer
  glMatrixMode(GL_MODELVIEW); // for 3d applying transformations that affect the position and orientation of objects
  glLoadIdentity();

}

void onTimer(int v) 
{
  glutTimerFunc(TIMER_PERIOD, onTimer, 0);
  //initial animation of the game
  if (game.animate)
  {
    timerCount++;
    scale -= 0.1;
    rotation += 9;
    if (scale <= 1)
    {
      scale = 1;
      rotation = 0;
      game.animate = false;
      game.isStarted = true;
    }
  }
  if (game.isStarted && !game.pause)
  {
    timerCount++;
    rotation += rotateSpeed;
    for (int i = 0; i < 4; i++)
    {
      if (fabs(hexagons[i].scale - 1.130) < 0.01) {
        if (input != hexagons[i].missingPart) {
          {
            game.isStarted = false;
            if (score.current > score.max) {
              score.max = score.current;
            }
          }
        }
        else { score.current++;
        }
      }
      hexagons[i].scale *= SCALE_SPEED;
      if (hexagons[i].scale >= maxScale)
      {
        hexagons[i].scale = initialScale;
      }
    }
    if (timerCount % 25 == 0 && COLOR_CHANGE)
    {
      for (int i = 0; i < 4; i++)
      {
        hexagons[i].color = { rand() % 100 / 200.0f + 0.5f, rand() % 100 / 200.0f + 0.5f,rand() % 100 / 200.0f + 0.5f };
      }
      background = { rand() % 100 / 300.0f, rand() % 100 / 300.0f ,rand() % 100 / 300.0f };
      rotateSpeed = (rand() % 100) / 20.0 - 2.5f;
    }
    else if (timerCount % 50 == 0)
    {
      int rnd = rand() % 60 - 30;
      if (rnd == 0)
        rnd = 180;
      rotation += rnd;
      timerCount = 0;
    }
  }
  glutPostRedisplay();
}

int main(int argc, char *argv[])
{
  initializeGlobals();
  srand(time(NULL)); // make sure every time you run the program take a new rendom value
  glutInit(&argc, argv); // utilize the GLUT
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE); // set the display mode RGB and enable double buffering
  glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT); // set width, height for window
  glutCreateWindow("hexagon game");
  //
  // display object and background
  //
  glutDisplayFunc(display); 
  glutReshapeFunc(reshape); // call the function window is resized.
  //
  // keyboard registration
  //
  glutKeyboardFunc(ASCIIKeyDown); // for exit
  glutSpecialFunc(SpecialKeyDown); // for move the game

  glEnable(GL_BLEND); //enables blending
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // set the src and destination for color in framebuffer

  glutTimerFunc(TIMER_PERIOD, onTimer, 0); // animation updates and inital the values

  glutMainLoop();
}

