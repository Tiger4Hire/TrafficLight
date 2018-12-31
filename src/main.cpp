#include "standard.h"
#include "Agent.h"
#include "TrafficLights.h"
#include "glu_objs.h"

#include <GL/glut.h>

#include <iostream>
#include <stdlib.h>
using namespace std;

static GLfloat speed = 0.0;
float height;
float width;
float depth;
int i;
void box(float lngh, float width, float depth);
void Update();
void Render();

TrafficLight lights;
TrafficLightController light_controller(lights);

void init(void)
{
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat mat_shininess[] = {50.0};
    GLfloat light_position[] = {5.0, 1.0, 5.0, 0.0};
    GLfloat mat_amb_diff_color_red[] = {1.0, 0.5, 0.0, 0.5};
    GLfloat mat_amb_diff_color_green[] = {0.0, 1.0, 0.0, 0.5};
    GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat light_ambient[] = {0.15, 0.15, 0.15, 0.15};
    GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff_color_green);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
}

void display(void)
{
    // glClear (GL_COLOR_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glColor3f (1.0, 1.0, 1.0);
    glLoadIdentity(); /* clear the matrix */
                      /* viewing transformation  */
    // gluLookAt (0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glTranslatef(0.0, 0.0, -4.0);
    gluLookAt(0, 0, 10, 0, 0, 0, 0, 1, 0);
    // Fan_Physics();
    //    glRasterPos2f(-width / 2, -height / 2);
    lights.Render();
    glutSwapBuffers();
    glutPostRedisplay();
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);
    glMatrixMode(GL_MODELVIEW);
    width = w;
    height = h;
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case '\e':
            exit(0);
            break;
        case ' ':
            light_controller.Send(ButtonPress{});
            break;
    }
}
void Update()
{
    lights.Update();
}
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(640, 480);
    width = 640;
    height = 480;
    glutInitWindowPosition(0, 0);
    glutCreateWindow("press  SpaceBar to change state");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(Update);
    std::thread controller_runner([]() { light_controller.Run(); });
    glutMainLoop();
    light_controller.Send(QuitAgent{});
    controller_runner.join();
    return 0;
}

void box(float lngh, float width, float depth)
{
    float a = lngh;
    float b = width;
    float c = depth;

    glBegin(GL_QUADS);

    /* Top face of box*/

    glVertex3f(a, b, -c);   // Top right vertex (Top of cube)
    glVertex3f(-a, b, -c);  // Top left vertex (Top of cube)
    glVertex3f(-a, b, c);   // Bottom left vertex (Top of cube)
    glVertex3f(a, b, c);    // Bottom right vertex (Top of cube)

    // Bottom face of box

    glVertex3f(a, -b, -c);   // Top right vertex (Bottom of cube)
    glVertex3f(-a, -b, -c);  // Top left vertex (Bottom of cube)
    glVertex3f(-a, -b, c);   // Bottom left vertex (Bottom of cube)
    glVertex3f(a, -b, c);    // Bottom right vertex (Bottom of cube)
    glColor3f(1.0, 0.0, 0.0);
    // Front of box

    glVertex3f(a, b, c);    // Top right vertex (Front)
    glVertex3f(-a, b, c);   // Top left vertex (Front)
    glVertex3f(-a, -b, c);  // Bottom left vertex (Front)
    glVertex3f(a, -b, c);   // Bottom right vertex (Front)
    glColor3f(1.0, 0.0, 0.0);
    // Back of box

    glVertex3f(a, -b, -c);   // Bottom right vertex (Back)
    glVertex3f(-a, -b, -c);  // Bottom left vertex (Back)
    glVertex3f(-a, b, -c);   // top left vertex (Back)
    glVertex3f(a, b, -c);    // Top right vertex (Back)
    glColor3f(1.0, 0.0, 0.0);
    // Left of box

    glVertex3f(-a, b, c);    // Top right vertex (Left)
    glVertex3f(-a, b, -c);   // Top left vertex (Left)
    glVertex3f(-a, -b, -c);  // Bottom left vertex (Left)
    glVertex3f(-a, -b, c);   // Bottom vertex (Left)
    glColor3f(1.0, 0.0, 0.0);
    // Right of box

    glVertex3f(a, b, -c);   // Top right vertex (Right)
    glVertex3f(a, b, c);    // Top left vertex (Right)
    glVertex3f(a, -b, c);   // Bottom left vertex (Right)
    glVertex3f(a, -b, -c);  // Bottom right vertex (Right)
    glColor3f(1.0, 0.0, 0.0);
    // End drawing the box
    glEnd();
    // return TRUE;
}
