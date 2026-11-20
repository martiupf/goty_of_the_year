varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec4 u_color;
uniform sampler2D u_texture;
uniform vec3 u_camera_pos;

uniform vec3 u_light_dir;
uniform vec3 u_light_color;
uniform vec3 u_ambient_light;

void main()
{
	vec4 color = u_color * texture2D(u_texture, v_uv);
	if (color.a < 0.01) discard;

	vec3 N = normalize(v_normal);
	vec3 L = normalize(-u_light_dir); // dirección hacia la fuente de luz

	float NdotL = max(dot(N, L), 0.0);
	vec3 light = u_ambient_light + (u_light_color * NdotL);

	gl_FragColor = vec4(color.rgb * light, color.a);
}