#version 330 core

uniform vec2 resolution;

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;

out vec4 out_color;
out vec2 out_uv;

vec2 resolution_project(vec2 point) {
     vec2 vec = (2.0 * point / resolution) - 1;
     return vec2(vec.x, -1.0 * vec.y);
}

void main() {
   gl_Position = vec4(resolution_project(position), 0, 1);
   out_color = color;
   out_uv = uv;
}