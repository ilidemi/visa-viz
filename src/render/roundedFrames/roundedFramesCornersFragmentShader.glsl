R""(
precision highp float;
varying vec2 fragPos;
varying vec2 fragCenterPos;
varying float fragRadius;
varying float fragBorderSize;
varying vec4 fragInsideColor;
varying vec4 fragBorderColor;
varying vec4 fragOutsideColor;

void main()
{
    vec2 firstSample = fragPos - 0.375;
    float innerRadius = fragRadius - fragBorderSize;
	vec4 sampleColorSum;
    
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            vec2 sample = firstSample + 0.25 * vec2(float(i), float(j));
            vec2 distanceVector = sample - fragCenterPos;
            float distance = length(distanceVector);

            if (distance <= innerRadius)
            {
                sampleColorSum += fragInsideColor;
            }
            else if (distance <= fragRadius)
            {
                sampleColorSum += fragBorderColor;
            }
            else
            {
                sampleColorSum += fragOutsideColor;
            }
        }
    }
    
	gl_FragColor = sampleColorSum / 16.0;
}
)"";