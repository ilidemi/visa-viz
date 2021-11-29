R""(
precision highp float;
attribute vec2 arrowCornerPos;
attribute vec2 vertexPos;
attribute vec2 centerUv;
varying vec2 fragPos;
varying vec2 fragCenterUv;
uniform mat4 mat;

void main()
{
    fragPos = vertexPos;
    fragCenterUv = centerUv;
    vec2 drawPos = arrowCornerPos + vertexPos;
    gl_Position = mat * vec4(drawPos, 0.0, 1.0);
}
)"";