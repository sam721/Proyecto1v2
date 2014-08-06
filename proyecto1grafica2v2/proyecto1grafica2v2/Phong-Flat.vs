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
flat varying vec4 color;
uniform mat4 m, v, p;
uniform mat3 normalmatrix;
struct light{
	vec3 position;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};
struct material{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};
light light0 = light(lightPosition, lightAmbient, lightDiffuse, lightSpecular);
material mymaterial = material(materialAmbient, materialDiffuse, materialSpecular, materialShininess);
void main(void){
	mat4 mvp = p*v*m;
	vec3 pos = (v*m*vec4(coord3d,1.0)).xyz;
	float dist = length(light0.position-pos);
	float att = 1.0 / (1.0 + 0.1*dist + 0.01*dist*dist);
	vec4 Ambient, Diffuse, Specular;
	Ambient = mymaterial.ambient*light0.ambient;
	vec3 L = normalize((v*vec4(light0.position,1.0)).xyz-pos), N = normalize(normalmatrix*normal);
	float NdotL = dot(N,L);
	Diffuse = light0.diffuse*mymaterial.diffuse*max(NdotL,0);
	vec3 R = normalize(2.0*NdotL*N-L);
	vec3 V = normalize(-pos.xyz);
	Specular = vec4(0.0, 0.0, 0.0, 1.0);
	if(NdotL>0.0)Specular = att*mymaterial.specular*light0.specular*pow(max(dot(R,V),0.0),mymaterial.shininess);
	color = Ambient+Diffuse+Specular;
	gl_Position = mvp*vec4(coord3d, 1.0);
}