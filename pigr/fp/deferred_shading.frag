#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;


uniform bool sharpen;
uniform bool edgeDetection;
uniform bool sobelEdgeDetection;


void main()
{
   // retrieve data from gbuffer
   vec3 FragPos = texture(gPosition, TexCoords).rgb;
   vec3 Normal = texture(gNormal, TexCoords).rgb;

   vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;

   vec2 texelSize = 1.0 / textureSize(gPosition, 0);
   vec2 up = vec2(0.0, texelSize.y);
   vec2 right = vec2(texelSize.x, 0.0);

   vec3 edgeColor = vec3(0.f);


   if (sharpen){
      // sharpen filter
      // apply to gAlbedoSpec
      vec4 sharp = -texture(gAlbedoSpec, TexCoords + up).rgba
      - texture(gAlbedoSpec, TexCoords - up).rgba
      - texture(gAlbedoSpec, TexCoords + right).rgba
      - texture(gAlbedoSpec, TexCoords - right).rgba
      + 5.0 * texture(gAlbedoSpec, TexCoords).rgba;

      Diffuse = sharp.rgb;


   }


   if (edgeDetection){
      // laplacian operator
      // apply to normals
       vec3 laplacian = texture(gNormal, TexCoords+up).rgb
          + texture(gNormal, TexCoords-up).rgb
          + texture(gNormal, TexCoords+right).rgb
          + texture(gNormal, TexCoords-right).rgb
          - 4.0 * Normal;
       float l = length(laplacian);

       // value in the range [0, 1], bigger values indicate less curvature
       float flatness = 1.0 - clamp(l, 0.0, 1.0);
       Diffuse *= flatness;




   }
    if(sobelEdgeDetection){

        mat3 sx = mat3(
        1.0, 2.0, 1.0,
        0.0, 0.0, 0.0,
        -1.0, -2.0, -1.0
        );
        mat3 sy = mat3(
        1.0, 0.0, -1.0,
        2.0, 0.0, -2.0,
        1.0, 0.0, -1.0
        );

        mat3 I;

        // Sampling 9 texels
        for (int i=0; i<3; i++) {
            for (int j=0; j<3; j++) {
                vec3 samplez  = texelFetch(gAlbedoSpec, ivec2(gl_FragCoord) + ivec2(i-1,j-1), 0 ).rgb;
                I[i][j] = length(samplez);
            }
        }

        float gx = dot(sx[0], I[0]) + dot(sx[1], I[1]) + dot(sx[2], I[2]);
        float gy = dot(sy[0], I[0]) + dot(sy[1], I[1]) + dot(sy[2], I[2]);

        float g = sqrt(pow(gx, 2.0)+pow(gy, 2.0));

        g = smoothstep(0.4, 0.6, g);
        Diffuse = mix(Diffuse, edgeColor, g);
    }


    FragColor = vec4(Diffuse, 1.0);




}

