#version 330 core
in vec3 v_dir;
uniform samplerCube u_texture;
out vec4 FragColor;

void main() {
    vec3 dir = v_dir;

    if (abs(dir.y) >= max(abs(dir.x), abs(dir.z))) {
        if (dir.y > 0.0) {
            // SOLO TOP (+Y): invierte vertical
            dir.z = -dir.z;
        } else {
            // SOLO BOTTOM (-Y): invierte vertical
            dir.z = -dir.z;
        }
    } else {
        // SOLO LOS 4 LADOS: no se ven afectados por lo de arriba
        dir.y = -dir.y;
    }

    FragColor = textureLod(u_texture, dir, 0.0);
}