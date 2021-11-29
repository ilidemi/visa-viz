R""(
precision highp float;
attribute vec2 pos;
attribute float borderDistance;
attribute float borderSize;
attribute vec4 insideColor;
attribute vec4 borderColor;
varying float fragBorderDistance;
varying float fragBorderSize;
varying vec4 fragInsideColor;
varying vec4 fragBorderColor;
uniform mat4 mat;

void main()
{
    fragBorderDistance = borderDistance;
    fragBorderSize = borderSize;
    fragInsideColor = insideColor;
    fragBorderColor = borderColor;
    gl_Position = mat * vec4(pos, 0.0, 1.0);
}
)"";