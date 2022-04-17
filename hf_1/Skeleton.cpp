//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Urban Denes Jakab
// Neptun : TOPKQH
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char * const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers

	uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec2 vp;	// Varying input: vp = vertex position is expected in attrib array 0

	void main() {
		float w = sqrt(pow(vp.x, 2) + pow(vp.y, 2) + 1) + 20;
		float pX = vp.x / (w + 1);
		float pY = vp.y / (w + 1);	

		gl_Position = vec4(pX, pY, w, 1) * MVP;
		//gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;		// transform vp from modeling space to normalized device space
	}
)";

// fragment shader in GLSL
const char * const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers
	
	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

unsigned int vao;
unsigned int vbo;
GPUProgram gpuProgram; // vertex and fragment shaders

// 2D camera //Source: smoothtrianlge.cpp available at online.vik.bme.hu
class Camera2D {
	vec2 wCenter; // center in world coordinates
	vec2 wSize;   // width and height in world coordinates
public:
	Camera2D() : wCenter(0, 0), wSize(2, 2) { }

	mat4 V() { return TranslateMatrix(-wCenter); }
	mat4 P() { return ScaleMatrix(vec2(2 / wSize.x, 2 / wSize.y)); }

	mat4 Vinv() { return TranslateMatrix(wCenter); }
	mat4 Pinv() { return ScaleMatrix(vec2(wSize.x / 2, wSize.y / 2)); }

	void Zoom(float s) { wSize = wSize * s; }
	void Pan(vec2 t) { wCenter = wCenter + t; }
};

Camera2D camera;

float pi = 2 * acos(0.0f);

class Atom {
	float cx = 1.0f; //circle width
	float cy = 1.0f; //circle height
	float radius = 10; //circle radius
	vec2 wTranslate; 
	int nP = 1000; //number of points
	float phi = 0;
	float charge;
	float vertices[1000];
public:
	Atom() {};
	Atom(vec2 pos) {
		//wTranslate = pos;
		vertices[0] = pos.x;
		vertices[1] = pos.y;
		charge = rand() % 20 - 10;
		printf("\tnew Atom at: %.0f %.0f (charge: %.0f)\n", pos.x, pos.y, charge);
	}
	//Source: smoothtrianlge.cpp available at online.vik.bme.hu
	mat4 M() {
		mat4 Mscale(cx, 0, 0, 0,
			0, cy, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 1); // scaling

		mat4 Mrotate(cosf(phi), sinf(phi), 0, 0,
			-sinf(phi), cosf(phi), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1); // rotation

		mat4 Mtranslate(1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 0,
			wTranslate.x, wTranslate.y, 0, 1); // translation

		return Mscale * Mrotate * Mtranslate;	// model transformation
	}
	//calculates points around the center coordinate
	void create() {
		for (unsigned int i = 2; i < nP; i++) {
			vertices[i] = vertices[0] + (radius * (cos(((i - 2) / 2) * 2.0f * pi / (nP / 2 - 2))));
			i++;
			vertices[i] = vertices[1] + (radius * (sin(((i - 3) / 2) * 2.0f * pi / (nP / 2 - 2))));
		}
	}
	void Draw() {
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		// Set color depending on charge
		int location = glGetUniformLocation(gpuProgram.getId(), "color");
		if(charge < 0)
			glUniform3f(location, 0.0f, 0.0f, charge * (-1) / 10.0f); // 3 floats
		else
			glUniform3f(location, charge / 10.0f, 0.0f, 0.0f); // 3 floats

		mat4 MVPTransform = M() *camera.V()* camera.P();
		gpuProgram.setUniform(MVPTransform, "MVP");
		glBindVertexArray(vao);	// make the vao and its vbos active playing the role of the data source
		glDrawArrays(GL_TRIANGLE_FAN, 0, nP/2);
	}
};

class Bond {
	float vertices[16] = {}; //coordinates for the ends of lines: 16 being max, because of the max of 8 atoms
	int points = 0; //# of points to be connected by lines
	vec2 wTranslate;
	float phi = 0;
	float cx = 1.0f;
	float cy = 1.0f;
public:
	Bond() {
		points = 0;
	}
	//Source: smoothtrianlge.cpp available at online.vik.bme.hu
	mat4 M() {
		mat4 Mscale(cx, 0, 0, 0,
			0, cy, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 1); // scaling

		mat4 Mrotate(cosf(phi), sinf(phi), 0, 0,
			-sinf(phi), cosf(phi), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1); // rotation

		mat4 Mtranslate(1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 0,
			wTranslate.x, wTranslate.y, 0, 1); // translation

		return Mscale * Mrotate * Mtranslate;	// model transformation
	}
	void draw() {		
		glBufferData(GL_ARRAY_BUFFER, points*sizeof(float), vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		int location = glGetUniformLocation(gpuProgram.getId(), "color");
		glUniform3f(location, 1.0f, 1.0f, 1.0f);

		glLineWidth(5.0f);

		mat4 MVPTransform = M() * camera.V() * camera.P();
		gpuProgram.setUniform(MVPTransform, "MVP");
		glBindVertexArray(vao);	// make the vao and its vbos active playing the role of the data source
		glDrawArrays(GL_LINE_STRIP, 0, points/2);
	}
	void addPoint(float x, float y) {
		printf("\tnew Bond endpoint at: %.0f %.0f\n", x, y);
		vertices[points] = x;
		vertices[++points] = y; //increases 'int points' by one
		points++;
	}
};

class Molecule {
public:
	int n = rand() % 8 + 2; //# of atoms
	Atom atoms[8];
	Bond b; 

	void init() {
		b = Bond();
		n = rand() % 8 + 2;
		printf("Atoms (%d):\n", n);
		for (int i = 0; i < n; i++) {
			int x = rand() % 100 - 50; //random position
			int y = rand() % 100 - 50; //random position
			atoms[i] = Atom(vec2(x,y));
			b.addPoint(x, y); //coordinates for lines
		}
		for (int i = 0; i < n; i++) {
			atoms[i].create();
		}
	}
	void draw() {
		b.draw();
		for (int i = 0; i < n; i++) {
			atoms[i].Draw();
		}
	}
};

Molecule m = Molecule();

// Initialization, create an OpenGL context
void onInitialization() {
	printf("onInitialization()\n");

	glViewport(0, 0, windowWidth, windowHeight);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// create program for the GPU
	gpuProgram.create(vertexSource, fragmentSource, "outColor");
	
	m.init();
}
// Window has become invalid: Redraw
void onDisplay() {
	printf("onDisplay()\n");

	glClearColor(0.5f, 0.5f, 0.5f, 0); //background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set color to (0, 1, 0) = green
	int location = glGetUniformLocation(gpuProgram.getId(), "color");
	glUniform3f(location, 0.0f, 1.0f, 0.0f); // 3 floats

	m.draw();

	glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
	//if (key == 'd') glutPostRedisplay();         // if d, invalidate display, i.e. redraw
	if (key == ' ') {
		printf("\nSpace pressed\n");
		m.init();
	}
	switch (key) {
	case 's': camera.Pan(vec2(-1, 0)); break;
	case 'd': camera.Pan(vec2(+1, 0)); break;
	case 'e': camera.Pan(vec2(0, 1)); break;
	case 'x': camera.Pan(vec2(0, -1)); break;
	case 'z': camera.Zoom(0.9f); break;
	case 'Z': camera.Zoom(1.1f); break;
	}
	glutPostRedisplay();
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {}


// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) {}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program;
}
