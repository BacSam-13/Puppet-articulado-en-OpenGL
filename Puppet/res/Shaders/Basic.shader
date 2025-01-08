#shader vertex
#version 330 core
    
layout(location = 0) in vec3 position;
    
out vec4 vertex_Color;

uniform mat4 mvp;
uniform mat4 M;

void main()
{
    gl_Position = mvp * vec4(position,1.0f);
    vertex_Color = vec4(clamp(position, 0.3f, 1.0f), 1.0f); // Generamos un color en base a la posicion, y no la posicion como tal

};

#shader fragment
#version 330 core
    
layout(location = 0) out vec4 color;
const float eps = 1.0; // Variable para modificar el color
in vec4 vertex_Color; 
    
void main()
{
    // Modificamos y regresamos un color para dibujar
    vec3 corrected_color = pow(vertex_Color.rgb, vec3(1.0 / eps));
    color = vec4(clamp(corrected_color, 0.0, 1.0), vertex_Color.a);
};