R""(
precision highp float;
varying vec2 fragPos;
uniform vec4 color;
uniform float width;
uniform float height;

void main()
{
    float sampleAlphaSum;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            vec2 sampleXy = vec2(fragPos.x - 0.375 + 0.25 * float(i), fragPos.y - 0.375 + 0.25 * float(j));
            sampleAlphaSum += float(sampleXy.x / width + 2.0 * abs(sampleXy.y / height - 0.5) <= 1.0);
        }
    }

    float alpha = sampleAlphaSum / 16.0;
    gl_FragColor = vec4(color.rgb, color.a * alpha);
}
)"";