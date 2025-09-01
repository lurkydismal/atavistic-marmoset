$input a_position, a_texcoord0
$output v_texcoord0

#include <bgfx_shader.sh>

void main()
{
    // Transform position to clip space
    gl_Position = mul(u_viewProj, mul(u_model[0], vec4(a_position, 1.0)));

    // Pass through texcoords
    v_texcoord0 = a_texcoord0;
}
