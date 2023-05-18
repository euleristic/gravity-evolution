#version 410 core

layout (location = 0) in dvec3 aPos;

uniform dmat4 projection;

void main() {
     gl_Position = vec4(projection * dvec4(aPos, 1.0));
}