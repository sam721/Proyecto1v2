attribute vec3 coord3d;
uniform mat4 m, v, p;
uniform vec4 lightAmbient;
varying vec4 color;
void main(void){
	color = lightAmbient;
	gl_Position = p*v*m*vec4(coord3d, 1.0);
}