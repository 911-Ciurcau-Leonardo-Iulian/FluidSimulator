#version 430 core

uniform sampler2D circle_alpha;

in vec4 vertex_color;
in vec2 uv;
out vec4 FragColor;

void main()
{
    float alpha = texture(circle_alpha, uv).r;
    FragColor = vec4(vertex_color.xyz, alpha);
}   
