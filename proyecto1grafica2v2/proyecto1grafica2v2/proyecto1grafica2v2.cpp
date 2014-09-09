// modern.cpp: define el punto de entrada de la aplicación de consola.
//

#include <stdio.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "FileLoader.h"

#include <iostream>
#define GLM_SWIZZLE
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
//#include <stdafx.h>
#define MIN(a,b) ((a<b)?a:b)
#define MAX(a,b) ((a>b)?a:b)
#include <fstream>
using namespace std;
using namespace glm;
typedef enum{ FLAT, GOURAUD, PHONG } shade_model;
typedef enum{ PHONGE, BLINN, COOK } reflection_model;
typedef enum{ POINTE, DIRECTIONAL, SPOTLIGHT, OFF} light_type;
bool draw_bounding = false;
const float PI = atan(1.0f)*4;
int s_width, s_height;
float global_move = 0.0, global_rotation = 0.0;
GLuint vbo_triangles;
vector<GLuint> vbo_obj_vertices, ibo_obj_elements, vbo_obj_normals;
GLuint attribute_coord2d, attribute_coord3d, attribute_normal, attribute_fnormal, uniform_m_transform, uniform_eye;
GLint uniform_m, uniform_v, uniform_p;
GLuint uniform_lightpos[5], uniform_lightdir[5], uniform_lightamb[5], uniform_lightdif[5], uniform_lightspec[5], uniform_spotdir[5],
uniform_spotexp[5], uniform_spotcutoff[5];
GLuint uniform_matamb, uniform_matdif, uniform_matspec, uniform_matshiny;
GLuint lightype[5];
int figure, light;
GLuint uniform_inverse_mv;
GLuint vbo_light_vertices, ibo_light_elements;
char buf[100010];
const int LMAX = 3;
pair<int,int> click;
bool pick;
i8vec3 obj_id_colors[12];
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
vec4 lAmbient[5], lDiffuse[5], lSpecular[5];
vec3 lPosition[5], lDirection[5], spotdirection[5];
float spotcutoff[5], spotexponent[5];
int l[5];
/*
class Light{
public:
	vec4 ambient, diffuse, specular;
	vec3 position, direction;
	float spotcutoff, spotexponent;
	vec3 spotdirection;
	int light_model;
	light_type l;
	Light(){
		position = vec3(0.0, 0.0, 0.0);
		ambient = vec4(1.0, 1.0, 1.0, 1.0);
		diffuse = vec4(1.0, 0.25, 0.74, 1.0);
		specular = vec4(1.0, 1.0, 1.0, 1.0);
		direction = vec3(0.0, 1.0, 1.0);
		spotdirection = vec3(0.0, 1.0, 1.0);
		spotcutoff = 0.0;
		spotexponent = 1.0;
		l = POINT;
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
*/
GLushort bounding_elements[] = 
	{0, 2, 3, 1,  
	 1, 3, 7, 5,  
	 5, 7, 6, 4,  
	 4, 6, 2, 0,  
	 4, 0, 1, 5,  
	 7, 3, 2, 6};
class object_3d{
public:
	vector<vec3> obj_vertices;
	vector<vec3> normals, fnormals;
	vector<GLushort> elements;
	vector<GLushort> fids;
	vector<int> shared_counter;
	vector<vec3> bounding_box;
	vec3 centroid, normal;
	GLuint vbov, vbon, ibo, vbob, ibob;
	float angle, max_norm;
	string name;
	vec3 scale, trans;
	quat q;
	vec4 ambient, diffuse, specular;
	float shininess;
	shade_model s;
	reflection_model r;
	float min_x, min_y, min_z, max_x, max_y, max_z;
	object_3d(){
	}
	object_3d(string _n){
		name = _n;
		ambient = vec4(0.0, 0.0, 0.2, 1.0); diffuse =  vec4(0.0, 0.0, 1.0, 1.0); specular =  vec4(1.0, 1.0, 1.0, 1.0);
		shininess = 1.0;
		scale = vec3(1.0);
		trans = vec3(0.0);
		q = quat(1.0, 0.0, 0.0, 0.0);
		s = GOURAUD;
		r = PHONGE;
		if(_n=="floor.obj"){
			ambient = vec4(0.2, 0.2, 0.2, 1.0);
			diffuse = vec4(0.9, 0.9, 0.9, 1.0);
			s = PHONG;
			//r = PHONGE;
		}
		min_x = min_y = min_z = 1e10;
		max_x = max_y = max_z = -1e10;
	}
	object_3d(int a, int b, vec4 A, vec4 D, vec4 S, float s){
		ambient = vec4(0.0, 0.0, 0.2, 1.0); diffuse =  vec4(0.0, 0.0, 1.0, 1.0); specular =  vec4(1.0, 1.0, 1.0, 1.0);
		shininess = 1.0;
	}
	float norm_inf(vec3 v){
		return abs(v.x)+abs(v.y)+abs(v.z);
	}
	void load_obj(const char* file_name){
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
				min_x = MIN(min_x, v1); min_y = MIN(min_y, v2); min_z = MIN(min_z, v3);
				max_x = MAX(max_x, v1); max_y = MAX(max_y, v2); max_z = MAX(max_z, v3);
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
		if(name!="floor.obj") for(int i=0; i<vertices; i++) max_norm = MAX(max_norm, norm_inf(obj_vertices[i]));
		else max_norm = 1;
	//	printf("%f\n", max_norm);
	//	for(int i=0; i<vertices; i++) obj_vertices[i] = obj_vertices[i]*(1.0f/max_norm);
		//for(int i=0; i<vertices; i++) normals[i] = normals[i]*(1.0f/max_norm);
		for(int i=0; i<vertices; i++){
			normals[i] = normals[i]*(1.0f/shared_counter[i]);
			normals[i] = normalize(normals[i]);
			if(shared_counter[i]==1) printf("YES %d\n", i);
		}
		bounding_box.push_back(vec3(min_x, max_y, max_z)); //0
		bounding_box.push_back(vec3(max_x, max_y, max_z)); //1
		bounding_box.push_back(vec3(min_x, min_y, max_z)); //2
		bounding_box.push_back(vec3(max_x, min_y, max_z)); //3
		bounding_box.push_back(vec3(min_x, max_y, min_z)); //4
		bounding_box.push_back(vec3(max_x, max_y, min_z)); //5
		bounding_box.push_back(vec3(min_x, min_y, min_z)); //6
		bounding_box.push_back(vec3(max_x, min_y, min_z)); //7

	}
	void load_off(const char* file_name)
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
vec3 camera_pos, camera_center;
vector<object_3d> figures;
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
string posnames[3] = {"lightPos1", "lightPos2", "lightPos3"};
string dirnames[3] = {"lightDir1", "lightDir2", "lightDir3"};
string ambnames[3] = {"lightAmb1", "lightAmb2", "lightAmb3"};
string difnames[3] = {"lightDif1", "lightDif2", "lightDif3"};
string specnames[3] = {"lightSpec1", "lightSpec2", "lightSpec3"};
string sdirname[3] = {"spotDir1", "spotDir2", "spotDir3"};
string sexpname[3] = {"spotExp1", "spotExp2", "spotExp3"};
string scutname[3] = {"spotCutOff1", "spotCutOff2", "spotCutOff3"};
string ltname[3] = {"lightType1", "lightType2", "lightType3"};
void link_uniforms_and_attributes(int program){
	const char* attribute_name = "coord3d";
	const char* fnormals_name = "fnormal";
	const char* eye_name = "eye";
	uniform_m = glGetUniformLocation(program, "m");
	uniform_v = glGetUniformLocation(program, "v");
	uniform_p = glGetUniformLocation(program, "p");
	uniform_eye = glGetUniformLocation(program, eye_name);
	
	attribute_coord3d = glGetAttribLocation(program, attribute_name);
	attribute_normal = glGetAttribLocation(program, "normal");

	for(int i=0; i<LMAX; i++){
		uniform_lightpos[i] = glGetUniformLocation(program, posnames[i].c_str());
		uniform_lightdir[i] = glGetUniformLocation(program, dirnames[i].c_str());
		uniform_lightamb[i] = glGetUniformLocation(program, ambnames[i].c_str());
		uniform_lightdif[i] = glGetUniformLocation(program, difnames[i].c_str());
		uniform_lightspec[i] = glGetUniformLocation(program, specnames[i].c_str());
		uniform_spotdir[i] = glGetUniformLocation(program, sdirname[i].c_str());
		uniform_spotexp[i] = glGetUniformLocation(program, sexpname[i].c_str());
		uniform_spotcutoff[i] = glGetUniformLocation(program, scutname[i].c_str());
		lightype[i] = glGetUniformLocation(program, ltname[i].c_str());
		if(uniform_lightpos[i]==-1 || uniform_lightdir[i]==-1 || uniform_lightamb[i]==-1 || uniform_lightdif[i]==-1 || uniform_lightspec[i]==-1) printf("FAIL\n");
		if(uniform_spotdir[i]==-1 || uniform_spotexp[i]==-1 || uniform_spotcutoff[i]==-1 || lightype[i]==-1) printf("FAIL\n");
	}
		/*
	uniform_lightpos[0] = glGetUniformLocation(program, "lightPosition");
//	if(uniform_lightpos==-1) printf("FAIL\n");
	uniform_lightdir[0] = glGetUniformLocation(program, "lightDirection");
	//if(uniform_lightdir==-1) printf("FAIL\n");
	uniform_lightamb[0] = glGetUniformLocation(program, "lightAmbient");
	//if(uniform_lightamb==-1) printf("FAIL\n");
	uniform_lightdif[0] = glGetUniformLocation(program, "lightDiffuse");
	//if(uniform_lightdif==-1) printf("FAIL\n");
	uniform_lightspec[0] = glGetUniformLocation(program, "lightSpecular");
	//if(uniform_lightspec==-1) printf("FAIL\n");
	uniform_spotdir[0] = glGetUniformLocation(program, "spotDirection");
	//if(uniform_spotdir==-1) printf("FAIL\n");
	uniform_spotexp[0] = glGetUniformLocation(program, "spotExponent");
	//if(uniform_spotexp==-1) printf("FAIL\n");
	uniform_spotcutoff[0] = glGetUniformLocation(program, "spotCutOff");
	(uniform_spotcutoff==-1) printf("FAIL\n");
	lightype = glGetUniformLocation(program, "LightType");
//	if(uniform_lightpos==-1 || uniform_lightamb==-1 || uniform_lightdif==-1 || uniform_lightspec==-1) printf("FAIL\n");
*/
	uniform_matamb = glGetUniformLocation(program, "materialAmbient");
	if(uniform_matamb==-1) printf("FAIL\n");
	uniform_matdif = glGetUniformLocation(program, "materialDiffuse");
	if(uniform_matdif==-1) printf("FAIL\n");
	uniform_matspec = glGetUniformLocation(program, "materialSpecular");
	if(uniform_matspec==-1) printf("FAIL\n");
	uniform_inverse_mv = glGetUniformLocation(program, "normalmatrix");
	if(uniform_inverse_mv==-1) printf("FAIL\n");
	uniform_matshiny = glGetUniformLocation(program, "materialShininess");
	if(uniform_matshiny==-1) printf("FAIL\n");
	uniform_inverse_mv = glGetUniformLocation(program, "normalmatrix");
}
object_3d selected;
int light_model, shading_model;
GLuint sh_ptrs[10][10];
vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
void draw_obj_picking(object_3d &obj, int picki){
	GLuint program = ambientshader;
	glUseProgram(program);
	mat4 model, view, projection;
	model = translate(mat4(1.0), obj.trans);
	model = translate(model, vec3(0.0,0.0,-4.0f));
	model = model*toMat4(obj.q);
	model = scale(model, vec3(obj.scale));
	model = scale(model, vec3(2.0));
	model = scale(model, vec3(1/obj.max_norm));
	model = translate(model, -obj.centroid);
	view = lookAt(camera_pos, camera_center, vec3(0.0, 1.0, 0.0));
	projection = perspective(45.0f, 1.0f*s_width/s_height, 1.0f, 30.0f);
	GLint uniform_amb = glGetUniformLocation(program, "lightAmbient");
	uniform_m = glGetUniformLocation(program, "m");
	uniform_v = glGetUniformLocation(program, "v");
	uniform_p = glGetUniformLocation(program, "p");
	attribute_coord3d = glGetAttribLocation(program, "coord3d");
	if(uniform_amb==-1) printf("FAIL 1\n");
	if(attribute_coord3d==-1) printf("FAIL 2\n");
	glUniformMatrix4fv(uniform_m, 1, GL_FALSE, value_ptr(model));
	glUniformMatrix4fv(uniform_v, 1, GL_FALSE, value_ptr(view));
	glUniformMatrix4fv(uniform_p, 1, GL_FALSE, value_ptr(projection));
	glUniform4f(uniform_amb, obj_id_colors[picki].x/255.0f, obj_id_colors[picki].y/255.0f, obj_id_colors[picki].z/255.0f, 1.0f);
	glBindBuffer(GL_ARRAY_BUFFER, obj.vbov);
	glEnableVertexAttribArray(attribute_coord3d);
	glVertexAttribPointer(
		attribute_coord3d,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
		);
	int size;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.ibo);
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
	glDisableVertexAttribArray(attribute_coord3d);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void draw_obj(object_3d &obj, int sm, int lm, bool B){
	GLuint program = sh_ptrs[lm][sm];
	glUseProgram(program);
//	if(uniform_matamb==-1 || uniform_matdif==-1 || uniform_matspec==-1 || uniform_matshiny==-1) printf("FAIL\n");
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.ibo);
	mat4 model, view, projection;
	model = translate(mat4(1.0), obj.trans);
	model = translate(model, vec3(0.0,0.0,-4.0f));
	model = model*toMat4(obj.q);
	model = scale(model, vec3(obj.scale));
	model = scale(model, vec3(2.0));
	model = scale(model, vec3(1/obj.max_norm));
	model = translate(model, -obj.centroid);
	view = lookAt(camera_pos, camera_center, vec3(0.0, 1.0, 0.0));
	projection = perspective(45.0f, 1.0f*s_width/s_height, 1.0f, 30.0f);
	mat3 normalmatrix = transpose(inverse(mat3(view*model)));
	
	link_uniforms_and_attributes(program);

	glUniformMatrix4fv(uniform_m, 1, GL_FALSE, value_ptr(model));
	glUniformMatrix4fv(uniform_v, 1, GL_FALSE, value_ptr(view));
	glUniformMatrix4fv(uniform_p, 1, GL_FALSE, value_ptr(projection));
	glUniform3fv(uniform_eye, 1, value_ptr(camera_pos));

	glUniformMatrix3fv(uniform_inverse_mv, 1, GL_FALSE, value_ptr(normalmatrix));
	for(int i=0; i<LMAX; i++){
		glUniform3fv(uniform_lightpos[i], 1, value_ptr(lPosition[i]));
		if(uniform_lightpos[i]==-1) printf("YOU LOSE\n");
		glUniform3fv(uniform_lightdir[i], 1, value_ptr(lDirection[i]));
		if(uniform_lightdir[i]==-1) printf("YOU LOSE\n");
		glUniform4fv(uniform_lightamb[i], 1, value_ptr(lAmbient[i]));
		if(uniform_lightamb[i]==-1) printf("YOU LOSE\n");
		glUniform4fv(uniform_lightdif[i], 1, value_ptr(lDiffuse[i]));
		if(uniform_lightdif[i]==-1) printf("YOU LOSE\n");
		glUniform4fv(uniform_lightspec[i], 1, value_ptr(lSpecular[i]));
		if(uniform_lightspec[i]==-1) printf("YOU LOSE\n");
		glUniform3fv(uniform_spotdir[i], 1, value_ptr(spotdirection[i]));
		glUniform1f(uniform_spotexp[i], spotexponent[i]);
		if(uniform_spotexp[i]==-1) printf("YOU LOSE\n");
		glUniform1f(uniform_spotcutoff[i], spotcutoff[i]);
		glUniform1i(lightype[i], l[i]);
		if(lightype[i]==-1) printf("FAIL %d\n", i);
	}
	//if(uniform_spotcutoff[i]==-1) printf("YOU LOSE\n");

	glUniform4fv(uniform_matamb, 1, value_ptr(obj.ambient));
	glUniform4fv(uniform_matdif, 1, value_ptr(obj.diffuse));
	glUniform4fv(uniform_matspec, 1, value_ptr(obj.specular));
	glUniform1f(uniform_matshiny, obj.shininess);


	glBindBuffer(GL_ARRAY_BUFFER, obj.vbov);
	glEnableVertexAttribArray(attribute_coord3d);
	glVertexAttribPointer(
		attribute_coord3d,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
		);
	glBindBuffer(GL_ARRAY_BUFFER, obj.vbon);
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
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.ibo);
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
	glDisableVertexAttribArray(attribute_coord3d);
	glDisableVertexAttribArray(attribute_normal);
//	glDisableVertexAttribArray(attribute_fnormal);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	if(!B || !draw_bounding) return;
	program = ambientshader;
	glUseProgram(program);
	glBindBuffer(GL_ARRAY_BUFFER, obj.vbob);
	uniform_m = glGetUniformLocation(program, "m");
	uniform_v = glGetUniformLocation(program, "v");
	uniform_p = glGetUniformLocation(program, "p");
	attribute_coord3d = glGetAttribLocation(program, "coord3d");
	if(attribute_coord3d==-1) printf("YOU SUCK\n");
	glEnableVertexAttribArray(attribute_coord3d);
	int uniform_la;
	uniform_la = glGetUniformLocation(program, "lightAmbient");
	if(uniform_la==-1 ) printf("YOU SUCK\n");
	glEnableVertexAttribArray(attribute_coord3d);
	glUniformMatrix4fv(uniform_m, 1, GL_FALSE, value_ptr(model));
	glUniformMatrix4fv(uniform_v, 1, GL_FALSE, value_ptr(view));
	glUniformMatrix4fv(uniform_p, 1, GL_FALSE, value_ptr(projection));
	glUniform4fv(uniform_la, 1, value_ptr(white));
	glVertexAttribPointer(
		attribute_coord3d,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
	);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.ibob);
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_QUADS, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
	glDisableVertexAttribArray(attribute_coord3d);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
void draw_sphere(int li, bool P){
	GLuint program = ambientshader;
	glUseProgram(program);
	mat4 model, view, projection;
	model = translate(mat4(1.0f), lPosition[li]);
	view = lookAt(camera_pos, camera_center, vec3(0.0, 1.0, 0.0));
	projection = perspective(45.0f, 1.0f*s_width/s_height, 1.0f, 30.0f);

	uniform_m = glGetUniformLocation(program, "m");
	uniform_v = glGetUniformLocation(program, "v");
	uniform_p = glGetUniformLocation(program, "p");
	uniform_lightamb[li] = glGetUniformLocation(program, "lightAmbient");
	attribute_coord3d = glGetAttribLocation(program, "coord3d");
//	if(uniform_lightamb==-1) printf("FAIL 1\n");
	if(attribute_coord3d==-1) printf("FAIL 2\n");
	glUniformMatrix4fv(uniform_m, 1, GL_FALSE, value_ptr(model));
	glUniformMatrix4fv(uniform_v, 1, GL_FALSE, value_ptr(view));
	glUniformMatrix4fv(uniform_p, 1, GL_FALSE, value_ptr(projection));
	if(!P) glUniform4fv(uniform_lightamb[0], 1, value_ptr(lAmbient[li]));
	else glUniform4f(uniform_lightamb[0], obj_id_colors[li+8].x/255.0f, obj_id_colors[li+8].y/255.0f, obj_id_colors[li+8].z/255.0f, 1.0f);
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
	lPosition[light] = lPosition[4];
	lAmbient[light] = lAmbient[4];
	lDiffuse[light] = lDiffuse[4];
	lSpecular[light] = lSpecular[4];
	lDirection[light] = lDirection[4];
	spotdirection[light] = spotdirection[4];
	spotcutoff[light] = spotcutoff[4];
	spotexponent[light] = spotexponent[4];
	l[light] = l[4];
	if(pick){
		bool found = false;
		for(int i=0; i<(int)figures.size(); i++){
			if(i==figure){
				figures[i].scale = selected.scale;
				figures[i].ambient = selected.ambient;
				figures[i].diffuse = selected.diffuse;
				figures[i].specular = selected.specular;
				figures[i].shininess = selected.shininess;
				figures[i].q = selected.q;
				figures[i].s = selected.s; figures[i].r = selected.r;
				figures[i].trans = selected.trans;
			}
			draw_obj_picking(figures[i], i);
		}
		for(int i=0; i<LMAX; i++) draw_sphere(i, pick);
		unsigned char pixeld[4];
		glReadPixels(click.first, click.second, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixeld);
		int ID = pixeld[0]+pixeld[1]*256+pixeld[2]*256*256;
		printf("%d\n", ID);
		if(ID>0 && ID<figures.size()){
			figure = ID-1;
			selected.q = figures[figure].q; selected.trans = figures[figure].trans;
			selected.ambient = figures[figure].ambient;
			selected.diffuse = figures[figure].diffuse;
			selected.specular = figures[figure].specular;
			selected.shininess = figures[figure].shininess;
			selected.scale = figures[figure].scale;
			selected.s = figures[figure].s;
			selected.r = figures[figure].r;
		}else if(ID>=9 && ID<=12){
			light = ID-9;
			lPosition[4] = lPosition[light];
			lAmbient[4] = lAmbient[light];
			lDiffuse[4] = lDiffuse[light];
			lSpecular[4] = lSpecular[light];
			lDirection[4] = lDirection[light];
			spotdirection[4] = spotdirection[light];
			spotcutoff[4] = spotcutoff[light];
			spotexponent[4] = spotexponent[light];
			l[4] = l[light];
		}	
	}else{
		for(int i=0; i<figures.size(); i++){
			if(i==figure){
				figures[i].scale = selected.scale;
				figures[i].ambient = selected.ambient;
				figures[i].diffuse = selected.diffuse;
				figures[i].specular = selected.specular;
				figures[i].shininess = selected.shininess;
				figures[i].q = selected.q;
				figures[i].s = selected.s; figures[i].r = selected.r;
				figures[i].trans = selected.trans;
			}
			//printf("%d %d %d\n", i, figures[i].s, figures[i].r);
			draw_obj(figures[i], figures[i].s, figures[i].r, i==figure);
		}
		for(int i=0; i<LMAX; i++) draw_sphere(i, false);
	}
	//draw_floor();
	TwDraw();
	if(!pick) glutSwapBuffers();
	pick = false;
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
	sh_ptrs[3][3] = ambientshader;
}
void data(){
	object_3d A = object_3d("figure.obj"), F = object_3d("monkey.obj");
	figures.push_back(A);
	figures.push_back(F);
	figures.push_back(object_3d("floor.obj"));
	for(int i=0; i<figures.size(); i++){
		figures[i].load_obj(figures[i].name.c_str());
		glGenBuffers(1, &figures[i].vbon);
		glBindBuffer(GL_ARRAY_BUFFER, figures[i].vbon);
		glBufferData(GL_ARRAY_BUFFER, figures[i].normals.size()*sizeof(vec3), &figures[i].normals[0], GL_STATIC_DRAW); 
	
		glGenBuffers(1, &figures[i].vbov);
		glBindBuffer(GL_ARRAY_BUFFER, figures[i].vbov);
		glBufferData(GL_ARRAY_BUFFER, figures[i].obj_vertices.size()*sizeof(vec3), &figures[i].obj_vertices[0], GL_STATIC_DRAW);

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

		glGenBuffers(1, &figures[i].ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, figures[i].ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*figures[i].elements.size(), &figures[i].elements[0], GL_STATIC_DRAW);

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

		glGenBuffers(1, &figures[i].vbob);
		glBindBuffer(GL_ARRAY_BUFFER, figures[i].vbob);
		glBufferData(GL_ARRAY_BUFFER, figures[i].bounding_box.size()*sizeof(vec3), &figures[i].bounding_box[0], GL_STATIC_DRAW);

		glGenBuffers(1, &figures[i].ibob);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, figures[i].ibob);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bounding_elements), bounding_elements, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	}
	for(int i=0; i<=4; i++){
		lPosition[i] =  vec3(0.0, 0.0, 0.0);
		lAmbient[i] = vec4(1.0, 1.0, 1.0, 1.0);
		lDiffuse[i] = vec4(1.0, 0.25, 0.74, 1.0);
		lSpecular[i] = vec4(1.0, 1.0, 1.0, 1.0);
		lDirection[i] = vec3(0.0, 1.0, 1.0);
		spotdirection[i] = vec3(0.0, 1.0, 1.0);
		spotcutoff[i] = 0.0;
		spotexponent[i] = 1.0;
		l[i] = OFF;
	}
	selected = figures[0];
	figure = 0;
}
mat4 model, view, projection;
void rotationIdle(){
	//float move = sinf(glutGet(GLUT_ELAPSED_TIME) / 1000.0 * (2*3.14) / 5); // -1<.+1 every 5 seconds
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
	figure = light = 0;
	pick = false;
	for(int i=1; i<=12; i++){
		obj_id_colors[i-1] = i8vec3((i & 0x000000FF) >>  0,(i & 0x0000FF00) >>  8, (i & 0x00FF0000) >> 16);
	}
	click = make_pair(0,0);
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
	}else if(key==' '){
		camera_center.y+=0.1; camera_pos.y+=0.1;
	}else if(key=='z'){
		camera_center.y-=0.1; camera_pos.y-=0.1;
	}else if(key=='f'){
		figure = (figure+1)%(figures.size()-1);
		selected.q = figures[figure].q; selected.trans = figures[figure].trans;
		selected.ambient = figures[figure].ambient;
		selected.diffuse = figures[figure].diffuse;
		selected.specular = figures[figure].specular;
		selected.shininess = figures[figure].shininess;
		selected.scale = figures[figure].scale;
		selected.s = figures[figure].s;
		selected.r = figures[figure].r;
	//	printf("%d\n", figure);
	}else if(key=='g'){
		light = (light+1)%LMAX;
		lPosition[4] = lPosition[light];
		lAmbient[4] = lAmbient[light];
		lDiffuse[4] = lDiffuse[light];
		lSpecular[4] = lSpecular[light];
		lDirection[4] = lDirection[light];
		spotdirection[4] = spotdirection[light];
		spotcutoff[4] = spotcutoff[light];
		spotexponent[4] = spotexponent[light];
		l[4] = l[light];
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
void mouseclick(int button, int state, int x, int y){
	if(!TwEventMouseButtonGLUT(button, state, x, y) && state==0){
		click = make_pair(x, s_height-y);
		pick = true;
	}
}
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	TwBar* bar, *lightbar;
	s_width = 800; s_height = 600;
	glutInitWindowSize(s_width, s_height);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("Mi ventana");
	InitOpenGL();
	TwGLUTModifiersFunc(glutGetModifiers);
	TwInit(TW_OPENGL, NULL);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouseclick);
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
	
	bar = TwNewBar("TweakBar"), lightbar = TwNewBar("LightBar");
	TwDefine("TweakBar size = '200 400' ");
	TwDefine("LightBar size = '200 400' ");
	TwAddButton(bar, "---Propiedades material---", NULL, NULL, "");

	TwAddVarRW(bar, "Material Ambient ", TW_TYPE_COLOR4F, &selected.ambient, "");
	TwAddVarRW(bar, "Material Diffuse ", TW_TYPE_COLOR4F, &selected.diffuse, "");
	TwAddVarRW(bar, "Material Specular ", TW_TYPE_COLOR4F, &selected.specular, "");
	TwAddVarRW(bar, "Material Shininess ", TW_TYPE_FLOAT, &selected.shininess, "");

	TwAddVarRW(bar, "Rotation ", TW_TYPE_QUAT4F, &selected.q, "");

	TwAddVarRW(bar, "Material Scale X ", TW_TYPE_FLOAT, &selected.scale.x, "step = 0.1 min = 0.0");
	TwAddVarRW(bar, "Material Scale Y ", TW_TYPE_FLOAT, &selected.scale.y, "step = 0.1 min = 0.0");
	TwAddVarRW(bar, "Material Scale Z ", TW_TYPE_FLOAT, &selected.scale.z, "step = 0.1 min = 0.0");
	TwAddVarRW(bar, "Material Pos X ", TW_TYPE_FLOAT, &selected.trans.x, "step = 0.1");
	TwAddVarRW(bar, "Material Pos Y ", TW_TYPE_FLOAT, &selected.trans.y, "step = 0.1");
	TwAddVarRW(bar, "Material Pos Z ", TW_TYPE_FLOAT, &selected.trans.z, "step = 0.1");

	TwAddVarRW(bar, "Bounding Box ", TW_TYPE_BOOLCPP, &draw_bounding, "");

	TwAddVarRW(lightbar, "Light Ambient ", TW_TYPE_COLOR4F, &lAmbient[4], "");
	TwAddVarRW(lightbar, "Light Diffuse ", TW_TYPE_COLOR4F, &lDiffuse[4], "");
	TwAddVarRW(lightbar, "Light Specular ", TW_TYPE_COLOR4F, &lSpecular[4], "");
	TwAddVarRW(lightbar, "Light Position X ", TW_TYPE_FLOAT, &lPosition[4].x, "step = 0.01");
	TwAddVarRW(lightbar, "Light Position Y ", TW_TYPE_FLOAT, &lPosition[4].y, "step = 0.01");
	TwAddVarRW(lightbar, "Light Position Z ", TW_TYPE_FLOAT, &lPosition[4].z, "step = 0.01");
	TwAddVarRW(lightbar, "Light Direction ", TW_TYPE_DIR3F, &lDirection[4], "");
	TwAddVarRW(lightbar, "Spotlight - Cutoff", TW_TYPE_FLOAT, &spotcutoff[4], "step = 1 min = 0.00 max = 360.0");
	TwAddVarRW(lightbar, "Spotlight - Exponent", TW_TYPE_FLOAT, &spotexponent[4], "step = 0.01");
	TwAddVarRW(lightbar, "Spotlight - Direction", TW_TYPE_DIR3F, &spotdirection[4], "");

	TwEnumVal shadings[] = {{FLAT, "Flat"}, {GOURAUD, "Gouraud"}, {PHONG, "Phong"}};
	TwEnumVal lightings[] = {{PHONGE, "Phong"}, {BLINN, "Blinn-Phong"}, {COOK, "Cook-Torrance"}};
	TwEnumVal types[] = {{POINTE, "Puntual "}, {DIRECTIONAL, "Direccional"}, {SPOTLIGHT, "Spotlight"}, {OFF, "Apagada"}};
	TwType stype, ltype, ttype;
	stype = TwDefineEnum("Shading Model", shadings, 3);
	ltype = TwDefineEnum("Reflection Model", lightings, 3);
	ttype = TwDefineEnum("Lighting Type", types, 4);
	TwAddVarRW(bar, "Shade Model", stype, &selected.s, NULL);
	TwAddVarRW(bar, "Reflect Model", ltype, &selected.r, NULL);
	TwAddVarRW(lightbar, "Light Type", ttype, &l[4], NULL);
	glutMainLoop();
	return 0;
}

