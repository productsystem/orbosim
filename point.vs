#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 instancePos;
layout (location = 2) in vec3 instanceColor;

uniform mat4 view;
uniform mat4 projection;

out vec3 fragColor;

void main()
{
    vec4 worldPos = vec4(aPos + instancePos, 1.0);
    gl_Position = projection * view * worldPos;
    gl_PointSize = 16.0;
    fragColor = instanceColor;
}
