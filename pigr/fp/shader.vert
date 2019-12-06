#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textCoord;


uniform mat4 model; // represents model in the world coord space
uniform mat4 view;  // represents the world in the eye coord space
uniform mat4 invTranspMV; // inverse of the transpose of (view * model) (used to multiply vectors if there is non-uniform scaling)
uniform mat4 projection; // camera projection matrix

// light uniform variables
uniform vec3 ambientLightColor;
uniform vec3 light1Position;
uniform vec3 light1Color;
uniform vec3 light2Position;
uniform vec3 light2Color;

// material properties
uniform vec3 reflectionColor;
uniform float ambientReflectance;
uniform float diffuseReflectance;
uniform float specularReflectance;
uniform float specularExponent;


out vec3 posEyeSpaceFrag;
out vec3 normEyeSpaceFrag;
out vec3 light1EyeSpaceFrag;
out vec3 light2EyeSpaceFrag;
out vec3 world_pos;
out vec3 world_normal;
out vec2 texCoordFrag;




void main() {
   
// vertex in eye space (for light computation in eye space)
   vec4 posEyeSpace = view * model * vec4(vertex, 1.0);
   // normal in eye space (for light computation in eye space)
   vec3 normEyeSpace = normalize((invTranspMV * vec4(normal, 0.0)).xyz);
   // light in eye space
   vec4 light1EyeSpace = view * vec4(light1Position, 1.0);

   // final vertex transform (for opengl rendering)
   gl_Position = projection * posEyeSpace;

   posEyeSpaceFrag = posEyeSpace.xyz;
   normEyeSpaceFrag = normEyeSpace;
   light1EyeSpaceFrag = light1EyeSpace.xyz;
   light2EyeSpaceFrag = (view * vec4(light2Position, 1.0)).xyz;
   world_pos = mat3(model) * vertex;
   texCoordFrag = textCoord;

}
