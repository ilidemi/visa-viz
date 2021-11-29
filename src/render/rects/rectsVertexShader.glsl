R""(
precision highp float;
attribute vec2 pos;
attribute vec4 color;
varying vec4 fragColor;
uniform mat4 mat;

void main()
{
    fragColor = color;
    gl_Position = mat * vec4(pos, 0.0, 1.0);
}
)"";