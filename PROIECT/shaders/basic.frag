#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

in vec4 fragPosEye;
in vec4 fragPosLightSpace;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

// lumina punctiforma si rosie
uniform int punctiformMode;
uniform int redLightMode;
uniform	vec3 lightColorP;
uniform	vec3 lightColorR;
uniform int fogMode;

uniform vec3 lightDirP;
uniform vec3 lightPos;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

// pt lumina punctiforma
vec3 ambientPunctiform;
vec3 diffusePunctiform;
vec3 specularPunctiform;

// pt lumina rosie
vec3 ambientRed;
vec3 diffuseRed;
vec3 specularRed;

// pt lumina punctiforma
float constant = 1.0f; 
float linear = 0.0045f;  
float quadratic = 0.0075f;

float shadow;

float computeShadow()
{
	// perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
	// Transform to [0,1] range  (from [-1,1] range)
	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	// Get closest depth value from light's perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

	// Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;
	
	if (normalizedCoords.z > 1.0f){
		return 0.0f;
	}

	// Check whether current frag pos is in shadow
	float bias = 0.005f;
	float shadow = (currentDepth - bias) > closestDepth ? 1.0f : 0.0f;

	return shadow;
}


void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
}

void computeRedLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambientRed = ambientStrength * lightColorR;

    //compute diffuse light
    diffuseRed = max(dot(normalEye, lightDirN), 0.0f) * lightColorR;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specularRed = specularStrength * specCoeff * lightColorR;

}

void computePunctiformLight()
{
    vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	vec4 lightPosEye = view * model * vec4(lightPos, 1.0f);

	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightPosEye.xyz - fragPosEye.xyz);
	
	//compute distance to light
	float dist = length(lightPosEye.xyz - fragPosEye.xyz);

	//compute view direction
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz); // vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz); 
	
    // compute half-vector
    vec3 halfVector = normalize(lightDirN + viewDirN);

    //compute attenuation
	float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));

	//compute ambient light
	ambientPunctiform = att * ambientStrength * lightColorP;
	
	//compute diffuse light
	diffusePunctiform = att * max(dot(normalEye, lightDirN), 0.0f) * lightColorP;
	
	//compute specular light
	//vec3 reflection = reflect(-lightDirN, normalEye);
	
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), 32);
    specularPunctiform = att * specularStrength * specCoeff * lightColorP; // specular = specularStrength * specCoeff * lightColor;
	
}

float computeFog()
{
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * 0.01f, 2));
	return clamp(fogFactor, 0.0f, 1.0f);
}



void main() 
{
    computeDirLight();
	shadow = computeShadow();

    //compute final vertex color
    //vec3 color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
    vec3 color = min((ambient + (1.0f - shadow)*diffuse) * texture(diffuseTexture, fTexCoords).rgb + (1.0f - shadow)*specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
	
    computeRedLight();
    ambientRed *= texture(diffuseTexture, fTexCoords).xyz;
    diffuseRed *= texture(diffuseTexture, fTexCoords).xyz;
    specularRed *= texture(specularTexture, fTexCoords).xyz;

    computePunctiformLight();
    ambientPunctiform *= texture(diffuseTexture, fTexCoords).xyz;
    diffusePunctiform *= texture(diffuseTexture, fTexCoords).xyz;
    specularPunctiform *= texture(specularTexture, fTexCoords).xyz;

    fColor = vec4(color, 1.0f);
    if(punctiformMode == 1){
        color = min(ambientPunctiform + diffusePunctiform + specularPunctiform, 1.0f);
        fColor = vec4(color, 1.0f);
    } else {
        if(redLightMode == 1){
            color = min(ambientRed + diffuseRed + specularRed, 1.0f);
            fColor = vec4(color, 1.0f);
        }else {
            if(fogMode == 1){
                float fogFactor = computeFog();
		        vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
		        fColor = fogColor * (1 - fogFactor) + vec4(color, 1.0f) * fogFactor;
            }
        }

    }

}
