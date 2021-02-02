// Copyright[2020] <lmao>

#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS

#include <math.h>
#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <chrono>
#include <random>

GLbyte *LoadTGAImage(const char *FileName, GLint *ImWidth, GLint *ImHeight,
                     GLint *ImComponents, GLenum *ImFormat) {
#pragma pack(1)
  typedef struct {
    GLbyte idlength;
    GLbyte colormaptype;
    GLbyte datatypecode;
    __int16 colormapstart;
    __int16 colormaplength;
    unsigned char colormapdepth;
    __int16 x_orgin;
    __int16 y_orgin;
    __int16 width;
    __int16 height;
    GLbyte bitsperpixel;
    GLbyte descriptor;
  } TGAHEADER;
#pragma pack(8)

  FILE *pFile;
  TGAHEADER tgaHeader;
  __int32 lImageSize;
  __int16 sDepth;
  GLbyte *pbitsperpixel = NULL;
  *ImWidth = 0;
  *ImHeight = 0;
  *ImFormat = GL_BGR_EXT;
  *ImComponents = GL_RGB8;
  pFile = fopen(FileName, "rb");
  if (pFile == NULL) return NULL;
  fread(&tgaHeader, sizeof(TGAHEADER), 1, pFile);
  *ImWidth = tgaHeader.width;
  *ImHeight = tgaHeader.height;
  sDepth = tgaHeader.bitsperpixel / 8;

  if (tgaHeader.bitsperpixel != 8 && tgaHeader.bitsperpixel != 24 &&
      tgaHeader.bitsperpixel != 32)
    return NULL;

  lImageSize = tgaHeader.width * tgaHeader.height * sDepth;
  pbitsperpixel =
      reinterpret_cast<GLbyte *>(malloc(lImageSize * sizeof(GLbyte)));

  if (pbitsperpixel == NULL) return NULL;

  if (fread(pbitsperpixel, lImageSize, 1, pFile) != 1) {
    free(pbitsperpixel);
    return NULL;
  }

  switch (sDepth) {
    case 3:
      *ImFormat = GL_BGR_EXT;
      *ImComponents = GL_RGB8;
      break;

    case 4:
      *ImFormat = GL_BGRA_EXT;
      *ImComponents = GL_RGBA8;
      break;

    case 1:
      *ImFormat = GL_LUMINANCE;
      *ImComponents = GL_LUMINANCE8;
      break;
  };
  fclose(pFile);
  return pbitsperpixel;
}

GLint ImWidth, ImHeight, ImComponents;
typedef float point3[3];
static GLint status = 0;
static __int64 timeDiff;
static GLfloat pix2angle;
static GLfloat upY = 1.0;

static std::chrono::steady_clock::time_point prevTime, nowTime;

static int x_pos = 0;
static int delta_x = 0;
static int y_pos = 0;
static int delta_y = 0;

int model = 5;
void axes();
void renderScene();
void init(int N);
void keyEvent(unsigned char key, int x, int y);
void mouseEvent(int btn, int state, int x, int y);
void motionEvent(GLsizei x, GLsizei y);
void changeSize(GLsizei horizontal, GLsizei vertical);

struct Egg {
  point3 **point_mask = nullptr;
  point3 **color_mask = nullptr;
  point3 **normalized = nullptr;
  float offset = 0;
  float scaling = 0;
  int N = 0;
  float scale;

  Egg(int N, float offset = 0.0, float scaling = 1.0) {
    this->N = N;
    this->point_mask = new point3 *[N];
    this->color_mask = new point3 *[N];
    this->normalized = new point3 *[N];
    this->offset = offset;
    this->scaling = scaling;
    this->scale = 1.0f / (static_cast<float>(N) - 1.0f);

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, 1);

    for (int i = 0; i < N; i++) {
      point_mask[i] = new point3[N];
      color_mask[i] = new point3[N];
      normalized[i] = new point3[N];

      for (int j = 0; j < N; j++) {
        point_mask[i][j][0] =
            ((calcX(i * scale, j * scale)) / scaling) - offset;
        point_mask[i][j][1] = (calcY(i * scale, j * scale) - 5.0f) / scaling;
        point_mask[i][j][2] = (calcZ(i * scale, j * scale)) / scaling;

        color_mask[i][j][0] = dist(mt);
        color_mask[i][j][1] = dist(mt);
        color_mask[i][j][2] = dist(mt);

        auto xv = calcXv(i * scale, j * scale) / scaling;
        auto xu = calcXu(i * scale, j * scale) / scaling;
        auto yv = calcYv(i * scale, j * scale) / scaling;
        auto yu = calcYu(i * scale, j * scale) / scaling;
        auto zv = calcZv(i * scale, j * scale) / scaling;
        auto zu = calcZu(i * scale, j * scale) / scaling;

        normalized[i][j][0] = yu * zv - zu * yv;
        normalized[i][j][1] = zu * xv - xu * zv;
        normalized[i][j][2] = xu * yv - yu * xv;

        auto length =
            sqrt(pow(normalized[i][j][0], 2) + pow(normalized[i][j][1], 2) +
                 pow(normalized[i][j][2], 2));

        if (length == 0) {
          if (i == 0) normalized[i][j][1] = -1;
          length = 1;
        }

        if (i >= N) {
          normalized[i][j][0] *= -1;
          normalized[i][j][1] *= -1;
          normalized[i][j][2] *= -1;
        }

        normalized[i][j][0] = (normalized[i][j][0] / length) - offset;
        normalized[i][j][1] = normalized[i][j][1] / length;
        normalized[i][j][2] = normalized[i][j][2] / length;
      }
    }
  }

  ~Egg() {
    for (int i = 0; i < N; i++) delete[] point_mask[i];

    delete[] point_mask;
  }

  void draw() {
    if (model == 1) {
      for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
          glBegin(GL_POINTS);
          glColor3f(1.0f, 1.0f, 0.0f);
          glVertex3fv(point_mask[i][j]);
          glEnd();
        }
      }
    } else if (model == 2) {
      for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
          glBegin(GL_LINES);
          if (j + 1 != N) {
            glVertex3fv(point_mask[i][j]);
            glVertex3fv(point_mask[i][j + 1]);
          }

          if (i + 1 != N) {
            glVertex3fv(point_mask[i][j]);
            glVertex3fv(point_mask[i + 1][j]);
          }

          if (i) {
            glVertex3fv(point_mask[i][j]);
            glVertex3fv(point_mask[i - 1][j]);
          }

          if (j) {
            glVertex3fv(point_mask[i][j]);
            glVertex3fv(point_mask[i][j - 1]);
          }
          glEnd();
        }
      }
    } else if (model == 3) {
      for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
          glBegin(GL_TRIANGLES);
          if (j + 1 != N && i + 1 != N) {
            glColor3fv(color_mask[i][j]);
            glVertex3fv(point_mask[i][j]);
            glColor3fv(color_mask[i][j + 1]);
            glVertex3fv(point_mask[i][j + 1]);
            glColor3fv(color_mask[i + 1][j]);
            glVertex3fv(point_mask[i + 1][j]);
          }

          if (i && j) {
            glColor3fv(color_mask[i][j]);
            glVertex3fv(point_mask[i][j]);
            glColor3fv(color_mask[i][j - 1]);
            glVertex3fv(point_mask[i][j - 1]);
            glColor3fv(color_mask[i - 1][j]);
            glVertex3fv(point_mask[i - 1][j]);
          }
          glEnd();
        }
      }
    } else if (model == 4) {
      for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
          glBegin(GL_TRIANGLES);
          glNormal3fv(normalized[i][j]);

          if (j + 1 != N && i + 1 != N) {
            glVertex3fv(point_mask[i][j]);
            glVertex3fv(point_mask[i][j + 1]);
            glVertex3fv(point_mask[i + 1][j]);
          }

          if (i && j) {
            glVertex3fv(point_mask[i][j]);
            glVertex3fv(point_mask[i][j - 1]);
            glVertex3fv(point_mask[i - 1][j]);
          }
          glEnd();
        }
      }
    } else if (model == 5) {
      if (timeDiff >= ImHeight || timeDiff >= ImWidth) {
        timeDiff = 0;
      }

      for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
          glBegin(GL_TRIANGLES);
          glNormal3fv(normalized[i][j]);
          float heigthScale = 1.0f / ImHeight;
          float widthScale = 1.0f / ImWidth;
          glTexCoord2f((i + timeDiff) * heigthScale,
                       (j + timeDiff) * widthScale);

          if (j + 1 != N && i + 1 != N) {
            glVertex3fv(point_mask[i][j]);
            glVertex3fv(point_mask[i][j + 1]);
            glVertex3fv(point_mask[i + 1][j]);
          }

          if (i && j) {
            glVertex3fv(point_mask[i][j]);
            glVertex3fv(point_mask[i][j - 1]);
            glVertex3fv(point_mask[i - 1][j]);
          }
          glEnd();
        }
      }
    }
  }

  float calcX(float u, float v) {
    return ((-90 * pow(u, 5.0) + 225 * pow(u, 4.0) - 270 * pow(u, 3.0) +
             180 * pow(u, 2.0) - 45 * u)) *
           cos(M_PI * v);
  }

  float calcY(float u, float v) {
    return 160 * pow(u, 4.0) - 320 * pow(u, 3.0) + 160 * pow(u, 2.0);
  }

  float calcZ(float u, float v) {
    return ((-90 * pow(u, 5.0) + 225 * pow(u, 4.0) - 270 * pow(u, 3.0) +
             180 * pow(u, 2.0) - 45 * u)) *
           sin(M_PI * v);
  }

  float calcXu(float u, float v) {
    return (-450 * pow(u, 4) + 900 * pow(u, 3) - 810 * pow(u, 2) + 360 * u -
            45) *
           cos(M_PI * v);
  }

  float calcXv(float u, float v) {
    return M_PI *
           (90 * pow(u, 5) - 225 * pow(u, 4) + 270 * (u, 3) - 180 * (u, 2) +
            45 * u) *
           sin(M_PI * v);
  }

  float calcYu(float u, float v) {
    return 640 * pow(u, 3) - 960 * pow(u, 2) + 320 * u;
  }

  float calcYv(float u, float v) { return 0; }

  float calcZu(float u, float v) {
    return (-450 * pow(u, 4) + 900 * pow(u, 3) - 810 * pow(u, 2) + 360 * u -
            45) *
           sin(M_PI * v);
  }

  float calcZv(float u, float v) {
    return (-M_PI) *
           (90 * pow(u, 5) - 225 * pow(u, 4) + 270 * (u, 3) - 180 * (u, 2) +
            45 * u) *
           cos(M_PI * v);
  }
};

struct Camera {
  GLfloat theta, phi, x, y, z, radius;

  Camera() {
    this->x = 0.0;
    this->y = 0.0;
    this->z = 0.0;
    this->radius = 10.0;
    this->theta = 0.0;
    this->phi = 0.0;
  };

  void updateAngle() {
    this->theta += (delta_x * pix2angle) / 10.0;
    this->phi += (delta_y * pix2angle) / 10.0;

    if (phi > 2 * M_PI)
      phi = 0;
    else if (phi < 0)
      phi = 2 * M_PI;

    phi > M_PI / 2 ? upY = -1.0 : upY = 1.0;

    if (phi > M_PI + (M_PI / 2)) upY = 1.0;
  }

  void updateCoordinates() {
    this->x = radius * cos(theta) * cos(phi);
    this->y = radius * sin(phi);
    this->z = radius * sin(theta) * cos(phi);
  }

  void resetCamera() {
    this->x = 0.0;
    this->y = 0.0;
    this->z = 0.0;
    this->radius = 10.0;
    this->theta = 0.0;
    this->phi = 0.0;
  }
};

void axes() {
  point3 x_min = {-5.0, 0.0, 0.0};
  point3 x_max = {5.0, 0.0, 0.0};

  point3 y_min = {0.0, -5.0, 0.0};
  point3 y_max = {0.0, 5.0, 0.0};

  point3 z_min = {0.0, 0.0, -5.0};
  point3 z_max = {0.0, 0.0, 5.0};

  glColor3f(1.0f, 0.0f, 0.0f);
  glBegin(GL_LINES);
  glVertex3fv(x_min);
  glVertex3fv(x_max);
  glEnd();

  glColor3f(0.0f, 1.0f, 0.0f);
  glBegin(GL_LINES);
  glVertex3fv(y_min);
  glVertex3fv(y_max);
  glEnd();

  glColor3f(0.0f, 0.0f, 1.0f);
  glBegin(GL_LINES);
  glVertex3fv(z_min);
  glVertex3fv(z_max);
  glEnd();
}

Egg *egg_big;
Egg *egg_smol;
Camera *camera;

void renderScene() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  if (status) camera->updateAngle();
  camera->updateCoordinates();
  glLightfv(
      GL_LIGHT0, GL_POSITION,
      new float[] {-camera->x * camera->radius, -camera->y * camera->radius,
                  camera->z * camera->radius, camera->radius});

  gluLookAt(10.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0);

  nowTime = std::chrono::high_resolution_clock::now();
  if (std::chrono::duration_cast<std::chrono::seconds>(nowTime - prevTime)
          .count() >= 1) {
    prevTime = nowTime;
    timeDiff += 1;
  }

  egg_big->draw();
  glFlush();
  glutSwapBuffers();
  glutPostRedisplay();
}

void init(int N) {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  GLbyte *pBytes;

  GLenum ImFormat;

  egg_big = new Egg(N);
  camera = new Camera();

  GLfloat mat_ambient[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat mat_diffuse[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat mat_shininess = {20.0};

  GLfloat light_position[] = {0.0, 0.0, 10.0, 1.0};
  GLfloat light_ambient[] = {0.1f, 0.1f, 0.1f, 1.0};
  GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};

  GLfloat att_constant = {1.0};
  GLfloat att_linear = {0.05};
  GLfloat att_quadratic = {0.001};

  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, att_constant);
  glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, att_linear);
  glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, att_quadratic);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);

  pBytes = LoadTGAImage("texture/D1_t.tga", &ImWidth, &ImHeight, &ImComponents,
                        &ImFormat);
  glTexImage2D(GL_TEXTURE_2D, 0, ImComponents, ImWidth, ImHeight, 0, ImFormat,
               GL_UNSIGNED_BYTE, pBytes);
  free(pBytes);
  glEnable(GL_TEXTURE_2D);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void changeSize(GLsizei horizontal, GLsizei vertical) {
  pix2angle = 360.0 / static_cast<float>(horizontal);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(70, 1.0, 1.0, 30.0);

  if (horizontal <= vertical)
    glViewport(0, (vertical - horizontal) / 2, horizontal, horizontal);
  else
    glViewport((horizontal - vertical) / 2, 0, vertical, vertical);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

int main() {
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(600, 600);
  glutCreateWindow("Egg");
  glutDisplayFunc(renderScene);
  glutReshapeFunc(changeSize);

  printf_s(
      "Controls:\nR/G/B - Red / Green / Blue lightning\nMouse - control"
      "lightning position\n");
  printf_s("Enter N: ");
  int N = 0;
  scanf_s("%d", &N);
  init(N);

  glEnable(GL_DEPTH_TEST);
  glutMainLoop();
}
