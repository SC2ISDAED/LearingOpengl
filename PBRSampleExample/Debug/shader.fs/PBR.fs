#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

//material parameters
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

//lights 
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 camPos;

const float PI=3.14159265359;

//------------------------------------------------------
//正态分布
float DistributionGGX(vec3 N,vec3 H,float roughness)
{
	float a = roughness*roughness;
	float a2= a*a;
	float Ndoth = max(dot(N,H),0.0);
	float Ndoth2=Ndoth*Ndoth;
	
	float nom = a2;
	float denom =(Ndoth2 *(a2-1.0)+1.0);
	denom = PI*denom *denom;
	
	return nom/max(denom,0.001);
}

float GeometrySchlickGGX(float NdotV,float roughness)
{
	float r=(roughness+1.0);
	float k =(r*r)/8.0;
	
	float nom =NdotV;
	float denom = NdotV  *(1.0-k)+k;
	
	return nom/denom;
}
float GeometrySmith(vec3 N,vec3 V, vec3 L,float roughness)
{
	float NdotV=max(dot(N,V),0.0);
	float NdotL=max(dot(N,L),0.0);
	float ggx2=GeometrySchlickGGX(NdotV,roughness);
	float ggx1=GeometrySchlickGGX(NdotL,roughness);
	
	return ggx1*ggx2;
}

vec3 fresnelSchlick(float cosTheta,vec3 FO)
{
	return FO+(1.0-FO)*pow(1-cosTheta,5.0);
}
void main()
{
	vec3 N=normalize(Normal);
	vec3 V= normalize(camPos-WorldPos);
	
	vec3 FO= vec3(0.04);
	FO = mix(FO,albedo,metallic);
	
	//反射率等式
	vec3 Lo= vec3(0.0);
	
	for(int i=0;i<4;i++)
	{
		//计算 每个光源的辐射
		vec3 L= normalize(lightPositions[i]-WorldPos);
		vec3 H = normalize(V+L);
		float distance = length(lightPositions[i]-WorldPos);
		float attenuation = 1.0/(distance*distance);
		vec3 radiance = lightColors[i] *attenuation;
		
		//Cook-Torrance BRDF;
		float NDF =  DistributionGGX(N,H,roughness);
		float G = GeometrySmith(N,V,L,roughness);
		vec3 F = fresnelSchlick(clamp(dot(H,V),0.0,1.0),FO);
		
		vec3 nominator = NDF * G * F;
		float denominator= 4* max(dot(N,V),0.0)* max(dot(N,L),0.0);
		vec3 specular = nominator/max(denominator,0.001);
		
		//kS  等于我们所求的的菲涅尔值
		vec3 kS = F;
		
		//考虑能量守恒，反射光和漫反射光不会大于1.0；
		//为了保证能量守恒，所以 kD= 1.0-kS;
		
		vec3 kD = vec3(1.0)-kS;
		//如果是金属值越高，那么金属表面的漫反射越容易被吸收完全，所以当金属值为1.0时，
		//漫反射为0；
		kD=kD*(1.0-metallic);
		//通过NdotL（costhea)达到放缩光源目的
		float NdotL = max(dot(N,L),0.0);
		
		//累加获得输出的辐射值
		Lo+=(kD *albedo/PI+specular)*radiance *NdotL;//注意的是前面算过的菲涅尔值便是ks，所以不再乘以ks
		
	}
	vec3 ambient =vec3(0.03) * albedo * ao;
	
	vec3 color = ambient+Lo;
	//HDR 色调映射
	color =color/(color+vec3(1.0));
	//gamga 矫正
	color = pow(color,vec3(1.0/2.2));
	
	FragColor = vec4(color,1.0);
}