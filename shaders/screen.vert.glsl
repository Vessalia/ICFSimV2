#version 460 core
out vec2 texCoord;

const vec2 vertices[6] = 
{
    {-1, 1}, {-1, -1}, {1, 1},
    {1, 1}, {-1, -1}, {1, -1}
};

const vec2 texCoords[6] =
{
    {0, 1}, {0, 0}, {1, 1},
    {1, 1}, {0, 0}, {1, 0}
};

void main()
{
    gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0); 
    texCoord = texCoords[gl_VertexID];
}  