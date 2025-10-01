#version 460 core
in vec3 fragColor;
out vec4 FragColor;

void main()
{
    vec2 uv = gl_PointCoord * 2.0 - 1.0;
    float dist = length(uv);
    float alpha = smoothstep(1.0, 0.0, dist);
    float glow = exp(-4.0 * dist * dist);
    FragColor = vec4(fragColor * glow, alpha);
}
