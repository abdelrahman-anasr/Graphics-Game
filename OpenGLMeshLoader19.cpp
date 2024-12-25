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

#define GLUT_KEY_ESCAPE 27

#define M_PI 3.14159265358979323846

#include <vector>
#include <chrono>


// Timer variables
float elapsedTime = 0.0f;
float deltaTime;

struct Point3D {
    float x, y, z;
};



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


//Player position
double playerx = 0.0;
double playery = 5.0; // Player height (important)
double playerz = 0.0;
double playerRotation = 0.0;


// Camera class
class Camera {
public:
    Vector3f eye, center, up, cameraFront;

    Camera() {
        eye = Vector3f(0.0f, 0.0f, 3.0f);
        center = Vector3f(0.0f, 0.0f, 0.0f);
        up = Vector3f(0.0f, 1.0f, 0.0f);
        cameraFront = Vector3f(0.0f, 0.0f, -1.0f);
    }

    // Sets the camera view using gluLookAt
    void look() {
        gluLookAt(
            eye.x, eye.y, eye.z,
            eye.x + cameraFront.x, eye.y + cameraFront.y, eye.z + cameraFront.z,
            up.x, up.y, up.z
        );
    }
};

// Intialize camera + Model + Textures
Camera camera;
GLuint tex_skybox;
GLTexture tex_ground;
Model_3DS model_player;

float radians(float degrees) {
    return degrees * M_PI / 180.0f;
}

// Variables for camera rotation
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 640, lastY = 360;
bool firstMouse = true;
Vector3f cameraFront;


// Helper Functions

// Calculate delta time for smooth frame rate
void calculateDeltaTime() {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = now - startTime;
    startTime = now;
    deltaTime = duration.count();
}

// Calculate angle between two points
float calculateAngle(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    return atan2(dy, sqrt(dx * dx + dz * dz)) * (180.0f / M_PI);
}



// Setups Functions

// Setup lights
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

// Setup camera
void setupCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 1280.0f / 720.0f, 0.1, 1000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera.look();
}



// Draw Functions

// Draw ground
void drawGround() {
    glEnable(GL_LIGHTING);

    glColor3f(1, 1, 1);
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);

    glPushMatrix();
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glTexCoord2f(0, 0);
    glVertex3f(-500, 0, -500);
    glTexCoord2f(10, 0);
    glVertex3f(500, 0, -500);
    glTexCoord2f(10, 10);
    glVertex3f(500, 0, 500);
    glTexCoord2f(0, 10);
    glVertex3f(-500, 0, 500);
    glEnd();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

// Draw skybox
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



// Player Movement

// Movement speed + sensitivity
const float moveSpeed = 0.1f;
const float sensitivity = 0.05f;

// Keyboard input
bool keyStates[256] = { false };

// Keyboard input for movement
void Keyboard(unsigned char key, int x, int y) {
    keyStates[key] = true;
    if (key == GLUT_KEY_ESCAPE) {
        exit(0);
    }
}

// Keyboard hold input for smooth movement
void KeyboardUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

// Mouse input for camera rotation
void Motion(int x, int y) {
    int windowWidth = 1280;
    int windowHeight = 720;
    int centerX = windowWidth / 2;
    int centerY = windowHeight / 2;

    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    float xoffset = x - lastX;
    float yoffset = lastY - y;
    lastX = x;
    lastY = y;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    camera.cameraFront.x = cos(radians(yaw)) * cos(radians(pitch));
    camera.cameraFront.y = sin(radians(pitch));
    camera.cameraFront.z = sin(radians(yaw)) * cos(radians(pitch));
    camera.cameraFront = camera.cameraFront.unit();

    camera.look();
    glutPostRedisplay();

    glutWarpPointer(centerX, centerY);
    lastX = centerX;
    lastY = centerY;
}

// Update player position
void updatePlayerPosition() {
    Vector3f front(camera.cameraFront.x, 0.0f, camera.cameraFront.z);
    front = front.unit();
    Vector3f right = front.cross(camera.up).unit();

    if (keyStates['w']) {
        playerx += front.x * moveSpeed;
        playerz += front.z * moveSpeed;
    }
    if (keyStates['s']) {
        playerx -= front.x * moveSpeed;
        playerz -= front.z * moveSpeed;
    }
    if (keyStates['a']) {
        playerx -= right.x * moveSpeed;
        playerz -= right.z * moveSpeed;
    }
    if (keyStates['d']) {
        playerx += right.x * moveSpeed;
        playerz += right.z * moveSpeed;
    }

    camera.eye.x = playerx;
    camera.eye.y = playery + 1.8f;
    camera.eye.z = playerz;

    camera.look();
}

// Update camera
void updateCamera() {

    cameraFront.x = cos(radians(yaw)) * cos(radians(pitch));
    cameraFront.y = sin(radians(pitch));
    cameraFront.z = sin(radians(yaw)) * cos(radians(pitch));
    cameraFront = cameraFront.unit();

    float distanceFromPlayer = 5.0f;
    camera.eye.x = playerx - cameraFront.x * distanceFromPlayer;
    camera.eye.y = playery + 5.0f;
    camera.eye.z = playerz - cameraFront.z * distanceFromPlayer;

    camera.center.x = playerx;
    camera.center.y = playery + 5.0f;
    camera.center.z = playerz;

    camera.up = Vector3f(0.0f, 5.0f, 0.0f);
}



// Game Functions (loading/intializations + update/Idle)

// Load textures + models
void LoadAssets() {
    model_player.Load("Models/Male1_3ds/Male1.3ds");
    tex_ground.Load("Textures/ground.bmp");
    loadBMP(&tex_skybox, "Textures/moon.bmp", true);
}

// Update function (player movement)
void update(float deltaTime) {
    updatePlayerPosition();

    updateCamera();
}

// Idle function (calculate deltatime + update)
void idle() {
    calculateDeltaTime();
    update(deltaTime);

    glutPostRedisplay();
}

// Display function (draw everything + setup lights/camera)
void Display() {
    float deltaTime = 0.016f;
    updatePlayerPosition();

    setupCamera();
    setupLights();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawGround();
    drawSkybox();

    glutSwapBuffers();
}

// Initialize OpenGL settings
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

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 100);

    glutCreateWindow("First Person Shooter");

    glutDisplayFunc(Display);
    glutIdleFunc(idle);
    glutKeyboardFunc(Keyboard);
    glutKeyboardUpFunc(KeyboardUp);
    glutPassiveMotionFunc(Motion);

    LoadAssets();
    Init();
    updateCamera();

    glutMainLoop();

    return 0;
}