varying vec3 norm;
uniform vec3 eye;
uniform vec3 lightPosition;
uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform vec4 materialAmbient;
uniform vec4 materialDiffuse;
uniform vec4 materialSpecular;
uniform float materialShininess;
uniform mat4 m, v, p;
uniform mat3 normalmatrix;
varying vec3 fPosition;
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
void main(void) {
	vec4 color;
	float dist = length((v*vec4(light0.position,1.0)).xyz-fPosition);
	float att = 1.0 / (1.0 + 0.1*dist + 0.01*dist*dist);
	vec4 Ambient, Diffuse, Specular;
	Ambient = mymaterial.ambient*light0.ambient;
	vec3 L = normalize((v*vec4(light0.position,1.0)).xyz-fPosition), N = normalize(normalmatrix*norm);
	float NdotL = dot(N,L);
	Diffuse = light0.diffuse*mymaterial.diffuse*max(NdotL,0);
	vec3 V = normalize(-fPosition);
	vec3 H = normalize(L+V);
	Specular = vec4(0.0, 0.0, 0.0, 1.0);
	if(NdotL>0.0) Specular = mymaterial.specular*light0.specular*pow(max(dot(N,H),0),mymaterial.shininess);
	color = Ambient+Diffuse+Specular*att;
    gl_FragColor = color;
}