#version 330 core

in vec4 cor_interpolada_pelo_rasterizador;

out vec4 color;

void main()
{
    color = cor_interpolada_pelo_rasterizador;
}
