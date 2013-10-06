/*
Ryan Bohinski
CSC 350
Assignment 5

Pool table animation.
*/

#include <iostream>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#ifdef _WIN32
#	include "stdafx.h"
#endif

using namespace std;

/* Latitudinal angle. */
static float latAngle = 0.0;
/* Longitudinal angle. */
static float longAngle = 0.0;
/* Angles to rotate scene. */
static float Xangle = 0.0, Yangle = 0.0, Zangle = 0.0;

static const double PI = atan(1) * 4;

/*
Defaults to 16 for an animation period that approximately matches the default refresh rate of monitors. (60 Hz - 16.6666 ms)
*/
static int animationPeriod = 16;

static const float camera_LocX = 0.0;
static const float camera_LocY = 0.0;
static const float camera_LocZ = 25.0;
static const float camera_LookX = 0.0;
static const float camera_LookY = 0.0;
static const float camera_LookZ = 0.0;
static const float camera_UpX = 0.0;
static const float camera_UpY = 1.0;
static const float camera_UpZ = 0.0;

static const float frustum_Left = -5.0;
static const float frustum_Right = 5.0;
static const float frustum_Bottom = -5.0;
static const float frustum_Top = 5.0;
static const float frustum_Near = 5.0;
static const float frustum_Far = 100.0;

/* Calcualte the clickable area shown in the window. */
static const float visible_X = ((frustum_Top - frustum_Bottom) / 2)
* (camera_LocZ) / (frustum_Near);
static const float visible_Y = ((frustum_Right - frustum_Left) / 2)
* (camera_LocZ) / (frustum_Near);

/* Declare the table size. */
static const float table_SizeX = 42.5;
static const float table_SizeY = 21.25;
static const float table_SizeZ = 5.0;
static const float table_BorderHeight = 2.5;
static const float table_BorderWidth = 1;

/*
Point order: Clockwise from top left.
Outer top, Inner top, Bottom.
*/
static float table_BorderPoints[] {
	(float)((-0.5 * table_SizeX) - table_BorderWidth),	(float)((0.5 * table_SizeY) + table_BorderWidth),	table_BorderHeight,
	(float)((0.5 * table_SizeX) + table_BorderWidth),	(float)((0.5 * table_SizeY) + table_BorderWidth),	table_BorderHeight,
	(float)((0.5 * table_SizeX) + table_BorderWidth),	(float)((-0.5 * table_SizeY) - table_BorderWidth),	table_BorderHeight,
	(float)((-0.5 * table_SizeX) - table_BorderWidth),	(float)((-0.5 * table_SizeY) - table_BorderWidth),	table_BorderHeight,
	(float)(-0.5 * table_SizeX),						(float)(0.5 * table_SizeY),							table_BorderHeight,
	(float)(0.5 * table_SizeX),							(float)(0.5 * table_SizeY),							table_BorderHeight,
	(float)(0.5 * table_SizeX),							(float)(-0.5 * table_SizeY),						table_BorderHeight,
	(float)(-0.5 * table_SizeX),						(float)(-0.5 * table_SizeY),						table_BorderHeight,
	(float)((-0.5 * table_SizeX) - table_BorderWidth),	(float)((0.5 * table_SizeY) + table_BorderWidth),	(float)(-1.0 * table_SizeZ - 0.5),
	(float)((0.5 * table_SizeX) + table_BorderWidth),	(float)((0.5 * table_SizeY) + table_BorderWidth),	(float)(-1.0 * table_SizeZ - 0.5),
	(float)((0.5 * table_SizeX) + table_BorderWidth),	(float)((-0.5 * table_SizeY) - table_BorderWidth),	(float)(-1.0 * table_SizeZ - 0.5),
	(float)((-0.5 * table_SizeX) - table_BorderWidth),	(float)((-0.5 * table_SizeY) - table_BorderWidth),	(float)(-1.0 * table_SizeZ - 0.5)
};

static unsigned char stripIndices[] = {
	0, 4, 3, 7, 2, 6, 1, 5, 0, 8, 3, 11, 2, 10, 1, 9, 0, 8, 9, 11, 10
};

/* Used for calculating the speed loss of the ball. */
static const float table_Friction = (float) 0.01;

static const float ball_Radius = 1.0;
static int ball_DetailLevel = 10;
static bool ball_Wire = true;

/* Information about the location and direction of the ball. */
static float ball_X = 0.0;
static float ball_Y = 0.0;
static float ball_XVelocity = 0.0;
static float ball_YVelocity = 0.0;
static float ball_XAngle = 0.0;
static float ball_YAngle = 0.0;
static float ball_XVelocity_Stop = (float)0.1;
static float ball_YVelocity_Stop = (float)0.1;

/* Drawing routine. */
void drawScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(camera_LocX, camera_LocY, camera_LocZ, camera_LookX, camera_LookY,
		camera_LookZ, camera_UpX, camera_UpY, camera_UpZ);

	/* Rotate scene. */
	glRotatef(Zangle, 0.0, 0.0, 1.0);
	glRotatef(Yangle, 0.0, 1.0, 0.0);
	glRotatef(Xangle, 1.0, 0.0, 0.0);

	/* Save the default matrix view. */
	glPushMatrix();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	/* Playing area. */
	glColor3f((float)0.38, (float)0.65, (float)0.435);
	/* Move the table so that the ball and table meet at Z = 0 */
	glTranslatef((float) 0.0, (float) 0.0, (float)-(table_SizeZ / 2.0));
	glScalef(table_SizeX, table_SizeY, table_SizeZ);
	glutSolidCube(1.0);

	/*
	Reset the matrix to draw the table border.
	*/
	glPopMatrix();
	glPushMatrix();

	/* Table border. */
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, table_BorderPoints);

	glColor3f((float)0.482, (float)0.263, (float)0.212);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLE_STRIP, 21, GL_UNSIGNED_BYTE, stripIndices);

	glDisableClientState(GL_VERTEX_ARRAY);

	/* Reset the matrix to draw the ball. */
	glPopMatrix();
	glPushMatrix();

	/* Rotating ball */
	/* Move the ball towards the camera so that it does not clip with the table. */
	glTranslatef(ball_X, ball_Y, ball_Radius);
	glRotatef(ball_XAngle, 0.0, 1.0, 0.0);
	glRotatef(ball_YAngle, -1.0, 0.0, 0.0);

	glColor3f((float) 0.95, (float) 0.95, (float) 0.95);
	if (ball_Wire) {
		glutWireSphere(ball_Radius, ball_DetailLevel, ball_DetailLevel);
	}
	else {
		glutSolidSphere(ball_Radius, ball_DetailLevel, ball_DetailLevel);
	}

	glPopMatrix();

	glutSwapBuffers();
}

/* Timer function for the animation. By default triggers every 16 ms. */
void animate(int value) {
	ball_X += (float)(ball_XVelocity / 60.0);
	ball_Y += (float)(ball_YVelocity / 60.0);

	ball_XAngle += (float)((ball_XVelocity / 60.0) / (2 * PI * ball_Radius)) * 360;
	ball_YAngle -= (float)((ball_YVelocity / 60.0) / (2 * PI * ball_Radius)) * 360;

	if (ball_XAngle >= 360) {
		ball_XAngle -= 360;
	}
	else if (ball_XAngle < 0) {
		ball_XAngle += 360;
	}

	if (ball_YAngle >= 360) {
		ball_YAngle -= 360;
	}
	else if (ball_YAngle < 0) {
		ball_YAngle += 360;
	}

	/* Checking for collision in the X direction. */
	if (ball_X >(table_SizeX / 2) - ball_Radius) {
		/* Ball hit the right border, reverse the X direction of motion. */
		ball_X = (table_SizeX / 2) - ball_Radius;
		ball_XVelocity = -ball_XVelocity;
	}
	else if (ball_X < (-table_SizeX / 2) - (-ball_Radius)) {
		/* Ball hit the left border, reverse the X direction of motion. */
		ball_X = (-table_SizeX / 2) - (-ball_Radius);
		ball_XVelocity = -ball_XVelocity;
	}

	/* Checking for collision in the Y direction. */
	if (ball_Y >(table_SizeY / 2) - ball_Radius) {
		/* Ball hit the top border, reverse the Y direction of motion. */
		ball_Y = (table_SizeY / 2) - ball_Radius;
		ball_YVelocity = -ball_YVelocity;
	}
	else if (ball_Y < (-table_SizeY / 2) - (-ball_Radius)) {
		/* Ball hit the bottom border, reverse the Y direction of motion. */
		ball_Y = (-table_SizeY / 2) - (-ball_Radius);
		ball_YVelocity = -ball_YVelocity;
	}

	/* Friction. */
	ball_XVelocity = ball_XVelocity * (1 - table_Friction);
	ball_YVelocity = ball_YVelocity * (1 - table_Friction);

	/* Check for speed limits so that the ball does not keep rolling forever. */
	if (ball_XVelocity > 0 && ball_XVelocity <= ball_XVelocity_Stop) {
		ball_XVelocity = 0.0;
	}
	if (ball_YVelocity > 0 && ball_YVelocity <= ball_YVelocity_Stop) {
		ball_YVelocity = 0.0;
	}

	if (ball_XVelocity < 0 && ball_XVelocity >= -ball_XVelocity_Stop) {
		ball_XVelocity = 0.0;
	}
	if (ball_YVelocity < 0 && ball_YVelocity >= -ball_YVelocity_Stop) {
		ball_YVelocity = 0.0;
	}

	glutTimerFunc(animationPeriod, animate, 1);
	glutPostRedisplay();
}

/* OpenGL window reshape routine. */
void resize(int w, int h) {
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(frustum_Left, frustum_Right,
		frustum_Bottom * ((float)h / (float)w),
		frustum_Top * ((float)h / (float)w), frustum_Near, frustum_Far);

	glMatrixMode(GL_MODELVIEW);
}

void mouseInput(int button, int state, int x, int y) {
	if (ball_XVelocity >= ball_XVelocity_Stop
		|| ball_XVelocity <= -ball_XVelocity_Stop
		|| ball_YVelocity >= ball_YVelocity_Stop
		|| ball_YVelocity <= -ball_YVelocity_Stop) {
			/* The ball is still moving, ignore the click. */
	}
	else {
		/* Get the coordinates realtive to the current view. By default, returns with a range from -100 to 100. */
		GLint viewport[4];
		GLdouble modelview[16];
		GLdouble projection[16];
		GLfloat winX, winY, winZ;
		GLdouble posX, posY, posZ;

		glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
		glGetDoublev(GL_PROJECTION_MATRIX, projection);
		glGetIntegerv(GL_VIEWPORT, viewport);

		winX = (float)x;
		winY = (float)viewport[3] - (float)y;
		winZ = 1;

		gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX,
			&posY, &posZ);

		/* Convert the range to the visible playing field.*/
		posX *= visible_X / 100;
		posY *= visible_Y / 100;

		switch (button) {
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN) {
				/*
				Positive = Moving right.
				Negative = Moving left.
				*/
				ball_XVelocity = (float)((ball_X - posX) * 5);
				/*
				Positive = Moving up.
				Negative = Moving down.
				*/
				ball_YVelocity = (float)((ball_Y - posY) * 5);
			}
			break;
		default:
			break;
		}
	}
}

/* Keyboard input processing routine. */
void keyInput(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
		exit(0);
		break;
	case 'w':
		Xangle -= 5.0;
		if (Xangle < -90.0)
			Xangle = -90.0;
		if (Xangle > 0.0)
			Xangle = 0.0;
		glutPostRedisplay();
		break;
	case 's':
		Xangle += 5.0;
		if (Xangle < -90.0)
			Xangle = -90.0;
		if (Xangle > 0.0)
			Xangle = 0.0;
		glutPostRedisplay();
		break;
	case 'a':
		Yangle += 5.0;
		if (Yangle > 360.0)
			Yangle -= 360.0;
		glutPostRedisplay();
		break;
	case 'd':
		Yangle -= 5.0;
		if (Yangle < 0.0)
			Yangle += 360.0;
		glutPostRedisplay();
		break;
	case 'q':
		Zangle += 5.0;
		if (Zangle > 360.0)
			Zangle -= 360.0;
		glutPostRedisplay();
		break;
	case 'e':
		Zangle -= 5.0;
		if (Zangle < 0.0)
			Zangle += 360.0;
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

/* Callback routine for non-ASCII key entry. */
void specialKeyInput(int key, int x, int y) {
	if (key == GLUT_KEY_DOWN)
		animationPeriod += 5;
	if (key == GLUT_KEY_UP)
	if (animationPeriod > 5)
		animationPeriod -= 5;
	glutPostRedisplay();
}

/* Routine to output interaction instructions to the C++ window. */
void printInteraction(void) {
	cout << "Interaction:" << endl;
	cout
		<< "Click to hit the ball, your position and distance from the ball affect the speed and direction it travels in."
		<< endl
		<< "Press the up/down arrow keys to speed up/slow down animation."
		<< endl
		<< "Press the w, a, s, d, q, and r keys to rotate the scene."
		<< endl;
}

/* Setup OpenGL. */
void setup(void) {
	printInteraction();

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 300);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("CSC350_Assignment_5.cpp");

	glClearColor(1.0, 1.0, 1.0, 0.0);
	glEnable(GL_DEPTH);
	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);
	glutSpecialFunc(specialKeyInput);
	glutMouseFunc(mouseInput);
	glutTimerFunc(5, animate, 1);
}

/* Main routine. */
int main(int argc, char **argv) {
	glutInit(&argc, argv);
	setup();
	glutMainLoop();

	return 0;
}
