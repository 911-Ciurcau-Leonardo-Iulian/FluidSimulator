#version 430 core
layout(std430, binding = 0) readonly buffer _Positions
{
    vec2 Positions[];
};

layout(std430, binding = 1) buffer _Velocities
{
    vec2 Velocities[];
};

uniform float scale;
uniform float velocityMax;
  
out vec4 vertex_color;
//out vec2 uv;

void main()
{
    uint instanceID = gl_InstanceID;

    float speed = length(Velocities[instanceID]);
	float speedT = clamp(speed / velocityMax, 0.0, 1.0);
	float colT = speedT;
				
	vec3 centreWorld = vec3(Positions[instanceID], 0);
	//vec3 worldVertPos = centreWorld + mul(unity_ObjectToWorld, v.vertex * scale);
	//vec3 objectVertPos = mul(unity_WorldToObject, float4(worldVertPos.xyz, 1));

	//v2f o;
	//o.uv = v.texcoord;
	//o.pos = UnityObjectToClipPos(objectVertPos);
	//o.colour = ColourMap.SampleLevel(linear_clamp_sampler, float2(colT, 0.5), 0);

    vertex_color = vec4(0.5, 0.0, 0.0, 1.0);
    gl_Position = vec4(Positions[instanceID], 0.0, 1.0); // see how we directly give a vec3 to vec4's constructor
}
