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
flat varying vec4 color;
uniform mat4 m, v, p;
uniform mat3 normalmatrix;

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
float fresnel(float fy, float HdotV){
	if(HdotV>1.0) HdotV = 1.0;
	return fy+(1-fy)*pow(1.0-max(0.0,HdotV),5.0);
}
float roughness(float m, float NdotH){
	if(NdotH<=0.0) return 1.0;
	float num = NdotH*NdotH-1.0, den = m*m*NdotH*NdotH;
	float expv = exp(num/den);
	return expv/(3.14*m*m*pow(NdotH,4.0));
}
float geometric_att(float NdotH, float NdotV, float NdotL, float VdotH, float LdotH){
	if(VdotH<=0.0) return 1.0;
	return min(min(1, 2*NdotH*NdotV/VdotH), 2*NdotH*NdotL/LdotH);
}
void main(void){
	const int MAXL = 3;
	light lights[MAXL];
	lights[0] = light(lightPos1, lightDir1, lightAmb1, lightDif1, lightSpec1, spotDir1, spotExp1, spotCutOff1, lightType1);
	lights[1] = light(lightPos2, lightDir2, lightAmb2, lightDif2, lightSpec2, spotDir2, spotExp2, spotCutOff2, lightType2);
	lights[2] = light(lightPos3, lightDir3, lightAmb3, lightDif3, lightSpec3, spotDir3, spotExp3, spotCutOff3, lightType3);
	vec3 pos = (v*m*vec4(coord3d, 1.0)).xyz;
	color = vec4(0.0, 0.0, 0.0, 1.0);
	float pi = atan(1.0)*4;
	for(int i=0; i<3; i++){
		if(lights[i].type==3) continue;
		float dist = length((v*vec4(lights[i].position,1.0)).xyz-pos);
		float att = 1.0 / (1.0 + 0.1*dist + 0.01*dist*dist);
		vec4 Ambient, Diffuse, Specular;
		Ambient = mymaterial.ambient*lights[i].ambient;
		vec3 L;
		if(lights[i].type!=1){
			L = normalize((v*vec4(lights[i].position,1.0)).xyz-pos);
		}else{
			L = -lights[i].direction;
		}
		vec3 N = normalize(normalmatrix*normal);
		if(LightType[i]==2){
			float LdotD = dot(-L, normalize(mat3(v)*lights[i].spotDirection));
			if(max(0.0, LdotD)<cos(2*pi*lights[i].spotCutOff/360.0)) att = 0.0;
			else att = att*pow(max(0.0, LdotD), lights[i].spotExponent);
		}
		float NdotL = dot(N,L);
		Diffuse = lights[i].diffuse*mymaterial.diffuse*max(NdotL,0);
		vec3 V = -pos;
		if(length(V)!=0) V = normalize(V);
		vec3 H = L+V;
		if(length(H)!=0) H = normalize(H);
		color = Ambient+Diffuse;
		float F, D, G;
		float NdotV, NdotH, VdotH, LdotH;
		NdotV = dot(N,V); NdotH = dot(N,H);  VdotH = dot(V,H); LdotH = dot(L,H);
		Specular = vec4(0.0, 0.0, 0.0, 1.0);
		vec4 spec = lights[i].specular*mymaterial.specular;
		float rs = 0.0;
		vec3 R = normalize(2.0*NdotL*N-L);
		float RdotV = dot(R,V);
		NdotL = max(0.0, NdotL);
		if(NdotL>0.0){
			F = fresnel(0.8, VdotH);
			D = roughness(0.3, NdotH);
			G = geometric_att(NdotH, NdotV, NdotL, VdotH, LdotH);
			rs = att*F*D*G/(NdotV*NdotL*3.14);
		}
		color +=  Ambient + NdotL*(spec*rs*pow(max(0.0,RdotV),mymaterial.shininess)+Diffuse);
	}
	gl_Position = p*v*m*vec4(coord3d,1.0);
}