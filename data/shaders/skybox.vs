#version 330 core
in vec3 a_vertex;

uniform mat4 u_model;
uniform mat4 u_viewprojection;

out vec3 v_dir;

void main() {
    v_dir = a_vertex;
    gl_Position = u_viewprojection * u_model * vec4(a_vertex, 1.0);
}