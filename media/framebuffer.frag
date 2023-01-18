#version 400 core

out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D infocus;
uniform sampler2D blured;

// This stage will be ran once both bluring and distance stages have been ran
// This joins the two images back into one
void main() {
    vec3 blur = texture(blured, texCoords).rgb;
    vec3 focused = texture(infocus, texCoords).rgb;

    // If the focused image is black, render the blurred image
    if (any(equal(focused, vec3(0.0f)))) {
        FragColor = vec4(blur, 1.0f);
    } else {
        FragColor = vec4(focused, 1.0f);
    }

}