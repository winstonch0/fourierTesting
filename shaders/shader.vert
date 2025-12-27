#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 iPos;
layout (location = 2) in float iRadius;

uniform mat4 projection;

void main()
{
    vec3 world = aPos * iRadius + iPos;
    gl_Position = projection * vec4(world, 1.0);
}
