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

uniform sampler2D u_shadow_map;
uniform mat4 u_light_viewprojection;
uniform int u_use_shadows;

uniform vec3 u_player1_pos;
uniform int u_player1_active;
uniform sampler2D u_player1_depth_map;
uniform mat4 u_player1_vp;

uniform vec3 u_player2_pos;
uniform int u_player2_active;
uniform sampler2D u_player2_depth_map;
uniform mat4 u_player2_vp;

float computeBlobShadow(vec3 world_pos, vec3 N, vec3 player_pos, sampler2D depth_map, mat4 player_vp)
{
	float dy = player_pos.y - world_pos.y;
	if (dy < -0.05 || dy > 14.0) return 0.0;

	// Solo proyectar en superficies orientadas hacia arriba (evita rayas en paredes verticales)
	float up_factor = smoothstep(0.3, 0.8, N.y);
	if (up_factor <= 0.0) return 0.0;

	// Comprobar si otra plataforma bloquea la vertical antes de llegar a este píxel
	vec4 p_space = player_vp * vec4(world_pos, 1.0);
	vec3 proj = p_space.xyz / p_space.w;
	proj = proj * 0.5 + 0.5;
	if (proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0 || proj.z < 0.0 || proj.z > 1.0)
		return 0.0;

	float closest_depth = texture2D(depth_map, proj.xy).r;
	if (proj.z - 0.008 > closest_depth)
		return 0.0;

	float t = clamp(max(0.0, dy) / 14.0, 0.0, 1.0);
	float radius = mix(0.65, 0.18, t);
	float alpha = 0.95 * (1.0 - t * 0.45);

	float dist_xz = length(world_pos.xz - player_pos.xz);
	float circle = 1.0 - smoothstep(radius * 0.65, radius, dist_xz);

	return circle * alpha * up_factor;
}

float computeShadow(vec3 world_pos, float NdotL)
{
	if (u_use_shadows == 0) return 0.0;

	vec4 light_space_pos = u_light_viewprojection * vec4(world_pos, 1.0);
	vec3 proj_coords = light_space_pos.xyz / light_space_pos.w;
	proj_coords = proj_coords * 0.5 + 0.5;

	if (proj_coords.z > 1.0 || proj_coords.x < 0.0 || proj_coords.x > 1.0 || proj_coords.y < 0.0 || proj_coords.y > 1.0)
		return 0.0;

	float bias = max(0.004 * (1.0 - NdotL), 0.0005);
	float texel_size = 1.0 / 2048.0;
	float shadow = 0.0;

	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcf_depth = texture2D(u_shadow_map, proj_coords.xy + vec2(x, y) * texel_size).r;
			shadow += (proj_coords.z - bias > pcf_depth) ? 1.0 : 0.0;
		}
	}

	return shadow / 9.0;
}

void main()
{
	vec4 color = u_color * texture2D(u_texture, v_uv);
	if (color.a < 0.01) discard;

	vec3 N = normalize(v_normal);
	vec3 L = normalize(-u_light_dir); // dirección hacia la fuente de luz

	float NdotL = max(dot(N, L), 0.0);
	float shadow = computeShadow(v_world_position, NdotL);
	vec3 light = u_ambient_light + (u_light_color * NdotL * (1.0 - shadow));

	float blob1 = (u_player1_active == 1) ? computeBlobShadow(v_world_position, N, u_player1_pos, u_player1_depth_map, u_player1_vp) : 0.0;
	float blob2 = (u_player2_active == 1) ? computeBlobShadow(v_world_position, N, u_player2_pos, u_player2_depth_map, u_player2_vp) : 0.0;
	float blob_shadow = max(blob1, blob2);

	gl_FragColor = vec4(color.rgb * light * (1.0 - blob_shadow), color.a);
}