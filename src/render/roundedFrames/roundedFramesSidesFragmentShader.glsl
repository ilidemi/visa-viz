R""(
precision highp float;
varying float fragBorderSize;
varying float fragBorderDistance;
varying vec4 fragInsideColor;
varying vec4 fragBorderColor;

void main()
{
    vec4 sampleColorSum;

    for (int i = 0; i < 4; i++)
    {
        float sampleDistance = fragBorderDistance - 0.375 + 0.25 * float(i);
        vec4 sampleColor = fragBorderColor * float(sampleDistance <= fragBorderSize) +
            fragInsideColor * float(sampleDistance > fragBorderSize);
        sampleColorSum += sampleColor;
    }

    gl_FragColor = sampleColorSum / 4.0;
}
)"";