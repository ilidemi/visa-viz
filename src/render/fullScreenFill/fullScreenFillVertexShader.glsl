R""(
precision highp float;
attribute vec2 uv;

void main()
{
    gl_Position = vec4(2.0 * uv - 1.0, 0.0, 1.0);
}
)"";