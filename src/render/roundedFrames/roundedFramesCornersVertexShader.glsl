R""(
precision highp float;
attribute vec2 pos;
attribute vec2 centerPos;
attribute float radius;
attribute float borderSize;
attribute vec4 insideColor;
attribute vec4 borderColor;
attribute vec4 outsideColor;
varying vec2 fragPos;
varying vec2 fragCenterPos;
varying float fragRadius;
varying float fragBorderSize;
varying vec4 fragInsideColor;
varying vec4 fragBorderColor;
varying vec4 fragOutsideColor;
uniform mat4 mat;

void main()
{
    fragPos = pos;
    fragCenterPos = centerPos;
    fragRadius = radius;
    fragBorderSize = borderSize;
    fragInsideColor = insideColor;
    fragBorderColor = borderColor;
    fragOutsideColor = outsideColor;
    gl_Position = mat * vec4(pos, 0.0, 1.0);
}
)"";