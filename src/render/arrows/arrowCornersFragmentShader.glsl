R""(
precision highp float;
varying vec2 fragPos;
varying vec2 fragCenterUv;
uniform vec4 color;
uniform float size;
uniform float radius;
uniform float thickness;

void main()
{
    float halfThickness = thickness / 2.0;
    vec2 centerXy = fragCenterUv * size;
    float sampleAlphaSum;
    
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            vec2 sampleXy = vec2(fragPos.x - 0.375 + 0.25 * float(i), fragPos.y - 0.375 + 0.25 * float(j));
            float distance = length(sampleXy - centerXy);
            float sampleAlpha =
                float(distance >= radius - halfThickness && distance < radius + halfThickness);
            sampleAlphaSum += sampleAlpha;
        }
    }
    
	float alpha = sampleAlphaSum / 16.0;
    gl_FragColor = vec4(color.rgb, color.a * alpha);
}
)"";