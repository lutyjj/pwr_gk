#define _USE_MATH_DEFINES

#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <math.h>
#include <iostream>

typedef float point3[3];
static GLint status = 0;

static GLfloat pix2angle;
static GLfloat upY = 1.0;

static int x_pos = 0;
static int delta_x = 0;
static int y_pos = 0;
static int delta_y = 0;

int model = 1;
void axes();
void renderScene();
void init(int N);
void keyEvent(unsigned char key, int x, int y);
void mouseEvent(int btn, int state, int x, int y);
void motionEvent(GLsizei x, GLsizei y);
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

	void updateAngle () {
		this->theta += (delta_x * pix2angle) / 10.0;
		this->phi += (delta_y * pix2angle) / 10.0;

		if (phi > 2 * M_PI)
			phi = 0;
		else if (phi < 0)
			phi = 2 * M_PI;

		phi > M_PI/2 ? upY = -1.0 : upY = 1.0;

		if (phi > M_PI + (M_PI / 2))
			upY = 1.0;
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

Egg* egg_big;
Egg* egg_smol;
Camera* camera;

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	if (status) camera->updateAngle();
	camera->updateCoordinates();
	gluLookAt(camera->x, camera->y, camera->z, 0.0, 0.0, 0.0, 0.0, upY, 0.0);

	axes();
	egg_big->draw();
	egg_smol->draw();

	glFlush();
	glutSwapBuffers();
}

void init(int N) {
	egg_big = new Egg(N);
	egg_smol = new Egg(N, 5.0f, 10.0f);
	camera = new Camera();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void keyEvent(unsigned char key, int x, int y) {
	if (key == 'p') model = 1;
	if (key == 'l') model = 2;
	if (key == 't') model = 3;
	if (key == 'x') init(egg_big->N + 1);
	if (key == 'z' && egg_big->N) init(egg_big->N - 1);

	if (key == '+') camera->radius -= 0.5;
	if (key == '-') camera->radius += 0.5;
	if (key == 'r') camera->resetCamera();

	renderScene();
}

void mouseEvent(int btn, int state, int x, int y) {
	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		x_pos = x;
		y_pos = y;
		status = 1;
	}
	else
		status = 0;
}

void motionEvent(GLsizei x, GLsizei y) {
	delta_x = x - x_pos;
	x_pos = x;

	delta_y = y - y_pos;
	y_pos = y;

	glutPostRedisplay();
}

void changeSize(GLsizei horizontal, GLsizei vertical) {
	pix2angle = 360.0/(float)horizontal;  // przeliczenie pikseli na stopnie
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

	std::cout << "Controls:\nP/L/T - Points / Lines / Triangles\nZ/X - +-N\nR - reset camera\n+/- - zoom in/out" << std::endl;
	int N = 0;
	std::cout << "Enter N: " << std::endl;
	std::cin >> N;
	init(N);
	glutKeyboardFunc(keyEvent); 
	glutMouseFunc(mouseEvent);
	glutMotionFunc(motionEvent);

	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
}