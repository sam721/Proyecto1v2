attribute vec3 coord3d;
attribute vec3 normal;
uniform vec3 eye;

uniform vec3 lightPos1; 
uniform vec3 lightPos2;
uniform vec3 lightPos3;
uniform vec3 lightDir1; 
uniform vec3 lightDir2;
uniform vec3 lightDir3; 
uniform vec4 lightAmb1;
uniform vec4 lightAmb2;
uniform vec4 lightAmb3;
uniform vec4 lightDif1;
uniform vec4 lightDif2;
uniform vec4 lightDif3;
uniform vec4 lightSpec1;
uniform vec4 lightSpec2;
uniform vec4 lightSpec3;
uniform vec3 spotDir1;
uniform vec3 spotDir2;
uniform vec3 spotDir3;
uniform float spotExp1;
uniform float spotExp2;
uniform float spotExp3;
uniform float spotCutOff1;
uniform float spotCutOff2;
uniform float spotCutOff3;
uniform int lightType1;
uniform int lightType2;
uniform int lightType3;

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