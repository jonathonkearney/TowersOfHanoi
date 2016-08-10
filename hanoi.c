#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>


GLenum errorCheck ()
{
    GLenum code = glGetError ();
    if (code != GL_NO_ERROR)
        fprintf(stderr, "OpenGL error %d: %s\n", code, gluErrorString (code) );
    return code;
}

void createCenterWindow(const int sizeX, const int sizeY, const char* title)
{
    glutInitWindowPosition(
        (glutGet(GLUT_SCREEN_WIDTH) - sizeX) / 2,
        (glutGet(GLUT_SCREEN_HEIGHT) - sizeY) / 2
    );
    glutInitWindowSize (sizeX, sizeY);
    glutCreateWindow (title);
}

GLfloat converge(GLfloat current, GLfloat target, GLfloat delta)
{
    if (current + delta < target)
        return current + delta;
    if (current - delta > target)
        return current - delta;
    return target;
}

// data structures

struct tower
{
    GLfloat pos[3];
};
typedef struct tower TOWER;

struct ring
{
    GLfloat color[3];
    GLfloat pos[3];
    double size;
};
typedef struct ring RING;

TOWER towers[3] = {
    { {-3,0,0} },
    { {0,0,0} },
    { {3,0,0} }
};

#define MAX_RING 5
#define RING_WIDTH 0.25
RING rings[MAX_RING] = {
    { {1,0,0}, {-3,RING_WIDTH,0}, 1 },
	{ {1,1,0}, {-3,RING_WIDTH+0.5,0}, 0.8},
	{ {0,1,0}, {-3,RING_WIDTH+1.0,0}, 0.7 },
	{ {0,1,1}, {-3,RING_WIDTH+1.5,0}, 0.6 },
	{ {0,0,1}, {-3,RING_WIDTH+2.0,0}, 0.5 }
};

// set to ring being animated; afterwards reset to NULL again
RING *animated = NULL;
GLfloat destination[3] = {0,0,0};

RING* getTopRing(int tower)
{
    int i;
    RING *top = NULL;
    for (i = 0; i < MAX_RING; i++)
        if ( rings[i].pos[0] == towers[tower].pos[0] && (top == NULL || top->pos[1] < rings[i].pos[1]) )
            top = &rings[i];
    return top;
}

// draw a single cube
void drawRing(RING *ring)
{
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ring->color);
	glPushMatrix();
	glTranslatef(ring->pos[0],ring->pos[1],ring->pos[2]);
	glRotatef(-90, 1, 0, 0);
    glutSolidTorus(RING_WIDTH, ring->size, 20, 20);
	glPopMatrix();
}

void drawTower(TOWER *tower)
{
	GLfloat coneColor[] = { 0.7, 0.7, 0.7, 1 };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, coneColor);
    GLUquadricObj *quadratic;
	glPushMatrix();
	glTranslatef(tower->pos[0],tower->pos[1],tower->pos[2]);
	glRotatef(-90, 1, 0, 0);
    quadratic = gluNewQuadric();
    gluCylinder(quadratic,0.4f,0.0f,3.0f,20,20);
	glPopMatrix();
	
	//was (quadratic,0.4f,0.0f,3.0f,20,20);
	
}

void drawFloor()
{
	GLfloat floorColor[] = { 0.5, 0.5, 1, 1 };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, floorColor);

	GLfloat zFar = -5, zNear = 1, xLeft = -6, xRight = 4;  
    glBegin(GL_POLYGON);
        glVertex3f(xLeft, -1, zFar);
        glVertex3f(xLeft, -1, zNear);
        glVertex3f(xRight, -1, zNear);
        glVertex3f(xRight, -1, zFar);
    glEnd();
}

// draw all objects (cubes & plane)
void draw ()
{
    int i;
    
    // clear the screen
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // draw all tower, rings and floor
    for (i = 0; i < 3; i++)
        drawTower( &towers[i] );

    for (i = 0; i < MAX_RING; i++)
        drawRing( &rings[i] );

    drawFloor();
    
    // draw the scene
    glFlush ();
    // change to new buffer (double buffering)
    glutSwapBuffers();
    // debug
    errorCheck();    
}

void update ()
{	
    const GLfloat deltaPos = 0.005;
    const GLfloat yTransfer = 3.5;
    
    if (animated == NULL)
        return;

    // update the positions of ring being animated - TODO
    if (animated->pos[0] == destination[0])
    {
        animated->pos[1] = converge(animated->pos[1], destination[1], deltaPos);
        if (animated->pos[1] == destination[1])
            animated = NULL;
    }
    else if (animated->pos[1] == yTransfer)
    {
        animated->pos[0] = converge(animated->pos[0], destination[0], deltaPos);
    }
    else
    {
        animated->pos[1] = converge(animated->pos[1], yTransfer, deltaPos);
    }
    
    // redraw
    glutPostRedisplay();
}

void keyHandler(GLubyte key, GLint xMouse, GLint yMouse)
{
    // remember last tower selection
    static int firstTower = -1;
    int tower;
    
    // ignore input while animation is running
    if (animated != NULL)
        return;
    
    switch(key)
    {
        case '1':
            tower = 0;
            break;
        case '2':
            tower = 1;
            break;
        case '3':
            tower = 2;
            break;
        default:
            return;
    }
    
    // source tower selection?
    if (firstTower < 0)
    {
        // valid choice? must have ring
        if ( getTopRing(tower) != NULL )
            firstTower = tower;
    }
    else
    {
        RING *first = getTopRing(firstTower);
        RING *second = getTopRing(tower);
        // valid choice? top ring at target tower must be larger, if any
        if ( second == NULL || second->size > first->size )
        {
            // start animation
            destination[0] = towers[tower].pos[0];
            destination[1] = (second == NULL ? -RING_WIDTH : second->pos[1]) + 2*RING_WIDTH;
            animated = first;
        }
        firstTower = -1;
    }
}


int main (int argc, char** argv)
{
    // create window
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    createCenterWindow(800, 600, "Towers of Hanoi");
    
    // version info - only accessible after window has been created
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));

    // enable culling, depth-testing and lighting
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    // set the view perspective - TODO
	glMatrixMode (GL_PROJECTION);
    gluPerspective( 30, 4.0/3, 0.5, 100 );
    glMatrixMode (GL_MODELVIEW);
    gluLookAt( 	5,8,12,
				0,0,0, 
				0,1,0 );
    
    // light source
    GLfloat lightPos[] = { 1, 2, 1, 0 };
    GLfloat lightColor[] = { 1, 1, 1, 1 };    
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor);
    glEnable(GL_LIGHT1);    
      
    // setup for animation
    glutDisplayFunc(draw);    
    glutIdleFunc(update);
    glutKeyboardFunc(keyHandler);
    glutMainLoop( );
}