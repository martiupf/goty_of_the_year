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

	gl_FragColor = vec4(color.rgb * light, color.a);
}