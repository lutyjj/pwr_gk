#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <iostream>

/* Global variables to be set by user */
float level = 0;
float intensity = 0;

/* Aux class to store points of vertexes in 2D */
struct Point2D {
	float x;
	float y;

	Point2D(float x, float y) {
		this->x = x;
		this->y = y;
	}
};

/* Perturbates one point of vertex */
float perturbation(float number, float step) {
	/* Calculate allowed perturbation based on max length of vertex and entered intensity */
	float temp = step * (intensity / 100.0f);
	/* Randomly add or subtract calculated	perturbation */
	rand() % 2 ? number += temp : number -= temp;

	return number;
}

/* Generates and sets random color */
void glRandomColor() {
	glColor3ub(rand() % 255, rand() % 255, rand() % 255);
}

/* Generates Sierpinski carpet */
void carpet(Point2D top_left, Point2D bottom_right, int level) {
	/* Calculate square's edge length to slice carpet into 3x3 squares*/
	float step = (bottom_right.x - top_left.x) / 3;

	/* Draw every square except central one */
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (i != 1 || j != 1) {
				/* Calculate coordinates of new square */
				float x_start = top_left.x + step * i;
				float x_end = top_left.x + step * (i + 1);
				float y_start = top_left.y - step * j;
				float y_end = top_left.y - step * (j + 1);

				if (!level) {
					glBegin(GL_POLYGON);
						glRandomColor();
						glVertex2f(perturbation(x_start, step), perturbation(y_start, step));
						glRandomColor();
						glVertex2f(perturbation(x_end, step), perturbation(y_start, step));
						glRandomColor();
						glVertex2f(perturbation(x_end, step), perturbation(y_end, step));
						glRandomColor();
						glVertex2f(perturbation(x_start, step), perturbation(y_end, step));
					glEnd();
				}
				else
					carpet(Point2D(x_start, y_start), Point2D(x_end, y_end), level - 1);
			}
		}
	}
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.0f, 1.0f, 0.0f);
	carpet(Point2D(-80.0f, 80.0f), Point2D(80.0f, -80.0f), level);
	glFlush();
}

void init(void) {
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
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
		glOrtho(-100.0, 100.0, -100.0 / AspectRatio, 100.0 / AspectRatio, 1.0, -1.0);
	else
		glOrtho(-100.0 * AspectRatio, 100.0 * AspectRatio, -100.0, 100.0, 1.0, -1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int main(void) {
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutCreateWindow("Sierpinski carpet");
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	init();

	std::cout << "Enter level of carpet: " << std::endl;
	std::cin >> level;
	level--;

	std::cout << "Enter probability of noise: " << std::endl;
	std::cin >> intensity;

	glutMainLoop();
}