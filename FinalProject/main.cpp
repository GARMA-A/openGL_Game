#include <GL/freeglut_std.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

// Forward declarations for functions used before their definitions
void levelCountdownTimer(int value);
void loadZombies(int count);
void update(int value);

struct Bullet
{
  float x, y;
  bool active;
  int damage;
  float r, g, b;
};

struct Zombie
{
  float x, y;
  bool isAlive;
  int health;
  float angle;
  float baseY;
  bool zigzagEnabled;
};

enum UpgradeType
{
  UPGRADE_NONE,
  UPGRADE_SERVER,
  UPGRADE_BULLET_ORANGE,
  UPGRADE_BULLET_RED
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

const int orangeUpgradeCost = 10;
const int redUpgradeCost = 30;
int bulletUpgradeLevel = 0;

bool shopOpen = false;

int maxAmmo = 20;
int currentAmmo = 20;
bool reloading = false;
float lastReloadTime = 0.0f;

bool levelCountdownActive = false;
int levelCountdownValue = 5;

void drawLevelCountdown()
{
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", levelCountdownValue);
  glColor3f(1.0f, 1.0f, 0.0f);
  glPushMatrix();
  // Center the text (adjust values as needed for your coordinate system)
  glTranslatef(-0.15f, -0.15f, 0.0f); // Center on screen
  glScalef(0.0045f, 0.0045f, 1.0f);   // Make the text very big
  for (char *c = buf; *c; ++c)
  {
    glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
  }
  glPopMatrix();
}

void startLevelCountdown()
{
  levelCountdownActive = true;
  levelCountdownValue = 5;
  // Remove all active bullets when countdown starts
  bullets.clear();
  glutTimerFunc(1000, levelCountdownTimer, 0);
}

void levelCountdownTimer(int value)
{
  if (!levelCountdownActive)
    return;
  levelCountdownValue--;
  if (levelCountdownValue > 0)
  {
    glutPostRedisplay();
    glutTimerFunc(1000, levelCountdownTimer, 0);
  }
  else
  {
    levelCountdownActive = false;
    loadZombies(6 + level); // Start new level zombies
    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // Resume game updates
  }
}

void levelCountdownTimer(int value);

void drawSquare(float x, float y, float width, float height, float r, float g,
                float b, float a = 1)
{
  glColor4f(r, g, b, a);
  glBegin(GL_POLYGON);
  glVertex2f(x, y);
  glVertex2f(x + width, y);
  glVertex2f(x + width, y + height);
  glVertex2f(x, y + height);
  glEnd();
}

void drawAmmoBar()
{
  float barWidth = 0.7f;         // Smaller width
  float barHeight = 0.04f;       // Smaller height
  float barX = -barWidth / 2.0f; // Centered horizontally
  float barY = -0.98f;           // Closer to bottom
  float squareGap = 0.008f;
  float squareWidth = (barWidth - (maxAmmo - 1) * squareGap) / maxAmmo;
  float squareHeight = barHeight * 0.8f;
  // Draw bar background
  drawSquare(barX, barY, barWidth, barHeight, 0.2f, 0.2f, 0.2f, 0.7f);
  // Draw ammo squares
  for (int i = 0; i < maxAmmo; ++i)
  {
    float x = barX + i * (squareWidth + squareGap);
    if (i < currentAmmo)
      drawSquare(x, barY + 0.005f, squareWidth, squareHeight, 1.0f, 1.0f, 0.0f, 1.0f); // filled
    else
      drawSquare(x, barY + 0.005f, squareWidth, squareHeight, 0.3f, 0.3f, 0.3f, 1.0f); // empty
  }
}

void drawBulletShopIcon(float x, float y, float r, float g, float b)
{
  glPushMatrix();
  glTranslatef(x, y, 0.0f);
  glScalef(0.15f, 0.15f, 1.0f);
  glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
  // Body
  glColor3f(r, g, b);
  glBegin(GL_POLYGON);
  glVertex2f(0.0f, 0.0f);
  glVertex2f(0.05f, 0.0f);
  glVertex2f(0.05f, 0.2f);
  glVertex2f(0.0f, 0.2f);
  glEnd();
  // Tip
  glColor3f(1.0f, 0.0f, 0.0f);
  glBegin(GL_TRIANGLES);
  glVertex2f(0.0f, 0.2f);
  glVertex2f(0.05f, 0.2f);
  glVertex2f(0.025f, 0.25f);
  glEnd();
  // Fins
  glColor3f(0.5f, 0.5f, 0.5f);
  glBegin(GL_TRIANGLES);
  glVertex2f(0.0f, 0.0f);
  glVertex2f(-0.02f, 0.05f);
  glVertex2f(0.0f, 0.05f);
  glVertex2f(0.05f, 0.0f);
  glVertex2f(0.07f, 0.05f);
  glVertex2f(0.05f, 0.05f);
  glEnd();
  glPopMatrix();
}

void reloadAmmo(int value)
{
  if (currentAmmo < maxAmmo)
  {
    currentAmmo++;
    glutPostRedisplay();
    glutTimerFunc(600, reloadAmmo, 0); // 0.6 seconds per bullet
  }
  else
  {
    reloading = false;
  }
}

void drawShieldShopIcon(float x, float y)
{
  glPushMatrix();
  glTranslatef(x, y, 0.0f);
  glScalef(0.06f, 0.06f, 1.0f); // Reduced scale for smaller icon
  glColor3f(0.2f, 0.8f, 1.0f);
  glBegin(GL_POLYGON);
  for (int i = 0; i <= 180; ++i)
  {
    float theta = M_PI * i / 180.0f;
    glVertex2f(cos(theta), sin(theta));
  }
  glEnd();
  glColor3f(0.1f, 0.4f, 0.7f);
  glBegin(GL_POLYGON);
  glVertex2f(0.0f, 0.0f);
  glVertex2f(-1.0f, 0.0f);
  glVertex2f(0.0f, -1.5f);
  glVertex2f(1.0f, 0.0f);
  glEnd();
  glPopMatrix();
}

// Update the bullet rendering logic
void drawBullet(float x, float y)
{
  // Set color based on upgrade level
  float r = 1.0f, g = 1.0f, b = 1.0f;
  if (bulletUpgradeLevel == 2)
  {
    r = 1.0f;
    g = 0.0f;
    b = 0.0f;
  } // Red
  else if (bulletUpgradeLevel == 1)
  {
    r = 1.0f;
    g = 0.5f;
    b = 0.0f;
  } // Orange

  glPushMatrix();
  // Move to bullet position
  glTranslatef(x, y, 0.0f);
  // Rotate 90 degrees counterclockwise (to the left)
  glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
  // Scale to half size
  glScalef(0.5f, 0.5f, 1.0f);

  // Body
  glColor3f(r, g, b);
  glBegin(GL_POLYGON);
  glVertex2f(0.0f, 0.0f);
  glVertex2f(0.05f, 0.0f);
  glVertex2f(0.05f, 0.2f);
  glVertex2f(0.0f, 0.2f);
  glEnd();

  // Tip
  glColor3f(1.0f, 0.0f, 0.0f);
  glBegin(GL_TRIANGLES);
  glVertex2f(0.0f, 0.2f);
  glVertex2f(0.05f, 0.2f);
  glVertex2f(0.025f, 0.25f);
  glEnd();

  // Fins
  glColor3f(0.5f, 0.5f, 0.5f);
  glBegin(GL_TRIANGLES);
  glVertex2f(0.0f, 0.0f);
  glVertex2f(-0.02f, 0.05f);
  glVertex2f(0.0f, 0.05f);

  glVertex2f(0.05f, 0.0f);
  glVertex2f(0.07f, 0.05f);
  glVertex2f(0.05f, 0.05f);
  glEnd();

  glPopMatrix();
}

void checkServerBreach()
{
  if (upgradeEndTime == 0.0f)
  {
    gameOver = true;
  }
  else
  {
    zombieEntryTimes.push_back(glutGet(GLUT_ELAPSED_TIME) / 1000.0f);

    if (zombieEntryTimes.size() >= 3)
    {
      float timeDiff = zombieEntryTimes.back() -
                       zombieEntryTimes[zombieEntryTimes.size() - 2];
      if (timeDiff < 2.0f)
      {
        gameOver = true;
      }
    }
  }
}

void purchaseUpgrade()
{
  if (points >= upgradeCost)
  {
    points -= upgradeCost;
    upgradeEndTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f + upgradeDuration;
  }
}

void drawZombie()
{
  drawSquare(-0.12f, 0.15f, 0.24f, 0.24f, 0.0f, 0.8f, 0.0f);
  drawSquare(-0.09f, 0.27f, 0.04f, 0.04f, 0.0f, 0.0f, 0.0f);
  drawSquare(0.05f, 0.27f, 0.04f, 0.04f, 0.0f, 0.0f, 0.0f);
  drawSquare(-0.15f, -0.05f, 0.3f, 0.25f, 0.0f, 0.7f, 1.0f);
  drawSquare(-0.27f, -0.05f, 0.1f, 0.25f, 0.0f, 0.8f, 0.0f);
  drawSquare(0.17f, -0.05f, 0.1f, 0.25f, 0.0f, 0.8f, 0.0f);
  drawSquare(-0.10f, -0.30f, 0.07f, 0.20f, 0.1f, 0.0f, 0.6f);
  drawSquare(0.03f, -0.30f, 0.07f, 0.20f, 0.1f, 0.0f, 0.6f);
}

void drawZones()
{
  drawSquare(serverLeft, -1.0f, serverRight - serverLeft, 2.0f, 0.3f, 1.0f,
             0.3f, 0.1f);

  drawSquare(hackerLeft, -1.0f, hackerRight - hackerLeft, 2.0f, 1.0f, 0.3f,
             0.3f, 0.1f);
}

void drawCircle(float cx, float cy, float r)
{
  const int numSegments = 360;
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(cx, cy);
  for (int i = 0; i <= numSegments; i++)
  {
    float theta = 2.0f * M_PI * float(i) / float(numSegments);
    float x = r * cosf(theta);
    float y = r * sinf(theta);
    glVertex2f(cx + x, cy + y);
  }
  glEnd();
}

void drawStickman()
{
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
  for (float angle = M_PI / 4; angle <= 3 * M_PI / 4; angle += 0.025f)
  {
    float x = -0.08f * cos(angle);
    float y = 0.75f + 0.05f * sin(angle);
    glVertex2f(x, y);
  }
  glEnd();

  // Hands
  drawCircle(-0.2f, 0.4f, 0.025f);
  drawCircle(0.2f, 0.4f, 0.025f);
}

void renderText(float x, float y, const std::string &text)
{
  glColor3f(1, 1, 1);
  glRasterPos2f(x, y);
  for (char c : text)
  {
    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
  }
}

void drawShop()
{
  // Background
  drawSquare(-0.7f, -0.8f, 1.4f, 1.6f, 0.1f, 0.1f, 0.1f, 0.95f);
  renderText(-0.15f, 0.7f, "SHOP");
  // Bullets category
  renderText(-0.6f, 0.5f, "Bullets:");
  // Normal bullet
  drawBulletShopIcon(-0.55f, 0.4f, 1.0f, 1.0f, 1.0f);
  renderText(-0.45f, 0.42f, "Normal: Basic bullet (Free)");
  // Orange bullet
  drawBulletShopIcon(-0.55f, 0.25f, 1.0f, 0.5f, 0.0f);
  renderText(-0.45f, 0.27f, "Orange: Stronger bullet (10 coins)");
  // Red bullet
  drawBulletShopIcon(-0.55f, 0.1f, 1.0f, 0.0f, 0.0f);
  renderText(-0.45f, 0.12f, "Red: Strongest bullet (30 coins)");
  // Bullet upgrade hint
  renderText(-0.45f, -0.02f, "Press B to upgrade bullet");
  // Shield category
  renderText(-0.6f, -0.2f, "Shield:");
  drawShieldShopIcon(-0.55f, -0.3f);
  renderText(-0.45f, -0.28f, "Upgrade server shield (20 coins)");
  renderText(-0.45f, -0.38f, "Press U to upgrade shield");
  // Close shop hint
  renderText(-0.2f, -0.7f, "Press S to close shop");
}

void loadZombies(int count)
{
  zombies.clear();
  for (int i = 0; i < count; ++i)
  {
    Zombie z;
    z.x = -1.0f - i * 0.3f;
    z.baseY = ((rand() % 180) - 90) / 100.0f;
    z.y = z.baseY;
    z.angle = 0.0f;
    z.zigzagEnabled = (level >= 4);
    z.isAlive = true;
    if (level >= 10)
    {
      z.health = 5;
    }
    else if (level >= 4)
    {
      z.health = 4;
    }
    else if (level >= 3)
    {
      z.health = 3;
    }
    else if (level >= 2)
    {
      z.health = 2;
    }
    else
    {
      z.health = 1;
    }
    zombies.push_back(z);
  }
}

void resetGame()
{
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

void display()
{
  glClear(GL_COLOR_BUFFER_BIT);
  drawZones();
  renderText(0.7f, 0.9f, "Points: " + std::to_string(points));
  if (upgradeEndTime > 0.0f)
  {
    float remaining = upgradeEndTime - (glutGet(GLUT_ELAPSED_TIME) / 1000.0f);
    std::string timerText = "Upgrade: " + std::to_string((int)remaining) + "s";
    renderText(-0.2f, 0.8f, timerText);
  }
  if (upgradeEndTime > 0.0f)
  {
    drawSquare(serverLeft, -1.0f, serverRight - serverLeft, 2.0f, 0.2f, 0.8f,
               0.2f, 0.5f);
  }
  else
  {
    drawSquare(serverLeft, -1.0f, serverRight - serverLeft, 2.0f, 0.0f, 0.3f,
               0.0f, 0.1f);
  }
  if (bulletUpgradeLevel < 2)
  {
    std::string bulletUpgradeText;
    if (bulletUpgradeLevel == 0 && points >= orangeUpgradeCost)
    {
      bulletUpgradeText = "Press B for Orange Bullets (10)";
    }
    else if (bulletUpgradeLevel == 1 && points >= redUpgradeCost)
    {
      bulletUpgradeText = "Press B for Red Bullets (30)";
    }

    if (!bulletUpgradeText.empty())
    {
      renderText(-0.45f, 0.75f, bulletUpgradeText);
    }
  }

  if (gameOver)
  {
    if (level == maxLevel)
    {
      renderText(-0.3f, 0.0f, "Good Job, you win! Press SPACE to Restart!");
    }
    else
    {
      renderText(-0.3f, 0.0f, "GG YOU LOSE - Press SPACE to Restart!");
    }
    if (serverUpgraded)
    {
      renderText(-0.2f, 0.8f, "SERVER UPGRADED!");
    }

    glutSwapBuffers();
    return;
  }
  else
  {
    if (points >= upgradeCost && upgradeEndTime <= 0.0f)
    {
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
  for (const auto &zombie : zombies)
  {
    if (zombie.isAlive)
    {
      glPushMatrix();
      glTranslatef(zombie.x, zombie.y, 0.0f);
      glScalef(0.2f, 0.2f, 1.0f);
      drawZombie();
      glPopMatrix();
    }
  }

  // Draw bullets
  for (const auto &bullet : bullets)
  {
    if (bullet.active)
    {
      drawBullet(bullet.x, bullet.y);
    }
  }

  renderText(-0.9f, 0.9f, "Level: " + std::to_string(level));
  if (shopOpen)
  {
    drawShop();
    glutSwapBuffers();
    return;
  }

  drawAmmoBar();

  if (levelCountdownActive)
  {
    drawLevelCountdown();
  }

  glutSwapBuffers();
}

void purchaseBulletUpgrade()
{
  if (bulletUpgradeLevel == 0 && points >= orangeUpgradeCost)
  {
    points -= orangeUpgradeCost;
    bulletUpgradeLevel = 1;
  }
  else if (bulletUpgradeLevel == 1 && points >= redUpgradeCost)
  {
    points -= redUpgradeCost;
    bulletUpgradeLevel = 2;
  }
}

void update(int value)
{
  if (gameOver || shopOpen || levelCountdownActive) // Pause game updates if shop is open or countdown is active
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
  if (currentTime > upgradeEndTime)
  {
    upgradeEndTime = 0.0f;
  }

  if (level > 1)
  {
    zombieBaseSpeed = 0.002f + (level - 1) * 0.0005f;
  }
  for (auto &zombie : zombies)
  {
    if (zombie.isAlive)
    {
      // Movement logic
      float horizontalSpeed = zombieBaseSpeed;
      if (zombie.x < hackerRight)
      {
        horizontalSpeed += zombieBaseSpeed * 3; // Faster in hacker zone
      }
      else
      {
        horizontalSpeed += zombieBaseSpeed;
      }
      if (zombie.zigzagEnabled)
      {
        zombie.angle += 0.1f;
        zombie.y = zombie.baseY + sin(zombie.angle) * 0.15f;
        horizontalSpeed *= 1.2f;
      }
      zombie.x += horizontalSpeed;

      // Game over condition
      if (zombie.x >= serverLeft)
      {
        checkServerBreach();
        zombie.isAlive = false;
      }
    }
  }
  for (auto &bullet : bullets)
  {
    if (bullet.active)
    {
      bullet.x -= 0.02f;

      // Check collisions
      for (auto &zombie : zombies)
      {
        if (zombie.isAlive)
        {
          float zxLeft = zombie.x - 0.12f * 0.2f;
          float zxRight = zombie.x + 0.12f * 0.2f;
          float zyTop = zombie.y + (0.15f + 0.24f) * 0.2f;
          float zyBottom = zombie.y - 0.3f * 0.2f;

          float bx = bullet.x + 0.025f;
          float by = bullet.y + 0.025f;

          if (bullet.x >= zombie.x - 0.024f && bullet.x <= zombie.x + 0.024f &&
              bullet.y >= zombie.y - 0.15f && bullet.y <= zombie.y + 0.15f)
          {
            bullet.active = false;
            zombie.health -= bullet.damage;
            if (zombie.health <= 0)
            {
              zombie.isAlive = false;
              zombiesKilled++;
              points += bullet.damage;
            }
            break;
          }
        }
      }

      // Remove off-screen bullets
      if (bullet.x < -1.1f)
        bullet.active = false;
    }
  }
  bool allDead = true;
  for (const auto &zombie : zombies)
  {
    if (zombie.isAlive)
    {
      allDead = false;
      break;
    }
  }
  if (allDead)
  {
    if (level < maxLevel)
    {
      level++;
      zombiesKilled = 0;
      startLevelCountdown(); // Start countdown before loading new level
    }
    else
    {
      gameOver = true;
    }
  }
  glutPostRedisplay();
  glutTimerFunc(16, update, 0);
}

void keyboard(unsigned char key, int x, int y)
{
  if (key == 27 && gameOver)
  {
    resetGame();
  }
  else if (key == 32 && !gameOver)
  {
    if (currentAmmo > 0)
    {
      Bullet b;
      b.x = 0.95f;
      b.y = playerY;
      b.active = true;
      switch (bulletUpgradeLevel)
      {
      case 2: // Red bullets
        b.damage = 3;
        b.r = 1.0f;
        b.g = 0.0f;
        b.b = 0.0f;
        break;
      case 1: // Orange bullets
        b.damage = 2;
        b.r = 1.0f;
        b.g = 0.5f;
        b.b = 0.0f;
        break;
      default: // Normal bullets
        b.damage = 1;
        b.r = 1.0f;
        b.g = 1.0f;
        b.b = 1.0f;
      }
      bullets.push_back(b);
      currentAmmo--;
      if (!reloading)
      {
        reloading = true;
        glutTimerFunc(200, reloadAmmo, 0);
      }
    }
  }
  else if (shopOpen)
  {
    if (key == 's' || key == 'S')
    {
      shopOpen = false;
      glutPostRedisplay();
      glutTimerFunc(16, update, 0); // Resume game updates
      return;
    }
    if (key == 'b' || key == 'B')
    {
      purchaseBulletUpgrade();
      glutPostRedisplay();
      return;
    }
    if (key == 'u' || key == 'U')
    {
      purchaseUpgrade();
      glutPostRedisplay();
      return;
    }
    return;
  }
  if (key == 's' || key == 'S')
  {
    shopOpen = true;
    glutPostRedisplay();
    return;
  }
  else if (key == 'u' || key == 'U' && !gameOver)
  {
    purchaseUpgrade();
  }
  else if (key == 27)
  {
    exit(0);
  }
  else if (key == 'b' || key == 'B')
  {
    purchaseBulletUpgrade();
  }
}

void specialDown(int key, int x, int y)
{
  if (key == GLUT_KEY_UP)
    keyUpPressed = true;
  else if (key == GLUT_KEY_DOWN)
    keyDownPressed = true;
}

void specialUp(int key, int x, int y)
{
  if (key == GLUT_KEY_UP)
    keyUpPressed = false;
  else if (key == GLUT_KEY_DOWN)
    keyDownPressed = false;
}

void init()
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glMatrixMode(GL_PROJECTION);
  gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  srand(static_cast<unsigned>(time(0)));

  loadZombies(10);
}

void drawRocketBullet(float x, float y, float r, float g, float b)
{
  // Body of the rocket
  glColor3f(r, g, b);
  glBegin(GL_POLYGON);
  glVertex2f(x, y);                // Bottom-left
  glVertex2f(x + 0.05f, y);        // Bottom-right
  glVertex2f(x + 0.05f, y + 0.2f); // Top-right
  glVertex2f(x, y + 0.2f);         // Top-left
  glEnd();

  // Tip of the rocket
  glColor3f(1.0f, 0.0f, 0.0f); // Red tip
  glBegin(GL_TRIANGLES);
  glVertex2f(x, y + 0.2f);           // Base-left
  glVertex2f(x + 0.05f, y + 0.2f);   // Base-right
  glVertex2f(x + 0.025f, y + 0.25f); // Tip
  glEnd();

  // Fins of the rocket
  glColor3f(0.5f, 0.5f, 0.5f); // Gray fins
  glBegin(GL_TRIANGLES);
  glVertex2f(x, y);                 // Bottom-left
  glVertex2f(x - 0.02f, y + 0.05f); // Left fin
  glVertex2f(x, y + 0.05f);         // Top-left fin

  glVertex2f(x + 0.05f, y);         // Bottom-right
  glVertex2f(x + 0.07f, y + 0.05f); // Right fin
  glVertex2f(x + 0.05f, y + 0.05f); // Top-right fin
  glEnd();
}

int main(int argc, char **argv)
{
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
