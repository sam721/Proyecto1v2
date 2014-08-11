varying vec3 norm;
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
uniform mat4 m, v, p;
uniform mat3 normalmatrix;
varying vec3 fPosition;

struct light{
	vec3 position;
	vec3 direction;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec3 spotDirection;
	float spotExponent;
	float spotCutOff;
	int type;
};


struct material{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};
material mymaterial = material(materialAmbient, materialDiffuse, materialSpecular, materialShininess);
void main(void) {
	const int MAXL = 3;
	light lights[MAXL];
	lights[0] = light(lightPos1, lightDir1, lightAmb1, lightDif1, lightSpec1, spotDir1, spotExp1, spotCutOff1, lightType1);
	lights[1] = light(lightPos2, lightDir2, lightAmb2, lightDif2, lightSpec2, spotDir2, spotExp2, spotCutOff2, lightType2);
	lights[2] = light(lightPos3, lightDir3, lightAmb3, lightDif3, lightSpec3, spotDir3, spotExp3, spotCutOff3, lightType3);
	vec4 color;
	vec4 Ambient, Diffuse, Specular;
	float pi = atan(1.0)*4.0;
	color = vec4(0.0, 0.0, 0.0, 1.0);
	for(int i=0; i<3; i++){
		if(lights[i].type==3) continue;
		Ambient = mymaterial.ambient*lights[i].ambient;
		float dist = length((v*vec4(lights[i].position,1.0)).xyz-fPosition);
		float att = 1.0 / (1.0 + 0.1*dist + 0.01*dist*dist);
		vec3 L;
		if(lights[i].type!=1){
			L = normalize((v*vec4(lights[i].position,1.0)).xyz-fPosition);
		}else{
			L = -lights[i].direction;
		}
		if(lights[i].type==2){
			float LdotD = dot(-L, normalize(mat3(v)*lights[i].spotDirection));
			if(max(0.0, LdotD)<cos(2*pi*lights[i].spotCutOff/360.0)) att = 0.0;
			else att = att*pow(max(0.0, LdotD), lights[i].spotExponent);
		}
		vec3 N = normalize(normalmatrix*norm);
		float NdotL = dot(N,L);
		Diffuse = att*lights[i].diffuse*mymaterial.diffuse*max(NdotL,0);
		vec3 V = normalize(-fPosition);
		vec3 H = normalize(L+V);
		Specular = vec4(0.0, 0.0, 0.0, 1.0);
		if(NdotL>0.0) Specular = att*mymaterial.specular*lights[i].specular*pow(max(dot(N,H),0),mymaterial.shininess);
		color = Ambient+Diffuse+Specular;
	}
    gl_FragColor = color;
}