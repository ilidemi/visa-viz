R""(
precision highp float;
attribute vec2 pos;
attribute vec2 textureUv;
varying vec2 uv;
uniform mat4 mat;

void main()
{
    uv = vec2(textureUv.x, 1.0 - textureUv.y);
    gl_Position = mat * vec4(pos, 0.0, 1.0);
}
)"";