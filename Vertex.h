#pragma once
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <vector>
#include <glm/ext/vector_float2.hpp>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec4 colour;
	glm::vec2 texture;
};