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
uniform mat3 normalmatrix;
uniform mat4 m, v, p;
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
	vec3 R = normalize(2.0f*NdotL*N-L);
	vec3 V = normalize(-fPosition);
	Specular = vec4(0.0, 0.0, 0.0, 1.0);
	if(NdotL>0.0) Specular = att*mymaterial.specular*light0.specular*pow(max(dot(R,V),0),mymaterial.shininess);
	color = Ambient+Diffuse+Specular;
    gl_FragColor = color;
}