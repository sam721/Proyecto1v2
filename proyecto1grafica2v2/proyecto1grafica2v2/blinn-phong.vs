attribute vec3 coord3d;
attribute vec3 normal;
uniform vec3 eye;
uniform vec3 lightPosition;
uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform vec4 materialAmbient;
uniform vec4 materialDiffuse;
uniform vec4 materialSpecular;
uniform float materialShininess;
varying vec3 fPosition;
varying vec3 norm;
uniform mat4 m, v, p;
uniform mat3 normalmatrix;
void main(void){
	gl_Position = p*v*m*vec4(coord3d, 1.0);
	vec3 pos = (v*m*vec4(coord3d, 1.0)).xyz;
	fPosition = pos;
	norm = normal;
}