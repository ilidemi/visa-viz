R""(
precision highp float;
attribute vec2 pos;
attribute vec2 vertexPos;
varying vec2 fragPos;
uniform mat4 mat;

void main()
{
    fragPos = vertexPos;
    gl_Position = mat * vec4(pos, 0.0, 1.0);
}
)"";