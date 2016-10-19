#include <GL/glut.h>
#include <ctime>
#include <cmath>

#define PI 3.14159265
#define CUBES 15

GLfloat angle[CUBES] = { 0.0 };

GLfloat fovy = 60.0;
GLfloat zNear = 0.0;
GLfloat zFar = 100.0;

GLfloat transX[CUBES];
GLfloat transY[CUBES];
GLfloat transZ[CUBES];

GLfloat red[CUBES];
GLfloat blue[CUBES];
GLfloat green[CUBES];

GLfloat spin[CUBES];

GLfloat ambred = 1.0;
GLfloat ambgreen = 1.0;
GLfloat ambblue = 1.0;

GLfloat lx = -10.0;
GLfloat ly = 0.0;
GLfloat lz = 1.0;
GLfloat lw = 0.0;

/***    drawing text on screen  ***/
void drawString(char *string)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);
	glOrtho(0, w, 0, h, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);

	glDisable(GL_LIGHTING);
	glColor3f(1, 0, 0);

	glRasterPos2i(20, 20);
	void *font = GLUT_BITMAP_HELVETICA_18;
	for (char* c = string; *c != '\0'; c++)
	{
		glutBitmapCharacter(font, *c);
	}

	glEnable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

/***  Creates random spin for blocks ***/
void initializeSpin()
{
	for (int i = 0; i<CUBES; i++)
	{
		spin[i] = (float)rand() / ((float)RAND_MAX / 2.5);
	}
}

/***    Creates Random Location on Far Plane    ***/
void reset(GLfloat &x, GLfloat &y, GLfloat &z)
{
	x = rand() % 101 - 50;
	y = rand() % 101 - 50;
	z = -95.0;
}

/***    Create Random Colors    ***/
void initializeColor(GLfloat &x, GLfloat &y, GLfloat &z)
{
	x = (float)((rand() % 10)*0.1);
	y = (float)((rand() % 10)*0.1);
	z = (float)((rand() % 10)*0.1);
}

/***    Create the Cube     ***/
void cube(int n)
{
	GLfloat calcThetaY;
	GLfloat calcThetaX;
	calcThetaY = abs(atan(transY[n] / transZ[n]) * 180 / PI);
	calcThetaX = abs(atan(transX[n] / transZ[n]) * 180 / PI);
	if ((calcThetaY > (fovy / 2) + 10) || (calcThetaX > (fovy / 2) + 10) || transZ[n] > zNear + 10)
	{
		reset(transX[n], transY[n], transZ[n]);
		initializeColor(red[n], green[n], blue[n]);
		spin[n] = (float)rand() / ((float)RAND_MAX / 2.5);
	}

	glPushMatrix();

	glTranslatef(transX[n], transY[n], transZ[n]);
	glRotatef(angle[n], 1.0, 0.0, 0.0);
	glRotatef(angle[n], 0.0, 1.0, 0.0);
	glRotatef(angle[n], 0.0, 0.0, 1.0);

	glColor3f(red[n], green[n], blue[n]);

	glutSolidCube(2);

	glPopMatrix();
}

void myInit(void)
{
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);

	for (int i = 0; i<CUBES; i++)
	{
		reset(transX[i], transY[i], transZ[i]);
	}

	for (int i = 0; i<CUBES; i++)
	{
		initializeColor(red[i], green[i], blue[i]);
	}

	initializeSpin();
}

void display(void)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	double w = glutGet(GLUT_WINDOW_WIDTH);
	double h = glutGet(GLUT_WINDOW_HEIGHT);
	gluPerspective(60, w / h, 1.0, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	GLfloat AmbientLight[] = { ambred, ambgreen, ambblue };
	glLightfv(GL_LIGHT1, GL_AMBIENT, AmbientLight);
	GLfloat LightPosition[] = { lx, ly, lz, lw };
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	for (int j = 0; j<CUBES; j++)
	{
		cube(j);
		angle[j] += spin[j];
		transZ[j] += 0.25;
	}

	drawString("Why aren't the cubes being displayed?");

	glutSwapBuffers();
}

void quitProgram(unsigned char key, int x, int y)
{
	if (key == 113) exit(0);
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("boxes2");
	myInit();
	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutKeyboardFunc(quitProgram);
	glutMainLoop();
	return 0;
}