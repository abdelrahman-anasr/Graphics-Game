#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <string.h>
#include <iostream>

#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"

#include <glut.h>


//Testing Change as Abdulrahman
#define GLUT_KEY_ESCAPE 27

#define M_PI 3.14159265358979323846

class Vector3f {
public:
    float x, y, z;

    Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
        x = _x;
        y = _y;
        z = _z;
    }

    Vector3f operator+(const Vector3f& v) const {
        return Vector3f(x + v.x, y + v.y, z + v.z);
    }

    Vector3f operator-(const Vector3f& v) const {
        return Vector3f(x - v.x, y - v.y, z - v.z);
    }

    Vector3f operator*(float n) const {
        return Vector3f(x * n, y * n, z * n);
    }

    Vector3f operator/(float n) const {
        return Vector3f(x / n, y / n, z / n);
    }

    Vector3f unit() const {
        float mag = sqrt(x * x + y * y + z * z);
        return *this / mag;
    }

    Vector3f cross(const Vector3f& v) const {
        return Vector3f(y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x);
    }
};



class Camera {
public:
    Vector3f eye, center, up;

    Camera() {
        eye = Vector3f(0.0f, 1.8f, 5.0f);
        center = Vector3f(0.0f, 1.8f, 0.0f);
        up = Vector3f(0.0f, 1.0f, 0.0f);
    }

    void look() {
        gluLookAt(
            eye.x, eye.y, eye.z,
            center.x, center.y, center.z,
            up.x, up.y, up.z
        );
    }
};

Camera camera;

GLuint tex_skybox;
GLTexture tex_ground;
Model_3DS model_player;

float radians(float degrees) {
    return degrees * M_PI / 180.0f;
}

float yaw = -90.0f;   
float pitch = 0.0f;
float lastX = 640, lastY = 360; 
bool firstMouse = true;
Vector3f cameraFront; 

void setupLights() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat position[] = { 0.0f, 10.0f, 0.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
}

void setupCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 1280.0f / 720.0f, 0.1, 1000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera.look();
}

void updateCamera() {
    cameraFront.x = cos(radians(yaw)) * cos(radians(pitch));
    cameraFront.y = sin(radians(pitch));
    cameraFront.z = sin(radians(yaw)) * cos(radians(pitch));
    cameraFront = cameraFront.unit();

    float distanceFromPlayer = 5.0f; 
    camera.eye.x = playerx - cameraFront.x * distanceFromPlayer;
    camera.eye.y = playery + 2.0f; 
    camera.eye.z = playerz - cameraFront.z * distanceFromPlayer;

    camera.center.x = playerx;
    camera.center.y = playery + 1.0f;
    camera.center.z = playerz;

    camera.up = Vector3f(0.0f, 1.0f, 0.0f);
}

void drawGround() {
    glEnable(GL_LIGHTING);

    glColor3f(1, 1, 1);
    glEnable(GL_TEXTURE_2D);  

    glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]); 

    glPushMatrix();
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0); 
    glTexCoord2f(0, 0);
    glVertex3f(-50, 0, -50);
    glTexCoord2f(10, 0);
    glVertex3f(50, 0, -50);
    glTexCoord2f(10, 10);
    glVertex3f(50, 0, 50);
    glTexCoord2f(0, 10);
    glVertex3f(-50, 0, 50);
    glEnd();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

void drawSkybox() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex_skybox);

    glPushMatrix();
    glColor3f(1, 1, 1);
    GLUquadricObj* qobj = gluNewQuadric();
    gluQuadricTexture(qobj, true);
    gluQuadricNormals(qobj, GL_SMOOTH);
    gluSphere(qobj, 500, 100, 100);
    gluDeleteQuadric(qobj);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

void drawModel() {
    glPushMatrix();
    glTranslated(playerx, playery, playerz);
    glRotated(-yaw + 90.0f, 0, 1, 0);
    glRotated( 90.0f, 1,0, 0); 

    glScaled(0.025, 0.025, 0.025);
    model_player.Draw();
    glPopMatrix();
}

void Display() {
    setupCamera();
    setupLights();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawGround();
    drawSkybox();
    drawModel();

    glutSwapBuffers();
}

void Keyboard(unsigned char key, int x, int y) {
    float moveSpeed = 0.2f;
    Vector3f front(cameraFront.x, 0.0f, cameraFront.z);
    front = front.unit();
    Vector3f right = front.cross(camera.up).unit();

    switch (key) {
    case 'w':
        playerx += front.x * moveSpeed;
        playerz += front.z * moveSpeed;
        break;
    case 's':
        playerx -= front.x * moveSpeed;
        playerz -= front.z * moveSpeed;
        break;
    case 'a':
        playerx -= right.x * moveSpeed;
        playerz -= right.z * moveSpeed;
        break;
    case 'd':
        playerx += right.x * moveSpeed;
        playerz += right.z * moveSpeed;
        break;
    case GLUT_KEY_ESCAPE:
        exit(0);
        break;
    }
    updateCamera();
    glutPostRedisplay();
}

void Motion(int x, int y) {
    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    float xoffset = x - lastX;
    float yoffset = lastY - y; 
    lastX = x;
    lastY = y;

    float sensitivity = 0.3f; 
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 45.0f)
        pitch = 45.0f;
    if (pitch < -45.0f)
        pitch = -45.0f;

    updateCamera();
    glutPostRedisplay();
}

void LoadAssets() {
    model_player.Load("Models/Male1_3ds/Male1.3ds");
    tex_ground.Load("Textures/ground.bmp");
    loadBMP(&tex_skybox, "Textures/sky.bmp", true);
}

void Init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    glShadeModel(GL_SMOOTH);

    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);

    glutSetCursor(GLUT_CURSOR_NONE);
}

void main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 100);

    glutCreateWindow("Escape The Island");

    glutDisplayFunc(Display);
    glutIdleFunc(Display);
    glutKeyboardFunc(Keyboard);
    glutPassiveMotionFunc(Motion);

    LoadAssets();
    Init();
    updateCamera();

    glutMainLoop();
}
