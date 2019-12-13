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

//Shader* shader;
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
void renderQuad();


struct Config {


    // light 1
    glm::vec3 light1Position = {-0.8f, 2.4f, 0.0f};

    float z_depth_min = 10;
    float r = 50;
    float z_focus = 12;
    float shininess = 5;

    int choice = 1;
    bool isDepth1On = true;
    bool isDepth2On = false;
    bool isNearSilOn = false;
    bool isSpecularOn = false;

    int texChoice = 1;

    // post processing
    bool sharpen = false;
    bool edgeDetection = false;


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



    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    Shader shaderGeometryPass("g_buffer.vert", "g_buffer.frag");
    Shader shaderLightingPass("deferred_shading.vert", "deferred_shading.frag");


    // model loading
	carModel = new Model(std::vector<string>{"car_body.obj", "car_paint.obj", "car_spoiler.obj", "car_windows.obj"});
	carWheel = new Model("car_wheel.obj");


	// configure g-buffer framebuffer
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gPosition, gNormal, gAlbedoSpec;
    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    // TEXTURES INIT

    /*  PREVIOUS VERSION CODE: 1D LUT

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

    */

    // TEXTURE 1 - DEPTH BASED ATTRIBUTE MAPPING 1
    GLuint tex_toon1;
    int nrChannels1, width1,height1;
    unsigned char* image1 = stbi_load("tex1.png", &width1, &height1, &nrChannels1, 0);


    glGenTextures(1, &tex_toon1);
    glBindTexture(GL_TEXTURE_2D, tex_toon1);


    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST); //magnify - more pixelly
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST); // minimize

    if (image1){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width1, height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, image1);
        glGenerateMipmap(GL_TEXTURE_2D);

    } else{
        std::cout << "ERROR::TEXTURE::LOADFROMFILE TEX 2 LOADING FAILED" << "\n";

    }

    glActiveTexture(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image1);


    // TEXTURE 2 - DEPTH BASED ATTRIBUTE MAPPING 2
    GLuint tex_toon2;
    int nrChannels2, width2,height2;
    unsigned char* image2 = stbi_load("tex2.png", &width2, &height2, &nrChannels2, 0);


    glGenTextures(1, &tex_toon2);
    glBindTexture(GL_TEXTURE_2D, tex_toon2);


    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST); //magnify - more pixelly
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST); // minimize

    if (image2){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width2, height2, 0, GL_RGBA, GL_UNSIGNED_BYTE, image2);
        glGenerateMipmap(GL_TEXTURE_2D);

    } else{
        std::cout << "ERROR::TEXTURE::LOADFROMFILE TEX 2 LOADING FAILED" << "\n";

    }

    glActiveTexture(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image2);

    // TEXTURE 3 - NEAR-SILHOUETTE ATTRIBUTE MAPPING
    GLuint tex_toon3;
    int nrChannels3, width3,height3;
    unsigned char* image3 = stbi_load("tex3.png", &width3, &height3, &nrChannels3, 0);


    glGenTextures(1, &tex_toon3);
    glBindTexture(GL_TEXTURE_2D, tex_toon3);


    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST); //magnify - more pixelly
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST); // minimize

    if (image3){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width3, height3, 0, GL_RGBA, GL_UNSIGNED_BYTE, image3);
        glGenerateMipmap(GL_TEXTURE_2D);

    } else{
        std::cout << "ERROR::TEXTURE::LOADFROMFILE TEX 3 LOADING FAILED" << "\n";

    }

    glActiveTexture(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image3);

    // TEXTURE 2 - SPECULAR HIGHLIGHTS ATTRIBUTE MAPPING
    GLuint tex_toon4;
    int nrChannels4, width4,height4;
    unsigned char* image4 = stbi_load("tex4.png", &width4, &height4, &nrChannels4, 0);


    glGenTextures(1, &tex_toon4);
    glBindTexture(GL_TEXTURE_2D, tex_toon4);


    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST); //magnify - more pixelly
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST); // minimize

    if (image4){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width4, height4, 0, GL_RGBA, GL_UNSIGNED_BYTE, image4);
        glGenerateMipmap(GL_TEXTURE_2D);

    } else{
        std::cout << "ERROR::TEXTURE::LOADFROMFILE TEX 4 LOADING FAILED" << "\n";

    }

    glActiveTexture(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image4);


    // shader configuration
    shaderLightingPass.use();
    shaderLightingPass.setInt("gPosition", 0);
    shaderLightingPass.setInt("gNormal", 1);
    shaderLightingPass.setInt("gAlbedoSpec", 2);



    // set up the z-buffer
    glDepthRange(-1,1); // make the NDC a right handed coordinate system, with the camera pointing towards -z
    glDepthFunc(GL_LESS); // draws fragments that are closer to the screen in NDC


    // IMGUI init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    glEnable(GL_CULL_FACE);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        static float lastFrame = 0.0f;
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.5f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // camera parameters
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 viewProjection = projection * view;

        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderGeometryPass.use();

        // SET UNIFORMS
        shaderGeometryPass.setVec3("eye", camera.Position);
        shaderGeometryPass.setMat4("projection", projection);
        shaderGeometryPass.setVec3("light1Position", config.light1Position);
        shaderGeometryPass.setInt("mapChoice", config.choice);
        shaderGeometryPass.setInt("texChoice", config.texChoice);
        shaderGeometryPass.setFloat("z_depth_min", config.z_depth_min);
        shaderGeometryPass.setFloat("r", config.r);
        shaderGeometryPass.setFloat("z_focus", config.z_focus);
        shaderGeometryPass.setFloat("shininess", config.shininess);



        // PREVIOUS VERSION
        //shaderGeometryPass.setInt("text_toon", tex_toon);
        //glBindTexture(GL_TEXTURE_1D, tex_toon);


        // ASSIGN TEXTURE ACCORDING TO MAPPING METHOD
        switch (config.texChoice){
            case 1: {
                shaderGeometryPass.setInt("text_toon2d", tex_toon1);
                glBindTexture(GL_TEXTURE_2D, tex_toon1);
                break;
            }
            case 2: {
                shaderGeometryPass.setInt("text_toon2d", tex_toon2);
                glBindTexture(GL_TEXTURE_2D, tex_toon2);
                break;
            }
            case 3:{
                shaderGeometryPass.setInt("text_toon2d", tex_toon3);
                glBindTexture(GL_TEXTURE_2D, tex_toon3);
                break;
            }
            case 4:{
                shaderGeometryPass.setInt("text_toon2d", tex_toon4);
                glBindTexture(GL_TEXTURE_2D, tex_toon4);
                break;
            }
        }


        // DRAWING OF MODELS

        // draw car
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 invTranspose = glm::inverse(glm::transpose(view * model));
        shaderGeometryPass.setMat4("invTranspMV", invTranspose);
        shaderGeometryPass.setMat4("view", view);
        shaderGeometryPass.setMat4("model", model);
        carModel->Draw();

        // draw wheel
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-.6247, .32, 1.2456));
        shaderGeometryPass.setMat4("model", model);
        invTranspose = glm::inverse(glm::transpose(view * model));
        shaderGeometryPass.setMat4("invTranspMV", invTranspose);
        shaderGeometryPass.setMat4("view", view);
        carWheel->Draw();

        // draw wheel
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-.6247, .32, -1.273));
        shaderGeometryPass.setMat4("model", model);
        invTranspose = glm::inverse(glm::transpose(view * model));
        shaderGeometryPass.setMat4("invTranspMV", invTranspose);
        shaderGeometryPass.setMat4("view", view);
        carWheel->Draw();

        // draw wheel
        model = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0, 1.0, 0.0));
        model = glm::translate(model, glm::vec3(-.6247, .32, 1.2456));
        shaderGeometryPass.setMat4("model", model);
        invTranspose = glm::inverse(glm::transpose(view * model));
        shaderGeometryPass.setMat4("invTranspMV", invTranspose);
        shaderGeometryPass.setMat4("view", view);
        carWheel->Draw();

        // draw wheel
        model = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0, 1.0, 0.0));
        model = glm::translate(model, glm::vec3(-.6247, .32, -1.273));
        shaderGeometryPass.setMat4("model", model);
        invTranspose = glm::inverse(glm::transpose(view * model));
        shaderGeometryPass.setMat4("invTranspMV", invTranspose);
        shaderGeometryPass.setMat4("view", view);
        carWheel->Draw();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

      // 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
        // -----------------------------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderLightingPass.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);


        shaderLightingPass.setBool("sharpen", config.sharpen);
        shaderLightingPass.setBool("edgeDetection", config.edgeDetection);

        renderQuad();


        // 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
        // ----------------------------------------------------------------------------------
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
        // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
        // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the
        // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT,
                          GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);




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

        ImGui::Text("Light source: ");
        ImGui::DragFloat3("light position", (float*)&config.light1Position, .1, -20, 20);
        ImGui::Separator();
        ImGui::Separator();

        ImGui::Text("Choose the method to calculate D: ");
        if(ImGui::RadioButton("Depth-based Mapping 1 ", config.isDepth1On)) {
            config.choice = 1;
            config.texChoice =1;
            config.isDepth1On = true;
            config.isDepth2On = false;
            config.isNearSilOn = false;
            config.isSpecularOn = false;
        }
        if(ImGui::RadioButton("Depth-based Mapping 2 ", config.isDepth2On)) {
            config.choice = 1;
            config.texChoice =2;
            config.isDepth1On = false;
            config.isDepth2On = true;
            config.isNearSilOn = false;
            config.isSpecularOn = false;
        }
        if(ImGui::RadioButton("Near-Silhouette Mapping", config.isNearSilOn)) {
            config.choice = 2;
            config.texChoice = 3;
            config.isNearSilOn = true;
            config.isDepth1On = false;
            config.isDepth2On = false;
            config.isSpecularOn = false;
        }
        if(ImGui::RadioButton("Specular Highlights Mapping", config.isSpecularOn)) {
            config.choice = 3;
            config.texChoice = 4;
            config.isNearSilOn = false;
            config.isDepth1On = false;
            config.isDepth2On = false;
            config.isSpecularOn = true;
        }
        ImGui::Separator();
        ImGui::Separator();

        ImGui::Text("Attributes: ");
        ImGui::SliderFloat("Z min", &config.z_depth_min, 0.1, 20 );
        //ImGui::SliderFloat("Z focus", &config.z_focus, config.z_depth_min, 10*config.z_depth_min );
        ImGui::SliderFloat("r", &config.r, 1.1, 100 );
        ImGui::SliderFloat("Shininess", &config.shininess, 0.01, 50 );

        ImGui::Separator();
        ImGui::Separator();

        ImGui::Text("Post processing: ");
        ImGui::Checkbox("Sharpen", &config.sharpen);
        ImGui::Checkbox("Edge detection", &config.edgeDetection);

        ImGui::Separator();
        ImGui::Separator();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}



void drawObjects(){


   // light uniforms
    //shader->setVec3("ambientLightColor", config.ambientLightColor * config.ambientLightIntensity);




/*	*/



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

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


