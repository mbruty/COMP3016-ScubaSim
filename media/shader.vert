#version 400 core

layout( location = 0 ) in vec3 vPosition;
layout( location = 1 ) in vec3 vNormal;
layout( location = 2 ) in vec4 vColour;
layout (location = 3) in vec2 aTexCoord;

uniform mat4 m_matrix;
uniform mat4 v_matrix;
uniform mat4 p_matrix;

out vec4 fragColour;
out vec2 TexCoord;
out vec3 normal;
out vec3 currentPos;

void main()
{
	mat4 mv_matrix = v_matrix * m_matrix;
	// view-space coordinate
    vec4 P = mv_matrix * vec4(vPosition,1.0);

	fragColour = vColour;

	gl_Position = p_matrix * P;
	TexCoord = aTexCoord;
	normal = vNormal;
	currentPos = vec3(m_matrix * vec4(vPosition, 1.0f));
}