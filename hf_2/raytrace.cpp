//=============================================================================================
// A kód alapja: Computer Graphics Sample Program: Ray-tracing-let
//=============================================================================================
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

const float epsilon = 0.0001f;
int cycle = 0;

struct Material {
	vec3 ka, kd, ks;
	float  shininess;
	Material(vec3 _kd, vec3 _ks, float _shininess) : ka(_kd * M_PI), kd(_kd), ks(_ks) { shininess = _shininess; }
};

struct Hit {
	float t;
vec3 position, normal;
Material* material;
Hit() { t = -1; }
};

struct Ray {
	vec3 start, dir;
	Ray(vec3 _start, vec3 _dir) { start = _start; dir = normalize(_dir); }
};

class Intersectable {
protected:
	Material* material;
public:
	virtual Hit intersect(const Ray& ray) = 0;
};



//Inspiried by: Grafika házi konzi @ Teams
vec4 quat(vec3 axis, float angle) {
	vec3 ret(axis * sin(angle / 2));
	return vec4(ret.x, ret.y, ret.z, cos(angle / 2));
}
vec4 quatInv(vec4 quat) {
	return vec4(-quat.x, -quat.y, -quat.z, quat.w);
}
vec4 quatMul(vec4 q1, vec4 q2) {
	vec3 ret = vec3 (q1.w * q2.x, q1.w * q2.y, q1.w * q2.z) + vec3(q2.w * q1.x, q2.w * q1.y, q2.w * q1.z) + cross(vec3(q1.x, q1.y, q1.z), vec3(q2.x, q2.y, q2.z));
	return vec4(
		ret.x, ret.y, ret.z,
		q1.w * q2.w - dot(vec3(q1.x, q1.y, q1.z), vec3(q2.x, q2.y, q2.z))
	);
}
vec3 quatRot(vec4 q, vec3 p) {
	vec4 qInv = quatInv(q);
	vec4 ret = quatMul(quatMul(q, vec4(p.x, p.y, p.z, 0)), qInv);
	return vec3(ret.x, ret.y, ret.z);
}



struct Sphere : public Intersectable {
	vec3 center;
	float radius;
	vec3 rot;

	Sphere(const vec3& _center, float _radius, Material* _material, vec3 _rot) {
		center = _center;
		radius = _radius;
		material = _material;
		rot = _rot;
	}

	Hit intersect(const Ray& ray) {

		vec4 q = quat(normalize(rot), cycle);
		vec3 origin = quatRot(q, ray.start);
		vec3 dir = quatRot(q, ray.dir);

		Hit hit;
		vec3 dist = origin - center;
		float a = dot(dir, dir);
		float b = dot(dist, dir) * 2.0f;
		float c = dot(dist, dist) - radius * radius;
		float discr = b * b - 4.0f * a * c;
		if (discr < 0) return hit;
		float sqrt_discr = sqrtf(discr);
		float t1 = (-b + sqrt_discr) / 2.0f / a;	// t1 >= t2 for sure
		float t2 = (-b - sqrt_discr) / 2.0f / a;
		if (t1 <= 0) return hit;
		hit.t = (t2 > 0) ? t2 : t1;
		hit.position = origin + dir * hit.t;
		hit.normal = (hit.position - center) * (1.0f / radius);
		hit.material = material;
		return hit;
	}
};

//Source: pathtracingfinal.cpp
//from: online.vik.bme.hu
struct Plane : public Intersectable {
	vec3 point, normal;

	Plane(const vec3& _point, const vec3& _normal, Material* _material) : Intersectable() {
		point = _point;
		normal = _normal * (1 / (sqrt(_normal.x * _normal.x + _normal.y * _normal.y + _normal.z * _normal.z) + epsilon));
		material = _material;
	}
	Hit intersect(const Ray& ray) {
		Hit hit;
		double NdotV = dot(normal, ray.dir);
		if (fabs(NdotV) < epsilon) 
			return hit;
		double t = dot(normal, point - ray.start) / NdotV;
		if (t < epsilon) 
			return hit;
		hit.t = t;
		hit.position = ray.start + ray.dir * hit.t;
		hit.normal = normal;
		if (dot(hit.normal, ray.dir) > 0) 
			hit.normal = hit.normal * (-1);
		hit.material = material;
		return hit;
	}
};

struct CylinderCap : public Intersectable {
	vec3 center;
	float radius;
	vec3 normal;
	CylinderCap(const vec3& _center, const vec3& _normal, float _radius, Material* _material) {
		center = _center;
		normal = _normal;
		radius = _radius;
		material = _material;
	}
	//Inspired by: pathtracingfinal.cpp
	//from: online.vik.bme.hu
	Hit intersect(const Ray& ray) {
		Hit hit;
		double NdotV = dot(normal, ray.dir);
		if (fabs(NdotV) < epsilon)
			return hit;
		double t = dot(normal, center - ray.start) / NdotV;
		if (t < epsilon)
			return hit;
		if (length((ray.start + ray.dir * t) - center) > radius) {
			return hit;
		}
		hit.t = t;
		hit.position = ray.start + ray.dir * hit.t;
		hit.normal = normal;
		if (dot(hit.normal, ray.dir) > 0)
			hit.normal = hit.normal * (-1);
		hit.material = material;
		return hit;
	}
	
};

//Inspired by: 9.5. Programozás: Napfénycső szimulátor sugárkövetéssel
//@: https://www.youtube.com/watch?v=nSHkU4fMK_g
struct Cylinder : public Intersectable {
	mat4 Q;
	float zmin, zmax;
	float height;
	vec3 translation;
	vec3 rot;

	Cylinder(mat4 _Q, float _zmin, float _zmax, vec3 _translation, float _height, Material* _material, vec3 _rot) {
		Q = _Q;
		zmin = _zmin;
		zmax = _zmax;
		translation = _translation;
		material = _material;
		height = _height;
		rot = _rot;
	}
	//quadratic surface gradient
	vec3 gradf(vec3 r) {
		vec4 g = vec4(r.x, r.y, r.z, 1) * Q * 2;
		return vec3(g.x, g.y, g.z);
	}

	Hit intersect(const Ray& ray) {

		vec4 q = quat(normalize(rot), cycle);
		vec3 origin = quatRot(q, ray.start);
		vec3 dir = quatRot(q, ray.dir);

		Hit hit;
		vec3 start = origin - translation;
		vec4 S(start.x, start.y, start.z, 1), D(dir.x, dir.y, dir.z, 0);
		float a = dot(D * Q, D), b = dot(S * Q, D) * 2, c = dot(S * Q, S);
		float discr = b * b - 4.0f * a * c;
		if (discr < 0)
			return hit;
		float sqrt_discr = sqrtf(discr);

		float t1 = (-b + sqrt_discr) / 2.0f / a;
		vec3 p1 = origin + dir * t1;
		if (p1.z < zmin || p1.z > zmax)
			t1 = -1;

		float t2 = (-b - sqrt_discr) / 2.0f / a;
		vec3 p2 = origin + dir * t2;
		if (p2.z < zmin || p2.z > zmax)
			t2 = -1;

		//cylinder cut
		vec3 dist = origin - translation;
		vec3 point = dist + dir * t1;
		if (point.y < 0 || point.y > height) {
			return hit;
		}

		if (t1 <= 0 && t2 <= 0)
			return hit;
		if (t1 <= 0)
			hit.t = 2;
		else if (t2 <= 0)
			hit.t = t1;
		else if (t2 < t1)
			hit.t = t2;
		else
			hit.t = t1;
		hit.position = start + dir * hit.t;
		hit.normal = normalize(gradf(hit.position));
		hit.position = hit.position + translation;
		hit.material = material;
		return hit;
	}
};

//Inspired by: 9.5. Programozás: Napfénycső szimulátor sugárkövetéssel
//@: https://www.youtube.com/watch?v=nSHkU4fMK_g
struct Paraboloid : public Intersectable {
	mat4 Q = mat4(	10, 0, 0, 0,
					0, 10, 0, 0,
					0, 0, 0, 1,
					0, 0, 1, 0
	);
	float zmin, zmax;
	vec3 translation;
	vec3 rot;

	Paraboloid(float _zmin, float _zmax, vec3 _translation, Material* _material, vec3 _rot) {
		zmin = _zmin;
		zmax = _zmax;
		translation = _translation;
		material = _material;
		rot = _rot;
	}
	//quadratic surface gradient
	vec3 gradf(vec3 r) {
		vec4 g = vec4(r.x, r.y, r.z, 1) * Q * 2;
		return vec3(g.x, g.y, g.z);
	}

	Hit intersect(const Ray& ray) {

		vec4 q = quat(normalize(rot), cycle);
		vec3 origin = quatRot(q, ray.start);
		vec3 dir = quatRot(q, ray.dir);

		Hit hit;
		vec3 start = origin - translation;
		vec4 S(start.x, start.y, start.z, 1), D(dir.x, dir.y, dir.z, 0);
		float a = dot(D * Q, D), b = dot(S * Q, D) * 2, c = dot(S * Q, S);
		float discr = b * b - 4.0f * a * c;
		if (discr < 0)
			return hit;
		float sqrt_discr = sqrtf(discr);

		float t1 = (-b + sqrt_discr) / 2.0f / a;
		vec3 p1 = origin + dir * t1;
		if (p1.z < zmin || p1.z > zmax)
			t1 = -1;

		float t2 = (-b - sqrt_discr) / 2.0f / a;
		vec3 p2 = origin + dir * t2;
		if (p2.z < zmin || p2.z > zmax)
			t2 = -1;

		if (t1 <= 0 && t2 <= 0)
			return hit;
		if (t1 <= 0)
			hit.t = 2;
		else if (t2 <= 0)
			hit.t = t1;
		else if (t2 < t1)
			hit.t = t2;
		else
			hit.t = t1;
		hit.position = start + dir * hit.t;
		hit.normal = normalize(gradf(hit.position));
		hit.position = hit.position + translation;
		hit.material = material;
		return hit;
	}
};

class Camera {
	vec3 eye, lookat, right, up;
	float fov;
public:
	void set(vec3 _eye, vec3 _lookat, vec3 vup, float fov) {
		this->fov = fov;
		eye = _eye;
		lookat = _lookat;
		vec3 w = eye - lookat;
		float focus = length(w);
		right = normalize(cross(vup, w)) * focus * tanf(fov / 2);
		up = normalize(cross(w, right)) * focus * tanf(fov / 2);
	}
	Ray getRay(int X, int Y) {
		vec3 dir = lookat + right * (2.0f * (X + 0.5f) / windowWidth - 1) + up * (2.0f * (Y + 0.5f) / windowHeight - 1) - eye;
		return Ray(eye, dir);
	}
	void Animate(float dt) {
		vec3 d = eye - lookat;
		eye = vec3(d.x * cos(dt) + d.z * sin(dt), d.y, -d.x * sin(dt) + d.z * cos(dt)) + lookat;
		set(eye, lookat, up, fov);
	}
};


struct Light {
	vec3 direction;
	vec3 Le;
	Light(vec3 _direction, vec3 _Le) {
		direction = normalize(_direction);
		Le = _Le;
	}
};


float rnd() { return (float)rand() / RAND_MAX; }

class Scene {
	std::vector<Intersectable *> objects;
	std::vector<Light *> lights;
	Camera camera;
	vec3 La;
public:
	void build() {
		vec3 eye = vec3(0, 1, 2), vup = vec3(0, 1, 0), lookat = vec3(0, 0, 0);
		float fov = 45 * M_PI / 180;
		camera.set(eye, lookat, vup, fov);

		La = vec3(0.01f, 0.01f, 0.01f);

		lights.push_back(new Light(vec3 (1, 1, 1), vec3(2, 2, 2)));

		Material * material = new Material(vec3(0.39f, 0.55f, 0.71f), vec3(2, 2, 2), 50);
		Material* planeMat = new Material(vec3(1.0f, 0.52f, 0.42f), vec3(2, 2, 2), 50);

		//sík
		objects.push_back(new Plane(vec3(0.0f, -0.3f, 0.0f), vec3(0, 1, 0), planeMat));
		//talp teteje
		objects.push_back(new CylinderCap(vec3(0.0f, -0.25f, 0.0f), vec3(0, 1, 0), 0.2f, material));
		//talp
		objects.push_back(new Cylinder(ScaleMatrix(vec3(-25.0f, 0.0f, -25.0f)), - 1.0f, 1.0f, vec3(0.0f, -0.3f, 0.0f), 0.05f, material, vec3(0, 1, 0)));
		// csukló 1
		objects.push_back(new Sphere(vec3(0.0f, -0.25f, 0.0f), 0.03f, material, vec3(0, 1, 0)));
		//rudi 1
		objects.push_back(new Cylinder(ScaleMatrix(vec3(-2500.0f, 0.0f, -2500.0f)), - 1.0f, 1.0f, vec3(0.0f, -0.3f, 0.0f), 0.3f, material, vec3(0, 1, 0)));
		//csukló 2
		objects.push_back(new Sphere(vec3(0.0f, 0.0f, 0.0f), 0.03f, material, vec3(0, 0, 1)));
		//rudi 2
		objects.push_back(new Cylinder(ScaleMatrix(vec3(-2500.0f, 0.0f, -2500.0f)), - 1.0f, 1.0f, vec3(0.0f, 0.0f, 0.0f), 0.3f, material, vec3(0, 0, 1)));
		//csukló 3
		objects.push_back(new Sphere(vec3(0.0f, 0.3f, 0.0f), 0.03f, material, vec3(0, 1, 1)));
		//búra
		objects.push_back(new Paraboloid(-0.2f, 0.1f, vec3(0.0f, 0.3f, -0.02f), material, vec3(0, 1, 1)));

	}

	void render(std::vector<vec4>& image) {
		for (int Y = 0; Y < windowHeight; Y++) {
#pragma omp parallel for
			for (int X = 0; X < windowWidth; X++) {
				vec3 color = trace(camera.getRay(X, Y));
				image[Y * windowWidth + X] = vec4(color.x, color.y, color.z, 1);
			}
		}
	}

	Hit firstIntersect(Ray ray) {
		Hit bestHit;
		for (Intersectable * object : objects) {
			Hit hit = object->intersect(ray); //  hit.t < 0 if no intersection
			if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t))  bestHit = hit;
		}
		if (dot(ray.dir, bestHit.normal) > 0) bestHit.normal = bestHit.normal * (-1);
		return bestHit;
	}

	bool shadowIntersect(Ray ray) {	// for directional lights
		for (Intersectable * object : objects) if (object->intersect(ray).t > 0) return true;
		return false;
	}
	
	vec3 trace(Ray ray, int depth = 0) {
		Hit hit = firstIntersect(ray);
		if (hit.t < 0) return La;
		vec3 outRadiance = hit.material->ka * La;
		for (Light * light : lights) {
			Ray shadowRay(hit.position + hit.normal * epsilon, light->direction);
			float cosTheta = dot(hit.normal, light->direction);
			if (cosTheta > 0 && !shadowIntersect(shadowRay)) {	// shadow computation
				outRadiance = outRadiance + light->Le * hit.material->kd * cosTheta;
				vec3 halfway = normalize(-ray.dir + light->direction);
				float cosDelta = dot(hit.normal, halfway);
				if (cosDelta > 0) outRadiance = outRadiance + light->Le * hit.material->ks * powf(cosDelta, hit.material->shininess);
			}
		}
		return outRadiance;
	}

	void Animate(float dt) {
		//camera.Animate(dt);
	}
};

GPUProgram gpuProgram; // vertex and fragment shaders
Scene scene;

// vertex shader in GLSL
const char *vertexSource = R"(
	#version 330
    precision highp float;

	layout(location = 0) in vec2 cVertexPosition;	// Attrib Array 0
	out vec2 texcoord;

	void main() {
		texcoord = (cVertexPosition + vec2(1, 1))/2;							// -1,1 to 0,1
		gl_Position = vec4(cVertexPosition.x, cVertexPosition.y, 0, 1); 		// transform to clipping space
	}
)";

// fragment shader in GLSL
const char *fragmentSource = R"(
	#version 330
    precision highp float;

	uniform sampler2D textureUnit;
	in  vec2 texcoord;			// interpolated texture coordinates
	out vec4 fragmentColor;		// output that goes to the raster memory as told by glBindFragDataLocation

	void main() {
		fragmentColor = texture(textureUnit, texcoord); 
	}
)";

class FullScreenTexturedQuad {
	unsigned int vao;	// vertex array object id and texture id
	Texture texture;
public:
	FullScreenTexturedQuad(int windowWidth, int windowHeight, std::vector<vec4>& image)
		: texture(windowWidth, windowHeight, image) 
	{
		glGenVertexArrays(1, &vao);	// create 1 vertex array object
		glBindVertexArray(vao);		// make it active

		unsigned int vbo;		// vertex buffer objects
		glGenBuffers(1, &vbo);	// Generate 1 vertex buffer objects

		// vertex coordinates: vbo0 -> Attrib Array 0 -> vertexPosition of the vertex shader
		glBindBuffer(GL_ARRAY_BUFFER, vbo); // make it active, it is an array
		float vertexCoords[] = { -1, -1,  1, -1,  1, 1,  -1, 1 };	// two triangles forming a quad
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_STATIC_DRAW);	   // copy to that part of the memory which is not modified 
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);     // stride and offset: it is tightly packed
	}

	void Draw() {
		glBindVertexArray(vao);	// make the vao and its vbos active playing the role of the data source
		gpuProgram.setUniform(texture, "textureUnit");
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);	// draw two triangles forming a quad
	}
};

FullScreenTexturedQuad * fullScreenTexturedQuad;

// Initialization, create an OpenGL context
void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
	scene.build();

	// create program for the GPU
	gpuProgram.create(vertexSource, fragmentSource, "fragmentColor");
}

// Window has become invalid: Redraw
void onDisplay() {

	std::vector<vec4> image(windowWidth * windowHeight);
	long timeStart = glutGet(GLUT_ELAPSED_TIME);
	scene.render(image);
	long timeEnd = glutGet(GLUT_ELAPSED_TIME);
	printf("Rendering time: %d milliseconds\n", (timeEnd - timeStart));

	// copy image to GPU as a texture
	fullScreenTexturedQuad = new FullScreenTexturedQuad(windowWidth, windowHeight, image);
	fullScreenTexturedQuad->Draw();
	glutSwapBuffers();									// exchange the two buffers
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {

}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) {
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
	cycle++;
	scene.Animate(0.5f);
	glutPostRedisplay();
}
