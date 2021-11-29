R""(
precision highp float;
attribute vec2 uv;
varying vec2 fragUv;
uniform vec2 pos;
uniform vec2 size;
uniform mat4 mat;

void main()
{
    fragUv = uv;
    vec2 drawPos = pos + uv * size;
    gl_Position = mat * vec4(drawPos, 0.0, 1.0);
}
)"";