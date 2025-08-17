#version 330 core
in vec3 v_normal;
out vec4 fragColor;
void main() {
    float nd = max(dot(normalize(v_normal), vec3(0.0, 0.0, 1.0)), 0.0);
    vec3 col = vec3(0.8) * nd + vec3(0.2);
    fragColor = vec4(col, 1.0);
}
