#include <GL/glut.h>
#include <cmath>

const float STICKMAN_HEIGHT = 0.9f;
const float STICKMAN_WIDTH = 0.4f;
const float MARGIN_X = 0.2f;
const float MARGIN_Y = 1.0f - STICKMAN_HEIGHT; // 0.1f

float xPos = -1.0f + MARGIN_X;
float yPos = -1.0f;
float speed = 0.008f;
enum MovementState { UP, RIGHT_DOWN, RIGHT_UP, DOWN, LEFT };
MovementState state = UP;

void init() {
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}

void drawCircle(float cx, float cy, float r) {
  glColor3f(1.0f, 1.0f, 1.0f);
  glBegin(GL_POLYGON);
  for (int i = 0; i < 100; ++i) {
    float theta = 2.0f * M_PI * float(i) / 100.0f;
    glVertex2f(cx + r * cosf(theta), cy + r * sinf(theta));
  }
  glEnd();

  glLineWidth(3.0f);
  glBegin(GL_LINE_LOOP);
  for (int i = 0; i < 100; ++i) {
    float theta = 2.0f * M_PI * float(i) / 100.0f;
    glVertex2f(cx + r * cosf(theta), cy + r * sinf(theta));
  }
  glEnd();
}

void drawStickman() {
  // Head (centered at 0.8y from feet)
  drawCircle(0.0f, 0.8f, 0.1f);

  // Body parts
  glColor3f(1.0f, 1.0f, 1.0f);
  glLineWidth(3.0f);
  glBegin(GL_LINES);
  glVertex2f(0.0f, 0.7f);
  glVertex2f(0.0f, 0.3f); // Body
  glVertex2f(0.0f, 0.6f);
  glVertex2f(-0.2f, 0.5f); // Left arm
  glVertex2f(0.0f, 0.6f);
  glVertex2f(0.2f, 0.5f); // Right arm
  glVertex2f(0.0f, 0.3f);
  glVertex2f(-0.2f, 0.0f); // Left leg
  glVertex2f(0.0f, 0.3f);
  glVertex2f(0.2f, 0.0f); // Right leg
  glEnd();

  // Eyes
  glColor3f(0.0f, 0.0f, 0.0f);
  glPointSize(5.0f);
  glBegin(GL_POINTS);
  glVertex2f(-0.03f, 0.83f);
  glVertex2f(0.03f, 0.83f);

  glColor3f(0.0f, 0.0f, 0.0f);
  glLineWidth(2.0f);
  glPushMatrix();

  glTranslatef(0.0f, 0.8f, 0.0f);
  glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
  glBegin(GL_LINE_STRIP);

  for (float angle = -M_PI / 4; angle <= M_PI / 4; angle += 0.05f) {
    float x = 0.06f * cos(angle);
    float y = 0.04f * sin(angle) - 0.05f;
    glVertex2f(x, y);
  }

  glEnd();

  drawCircle(-0.2f, 0.5f, 0.025f);
  drawCircle(0.2f, 0.5f, 0.025f);
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);

  glPushMatrix();
  glTranslatef(xPos, yPos, 0.0f);
  drawStickman();
  glPopMatrix();

  glFlush();
}

void timer(int value) {
  const float diagonalSpeed = speed * 0.7071f;

  switch (state) {
  case UP:
    yPos += speed;
    if (yPos >= 1.0f - STICKMAN_HEIGHT) {
      state = RIGHT_DOWN;
    }
    break;

  case RIGHT_DOWN:
    xPos += diagonalSpeed;
    yPos -= diagonalSpeed;
    if (xPos >= 0.0f && yPos <= -1.0f) {
      state = RIGHT_UP;
    }
    break;

  case RIGHT_UP:
    xPos += diagonalSpeed;
    yPos += diagonalSpeed;
    if (xPos >= 1.0f - MARGIN_X && yPos >= 1.0f - STICKMAN_HEIGHT) {
      state = DOWN;
    }
    break;

  case DOWN: // Vertical down (right side)
    yPos -= speed;
    if (yPos <= -1.0f) { // Feet reach bottom
      state = LEFT;
    }
    break;

  case LEFT: // Horizontal left
    xPos -= speed;
    if (xPos <= -1.0f + MARGIN_X) {
      state = UP;
    }
    break;
  }

  xPos = fmax(fmin(xPos, 1.0f - MARGIN_X), -1.0f + MARGIN_X);
  yPos = fmax(fmin(yPos, 1.0f - STICKMAN_HEIGHT), -1.0f);

  glutPostRedisplay();
  glutTimerFunc(16, timer, 0);
}

int main(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(500, 500);
  glutCreateWindow("Girgis Emad Stick Man charachter");
  init();
  glutDisplayFunc(display);
  glutTimerFunc(0, timer, 0);
  glutMainLoop();
  return 0;
}
