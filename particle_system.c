////////////////////////////////////////////////////////////////
// School of Computer Science
// The University of Manchester
//
// This code is licensed under the terms of the Creative Commons
// Attribution 2.0 Generic (CC BY 3.0) License.
//
// Skeleton code for COMP37111 coursework, 2012-13
//
// Authors: Arturs Bekasovs and Toby Howard
//
/////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#ifdef MACOSX
  #include <GLUT/glut.h>
#else
  #include <GL/glut.h>
#endif


#define MAX_PARTICLES 100000
// modeling units of ground extent in each X and Z direction.
#define EDGE 50
#define DEG_TO_RAD 0.017453293
#define MAX_VELOCITY 10.0
#define NORMAL_GRAVITY 9.8
#define LOW_GRAVITY 3.5
#define NO_GRAVITY 0.0
#define LARGE_POINT 8.0
#define NORMAL_POINT 1.5
#define NORMAL_SPEED 0.025
#define LOW_SPEED 0.0025
#define RUN_SPEED  0.17
#define TURN_ANGLE 4.0


// define the gravity
GLfloat gravity = NORMAL_GRAVITY;
GLfloat time_delta = NORMAL_SPEED;
GLfloat point_size = NORMAL_POINT;

float constant_value;

int particles_created = 0;

int transparent_mode = 0;
int random_color = 0;
int spread_mode = 0;
int point_render = 1;
int fixed_view = 1;
int bounce_mode = 0;

GLdouble lat,     lon;              /* View angles (degrees)    */
GLdouble mlat,    mlon;             /* Mouse look offset angles */
GLfloat  eyex,    eyey,    eyez;    /* Eye point                */
GLfloat  centerx, centery, centerz; /* Look point               */
GLfloat  upx,     upy,     upz;     /* View up vector           */

static GLfloat change_color[][4] = {
  { 1.0, 1.0, 0.0, 0.3 }, // R,G,B,transparency
  { 0.0, 1.0, 1.0, 0.3 },
  { 1.0, 0.0, 1.0, 0.3 },
  { 0.86, 0.27, 0.6, 0.3 },
  { 0.48, 0.81, 0.37, 0.3 },
  { 0.35, 0.81, 0.91, 0.3 },
  { 1.0, 0.44, 0.27, 0.3 },
  { 0.76, 0.16, 0.36, 0.3 },
  { 0.22, 0.16, 0.76, 0.3 },
  { 1.0, 0.0, 0.0, 0.3 },
  { 0.0, 1.0, 0.0, 0.3 },
  { 0.0, 0.0, 1.0, 0.3 },
};

#define NUM_COLORS (sizeof(change_color)/sizeof(change_color[0]))
static GLfloat default_color[] = {0.506, 0.694, 1.0, 0.35}; // default water color

double lastTime = 0.0;
int nbFrames = 0;

typedef struct _singleParti{
  // three dimension in position
  GLfloat position[3];
  // three dimension in velocity
  GLfloat velocity[3];
  // color of each particle
  GLfloat color[4];
  // time of the particle travel
  GLfloat time;
} singleParti;

// The particle system array
static singleParti particles[MAX_PARTICLES];

// Display list for coordinate axis
GLuint axisList;

int AXIS_SIZE= 200;
int axisEnabled= 1;

///////////////////////////////////////////////
// random functions

float myRandom()
//Return random float within range [0,1]
{
  return (rand()/(float)RAND_MAX);
} // myRandom

float randomRange(float low, float high)
//Return random float within the given range
{
  return (low + (high - low) * myRandom());
} // randomRange

///////////////////////////////////////////////
// main functions of the particle system
void newParticles(){
  int current_particles = particles_created;
  int i;
  for (i=current_particles; i<current_particles + 25; i++){
    particles[i].time = 0.0;
    // initiate all dimension to zero
    int dim;
    for(dim=0;dim<3;dim++){
      particles[i].position[dim] = 0.0;
    } // for
    // initialise the velocity in Y axes to MAX_VELOCITY
    // which is the ejection velocity
    particles[i].velocity[1] = MAX_VELOCITY;
    // According to velocity in Y calculate the velocity in other directions
    // control the direction of the water moves
    float direction = randomRange(0.0, 360.0) * M_PI/180.0;
    // calculate the displacement for x and z
    float dx = cos(direction);
    float dz = sin(direction);
    // adjusting the output angle
    float angle;
    if (spread_mode){
      angle = randomRange(52.0, 77.0) * M_PI/180.0;
    } // if
    else{
      angle = randomRange(68.0, 72.0) * M_PI/180.0;
      // angle = 72.0 * M_PI/180.0;
    } // else
    // calculate the velocity in x and z
    particles[i].velocity[0] = (particles[i].velocity[1] / sin(angle)) * cos(angle) * dx;
    particles[i].velocity[2] = (particles[i].velocity[1] / sin(angle)) * cos(angle) * dz;
    // set the color
    if (random_color){
      int index = rand() % NUM_COLORS;
      int c;
      for(c=0;c<4;c++){
        particles[i].color[c] = change_color[index][c];
      } // for
    } // if
    else {
      int c;
      for(c=0;c<4;c++){
        particles[i].color[c] = default_color[c];
      } // for
    } // else
    particles_created++;
  } // for
} // newParticles

void moveParticles(){
  int i;
  for(i = 0; i < particles_created; i++){
    if(particles[i].position[1]<0 && particles[i].time>8){
      // remove that particle
      int now = particles_created;
      int c;
      for (c = i; c < now - 1; c++){
        particles[c] = particles[c+1];
      } // for
      particles_created--;
    } // if
    else if (particles[i].position[1]<0){
      if (bounce_mode){
        GLfloat randomEngryDecrease = randomRange(0.15, 0.45);
        particles[i].velocity[1] *= (-randomEngryDecrease);
        GLfloat displace_y = particles[i].velocity[1] * time_delta - (gravity * pow(time_delta,2))/2;
        particles[i].position[1] += displace_y;
        particles[i].time += (time_delta/2);
      } // if
      else{
        particles[i].time += time_delta;
      } // else
    } // else if
    else {
      GLfloat displace_y;
      // s -> displacement
      // u -> initial velocity
      // v -> terminal velocity
      // t -> change in time

      // s = ut + (1/2)at^2
      displace_y = particles[i].velocity[1] * time_delta - constant_value;
      particles[i].position[1] += displace_y;

      // v = u + at
      particles[i].velocity[1] += (time_delta*(-gravity));

      //add time count to determine when the particle will be dead
      float new_time = particles[i].time + time_delta;
      // s = ut
      particles[i].position[0] = particles[i].velocity[0] * new_time;
      particles[i].position[2] = particles[i].velocity[2] * new_time;
      particles[i].time = new_time;
    } // else
  } // for
} // moveParticles

void showScene(){
  glColor3f(0.55, 0.55, 0.55);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-EDGE, -0.05, -EDGE);
  glTexCoord2f(20.0, 0.0);
  glVertex3f(EDGE, -0.05, -EDGE);
  glTexCoord2f(20.0, 20.0);
  glVertex3f(EDGE, -0.05, EDGE);
  glTexCoord2f(0.0, 20.0);
  glVertex3f(-EDGE, -0.05, EDGE);
  glEnd();

  /* If enabled, draw coordinate axis. The effect of calling glCallList() on a
  display list named list is to execute again all the OpenGL commands stored in
  the list*/
  if(axisEnabled) glCallList(axisList);
} // showScene

void showParticlesAndScene(){
  showScene();
  int i;
  if (transparent_mode){
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } // if
  glPointSize(point_size);
  glBegin(GL_POINTS);
  glDisable(GL_TEXTURE_2D);
  // show the particles
  for(i = 0; i < particles_created; i++){
      if(point_render){
        glColor4fv(particles[i].color);
        glVertex3fv(particles[i].position);
      } else{
        // reference from https://stackoverflow.com/questions/33191717/moving-3d-shape-in-opengl/33193987
        glPushMatrix();
        glTranslatef(particles[i].position[0], particles[i].position[1], particles[i].position[2]);
        glColor4fv(particles[i].color);
        glScalef(1.0, 1.0, 1.0);
        glutSolidSphere(0.05, 5, 5);
        glPopMatrix();
      } // else
  } // for
  glEnd();
  glDisable(GL_BLEND);
} // showParticles

// main particle system functions
void particle_sys_main(){
  // restart the demo when the particles are too much
  if (particles_created >= (MAX_PARTICLES - 25))
  {
    particles_created = 0;
  } // if
  newParticles();
  // clock_t t = clock();
  showParticlesAndScene();
  moveParticles();
  // t = clock() - t;
  // float time_taken = 1000 * ((float)t)/CLOCKS_PER_SEC; // in seconds
  // printf("Took %f milliseconds to render and move %d particles \n", time_taken, particles_created);
  // printf("%f\n",time_taken);
  glutPostRedisplay();
} // particle_sys_main

///////////////////////////////////////////////
// referenced from http://www.opengl-tutorial.org/miscellaneous/an-fps-counter/
void fpsCount(){
  // get the time elapsed in milliseconds
  double currentTime =  glutGet(GLUT_ELAPSED_TIME);
  nbFrames++;
  if ( currentTime - lastTime >= 1000.0 ){
    // If last prinf() was more than 1 sec ago
    // printf and reset timer
    // printf("%d frames/second\n", nbFrames);
    printf("%d\n", nbFrames);
    nbFrames = 0;
    lastTime += 1000.0;
  } // if
} // fpsCount

///////////////////////////////////////////////

void calculate_lookpoint()
{   // Given an eyepoint and latitude and longitude angles, will
    // compute a look point one unit away

    if(-90<lat && lat<90)
    {
      centerx = eyex + cos(lat*DEG_TO_RAD)*sin(lon*DEG_TO_RAD);
      centery = eyey + sin(lat*DEG_TO_RAD);
      centerz = eyez + cos(lat*DEG_TO_RAD)*cos(lon*DEG_TO_RAD);
    } // if
    else if (-90>lat)
    {
      centerx = eyex + cos(-89.99*DEG_TO_RAD)*sin(lon*DEG_TO_RAD);
      centery = eyey + sin(-89.99*DEG_TO_RAD);
      centerz = eyez + cos(-89.99*DEG_TO_RAD)*cos(lon*DEG_TO_RAD);
    } // else if
    else
    {
      centerx = eyex + cos(89.99*DEG_TO_RAD)*sin(lon*DEG_TO_RAD);
      centery = eyey + sin(89.99*DEG_TO_RAD);
      centerz = eyez + cos(89.99*DEG_TO_RAD)*cos(lon*DEG_TO_RAD);
    } // else
} // calculate_lookpoint

// this function is registed as the display function
void display()
{
  glLoadIdentity(); /* initialise the matrix to the unit transformation */
  if(fixed_view){
    // specify the position and orientation of the camera
    gluLookAt(10.0, 6.0, 10.0,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);
  } // if
  else {
    calculate_lookpoint(); /* Compute the centre of interest   */
    gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
  } // else
  // Clear the screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  particle_sys_main();
  fpsCount();
  /* swaps the back buffer with the front buffer, at the next opportunity, which
  is normally the next vertical retrace of the monitor */
  glutSwapBuffers();
} // display

///////////////////////////////////////////////

void keyboard(unsigned char key, int x, int y)
{
  switch(key){
    case 27: exit(0);
             break;
    case 'a': axisEnabled = 1 - axisEnabled;
              break;
    case 'r': particles_created = 0;
              transparent_mode = 0;
              random_color = 0;
              spread_mode = 0;
              point_render = 1;
              fixed_view = 1;
              bounce_mode = 0;
              gravity = NORMAL_GRAVITY;
              time_delta = NORMAL_SPEED;
              point_size = NORMAL_POINT;
              break;
    case ',': eyex += RUN_SPEED*sin((lon+90)*DEG_TO_RAD);
              eyez += RUN_SPEED*cos((lon+90)*DEG_TO_RAD);
              break;
    case '.': eyex += RUN_SPEED*sin((lon-90)*DEG_TO_RAD);
              eyez += RUN_SPEED*cos((lon-90)*DEG_TO_RAD);
              break;
  } // switch
  // tells OpenGL that the application is asking for the display to be refreshed
  glutPostRedisplay();
} // keyboard

void cursor_keys(int key, int x, int y) {
  switch (key) {
      /* To be completed */
      // Rotate your view to the left (increase longitude angle) by TURN_ANGLE degrees.
      case GLUT_KEY_LEFT:
        lon += TURN_ANGLE;
        break;

      // Rotate your view to the right (decrease longitude angle) by TURN_ANGLE degrees.
      case GLUT_KEY_RIGHT:
        lon -= TURN_ANGLE;
        break;

      // Tilt your view up (increase latitude angle) by TURN_ANGLE degrees.
      case GLUT_KEY_PAGE_UP:
        if (lat + TURN_ANGLE < 90)
          lat += TURN_ANGLE;
        break;

      // Tilt your view down (decrease latitude angle) by TURN_ANGLE degrees.
      case GLUT_KEY_PAGE_DOWN:
        if (lat - TURN_ANGLE > -90)
          lat -= TURN_ANGLE;
        break;

      // Re-centre (set back to zero) your latitude angle.
      case GLUT_KEY_HOME:
        lat = 0.0;
        break;

      case GLUT_KEY_UP:
        eyex += RUN_SPEED*sin(lon*DEG_TO_RAD);
        eyez += RUN_SPEED*cos(lon*DEG_TO_RAD);
        break;

      case GLUT_KEY_DOWN:
        eyex -= RUN_SPEED*sin(lon*DEG_TO_RAD);
        eyez -= RUN_SPEED*cos(lon*DEG_TO_RAD);
        break;
  } // switch
} // cursor_keys

///////////////////////////////////////////////

void reshape(int width, int height)
{
  /* Called when the window is created, moved or resized */
  glClearColor(0.95, 0.95, 0.95, 1.0);
  glViewport(0, 0, (GLsizei)width, (GLsizei)height);
  glMatrixMode(GL_PROJECTION); /* Select the projection matrix */
  glLoadIdentity();
  gluPerspective(60, (GLfloat)width / (GLfloat)height, 1.0, 10000.0);
  glMatrixMode(GL_MODELVIEW); /* Select the modelview matrix */
} // reshape

void menu(int option){
  switch(option)
  {
    case 1: point_size = LARGE_POINT;
            point_render = 1;
            break;
    case 2: point_size = NORMAL_POINT;
            point_render = 1;
            break;
    case 3: point_render = 0;
            break;
    case 4: gravity = NORMAL_GRAVITY;
            constant_value = (gravity * pow(time_delta,2))/2;
            break;
    case 5: gravity = LOW_GRAVITY;
            constant_value = (gravity * pow(time_delta,2))/2;
            break;
    case 6: gravity = NO_GRAVITY;
            constant_value = (gravity * pow(time_delta,2))/2;
            break;
    case 7: time_delta = NORMAL_SPEED;
            constant_value = (gravity * pow(time_delta,2))/2;
            break;
    case 8: time_delta = LOW_SPEED;
            constant_value = (gravity * pow(time_delta,2))/2;
            break;
    case 9: spread_mode = 1;
            particles_created = 0;
            break;
    case 10: spread_mode = 0;
             particles_created = 0;
             break;
    case 11: random_color = 1- random_color;
             particles_created = 0;
             break;
    case 12: transparent_mode = 1 - transparent_mode;
             break;
    case 13: fixed_view = 1 - fixed_view;
             break;
    case 14: bounce_mode = 1 - bounce_mode;
             particles_created = 0;
             break;
  } // switch
} // menu

void menuCreation()
{
  glutCreateMenu(menu);
  glutAddMenuEntry ("Large point", 1);
  glutAddMenuEntry ("Small point", 2);
  glutAddMenuEntry ("Sphere", 3);
  glutAddMenuEntry ("Normal gravity", 4);
  glutAddMenuEntry ("Low gravity", 5);
  glutAddMenuEntry ("No gravity", 6);
  glutAddMenuEntry ("Normal flying speed", 7);
  glutAddMenuEntry ("Low flying speed", 8);
  glutAddMenuEntry ("Wide spread", 9);
  glutAddMenuEntry ("Normal spread", 10);
  glutAddMenuEntry ("Toggle colourful points mode",11);
  glutAddMenuEntry ("Toggle transparent mode",12);
  glutAddMenuEntry ("Toggle flying around view",13);
  glutAddMenuEntry ("Toggle bounce mode",14);
  glutAttachMenu (GLUT_RIGHT_BUTTON);
} // menuCreation

///////////////////////////////////////////////

void makeAxes() {
// Create a display list for drawing coord axis
  axisList = glGenLists(1);
  glNewList(axisList, GL_COMPILE);
      glLineWidth(2.0);
      glBegin(GL_LINES);
      glColor3f(1.0, 0.0, 0.0);       // X axis - red
      glVertex3f(0.0, 0.0, 0.0);
      glVertex3f(AXIS_SIZE, 0.0, 0.0);
      glColor3f(0.0, 1.0, 0.0);       // Y axis - green
      glVertex3f(0.0, 0.0, 0.0);
      glVertex3f(0.0, AXIS_SIZE, 0.0);
      glColor3f(0.0, 0.0, 1.0);       // Z axis - blue
      glVertex3f(0.0, 0.0, 0.0);
      glVertex3f(0.0, 0.0, AXIS_SIZE);
    glEnd();
  glEndList();
} // makeAxes

///////////////////////////////////////////////
void initGraphics(int argc, char *argv[])
{
  glutInit(&argc, argv); /* Initialise OpenGL */
  glutInitWindowSize(800, 600); /* control the size of the window */
  glutInitWindowPosition(100, 100); /* control the position of the window */
  /* sets the current display mode, which which will be used for a window
  created using glutCreateWindow(), GLUT DOUBLE: selects a double-buffered
  window */
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
  glutCreateWindow("COMP37111 Particles"); /* Create the window */

  /* Set initial view parameters */
  eyex=  10.0; /* Set eyepoint at eye height within the scene */
  // eyey=  10.0;
  eyey =  4.0;
  eyez=  10.0;

  upx= 0.0;   /* Set up direction to the +Y axis  */
  upy= 1.0;
  upz= 0.0;

  lat= 0.0;   /* Look horizontally ...  */
  lon= 0.0;   /* ... along the +Z axis  */

  constant_value = (gravity * pow(time_delta,2))/2;

  glutDisplayFunc(display); /* Register the "display" function */
  /* registers the application function to call when OpenGL detects a key
  press generating an ASCII character */
  glutKeyboardFunc(keyboard);
  glutSpecialFunc (cursor_keys);
  menuCreation();
  /* registers the application callback to call when the window is first created,
  and also if the window manager subsequently informs OpenGL that the user has
  reshaped the window */
  glutReshapeFunc(reshape);
  makeAxes();
  lastTime =  glutGet(GLUT_ELAPSED_TIME);
} // initGraphics

/////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  double f;
  srand(time(NULL)); /* Seed the random-number generator with current time */
  initGraphics(argc, argv);
  glEnable(GL_POINT_SMOOTH); /* explicitly ‘enabled’ one property by the application */
  glutMainLoop(); /* Enter the OpenGL main loop */
} // main
