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
uniform mat3 normalmatrix;
varying vec4 color;
uniform mat4 m, v, p;
float pi = atan(1.0)*4;
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
light light0 = light(lightPosition, lightAmbient, lightDiffuse, lightSpecular);
material mymaterial = material(materialAmbient, materialDiffuse, materialSpecular, materialShininess);
void main(void){
	vec3 pos = (v*m*vec4(coord3d, 1.0)).xyz;
	float dist = length((v*vec4(light0.position,1.0)).xyz-pos);
	float att = 1.0 / (1.0 + 0.1*dist + 0.01*dist*dist);
	vec4 Ambient, Diffuse, Specular;
	Ambient = mymaterial.ambient*light0.ambient;
	vec3 L = normalize((v*vec4(light0.position,1.0)).xyz-pos), N = normalize(normalmatrix*normal);
	float NdotL = dot(N,L);
	Diffuse = light0.diffuse*mymaterial.diffuse*max(NdotL,0);
	vec3 V = -pos;
	if(length(V)!=0) V = normalize(V);
	vec3 H = L+V;
	if(length(H)!=0) H = normalize(H);
	color = Ambient+Diffuse;
	float F, D, G;
	float NdotV, NdotH, VdotH, LdotH;
	NdotV = dot(N,V); NdotH = dot(N,H);  VdotH = dot(V,H); LdotH = dot(L,H);
	Specular = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 spec = light0.specular*mymaterial.specular;
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
	color =  Ambient + NdotL*(spec*rs*pow(max(0.0,RdotV),mymaterial.shininess)+Diffuse);
	gl_Position = p*v*m*vec4(coord3d,1.0);
}