R""(
attribute vec4 pos;
attribute vec2 textureUv;
varying vec2 uv;
uniform mat4 mat;

void main()
{
    uv = textureUv;
    gl_Position = mat * pos;
}
)"";