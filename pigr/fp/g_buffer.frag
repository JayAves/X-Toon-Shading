#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 posEyeSpaceFrag;
in vec3 normEyeSpaceFrag;
in vec3 light1EyeSpaceFrag;
in vec3 world_pos;
in vec3 world_normal;
in vec2 texCoordFrag;


// light uniform variables
uniform vec3 light1Position;


// Textures
uniform sampler1D tex_toon; // PREVIOUS VERSION
uniform sampler2D tex_toon2d;

// Choice of texture
uniform int texChoice;

// Choice of mapping alg
uniform int mapChoice;

// Choice of custom color
uniform vec4 texCustomColor;

// camera position
uniform vec3 eye;



// Parameters
uniform float z_depth_min; // must be > 0
uniform float r; // must be > 1
uniform float z_focus;
uniform float shininess;

float z_depth_max = r * z_depth_min;

void main()
{
   // store the fragment position vector in the first gbuffer texture
   gPosition = world_pos;

   // also store the per-fragment normals into the gbuffer
   //vec3 normalMap = texture(texture_normal1, TexCoords).rgb;
   //normalMap = normalMap * 2.0 - 1.0;
   //normalMap = normalize(TBN * normalMap);
   //gNormal = normalize(mix(normalize(Normal), normalMap, normalMappingMix));
   gNormal = normalize(world_normal);
   // and the diffuse per-fragment color


   // ALL CALCULATIONS FOR DIFFUSE COLOR


   // Calculate per-pixel normal, light vector, view and reflection vector
   vec3 N = normalize(normEyeSpaceFrag);
   vec3 L = normalize(light1Position - posEyeSpaceFrag);
   vec3 V = normalize(posEyeSpaceFrag);
   vec3 L_EyeSpace = normalize(light1EyeSpaceFrag-posEyeSpaceFrag).xyz;
   vec3 R_EyeSpace =  - L_EyeSpace - 2 * dot(-L_EyeSpace, normEyeSpaceFrag) * normEyeSpaceFrag;

   // toon shading 1D LUT - PREVIOUS VERSION
   //float tc = pow(max(0.0, dot(N, L)), shininess);
   //vec4 color = texture(tex_toon, tc);

   // Depth based attribute mapping //
   float z_depth = distance(posEyeSpaceFrag, vec3(0.0f,0.0f,0.0f));
   float D = 1 -log(z_depth/z_depth_min) / log(z_depth_max/z_depth_min);
   vec4 depth_color = texture(tex_toon2d, vec2(dot(N, L), D));


   // Near-silhouette attribute mapping
   float D2 = pow(abs(dot(N,V)), r);
   vec4 orientation_color = texture(tex_toon2d, vec2(dot(N, L), D2));

   // Specular highlights
   float D3 = pow(abs(dot(V, normalize(R_EyeSpace))), shininess);
   vec4 specular_color = texture(tex_toon2d, vec2(dot(N, L), D3));


   vec4 mapColor; // final color

   switch(mapChoice){
      case 1: {mapColor = depth_color; break;}
      case 2: {mapColor = orientation_color; break;}
      case 3: {mapColor = specular_color; break;}
   }

   gAlbedoSpec.rgb = mapColor.rgb;

   gAlbedoSpec.a = mapColor.a; // NOT USED IN FRAGMENT
}

