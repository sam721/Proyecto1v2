// modern.cpp: define el punto de entrada de la aplicación de consola.
//

#include <stdio.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "FileLoader.h"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <AntTweakBar.h>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <math.h>
#include <GL/GLU.h>
#define MAX(a,b) ((a>b)?a:b)
#include <fstream>
using namespace std;
using namespace glm;
const float PI = atan(1.0f)*4;
int s_width, s_height;
float global_move = 0.0, global_rotation = 0.0;
GLuint vbo_triangles, vbo_obj_vertices, vbo_obj_colors, ibo_obj_elements,vbo_obj_normals, vbo_obj_fnormals, ibo_obj_fnormals;
GLuint attribute_coord2d, attribute_coord3d, attribute_normal, attribute_fnormal, uniform_m_transform, uniform_eye;
GLint uniform_m, uniform_v, uniform_p;
GLuint uniform_lightpos, uniform_lightamb, uniform_lightdif, uniform_lightspec;
GLuint uniform_matamb, uniform_matdif, uniform_matspec, uniform_matshiny;
GLuint uniform_inverse_mv;
GLuint vbo_light_vertices, ibo_light_elements;
char buf[100010];

GLfloat light_vertices[] = {
    // front
    -0.05, -0.05,  0.05,
     0.05, -0.05,  0.05,
     0.05,  0.05,  0.05,
    -0.05,  0.05,  0.05,
    // back
    -0.05, -0.05, -0.05,
     0.05, -0.05, -0.05,
     0.05,  0.05, -0.05,
    -0.05,  0.05, -0.05,
  };
  
  GLushort light_elements[] = {
    // front
    0, 1, 2,
    2, 3, 0,
    // top
    3, 2, 6,
    6, 7, 3,
    // back
    7, 6, 5,
    5, 4, 7,
    // bottom
    4, 5, 1,
    1, 0, 4,
    // left
    4, 0, 3,
    3, 7, 4,
    // right
    1, 5, 6,
    6, 2, 1,
  };
/*Programs for shading*/
GLuint phongflat,
	   phonggouraud,
	   phongphong,
	   blinnflat,
	   blinngouraud,
	   blinnphong,
	   cookflat,
	   cookgouraud,
	   cookphong,
	   ambientshader;
class Light{
public:
	vec4 ambient, diffuse, specular;
	vec3 position;
	int light_model;
	Light(){
		position = vec3(0.0, 0.0, 0.0);
		ambient = vec4(1.0, 1.0, 1.0, 1.0);
		diffuse = vec4(1.0, 0.25, 0.74, 1.0);
		specular = vec4(1.0, 1.0, 1.0, 1.0);
	}
	Light(int a){
		light_model = a;
	}
	Light(int a, vec4 p, vec4 A, vec4 d, vec4 s){
		//light_model = a; ambient = A; diffuse = d; specular = s;
	//	position = vec4(0.0, 0.0, 0.0, 1.0);
		ambient = vec4(1.0, 1.0, 1.0, 1.0);
		diffuse = vec4(1.0, 0.25, 0.74, 1.0);
		specular = vec4(1.0, 1.0, 1.0, 1.0);
	}
};
class object_3d{
public:
	vector<vec3> obj_vertices;
	vector<vec3> normals, fnormals;
	vector<GLushort> elements;
	vector<GLushort> fids;
	vector<int> shared_counter;
	vec3 centroid, normal;
	mat4 scale, translation;
	int shade_model, reflect_model;
	float angle, max_norm;
	float angle_x, angle_y, angle_z;
	float scale_x, scale_y, scale_z;
	quat q;
	vec4 ambient, diffuse, specular;
	float shininess;
	object_3d(){
		scale = mat4(1.0f);
		angle = 0.0f;
		translation = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));
		ambient = vec4(0.0, 0.0, 0.2, 1.0); diffuse =  vec4(0.0, 0.0, 1.0, 1.0); specular =  vec4(1.0, 1.0, 1.0, 1.0);
		shininess = 1.0;
		angle_x = angle_y = angle_z = 0.0;
		scale_x = scale_y = scale_z = 1.0;
		q = quat(1.0, 0.0, 0.0, 0.0);
	}
	object_3d(int a, int b, vec4 A, vec4 D, vec4 S, float s){
		scale = mat4(1.0f);
		angle = 0.0f;
		translation = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));
		shade_model = a; reflect_model = b;
		ambient = vec4(0.0, 0.0, 0.2, 1.0); diffuse =  vec4(0.0, 0.0, 1.0, 1.0); specular =  vec4(1.0, 1.0, 1.0, 1.0);
		shininess = 1.0;
		angle_x = angle_y = angle_z = 0.0;
	}
	float norm_inf(vec3 v){
		return abs(v.x)+abs(v.y)+abs(v.z);
	}
	void load_obj(char* file_name){
		int vertices, faces; vertices = faces = 0;
		ifstream in(file_name, ios::in);
		string line;
		char type[3];
		float v1, v2, v3;
		int f1, f2, f3;
		centroid = vec3(0.0f, 0.0f, 0.0f);
		while(getline(in,line)){
			if(line.substr(0,2)=="v "){
				istringstream s(line.substr(2));
				s>>v1>>v2>>v3;
				obj_vertices.push_back(vec3(v1,v2,v3));
				normals.push_back(vec3(0.0,0.0,0.0));
				shared_counter.push_back(0);
				vertices++;
				centroid = centroid+vec3(v1,v2,v3);
			}else if(line.substr(0,2)=="f "){
				istringstream s(line.substr(2));
				string s1, s2, s3;
				s>>s1>>s2>>s3;
				size_t r = s1.find("/");
				int pos;
				if(r==string::npos) pos = s1.length();
				else pos = (int)r;
				f1 = atoi(s1.substr(0,r).c_str());
				r = s2.find("/");
				if(r==string::npos) pos = s2.length();
				else pos = (int)r;
				f2 = atoi(s2.substr(0,pos).c_str());
				r = s3.find("/");
				if(r==string::npos) pos = s3.length();
				else pos = (int)r;
				f3 = atoi(s3.substr(0,pos).c_str());
				f1--; f2--; f3--;
				elements.push_back(f1); elements.push_back(f2); elements.push_back(f3);
				normal = normalize(cross(obj_vertices[f2]-obj_vertices[f1], obj_vertices[f3]-obj_vertices[f1]));
				fnormals.push_back(normal);
				fids.push_back(faces); fids.push_back(faces); fids.push_back(faces);
				faces++;
				normals[f1]+=normal; normals[f2]+=normal; normals[f3]+=normal;
				shared_counter[f1]++; shared_counter[f2]++; shared_counter[f3]++;
				faces++;
			}
		}
		centroid = centroid*(1.0f/vertices);
		max_norm = -1;
	//	for(int i=0; i<vertices; i++) obj_vertices[i] -= centroid;
	//	for(int i=0; i<vertices; i++) normalize(obj_vertices[i]);
		for(int i=0; i<vertices; i++) max_norm = MAX(max_norm, norm_inf(obj_vertices[i]));
	//	printf("%f\n", max_norm);
	//	for(int i=0; i<vertices; i++) obj_vertices[i] = obj_vertices[i]*(1.0f/max_norm);
		//for(int i=0; i<vertices; i++) normals[i] = normals[i]*(1.0f/max_norm);
		for(int i=0; i<vertices; i++){
			normals[i] = normals[i]*(1.0f/shared_counter[i]);
			normals[i] = normalize(normals[i]);
			if(shared_counter[i]==1) printf("YES %d\n", i);
		}
	}
	void load_off(char* file_name)
	{
		int vertices, faces, lines;
		FILE* f = fopen(file_name, "r");
		fscanf(f, "%*s");
		float v1, v2, v3; int v;
		int f1, f2, f3;
		fscanf(f, "%d %d %d", &vertices, &faces, &lines);
		printf("%d %d %d\n", vertices, faces, lines);
		centroid = vec3(0.0);
		for(int i=0; i<vertices; i++){
			fscanf(f, "%f %f %f", &v1, &v2, &v3);
			obj_vertices.push_back(vec3(v1,v2,v3));
			normals.push_back(vec3(0.0));
			shared_counter.push_back(0.0);
			centroid = centroid+obj_vertices[i];
			//printf("%d %f %f %f\n", v, v1, v2, v3);
		}
		for(int i=0; i<faces; i++){
			fscanf(f, "%d %d %d %d", &v, &f1, &f2, &f3);
			//printf("%d %d %d\n", f1, f2, f3);
			elements.push_back(f1); elements.push_back(f2); elements.push_back(f3);
			shared_counter[f1]++; shared_counter[f2]++; shared_counter[f3]++;
			normal = normalize(cross(obj_vertices[f2]-obj_vertices[f1], obj_vertices[f3]-obj_vertices[f1]));
			normals[f1]+=normal; normals[f2]+=normal; normals[f3]+=normal;
		}
		centroid = centroid*(1.0f/vertices);
		max_norm = -1;
	//	for(int i=0; i<vertices; i++) obj_vertices[i] -= centroid;
	//	for(int i=0; i<vertices; i++) normalize(obj_vertices[i]);
		for(int i=0; i<vertices; i++) max_norm = MAX(max_norm, norm_inf(obj_vertices[i]));
	//	printf("%f\n", max_norm);
	//	for(int i=0; i<vertices; i++) obj_vertices[i] = obj_vertices[i]*(1.0f/max_norm);
		//for(int i=0; i<vertices; i++) normals[i] = normals[i]*(1.0f/max_norm);
		for(int i=0; i<vertices; i++){
			normals[i] = normals[i]*(1.0f/shared_counter[i]);
			normalize(normals[i]);
			if(shared_counter[i]==1) printf("YES %d\n", i);
		}
	}
};
object_3d obj; Light L;
vec3 camera_pos, camera_center;
/*
void load_off(char* file_name)
{
	int vertices, faces, lines;
	FILE* f = fopen(file_name, "r");
	fscanf(f, "%*s");
	float v1, v2, v3; int v;
	int f1, f2, f3;
	fscanf(f, "%d %d %d", &vertices, &faces, &lines);
	printf("%d %d %d\n", vertices, faces, lines);
	for(int i=0; i<vertices; i++){
		fscanf(f, "%f %f %f", &v1, &v2, &v3);
		obj_vertices.push_back(vec4(v1,v2,v3,1.0f));
		//printf("%d %f %f %f\n", v, v1, v2, v3);
	}
	for(int i=0; i<faces; i++){
		fscanf(f, "%d %d %d %d", &v, &f1, &f2, &f3);
		//printf("%d %d %d\n", f1, f2, f3);
		elements.push_back(f1); elements.push_back(f2); elements.push_back(f3);
	}	
}
*/
int light_model, shading_model;
GLuint sh_ptrs[10][10];
void draw_obj(int ind, int lm, int sm){
	GLuint program = sh_ptrs[lm][sm];
	glUseProgram(program);
	const char* attribute_name = "coord3d";
	const char* fnormals_name = "fnormal";
	const char* eye_name = "eye";
	uniform_m = glGetUniformLocation(program, "m");
	uniform_v = glGetUniformLocation(program, "v");
	uniform_p = glGetUniformLocation(program, "p");
//	if(uniform_m==-1) printf("M FAIL\n");
//	if(uniform_v==-1) printf("V FAIL\n");
//	if(uniform_p==-1) printf("P FAIL\n");
	uniform_eye = glGetUniformLocation(program, eye_name);
	//if(uniform_eye==-1) printf("EYE FAIL\n");
	attribute_coord3d = glGetAttribLocation(program, attribute_name);
	//if(attribute_coord3d==-1) printf("FAIL\n");
	attribute_normal = glGetAttribLocation(program, "normal");
	//attribute_fnormal = glGetAttribLocation(program, fnormals_name);
	//if(attribute_fnormal==-1) printf("FAIL %s\n", fnormals_name);
	uniform_lightpos = glGetUniformLocation(program, "lightPosition");
	uniform_lightamb = glGetUniformLocation(program, "lightAmbient");
	uniform_lightdif = glGetUniformLocation(program, "lightDiffuse");
	uniform_lightspec = glGetUniformLocation(program, "lightSpecular");
//	if(uniform_lightpos==-1 || uniform_lightamb==-1 || uniform_lightdif==-1 || uniform_lightspec==-1) printf("FAIL\n");
	uniform_matamb = glGetUniformLocation(program, "materialAmbient");
	uniform_matdif = glGetUniformLocation(program, "materialDiffuse");
	uniform_matspec = glGetUniformLocation(program, "materialSpecular");
	uniform_inverse_mv = glGetUniformLocation(program, "normalmatrix");
	uniform_matshiny = glGetUniformLocation(program, "materialShininess");
//	if(uniform_matamb==-1 || uniform_matdif==-1 || uniform_matspec==-1 || uniform_matshiny==-1) printf("FAIL\n");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_obj_elements);
	mat4 model, view, projection;
	model = translate(mat4(1.0f), vec3(0.0,0.0,-4.0f));
	model = model*toMat4(obj.q);
	model = scale(model, vec3(obj.scale_x, obj.scale_y, obj.scale_z));
	model = scale(model, vec3(2.0));
	model = scale(model, vec3(1/obj.max_norm));
	model = translate(model, -obj.centroid);
	view = lookAt(camera_pos, camera_center, vec3(0.0, 1.0, 0.0));
	projection = perspective(45.0f, 1.0f*s_width/s_height, 1.0f, 10.0f);
	mat3 normalmatrix = transpose(inverse(mat3(view*model)));
	glUniformMatrix4fv(uniform_m, 1, GL_FALSE, value_ptr(model));
	glUniformMatrix4fv(uniform_v, 1, GL_FALSE, value_ptr(view));
	glUniformMatrix4fv(uniform_p, 1, GL_FALSE, value_ptr(projection));
	glUniform3fv(uniform_eye, 1, value_ptr(camera_pos));

	glUniformMatrix3fv(uniform_inverse_mv, 1, GL_FALSE, value_ptr(normalmatrix));
	glUniform3fv(uniform_lightpos, 1, value_ptr(L.position));
	glUniform4fv(uniform_lightamb, 1, value_ptr(L.ambient));
	glUniform4fv(uniform_lightdif, 1, value_ptr(L.diffuse));
	glUniform4fv(uniform_lightspec, 1, value_ptr(L.specular));

	glUniform4fv(uniform_matamb, 1, value_ptr(obj.ambient));
	glUniform4fv(uniform_matdif, 1, value_ptr(obj.diffuse));
	glUniform4fv(uniform_matspec, 1, value_ptr(obj.specular));
	glUniform1f(uniform_matshiny, obj.shininess);


	glBindBuffer(GL_ARRAY_BUFFER, vbo_obj_vertices);
	glEnableVertexAttribArray(attribute_coord3d);
	glVertexAttribPointer(
		attribute_coord3d,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
		);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_obj_normals);
	glEnableVertexAttribArray(attribute_normal);
	glVertexAttribPointer(
		attribute_normal,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
		);
	/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_obj_fnormals);
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size2);
	glEnableVertexAttribArray(attribute_fnormal);
	glVertexAttribPointer(
		attribute_fnormal,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
		);
		*/	
	int size;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_obj_elements);
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
	glDisableVertexAttribArray(attribute_coord3d);
	glDisableVertexAttribArray(attribute_normal);
//	glDisableVertexAttribArray(attribute_fnormal);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void draw_sphere(){
	GLuint program = ambientshader;
	glUseProgram(program);
	mat4 model, view, projection;
	model = translate(mat4(1.0f), vec3(L.position.x,L.position.y,L.position.z));
	view = lookAt(camera_pos, camera_center, vec3(0.0, 1.0, 0.0));
	projection = perspective(45.0f, 1.0f*s_width/s_height, 1.0f, 10.0f);

	uniform_m = glGetUniformLocation(program, "m");
	uniform_v = glGetUniformLocation(program, "v");
	uniform_p = glGetUniformLocation(program, "p");
	uniform_lightamb = glGetUniformLocation(program, "lightAmbient");
	attribute_coord3d = glGetAttribLocation(program, "coord3d");
	if(uniform_lightamb==-1) printf("FAIL 1\n");
	if(attribute_coord3d==-1) printf("FAIL 2\n");
	glUniformMatrix4fv(uniform_m, 1, GL_FALSE, value_ptr(model));
	glUniformMatrix4fv(uniform_v, 1, GL_FALSE, value_ptr(view));
	glUniformMatrix4fv(uniform_p, 1, GL_FALSE, value_ptr(projection));
	glUniform4fv(uniform_lightamb, 1, value_ptr(L.ambient));
	glBindBuffer(GL_ARRAY_BUFFER, vbo_light_vertices);
	glEnableVertexAttribArray(attribute_coord3d);
	glVertexAttribPointer(
		attribute_coord3d,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
		);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_light_elements);
	int size;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
	glDisableVertexAttribArray(attribute_coord3d);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void display(){
	glClearColor(0.0,0.0,0.0,0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw_obj(0, light_model, shading_model);
	draw_sphere();
	TwDraw();
	glutSwapBuffers();
	glutPostRedisplay();
}
void shaders(char* vs_name, char* fs_name, GLuint &program)
{
	const char* vs_source = file_read(vs_name), *fs_source = file_read(fs_name);
	GLint compile_ok = GL_FALSE, link_ok = GL_FALSE;
	GLuint vs = glCreateShader(GL_VERTEX_SHADER), fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vs, 1, &vs_source, NULL);
	glCompileShader(vs);
	char errbuf[4096];
	GLsizei len;
	glGetShaderInfoLog(vs, sizeof(errbuf), &len, errbuf);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &compile_ok);
	printf("H %s\n", errbuf);
	if(!compile_ok){
		fprintf(stderr, "Error in Vertex shader\n");
		//return;
	}
	glShaderSource(fs, 1, &fs_source, NULL);
	glCompileShader(fs);
	compile_ok = GL_FALSE;
	glGetShaderInfoLog(fs, sizeof(errbuf), &len, errbuf);
	printf("%s\n", errbuf);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &compile_ok);
	if(!compile_ok){
		fprintf(stderr, "Error in fragment shader\n");
		return;
	}
	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramInfoLog(program, sizeof(errbuf), &len, errbuf); 
	printf("%s\n", errbuf);
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if(!link_ok) printf("%s\n", errbuf);
}
void init_shaders(){
	shaders("Phong-Flat.vs","Phong-Flat.fs", phongflat);
	shaders("phong-gouraud.vs","phong-gouraud.fs", phonggouraud);
	shaders("phong-phong.vs","phong-phong.fs", phongphong);
	shaders("blinn-Flat.vs","blinn-Flat.fs", blinnflat);
	shaders("blinn-gouraud.vs","blinn-gouraud.fs", blinngouraud);
	shaders("blinn-phong.vs" ,"blinn-phong.fs" , blinnphong);
	shaders("cook-flat.vs", "cook-flat.fs", cookflat);
	shaders("cook-gouraud.vs", "cook-gouraud.fs", cookgouraud);
	shaders("cook-phong.vs", "cook-phong.fs", cookphong);
	shaders("ambientshader.vs", "ambientshader.fs", ambientshader);
	sh_ptrs[0][0] = phongflat;
	sh_ptrs[0][1] = phonggouraud;
	sh_ptrs[0][2] = phongphong;
	sh_ptrs[1][0] = blinnflat;
	sh_ptrs[1][1] = blinngouraud;
	sh_ptrs[1][2] = blinnphong;
	sh_ptrs[2][0] = cookflat;
	sh_ptrs[2][1] = cookgouraud;
	sh_ptrs[2][2] = cookphong;
}
void data(){
	obj.load_obj("figure.obj");
	glGenBuffers(1, &vbo_obj_normals);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_obj_normals);
	glBufferData(GL_ARRAY_BUFFER, obj.normals.size()*sizeof(vec3), &obj.normals[0], GL_STATIC_DRAW); 
	
	glGenBuffers(1, &vbo_obj_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_obj_vertices);
	glBufferData(GL_ARRAY_BUFFER, obj.obj_vertices.size()*sizeof(vec3), &obj.obj_vertices[0], GL_STATIC_DRAW);

	/*
	glGenBuffers(1, &vbo_light_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_light_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(light_vertices), light_vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	*/
	//glGenBuffers(1, &vbo_obj_fnormals);
	//glBindBuffer(GL_ARRAY_BUFFER, vbo_obj_fnormals);
	//glBufferData(GL_ARRAY_BUFFER, obj.fnormals.size()*sizeof(vec3), &obj.fnormals[0], GL_STATIC_DRAW);

	//glGenBuffers(1, &vbo_light_vertices);
	//glBindBuffer(GL_ARRAY_BUFFER, vbo_light_vertices);
	//glBufferData(GL_ARRAY_BUFFER,

	glGenBuffers(1, &ibo_obj_elements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_obj_elements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*obj.elements.size(), &obj.elements[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//glGenBuffers(1, &ibo_obj_fnormals);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_obj_fnormals);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*obj.fids.size(), &obj.fids[0], GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glGenBuffers(1, &vbo_light_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_light_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(light_vertices), light_vertices, GL_STATIC_DRAW);
	
	glGenBuffers(1, &ibo_light_elements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_light_elements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(light_elements), light_elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
}
mat4 model, view, projection;
void rotationIdle(){
	//float move = sinf(glutGet(GLUT_ELAPSED_TIME) / 1000.0 * (2*3.14) / 5); // -1<->+1 every 5 seconds
	//float angle = glutGet(GLUT_ELAPSED_TIME) / 1000.0 * 45;  // 45° per second
	//glUseProgram(program);
	//vec3 axis_z(0.0,1.0,0.0);
	//mat4 m_transform = /*mat4(1.0f);*/translate(mat4(1.0f), vec3(move,0.0,0.0)) * rotate(mat4(1.0f), angle, axis_z);
	//glUniformMatrix4fv(uniform_m_transform, 1, GL_FALSE, value_ptr(m_transform));
	/*
	glUseProgram(program);
	float angle = glutGet(GLUT_ELAPSED_TIME) / 1000.0 * 45;  // 45° per second
	vec3 axis_z(0.0, 0.0, 1.0);
	mat4 anim = glm::rotate(glm::mat4(1.0f), angle, axis_z);
	model = translate(mat4(1.0f), vec3(0.0,0.0,-4.0f));
	view = lookAt(vec3(0.0, 2.0, 0.0), vec3(0.0, 0.0, -4.0), vec3(0.0, 1.0, 0.0));
	projection = perspective(45.0f, 1.0f*s_width/s_height, 1.0f, 10.0f);
	mat4 mvp = projection*view*model*anim;
	//glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, value_ptr(mvp));
	glutPostRedisplay();
	*/
}
void reshape(int width, int height)
{
	s_width = width; s_height = height;
	glViewport(0,0,s_width,s_height);
	TwWindowSize(s_width,s_height);
}
/*
void moving_camera(int x, int y){
	float xx = 1.0f*x, yy = 1.0f*y;
	camera_center = vec3(xx, yy, camera_center.z);
	glutPostRedisplay();
	printf("%f %f %f\n", camera_center.x, camera_center.y, camera_center.z);
}
*/
float hor_angle, ver_angle;
float X, Y, Z;
void InitOpenGL()
{
	glEnable(GL_DEPTH_TEST);
	camera_pos = vec3(0.0, 0.0, 0.0);
	camera_center = vec3(0.0, 0.0, -4.0);
	light_model = 0; shading_model = 1;
	hor_angle = ver_angle = 0.0f;
	X = 0.0f; Y = 0.0f; Z = 1.0f;
}
vec3 orig_center_vision = vec3(0.0, 0.0, -4.0);
void keyboard(unsigned char key, int x, int y){
	if(key=='1') light_model = 0;
	else if(key=='2') light_model = 1;
	else if(key=='3') light_model = 2;
	else if(key=='q') shading_model = 0;
	else if(key=='w') shading_model = 1;
	else if(key=='e') shading_model = 2;
	else if(key=='n'){
		ver_angle-=1;
		float radh = 2*PI*hor_angle/360.0f, radv = 2*PI*ver_angle/360.0f;
		X = sin(radh)*cos(radv);
		Y = sin(radh)*sin(radv);
		Z = cos(radh);
		camera_center = vec3(rotate(mat4(1.0f), -1.0f, vec3(1,0,0))*vec4(camera_center-camera_pos,1.0))+camera_pos;
	}else if(key=='m'){
		ver_angle+=1;
		float radh = 2*PI*hor_angle/360.0f, radv = 2*PI*ver_angle/360.0f;
		X = sin(radh)*cos(radv);
		Y = sin(radh)*sin(radv);
		Z = cos(radh);
		camera_center = vec3(rotate(mat4(1.0f), 1.0f, vec3(1,0,0))*vec4(camera_center-camera_pos,1.0))+camera_pos;
	}else if(key=='f'){
	}
	//display();
}
void specialkeys(int key, int x, int y){
	vec3 axis_x = vec3(1.0, 0.0, 0.0), axis_y = vec3(0.0, 1.0, 0.0), axis_z = vec3(0.0, 0.0, 1.0);
	if(key==GLUT_KEY_UP){
		camera_pos.x-=0.1*X; camera_pos.y-=0.1*Y; camera_pos.z-=0.1*Z;
		camera_center.x-=0.1*X; camera_center.y-=0.1*Y; camera_center.z-=0.1*Z;
	}else if(key==GLUT_KEY_DOWN){
		camera_pos.x+=0.1*X; camera_pos.y+=0.1*Y; camera_pos.z+=0.1*Z;
		camera_center.x+=0.1*X; camera_center.y+=0.1*Y; camera_center.z+=0.1*Z;
	}else if(key==GLUT_KEY_LEFT){
		hor_angle+=1;
		float radh = 2*PI*hor_angle/360.0f, radv = 2*PI*ver_angle/360.0f;
		X = sin(radh)*cos(radv);
		Y = sin(radh)*sin(radv);
		Z = cos(radh);
		camera_center = vec3(rotate(mat4(1.0f), 1.0f, vec3(0,1,0))*vec4(camera_center-camera_pos,1.0))+camera_pos;
		//camera_pos.x-=0.1;
		//camera_center.x-=0.1;
		//mat4 rot = rotate(mat4(1.0f), angle_rotation, axis_y);
		//camera_center = vec3(rot*vec4(camera_center, 1.0));
	}else if(key==GLUT_KEY_RIGHT){
		hor_angle-=1;
		float radh = 2*PI*hor_angle/360.0f, radv = 2*PI*ver_angle/360.0f;
		X = sin(radh)*cos(radv);
		Y = sin(radh)*sin(radv);
		Z = cos(radh);
		camera_center = vec3(rotate(mat4(1.0f), -1.0f, vec3(0,1,0))*vec4(camera_center-camera_pos,1.0))+camera_pos;
		//camera_pos.x+=0.1;
		//camera_center.x+=0.1;
	//	mat4 rot = rotate(mat4(1.0f), -angle_rotation, axis_y);
		//camera_center = vec3(rot*vec4(camera_center, 1.0));
	}
	printf("%f\n", hor_angle);
//	display();
}
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	TwBar* bar;
	s_width = 800; s_height = 600;
	glutInitWindowSize(s_width, s_height);
	glutCreateWindow("Mi ventana");
	InitOpenGL();
	TwGLUTModifiersFunc(glutGetModifiers);
	TwInit(TW_OPENGL, NULL);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc((GLUTmousebuttonfun)TwEventMouseButtonGLUT);
	glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	//glutIdleFunc(rotationIdle);
	glutSpecialFunc(specialkeys);
	
	//glutPassiveMotionFunc(moving_camera);
	GLenum glew_status = glewInit();
	if(glew_status != GLEW_OK){	
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
		return 0;
	}else printf("NO ERROR\n");
	init_shaders();
	data();
	
	bar = TwNewBar("TweakBar");
	TwDefine("TweakBar size = '200 200' ");
	TwAddButton(bar, "---Propiedades material---", NULL, NULL, "");

	TwAddVarRW(bar, "Material Ambient ", TW_TYPE_COLOR4F, &obj.ambient, "");
	TwAddVarRW(bar, "Material Diffuse ", TW_TYPE_COLOR4F, &obj.diffuse, "");
	TwAddVarRW(bar, "Material Specular ", TW_TYPE_COLOR4F, &obj.specular, "");
	TwAddVarRW(bar, "Material Shininess ", TW_TYPE_FLOAT, &obj.shininess, "");

	TwAddVarRW(bar, "Rotation ", TW_TYPE_QUAT4F, &obj.q, "");

	TwAddVarRW(bar, "Material Scale X ", TW_TYPE_FLOAT, &obj.scale_x, "step = 0.1");
	TwAddVarRW(bar, "Material Scale Y ", TW_TYPE_FLOAT, &obj.scale_y, "step = 0.1");
	TwAddVarRW(bar, "Material Scale Z ", TW_TYPE_FLOAT, &obj.scale_z, "step = 0.1");

	TwAddButton(bar, "---Propiedades luz---", NULL, NULL, "");

	TwAddVarRW(bar, "Light Ambient ", TW_TYPE_COLOR4F, &L.ambient, "");
	TwAddVarRW(bar, "Light Diffuse ", TW_TYPE_COLOR4F, &L.diffuse, "");
	TwAddVarRW(bar, "Light Specular ", TW_TYPE_COLOR4F, &L.specular, "");
	TwAddVarRW(bar, "Light Position X ", TW_TYPE_FLOAT, &L.position.x, "step = 0.01");
	TwAddVarRW(bar, "Light Position Y ", TW_TYPE_FLOAT, &L.position.y, "step = 0.01");
	TwAddVarRW(bar, "Light Position Z ", TW_TYPE_FLOAT, &L.position.z, "step = 0.01");

	glutMainLoop();
	return 0;
}

