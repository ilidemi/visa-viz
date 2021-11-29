R""(
precision highp float;
uniform sampler2D tex;
varying vec2 uv;
uniform vec2 size;

void main()
{
    float drawRadius = size.x / 2.0;
    float x = uv.x * size.x;
    float y = uv.y * size.x;
    vec2 center = vec2(drawRadius, drawRadius);
	float alphaSum;
    
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            vec2 sample = vec2(x - 0.375 + 0.25 * float(i), y - 0.375 + 0.25 * float(j));
            alphaSum += float(
                pow(sample.x - center.x, 2.0) + pow(sample.y - center.y, 2.0) <= pow(drawRadius - 0.5, 2.0));
        }
    }
    
    float alpha = alphaSum / 16.0;
    gl_FragColor = vec4(1.0, 1.0, 1.0, alpha) * texture2D(tex, uv);
}
)"";