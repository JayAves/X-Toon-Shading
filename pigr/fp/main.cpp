#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <vector>
#include <chrono>

#include "shader.h"
#include "camera.h"
#include "model.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

Shader* shader;
Model* carModel;
Model* carWheel;
Camera camera(glm::vec3(0.0f, 1.6f, 5.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
float deltaTime;
// used to stop camera movement when GUI is open
bool isPaused = false;

void drawObjects();
void drawGui();
void processInput(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);


struct Config {

    // ambient light
   // glm::vec3 ambientLightColor = {1.0f, 1.0f, 1.0f};
   // float ambientLightIntensity = 0.2f;

    // light 1
    glm::vec3 light1Position = {-0.8f, 2.4f, 0.0f};
   // glm::vec3 light1Color = {1.0f, 1.0f, 1.0f};
    //float light1Intensity = 1.0f;

    float z_depth_min = 5;
    float r = 50;
    float z_focus = 7;
    float shininess = 5;

    int choice = 1;
    bool isDepthOn = true;
    bool isNearSilOn = false;
    bool isSpecularOn = false;

    // light 2
   // glm::vec3 light2Position = {1.8f, .7f, 2.2f};
   // glm::vec3 light2Color = {2.f, 0.0f, 1.0f};
    //float light2Intensity = 1.0f;

    // material
   /* glm::vec3 reflectionColor = {1.0f, 0.0f, 1.0f};
    float ambientReflectance = 0.5f;
    float diffuseReflectance = 0.5f;
    float specularReflectance = 0.7f;
    float specularExponent = 20.0f;

    // attenuation (c0, c1 and c2 on the slides)
    float attenuationC0 = 0.5;
    float attenuationC1 = 0.1;
    float attenuationC2 = 0.1; */

} config;



int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Pigr X-Toon", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_input_callback);
    glfwSetKeyCallback(window, key_input_callback);
	glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


	shader = new Shader("shader.vert", "shader.frag");
	carModel = new Model(std::vector<string>{"car_body.obj", "car_paint.obj", "car_spoiler.obj", "car_windows.obj"});
	carWheel = new Model("car_wheel.obj");




    //declare our LUT
    GLuint  tex_toon;
    static GLubyte toon_tex_data[] =
            {
                    0x44, 0x00, 0x00, 0x00,
                    0x88, 0x00, 0x00, 0x00,
                    0xCC, 0x00, 0x00, 0x00,
                    0xFF, 0x00, 0x00, 0x00
            };



    glGenTextures(1 , &tex_toon); //Generate texture
    glBindTexture(GL_TEXTURE_1D, tex_toon); //Bind texture, a 1D texture
   // glTextureStorage1D(GL_TEXTURE_1D, 1, GL_RGB8,  4);


    //Pass the data
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //Conditions

    glTexImage1D(GL_TEXTURE_1D, 0,
                    GL_RGB8, sizeof(toon_tex_data) / 4, 0,
                    GL_RGBA, GL_UNSIGNED_BYTE,
                    toon_tex_data);

    glGenerateMipmap(GL_TEXTURE_1D);


    GLuint tex_toon2;
    int nrChannels, width,height;
    unsigned char* image = stbi_load("tex1.png", &width, &height, &nrChannels, 0);


    glGenTextures(1, &tex_toon2);
    glBindTexture(GL_TEXTURE_2D, tex_toon2);


    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST); //magnify - more pixelly
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST); // minimize

    if (image){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);

    } else{
        std::cout << "ERROR::TEXTURE::LOADFROMFILE TEX 2 LOADING FAILED" << "\n";

    }

    glActiveTexture(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);


    GLuint te;
    GLubyte m[sizeof(toon_tex_data)][sizeof(toon_tex_data)];


    for (int i=0; i< sizeof(toon_tex_data); i++){
        for(int j=0; j< sizeof(toon_tex_data); j++){
            m[i][j] = toon_tex_data[i] + 10*j;

        }
    }





    // set up the z-buffer
    glDepthRange(-1,1); // make the NDC a right handed coordinate system, with the camera pointing towards -z
    glEnable(GL_DEPTH_TEST); // turn on z-buffer depth test
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC


    // IMGUI init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");


    // render loop
    while (!glfwWindowShouldClose(window))
    {
        static float lastFrame = 0.0f;
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader->use();
        shader->setInt("text_toon", tex_toon);
        glBindTexture(GL_TEXTURE_1D, tex_toon);
        shader->setInt("text_toon2", tex_toon2);
        glBindTexture(GL_TEXTURE_2D, tex_toon2);

        drawObjects();


		if (isPaused) {
			drawGui();
		}

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	delete carModel;
    delete carWheel;
    delete shader;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}



void drawGui(){
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Settings");

       /* ImGui::Text("Ambient light: ");
        ImGui::ColorEdit3("ambient light color", (float*)&config.ambientLightColor);
        ImGui::SliderFloat("ambient light intensity", &config.ambientLightIntensity, 0.0f, 1.0f);
        ImGui::Separator(); */

        ImGui::Text("Light source: ");
        ImGui::DragFloat3("light position", (float*)&config.light1Position, .1, -20, 20);
       // ImGui::ColorEdit3("light color", (float*)&config.light1Color);
        //ImGui::SliderFloat("light intensity", &config.light1Intensity, 0.0f, 1.0f);
        ImGui::Separator();
        ImGui::Separator();

        ImGui::Text("Choose the method to calculate D: ");
        if(ImGui::RadioButton("Depth-based Mapping", config.isDepthOn)) {
            config.choice = 1;
            config.isDepthOn = true;
            config.isNearSilOn = false;
            config.isSpecularOn = false;
        }
        if(ImGui::RadioButton("Near-Silhouette Mapping", config.isNearSilOn)) {
            config.choice = 2;
            config.isNearSilOn = true;
            config.isDepthOn = false;
            config.isSpecularOn = false;
        }
        if(ImGui::RadioButton("Specular Highlights Mapping", config.isSpecularOn)) {
            config.choice = 3;
            config.isNearSilOn = false;
            config.isDepthOn = false;
            config.isSpecularOn = true;
        }
        ImGui::Separator();
        ImGui::Separator();

        ImGui::Text("Attributes: ");
        ImGui::SliderFloat("Z min", &config.z_depth_min, 0.1, 20 );
        ImGui::SliderFloat("Z focus", &config.z_focus, config.z_depth_min, 10*config.z_depth_min );
        ImGui::SliderFloat("r", &config.r, 1.1, 100 );
        ImGui::SliderFloat("Shininess", &config.shininess, 0.01, 50 );
        ImGui::Separator();
        ImGui::Separator();



      /*  ImGui::Text("Light 2: ");
        ImGui::DragFloat3("light 2 position", (float*)&config.light2Position, .1, -20, 20);
        ImGui::ColorEdit3("light 2 color", (float*)&config.light2Color);
        ImGui::SliderFloat("light 2 intensity", &config.light2Intensity, 0.0f, 1.0f);
        ImGui::Separator();

        ImGui::Text("Material: ");
        ImGui::ColorEdit3("reflection color", (float*)&config.reflectionColor);
        ImGui::SliderFloat("ambient reflectance", &config.ambientReflectance, 0.0f, 1.0f);
        ImGui::SliderFloat("diffuse reflectance", &config.diffuseReflectance, 0.0f, 1.0f);
        ImGui::SliderFloat("specular reflectance", &config.specularReflectance, 0.0f, 1.0f);
        ImGui::SliderFloat("specular exponent", &config.specularExponent, 0.0f, 100.0f);
        ImGui::Separator();

        ImGui::Text("Attenuation: ");
        ImGui::SliderFloat("attenuation c0", &config.attenuationC0, 0.0f, 1.0f);
        ImGui::SliderFloat("attenuation c1", &config.attenuationC1, 0.0f, 1.0f);
        ImGui::SliderFloat("attenuation c2", &config.attenuationC2, 0.0f, 1.0f);
        ImGui::Separator();
*/

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}



void drawObjects(){


   // light uniforms
    //shader->setVec3("ambientLightColor", config.ambientLightColor * config.ambientLightIntensity);
    shader->setVec3("light1Position", config.light1Position);
    shader->setInt("mapChoice", config.choice);
    shader->setFloat("z_depth_min", config.z_depth_min);
    shader->setFloat("r", config.r);
    shader->setFloat("z_focus", config.z_focus);
    shader->setFloat("shininess", config.shininess);


   // shader->setVec3("light1Color", config.light1Color * config.light1Intensity);
  /*  shader->setVec3("light2Position", config.light2Position);
    shader->setVec3("light2Color", config.light2Color * config.light2Intensity);

    // material uniforms
    shader->setVec3("reflectionColor", config.reflectionColor);
    shader->setFloat("ambientReflectance", config.ambientReflectance);
    shader->setFloat("diffuseReflectance", config.diffuseReflectance);
    shader->setFloat("specularReflectance", config.specularReflectance);
    shader->setFloat("specularExponent", config.specularExponent);

    // attenuation uniforms
    shader->setFloat("attenuationC0", config.attenuationC0);
    shader->setFloat("attenuationC1", config.attenuationC1);
    shader->setFloat("attenuationC2", config.attenuationC2); */


    // camera parameters
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 viewProjection = projection * view;

	// set projection matrix uniform
    shader->setMat4("projection", projection);

	// draw car
	glm::mat4 model = glm::mat4(1.0f);
    shader->setMat4("model", model);
	glm::mat4 invTranspose = glm::inverse(glm::transpose(view * model));
    shader->setMat4("invTranspMV", invTranspose);
    shader->setMat4("view", view);
    carModel->Draw();


	// draw wheel
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-.6247, .32, 1.2456));
	shader->setMat4("model", model);
	invTranspose = glm::inverse(glm::transpose(view * model));
	shader->setMat4("invTranspMV", invTranspose);
	shader->setMat4("view", view);
	carWheel->Draw();

	// draw wheel
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-.6247, .32, -1.273));
	shader->setMat4("model", model);
	invTranspose = glm::inverse(glm::transpose(view * model));
	shader->setMat4("invTranspMV", invTranspose);
	shader->setMat4("view", view);
	carWheel->Draw();

	// draw wheel
	model = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0, 1.0, 0.0));
	model = glm::translate(model, glm::vec3(-.6247, .32, 1.2456));
	shader->setMat4("model", model);
	invTranspose = glm::inverse(glm::transpose(view * model));
	shader->setMat4("invTranspMV", invTranspose);
	shader->setMat4("view", view);
	carWheel->Draw();

	// draw wheel
	model = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0, 1.0, 0.0));
	model = glm::translate(model, glm::vec3(-.6247, .32, -1.273));
	shader->setMat4("model", model);
	invTranspose = glm::inverse(glm::transpose(view * model));
	shader->setMat4("invTranspMV", invTranspose);
	shader->setMat4("view", view);
	carWheel->Draw();



}



void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

	if (isPaused)
		return;

	// movement commands
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

}



void cursor_input_callback(GLFWwindow* window, double posX, double posY){

	// camera rotation
    static bool firstMouse = true;
    if (firstMouse)
    {
        lastX = posX;
        lastY = posY;
        firstMouse = false;
    }

    float xoffset = posX - lastX;
    float yoffset = lastY - posY; // reversed since y-coordinates go from bottom to top

    lastX = posX;
    lastY = posY;

	if (isPaused)
		return;

    camera.ProcessMouseMovement(xoffset, yoffset);
}


void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods){

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        isPaused = !isPaused;
        glfwSetInputMode(window, GLFW_CURSOR, isPaused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}