#version 330 core

layout (location = 0) in vec4 model_coefficients;
layout (location = 1) in vec4 color_coefficients;

out vec4 cor_interpolada_pelo_rasterizador;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool render_as_black;

void main()
{
    gl_Position = projection * view * model * model_coefficients;

    if ( render_as_black )
    {
      cor_interpolada_pelo_rasterizador = vec4(0.0f,0.0f,0.0f,1.0f);
    }
    else
    {
      cor_interpolada_pelo_rasterizador = color_coefficients;
    }
}
