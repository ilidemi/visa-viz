R""(
precision highp float;
attribute vec2 pos;
attribute vec2 vertexPos;
attribute float size;
varying vec2 fragPos;
varying float fragSize;
uniform mat4 mat;

void main()
{
    fragPos = vertexPos;
    fragSize = size;
    gl_Position = mat * vec4(pos, 0.0, 1.0);
}
)"";