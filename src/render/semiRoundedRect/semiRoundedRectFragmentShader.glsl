R""(
precision highp float;
varying vec2 fragUv;
uniform vec2 size;
uniform vec4 color;
uniform float topLeftRadius;
uniform float topRightRadius;
uniform float bottomLeftRadius;
uniform float bottomRightRadius;

float roundedRectSdf(vec2 quarterUv, vec2 quarterRect, float radius)
{
    return length(max(abs(quarterUv) - (quarterRect - radius), vec2(0.0, 0.0))) - radius;
}

void main()
{
    float x = fragUv.x * size.x;
    float y = fragUv.y * size.y;
	float alphaSum = 0.0;
    vec2 quarterRect = size / 2.0;
    float radius;
    if (y <= quarterRect.y)
    {
        radius = x <= quarterRect.x ? topLeftRadius : topRightRadius;
    }
    else
    {
        radius = x <= quarterRect.x ? bottomLeftRadius : bottomRightRadius;
    }
    
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            vec2 sampleXy = vec2(x - 0.375 + 0.25 * float(i), y - 0.375 + 0.25 * float(j));
            vec2 sampleQuarterUv = sampleXy - quarterRect;
            float distance = roundedRectSdf(sampleQuarterUv, quarterRect, radius);
            alphaSum += color.a * float(distance <= 0.0);
        }
    }
    
    float alpha = alphaSum / 16.0;
	gl_FragColor = vec4(color.rgb, alpha);
}
)"";