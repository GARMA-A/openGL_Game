#include "stick_man.cpp"
#include <GL/glut.h>

int test(int argc = 0, char **argv = 0) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(500, 500);
  glutCreateWindow("Girgis Emad Stick Man charachter");
  init();
  glutDisplayFunc(display);
  glutTimerFunc(0, timer, 0);
  glutMainLoop();
}
