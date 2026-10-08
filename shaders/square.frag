#version 330 core

flat in int state;

out vec4 FragColor;

void main()
{
    if (state == 0)
        FragColor = vec4(0.4, 0.4, 0.4, 1.0);
    else if (state == 1)
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    else if (state == 2)
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}