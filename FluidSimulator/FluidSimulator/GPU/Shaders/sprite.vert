#version 430 core
layout(std430, binding = 0) readonly buffer _Positions
{
    vec2 Positions[];
};

layout(std430, binding = 1) buffer _Velocities
{
    vec2 Velocities[];
};

layout (location = 0) in vec2 aPos;

uniform float scale;
uniform float aspect_ratio;
uniform float max_speed;

uniform sampler1D Heatmap;
  
out vec4 vertex_color;
out vec2 uv;

void main()
{
    uint instanceID = gl_InstanceID;
    vec2 vertex_positions[6] = {
        { -scale, -scale * aspect_ratio },
        { scale, -scale * aspect_ratio },
        { -scale, scale * aspect_ratio },
        { -scale, scale * aspect_ratio },
        { scale, scale * aspect_ratio },
        { scale, -scale * aspect_ratio }
    };
    vec2 uvs[6] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
        { 1.0f, 0.0f }
    };

    float speed = length(Velocities[instanceID]);
	float speedT = clamp(speed / max_speed, 0.0, 1.0);
				
	//vec3 centreWorld = vec3(Positions[instanceID], 0);
	//vec3 worldVertPos = centreWorld + mul(unity_ObjectToWorld, v.vertex * scale);
	//vec3 objectVertPos = mul(unity_WorldToObject, float4(worldVertPos.xyz, 1));

	//v2f o;
	//o.uv = v.texcoord;
	//o.pos = UnityObjectToClipPos(objectVertPos);
	//o.colour = ColourMap.SampleLevel(linear_clamp_sampler, float2(colT, 0.5), 0);

    vertex_color = texture(Heatmap, speedT);
    uint vertex_id = gl_VertexID % 6;
    uv = uvs[vertex_id];
    gl_Position = vec4(Positions[instanceID] / 2000.0f + vertex_positions[vertex_id], 0.0, 1.0);
}
