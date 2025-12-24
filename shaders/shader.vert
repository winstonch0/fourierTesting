#version 410 core
layout (location = 0) in vec3 aPos;

out vec4 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vec3 color = (aPos + 0.5f);
    vertexColor = vec4(color, 1.0f);
}