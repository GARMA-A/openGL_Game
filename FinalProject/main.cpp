#include <GL/freeglut_std.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

struct Bullet {
  float x, y;
  bool active;
};

const float serverLeft = 0.7f;
const float serverRight = 1.0f;
const float hackerLeft = -1.0f;
const float hackerRight = -0.7f;

float lastZombieEntryTime = 0.0f;
float dangerWindowRemaining = 0.0f;

int points = 0;
bool serverUpgraded = false;
std::vector<float> zombieEntryTimes;
float upgradeEndTime = 0.0f;
const float upgradeCost = 20;
const float upgradeDuration = 30.0f;

struct Zombie {
  float x, y;
  bool isAlive;
  int health;
};

std::vector<Bullet> bullets;
std::vector<Zombie> zombies;

float zombieBaseSpeed = 0.002f;
float playerY = 0.0f;
bool keyUpPressed = false;
bool keyDownPressed = false;

const int screenWidth = 1200;
const int screenHeight = 900;

bool gameOver = false;
int level = 1;
int zombiesKilled = 0;
int maxLevel = 20;

void checkServerBreach() {
  if (upgradeEndTime == 0.0f) {
    gameOver = true;
  } else {
    zombieEntryTimes.push_back(glutGet(GLUT_ELAPSED_TIME) / 1000.0f);

    // Check if 2 entries within 2 seconds
    if (zombieEntryTimes.size() >= 3) {
      float timeDiff = zombieEntryTimes.back() -
                       zombieEntryTimes[zombieEntryTimes.size() - 2];
      if (timeDiff < 2.0f) {
        gameOver = true;
      }
    }
  }
}

void purchaseUpgrade() {
  if (points >= upgradeCost) {
    points -= upgradeCost;
    upgradeEndTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f + upgradeDuration;
  }
}

void drawSquare(float x, float y, float width, float height, float r, float g,
                float b, float a = 1) {
  glColor4f(r, g, b, a);
  glBegin(GL_POLYGON);
  glVertex2f(x, y);
  glVertex2f(x + width, y);
  glVertex2f(x + width, y + height);
  glVertex2f(x, y + height);
  glEnd();
}
void drawZombie() {
  drawSquare(-0.12f, 0.15f, 0.24f, 0.24f, 0.0f, 0.8f, 0.0f);
  drawSquare(-0.09f, 0.27f, 0.04f, 0.04f, 0.0f, 0.0f, 0.0f);
  drawSquare(0.05f, 0.27f, 0.04f, 0.04f, 0.0f, 0.0f, 0.0f);
  drawSquare(-0.15f, -0.05f, 0.3f, 0.25f, 0.0f, 0.7f, 1.0f);
  drawSquare(-0.27f, -0.05f, 0.1f, 0.25f, 0.0f, 0.8f, 0.0f);
  drawSquare(0.17f, -0.05f, 0.1f, 0.25f, 0.0f, 0.8f, 0.0f);
  drawSquare(-0.10f, -0.30f, 0.07f, 0.20f, 0.1f, 0.0f, 0.6f);
  drawSquare(0.03f, -0.30f, 0.07f, 0.20f, 0.1f, 0.0f, 0.6f);
}

void drawZones() {
  drawSquare(serverLeft, -1.0f, serverRight - serverLeft, 2.0f, 0.3f, 1.0f,
             0.3f, 0.1f);

  drawSquare(hackerLeft, -1.0f, hackerRight - hackerLeft, 2.0f, 1.0f, 0.3f,
             0.3f, 0.1f);
}

void drawCircle(float cx, float cy, float r) {
  const int numSegments = 360;
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(cx, cy);
  for (int i = 0; i <= numSegments; i++) {
    float theta = 2.0f * M_PI * float(i) / float(numSegments);
    float x = r * cosf(theta);
    float y = r * sinf(theta);
    glVertex2f(cx + x, cy + y);
  }
  glEnd();
}

void drawStickman() {
  // Head
  glColor3f(1.0f, 1.0f, 1.0f);
  drawCircle(0.0f, 0.8f, 0.1f);

  // Body
  glColor3f(1.0f, 1.0f, 1.0f);
  glLineWidth(3.0f);
  glBegin(GL_LINES);
  // Body line
  glVertex2f(0.0f, 0.7f);
  glVertex2f(0.0f, 0.3f);
  // Left arm
  glVertex2f(0.0f, 0.6f);
  glVertex2f(-0.2f, 0.4f);
  // Right arm
  glVertex2f(0.0f, 0.6f);
  glVertex2f(0.2f, 0.4f);
  // Left leg
  glVertex2f(0.0f, 0.3f);
  glVertex2f(-0.2f, 0.0f);
  // Right leg
  glVertex2f(0.0f, 0.3f);
  glVertex2f(0.2f, 0.0f);
  glEnd();

  // Face
  glColor3f(0.0f, 0.0f, 0.0f);
  glPointSize(5.0f);
  glBegin(GL_POINTS);
  glVertex2f(-0.03f, 0.83f);
  glVertex2f(0.03f, 0.83f);
  glEnd();

  // Mouth
  glLineWidth(3.0f);
  glBegin(GL_LINE_STRIP);
  for (float angle = M_PI / 4; angle <= 3 * M_PI / 4; angle += 0.025f) {
    float x = -0.08f * cos(angle);
    float y = 0.75f + 0.05f * sin(angle);
    glVertex2f(x, y);
  }
  glEnd();

  // Hands
  drawCircle(-0.2f, 0.4f, 0.025f);
  drawCircle(0.2f, 0.4f, 0.025f);
}

void renderText(float x, float y, const std::string &text) {
  glColor3f(1, 1, 1);
  glRasterPos2f(x, y);
  for (char c : text) {
    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
  }
}

void loadZombies(int count) {
  zombies.clear();
  for (int i = 0; i < count; ++i) {
    Zombie z;
    z.x = -1.0f - i * 0.3f;
    z.y = ((rand() % 180) - 90) / 100.0f;
    z.isAlive = true;
    z.health = (level >= 3) ? 3 : 1;
    zombies.push_back(z);
  }
}

void resetGame() {
  zombies.clear();
  bullets.clear();
  playerY = 0.0f;
  level = 1;
  zombieBaseSpeed = 0.002f;
  zombiesKilled = 0;
  gameOver = false;
  points = 0;
  serverUpgraded = false;
  zombieEntryTimes.clear();
  upgradeEndTime = 0.0f;
  loadZombies(10);
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);
  drawZones();
  renderText(0.7f, 0.9f, "Points: " + std::to_string(points));
  if (upgradeEndTime > 0.0f) {
    float remaining = upgradeEndTime - (glutGet(GLUT_ELAPSED_TIME) / 1000.0f);
    std::string timerText = "Upgrade: " + std::to_string((int)remaining) + "s";
    renderText(-0.2f, 0.8f, timerText);
  }
  if (upgradeEndTime > 0.0f) {
    drawSquare(serverLeft, -1.0f, serverRight - serverLeft, 2.0f, 0.2f, 0.8f,
               0.2f, 0.5f);
  } else {
    drawSquare(serverLeft, -1.0f, serverRight - serverLeft, 2.0f, 0.0f, 0.3f,
               0.0f, 0.1f);
  }

  if (gameOver) {
    if (level == maxLevel) {
      renderText(-0.3f, 0.0f, "Good Job, you win! Press SPACE to Restart!");
    } else {
      renderText(-0.3f, 0.0f, "GG YOU LOSE - Press SPACE to Restart!");
    }
    if (serverUpgraded) {
      renderText(-0.2f, 0.8f, "SERVER UPGRADED!");
    }

    glutSwapBuffers();
    return;
  } else {
    if (points >= upgradeCost && upgradeEndTime <= 0.0f) {
      renderText(-0.45f, 0.85f,
                 "UPGRADE AVAILABLE! Press U to strengthen server (20 coins)");
    }
  }

  // Draw stickman player
  glPushMatrix();
  glTranslatef(0.95f, playerY, 0.0f);
  glScalef(0.2f, 0.2f, 1.0f);
  drawStickman();
  glPopMatrix();

  // Draw zombies
  for (const auto &zombie : zombies) {
    if (zombie.isAlive) {
      glPushMatrix();
      glTranslatef(zombie.x, zombie.y, 0.0f);
      glScalef(0.2f, 0.2f, 1.0f);
      drawZombie();
      glPopMatrix();
    }
  }

  // Draw bullets
  for (const auto &bullet : bullets) {
    if (bullet.active)
      drawSquare(bullet.x, bullet.y, 0.05f, 0.05f, 1.0f, 1.0f, 1.0f);
  }

  renderText(-0.9f, 0.9f, "Level: " + std::to_string(level));
  glutSwapBuffers();
}

// Rest of the functions remain unchanged...
// ... (keyboard, specialDown, specialUp, init, main)
void update(int value) {
  if (gameOver)
    return;

  if (keyUpPressed)
    playerY += 0.015f;
  if (keyDownPressed)
    playerY -= 0.015f;
  if (playerY > 0.9f)
    playerY = 0.9f;
  if (playerY < -0.95f)
    playerY = -0.95f;

  float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
  if (currentTime > upgradeEndTime) {
    upgradeEndTime = 0.0f;
  }

  if (level > 1) {
    zombieBaseSpeed = 0.002f + (level - 1) * 0.0005f;
  }
  for (auto &zombie : zombies) {
    if (zombie.isAlive) {
      // Movement logic
      if (zombie.x < hackerRight) {
        zombie.x += zombieBaseSpeed * 2; // Faster in hacker zone
      } else {
        zombie.x += zombieBaseSpeed;
      }

      // Game over condition
      if (zombie.x >= serverLeft) {
        checkServerBreach();
        zombie.isAlive = false;
      }
    }
  }
  for (auto &bullet : bullets) {
    if (bullet.active) {
      bullet.x -= 0.02f;

      // Check collisions
      for (auto &zombie : zombies) {
        if (zombie.isAlive) {
          float zxLeft = zombie.x - 0.12f * 0.2f;
          float zxRight = zombie.x + 0.12f * 0.2f;
          float zyTop = zombie.y + (0.15f + 0.24f) * 0.2f;
          float zyBottom = zombie.y - 0.3f * 0.2f;

          float bx = bullet.x + 0.025f;
          float by = bullet.y + 0.025f;

          if (bx >= zxLeft && bx <= zxRight && by >= zyBottom && by <= zyTop) {
            bullet.active = false;
            zombie.health--;

            if (zombie.health <= 0) {
              zombie.isAlive = false;
              zombiesKilled++;
              points++;
            }
            break; // Bullet can only hit one zombie
          }
        }
      }

      // Remove off-screen bullets
      if (bullet.x < -1.1f)
        bullet.active = false;
    }
  }
  bool allDead = true;
  for (const auto &zombie : zombies) {
    if (zombie.isAlive) {
      allDead = false;
      break;
    }
  }
  if (allDead) {
    if (level < maxLevel) {
      level++;
      zombiesKilled = 0;
      loadZombies(6 + level); // Increase zombie count per level
    } else {
      gameOver = true;
    }
  }
  glutPostRedisplay();
  glutTimerFunc(16, update, 0);
}

void keyboard(unsigned char key, int x, int y) {
  if (key == 27 && gameOver) {
    resetGame();
  } else if (key == 32 && !gameOver) {

    // if (bullets.empty() || !bullets.back().active) {
    Bullet b;
    b.x = 0.95f;
    b.y = playerY;
    b.active = true;
    bullets.push_back(b);
    // }
  } else if (key == 'u' || key == 'U' && !gameOver) {
    purchaseUpgrade();
  } else if (key == 27) {
    exit(0);
  }
}

void specialDown(int key, int x, int y) {
  if (key == GLUT_KEY_UP)
    keyUpPressed = true;
  else if (key == GLUT_KEY_DOWN)
    keyDownPressed = true;
}

void specialUp(int key, int x, int y) {
  if (key == GLUT_KEY_UP)
    keyUpPressed = false;
  else if (key == GLUT_KEY_DOWN)
    keyDownPressed = false;
}

void init() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glMatrixMode(GL_PROJECTION);
  gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  srand(static_cast<unsigned>(time(0)));

  loadZombies(10);
}

int main(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(screenWidth, screenHeight);
  glutCreateWindow("Zombie Shooter");
  glutFullScreen();

  init();
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(specialDown);
  glutSpecialUpFunc(specialUp);
  glutTimerFunc(25, update, 0);
  glutMainLoop();

  return 0;
}
