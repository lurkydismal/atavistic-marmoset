#if 0
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
uniform mat4 u_modelViewProj;
out vec3 v_normal;
void main() {
    gl_Position = u_modelViewProj * vec4(a_position, 1.0);
    v_normal = normalize(a_normal);
}
#endif

attribute vec3 a_position;
attribute vec3 a_normal;
uniform mat4 u_modelViewProj;
varying vec3 v_normal;

void main() {
    gl_Position = u_modelViewProj * vec4(a_position, 1.0);
    v_normal = normalize(a_normal);
}
