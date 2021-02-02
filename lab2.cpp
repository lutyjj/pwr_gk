#define _USE_MATH_DEFINES

#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <math.h>
#include <iostream>

typedef float point3[3];
static GLfloat theta[] = { 0.0, 0.0 };
int model = 1;
bool axis = false;
void spinEgg();
void axes();
void renderScene();
void init(int N);
void keys(unsigned char key, int x, int y);
void changeSize(GLsizei horizontal, GLsizei vertical);

struct Egg {
	point3** point_mask = nullptr;
	point3** color_mask = nullptr;
	float offset = 0;
	float scaling = 0;
	int N = 0;

	Egg(int N, float offset = 0, float scaling = 0) {
		this->N = N;
		this->point_mask = new point3 * [N];
		this->color_mask = new point3 * [N];
		this->offset = offset;
		this->scaling = scaling;
		float scale = 1.0f / (float(N) - 1.0f);

		for (int i = 0; i < N; i++) {
			point_mask[i] = new point3[N];
			color_mask[i] = new point3[N];

			for (int j = 0; j < N; j++) {
				if (offset && scaling) {
					point_mask[i][j][0] = ((calcX(i * scale, j * scale)) / scaling) - offset;
					point_mask[i][j][1] = (calcY(i * scale, j * scale) - 5.0f) / scaling;
					point_mask[i][j][2] = (calcZ(i * scale, j * scale)) / scaling;
				}
				else {
					point_mask[i][j][0] = calcX(i * scale, j * scale);
					point_mask[i][j][1] = calcY(i * scale, j * scale) - 5.0f;
					point_mask[i][j][2] = calcZ(i * scale, j * scale);
				}

				color_mask[i][j][0] = rand() % 2;
				color_mask[i][j][1] = rand() % 2;
				color_mask[i][j][2] = rand() % 2;
			}
		}
	}

	~Egg() {
		for (int i = 0; i < N; i++)
			delete[] point_mask[i];

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
		}
		else if (model == 2) {
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
		}
		else if (model == 3) {
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
		}
	}

	float calcX(float u, float v) {
		return ((-90 * pow(u, 5.0) + 225 * pow(u, 4.0) - 270 * pow(u, 3.0) + 180 * pow(u, 2.0) - 45 * u)) * cos(M_PI * v);
	}

	float calcY(float u, float v) {
		return 160 * pow(u, 4.0) - 320 * pow(u, 3.0) + 160 * pow(u, 2.0);
	}

	float calcZ(float u, float v) {
		return ((-90 * pow(u, 5.0) + 225 * pow(u, 4.0) - 270 * pow(u, 3.0) + 180 * pow(u, 2.0) - 45 * u)) * sin(M_PI * v);
	}
};

void axes() {
	point3  x_min = { -5.0, 0.0, 0.0 };
	point3  x_max = { 5.0, 0.0, 0.0 };

	point3  y_min = { 0.0, -5.0, 0.0 };
	point3  y_max = { 0.0,  5.0, 0.0 };

	point3  z_min = { 0.0, 0.0, -5.0 };
	point3  z_max = { 0.0, 0.0,  5.0 };

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

void spinEgg() {
	theta[0] -= 0.05;
	if (theta[0] > 360.0) theta[0] -= 360.0;

	theta[1] -= 0.05;
	if (theta[1] > 360.0) theta[1] -= 360.0;

	glutPostRedisplay();
}

//temp
Egg* egg_big;
Egg* egg_smol;
void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	axes();
	axis ? glRotatef(theta[0], 1.0, 0.0, 0.0) : glRotatef(theta[1], 0.0, 1.0, 0.0);
	egg_big->draw();
	egg_smol->draw();
	glFlush();
	glutSwapBuffers();
}

void init(int N) {
	egg_big = new Egg(N);
	egg_smol = new Egg(N, 5.0f, 10.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void keys(unsigned char key, int x, int y) {
	if (key == 'p') model = 1;
	if (key == 'l') model = 2;
	if (key == 't') model = 3;
	if (key == 'a') axis = !axis;
	if (key == 'x') init(egg_big->N + 1);
	else if (key == 'z' && egg_big->N) init(egg_big->N - 1);

	renderScene();
}

void changeSize(GLsizei horizontal, GLsizei vertical) {
	GLfloat AspectRatio;
	if (vertical == 0)
		vertical = 1;

	glViewport(0, 0, horizontal, vertical);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	AspectRatio = (GLfloat)horizontal / (GLfloat)vertical;

	if (horizontal <= vertical)
		glOrtho(-7.5, 7.5, -7.5 / AspectRatio, 7.5 / AspectRatio, 10.0, -10.0);
	else
		glOrtho(-7.5 * AspectRatio, 7.5 * AspectRatio, -7.5, 7.5, 10.0, -10.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void main() {
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Egg");
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	std::cout << "Controls:\nP/L/T - Points / Lines / Triangles\nZ/X - +-N\nA - switch axis" << std::endl;
	int N = 0;
	std::cout << "Enter N: " << std::endl;
	std::cin >> N;
	init(N);
	glutKeyboardFunc(keys);
	glutIdleFunc(spinEgg);

	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
}