R""(
precision highp float;
attribute vec2 textureUv;
varying vec2 uv;
uniform vec2 pos;
uniform vec2 size;
uniform mat4 mat;

void main()
{
    uv = vec2(textureUv.x, 1.0 - textureUv.y);
    vec2 drawPos = pos + textureUv * size;
    gl_Position = mat * vec4(drawPos, 0.0, 1.0);
}
)"";