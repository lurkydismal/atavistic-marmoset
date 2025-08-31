uniform sampler2D s_texColor;

varying vec2 v_texcoord0;

void main() {
    // vec4 texColor = texture2D(s_texColor, v_texcoord0);
    // gl_FragColor = vec4(1.0);
    gl_FragColor = texture2D(s_texColor, vec2(0.5, 0.5));
}
