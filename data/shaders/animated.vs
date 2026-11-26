
attribute vec3 a_vertex;
attribute vec3 a_normal;
attribute vec2 a_uv;
attribute vec4 a_color;

// Cuatro huesos y sus pesos por vértice
attribute vec4 a_bones;
attribute vec4 a_weights;

uniform mat4 u_model;
uniform mat4 u_viewprojection;

// Matrices de los huesos
uniform mat4 u_bones[128];

varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

void main()
{
    // Combinar las transformaciones de los cuatro huesos
    mat4 skin_matrix =
        u_bones[int(a_bones.x)] * a_weights.x +
        u_bones[int(a_bones.y)] * a_weights.y +
        u_bones[int(a_bones.z)] * a_weights.z +
        u_bones[int(a_bones.w)] * a_weights.w;

    // Transformar vértice y normal con el esqueleto
    vec4 animated_vertex = skin_matrix * vec4(a_vertex, 1.0);
    vec3 animated_normal = mat3(skin_matrix) * a_normal;

    v_position = animated_vertex.xyz;

    vec4 world_position = u_model * animated_vertex;
    v_world_position = world_position.xyz;

    v_normal = (u_model * vec4(animated_normal, 0.0)).xyz;

    v_color = a_color;
    v_uv = vec2(a_uv.x, 1.0 - a_uv.y);

    gl_Position = u_viewprojection * world_position;
}
