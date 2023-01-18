// Edited from the learnopengl post processing tutorial
#version 450 core

out vec4 FragColor;
  
in vec2 texCoords;

uniform sampler2D image;
uniform bool horizontal;

const int radius = 8;
float weights[radius];
float spreadBlur = 4.0f;

void main() {
    
    // Calculate the weights using the Gaussian equation
    float x = 0.0f;
    for (int i = 0; i < radius; i++)
    {
        // Decides the distance between each sample on the Gaussian function
        if (spreadBlur <= 2.0f)
            x += 3.0f / radius;
        else
            x += 6.0f / radius;

        weights[i] = exp(-0.5f * pow(x / spreadBlur, 2.0f)) / (spreadBlur * sqrt(2 * 3.14159265f));
    }

    vec2 offset = 1.0 / textureSize(image, 0); // gets size of single texel
    vec3 result = texture(image, texCoords).rgb * weights[0]; // current fragment's contribution

    if(horizontal)
    {
        for(int i = 1; i < radius; ++i)
        {
            result += texture(image, texCoords + vec2(offset.x * i, 0.0)).rgb * weights[i];
            result += texture(image, texCoords - vec2(offset.x * i, 0.0)).rgb * weights[i];
        }
    }
    else
    {
        for(int i = 1; i < radius; ++i)
        {
            result += texture(image, texCoords + vec2(0.0, offset.y * i)).rgb * weights[i];
            result += texture(image, texCoords - vec2(0.0, offset.y * i)).rgb * weights[i];
        }
    }

    FragColor = vec4(result, 1.0);
}