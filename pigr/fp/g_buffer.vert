#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textCoord;

out vec3 posEyeSpaceFrag;
out vec3 normEyeSpaceFrag;
out vec3 light1EyeSpaceFrag;
out vec3 world_pos;
out vec3 world_normal;
out vec2 texCoordFrag;


uniform mat4 model; // represents model in the world coord space
uniform mat4 view;  // represents the world in the eye coord space
uniform mat4 invTranspMV; // inverse of the transpose of (view * model) (used to multiply vectors if there is non-uniform scaling)
uniform mat4 projection; // camera projection matrix
uniform vec3 light1Position;


void main()
{
   // vertex in eye space (for light computation in eye space)
   vec4 posEyeSpace = view * model * vec4(vertex, 1.0);
   // normal in eye space (for light computation in eye space)
   vec3 normEyeSpace = normalize((invTranspMV * vec4(normal, 0.0)).xyz);
   // light in eye space
   vec4 light1EyeSpace = view * vec4(light1Position, 1.0);

   mat4 modelView = view * model;
   mat4 normalMatrix = transpose(inverse(modelView));

   // FINAL TRANSFORM
   gl_Position = projection * posEyeSpace;


   // OUT VECTORS
   posEyeSpaceFrag = posEyeSpace.xyz;
   normEyeSpaceFrag = normEyeSpace;
   light1EyeSpaceFrag = light1EyeSpace.xyz;
   world_pos = (model * vec4(vertex, 1.0)).xyz;
   world_normal = transpose(inverse(mat3(model))) * normal;
   texCoordFrag = textCoord;


}

