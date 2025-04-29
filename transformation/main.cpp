#include <GL/glut.h>
#include <cmath>
#include <cstring>

enum AnimationState { MOVING, ROTATING, SCALING, STOPPED };
AnimationState animState = MOVING;

// Animation variables
float xPosition = 0.0f;
const float MOVE_SPEED = 0.015f;
const float MAX_X = 0.7f;
bool movingLeft = true;

float rotationAngle = 0.0f;
const float ROTATION_SPEED = 2.0f;

float scaleX = 1.0f;
float scaleY = 1.0f;
const float SCALE_SPEED = 0.01f;
const float MAX_SCALE = 1.61f;

void init() {
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}

void transformVertex(float x, float y) {
  // Apply scaling first
  x *= scaleX;
  y *= scaleY;

  if (animState == ROTATING) {
    float centerY = 0.5 * scaleY;
    float translatedY = y - centerY;
    float rad = rotationAngle * M_PI / 180.0f;

    float newX = x * cos(rad) - translatedY * sin(rad);
    float newY = x * sin(rad) + translatedY * cos(rad);

    x = newX;
    y = newY + centerY;
  }

  x += xPosition;
  y -= 0.45f;

  glVertex2f(x, y);
}

void drawCircle(float cx, float cy, float r) {
  glColor3f(1.0f, 1.0f, 1.0f);
  glBegin(GL_POLYGON);
  for (int i = 0; i < 100; ++i) {
    float theta = 2.0f * M_PI * float(i) / 100.0f;
    float x = cx + r * cosf(theta);
    float y = cy + r * sinf(theta);
    transformVertex(x, y);
  }
  glEnd();

  glLineWidth(3.0f);
  glBegin(GL_LINE_LOOP);
  for (int i = 0; i < 100; ++i) {
    float theta = 2.0f * M_PI * float(i) / 100.0f;
    transformVertex(cx + r * cosf(theta), cy + r * sinf(theta));
  }
  glEnd();
}

void drawStickman() {
  // Head
  drawCircle(0.0f, 0.8f, 0.1f);

  // Body
  glColor3f(1.0f, 1.0f, 1.0f);
  glLineWidth(3.0f);
  glBegin(GL_LINES);
  transformVertex(0.0f, 0.7f);
  transformVertex(0.0f, 0.3f); // Body
  transformVertex(0.0f, 0.6f);
  transformVertex(-0.2f, 0.4f); // Left arm
  transformVertex(0.0f, 0.6f);
  transformVertex(0.2f, 0.4f); // Right arm
  transformVertex(0.0f, 0.3f);
  transformVertex(-0.2f, 0.0f); // Left leg
  transformVertex(0.0f, 0.3f);
  transformVertex(0.2f, 0.0f); // Right leg
  glEnd();

  // Face
  glColor3f(0.0f, 0.0f, 0.0f);
  glPointSize(5.0f);
  glBegin(GL_POINTS);
  transformVertex(-0.03f, 0.83f);
  transformVertex(0.03f, 0.83f);
  glEnd();

  glLineWidth(3.0f);
  glColor3f(0.0f, 0.0f, 0.0f);
  glBegin(GL_LINE_STRIP);
  for (float angle = M_PI / 4; angle <= 3 * M_PI / 4; angle += 0.025f) {
    float x = -0.08f * cos(angle);
    float y = 0.75f + 0.05f * sin(angle);
    transformVertex(x, y);
  }
  glEnd();

  // Hands
  drawCircle(-0.2f, 0.4f, 0.025f);
  drawCircle(0.2f, 0.4f, 0.025f);
}

void drawStateLabel() {
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  const char *label = "";
  switch (animState) {
  case MOVING:
    label = "MOVING";
    break;
  case ROTATING:
    label = "ROTATING";
    break;
  case SCALING:
    label = "SCALING";
    break;
  case STOPPED:
    label = "STOPPED";
    break;
  }

  glColor3f(1, 1, 1);
  glRasterPos2i(10, glutGet(GLUT_WINDOW_HEIGHT) - 30);
  for (const char *c = label; *c; c++)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

void updateAnimations() {
  switch (animState) {
  case MOVING:
    if (movingLeft && (xPosition -= MOVE_SPEED) < -MAX_X)
      movingLeft = false;
    else if (!movingLeft && (xPosition += MOVE_SPEED) > MAX_X) {
      animState = ROTATING;
      xPosition = 0;
    }
    break;

  case ROTATING:
    if ((rotationAngle += ROTATION_SPEED) >= 360) {
      rotationAngle = 0;
      animState = SCALING;
    }
    break;

  case SCALING:
    if (scaleX < MAX_SCALE) {
      scaleX += SCALE_SPEED;
      scaleY += SCALE_SPEED;
    } else
      animState = STOPPED;
    break;

  case STOPPED:
    break;
  }
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);
  drawStickman();
  drawStateLabel();
  glFlush();
}

void timer(int value) {
  if (animState != STOPPED) {
    updateAnimations();
    glutPostRedisplay();
  }
  glutTimerFunc(16, timer, 0);
}

int main(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitWindowSize(700, 700);
  glutCreateWindow("Girgis Emad Stick Man Character");
  init();
  glutDisplayFunc(display);
  glutTimerFunc(0, timer, 0);
  glutMainLoop();
  return 0;
}
