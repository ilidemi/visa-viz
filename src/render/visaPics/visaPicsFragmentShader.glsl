R""(
precision highp float;
varying vec2 fragPos;
varying float fragSize;
uniform sampler2D tex;

void main()
{
    float radius = fragSize / 2.0;
    vec2 center = vec2(radius, radius);
	float alphaSum;
    
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            vec2 sample = vec2(fragPos.x - 0.375 + 0.25 * float(i), fragPos.y - 0.375 + 0.25 * float(j));
            alphaSum += float(
                pow(sample.x - center.x, 2.0) + pow(sample.y - center.y, 2.0) <= pow(radius - 0.5, 2.0));
        }
    }
    float alpha = alphaSum / 16.0;
    
    vec2 uv = vec2(fragPos.x / fragSize, 1.0 - fragPos.y / fragSize);
    gl_FragColor = vec4(texture2D(tex, uv).rgb, alpha);
}
)"";