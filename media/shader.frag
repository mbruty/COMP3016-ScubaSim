#version 450 core

// output to both textures

layout (location = 0) out vec4 fColour;
layout (location = 1) out vec4 inFocus;

in vec3 currentPos;
in vec4 fragColour;
in vec2 TexCoord;
in vec3 normal;

uniform vec3 camPos;
uniform vec3 camDirection;
uniform vec3 lightColour;
uniform vec3 ambient;
uniform vec3 jellyfishPos;
uniform vec3 jellyfishAmbient;
uniform bool torchIsOn;
uniform bool ignoreDistFromCamera; // Variable for background

vec3 calcColourSkew(float dist) {
    if (camPos.y > 0 && currentPos.y > 0)
        return vec3(0.0f);
    vec3 colorNeg = vec3(1.0f);
    
    // Red fully dissapears from ambient at 5m in typical conditions
    float clampedDistance = clamp(dist, 0.0f, 5.0f);
    float normalizedColor = clampedDistance / 5.0f;
    colorNeg.r = normalizedColor;

    // Green fully dissapears from ambient at 25m in typical conditions
    clampedDistance = clamp(dist, 0.0f, 25.0f);
    normalizedColor = clampedDistance / 25.0f;
    colorNeg.g = normalizedColor;

    // Blue fully dissapears from ambient at 35m in typical conditions
    clampedDistance = clamp(dist, 0.0f, 35.0f);
    normalizedColor = clampedDistance / 35.0f;
    colorNeg.b = normalizedColor;

    return colorNeg;
}

vec3 jellyFishLight() {

    // ambient
    vec3 ambient = vec3(0.894, 0.51, 0.851);

    // diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(jellyfishPos - currentPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = vec3(0.5f, 0.5f, 0.5f) * diff * fragColour.rgb;
    float distanceFromCam = distance(camPos, currentPos) / 10;
    float dist = distance(jellyfishPos, currentPos) / 10;
    diffuse -= calcColourSkew(dist + distanceFromCam);
    // specular
    vec3 viewDir = normalize(camPos - currentPos);
    vec3 reflectDir = reflect(-lightDir, norm);  

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0f);
    vec3 specular = vec3(1.0f, 1.0f, 1.0f) * spec * vec3(1.0f, 1.0f, 1.0f);  

    // attenuation
    float attenDistance = length(jellyfishPos - currentPos);
    float attenuation = 1.0 / (1.0f + 0.027f * attenDistance + 0.0028f * (attenDistance * attenDistance));   

    ambient  *= attenuation;  
    diffuse   *= attenuation;
    specular *= attenuation; 
        
    vec3 result = ambient + diffuse + specular;
    return result;
}

vec3 torch() {
    float distanceFromCam = distance(camPos, currentPos) / 10;
    float depth = -(currentPos.y / 10);

	// controls how big the area that is lit up is
	float outerCutOff = 0.82f;
	float cutOff = 0.91f;

    vec3 lightDir = normalize(camPos - currentPos);
    
    // check if lighting is inside the spotlight cone
    float theta = dot(lightDir, normalize(-camDirection)); 
    
    if(theta > cutOff && torchIsOn && !ignoreDistFromCamera)
    {    
        vec3 calcAmbient = ambient - calcColourSkew(distanceFromCam);
        vec3 diffuseTex = fragColour.rgb;

        // ambient
        vec3 ambientVec = calcAmbient * diffuseTex;
        // diffuse 
        vec3 norm = normalize(normal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = lightColour * diff * diffuseTex;  
        
        // specular
        vec3 viewDir = normalize(camPos - currentPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 0.5f);
        vec3 specular = vec3(1.0f, 1.0f, 1.0f) * spec * fragColour.rgb; 
        
        // attenuation
        float distance    = length(camPos - currentPos);
        float attenuation = 1.0 / (1.0f + 0.09f * distance + 0.032f * (distance * distance));    

        // ambientVec  *= attenuation; // remove attenuation from ambient, as otherwise at large distances the light would be darker inside than outside the spotlight due the ambient term in the else branche
        diffuse   *= attenuation;
        specular *= attenuation;   

        vec3 result = ambientVec + diffuse + specular;

        return result;
    }
    else 
    {
        vec3 calcAmbient = ambient - calcColourSkew(depth + distanceFromCam);

        if (ignoreDistFromCamera) {
            calcAmbient = ambient - calcColourSkew(-camPos.y / 10);
        }

        // else, use ambient light so scene isn't completely dark outside the spotlight.
        return calcAmbient * fragColour.rgb;
    }
}

void main() {
    vec3 torchColour = torch();
	vec3 jellyColour = jellyFishLight();
    vec3 result = torchColour + jellyColour;
    fColour = vec4(result, 1.0f);
    
    float distance = distance(camPos, currentPos);
    // If distance is 5 meters, or the fragment is above the water, keep in focus
	if (distance < 50.0f || currentPos.y >= 0)
		inFocus = vec4(fColour);
	else
		inFocus = vec4(0.0f, 0.0f, 0.0f, 1.0f);

}
