#version 330 core

in vec3 posEyeSpaceFrag;
in vec3 normEyeSpaceFrag;
in vec3 light1EyeSpaceFrag;
in vec3 light2EyeSpaceFrag;
in vec3 world_pos;
in vec3 world_normal;
in vec2 texCoordFrag;
in vec3 N;
in vec3 P;


// light uniform variables
uniform vec3 light1Color;
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


// OUT
out vec4 FragColor;


void main()
{

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
   float z_depth = distance(posEyeSpaceFrag, vec3(0.0f, 0.0f, 0.0f));
   float D;
   /*if ( z_depth < z_focus)
      D = 1 - (log(z_depth/(z_focus - z_depth_min)))/(log((z_focus-z_depth_max)/( z_focus-z_depth_min)));
   else
      D = log(z_depth/(z_focus + z_depth_min))/(log((z_focus + z_depth_max)/(z_focus + z_depth_min))); */
   D = 1 -log(z_depth/z_depth_min) / log(z_depth_max/z_depth_min);
   D = clamp(D, 0, 1);

   vec4 depth_color = texture(tex_toon2d, vec2(dot(N, L), D));


   // Near-silhouette attribute mapping
   float D2 = pow(abs(dot(N, V)), r);
   vec4 orientation_color = texture(tex_toon2d, vec2(dot(N, L), D2));

   // Specular highlights
   float D3 = pow(abs(dot(V, normalize(R_EyeSpace))), shininess);
   vec4 specular_color = texture(tex_toon2d, vec2(dot(N, L), D3));
   vec4 mapColor;

   switch (mapChoice){
      case 1: { mapColor = depth_color; break; }
      case 2: { mapColor = orientation_color; break; }
      case 3: { mapColor = specular_color; break; }
   }


   // simple contour
   float c=abs(dot(normalize(P), normalize(N)));
   if (c<0.175)
   FragColor= vec4(0.f);
   else
   FragColor = mapColor;


}
