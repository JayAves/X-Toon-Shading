#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;


uniform bool sharpen;
uniform bool edgeDetection;


void main()
{
   // retrieve data from gbuffer
   vec3 FragPos = texture(gPosition, TexCoords).rgb;
   vec3 Normal = texture(gNormal, TexCoords).rgb;

   vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;

   vec2 texelSize = 1.0 / textureSize(gPosition, 0);
   vec2 up = vec2(0.0, texelSize.y);
   vec2 right = vec2(texelSize.x, 0.0);


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


   FragColor = vec4(Diffuse, 1.0);



}

