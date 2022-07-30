// Draw a line

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer.h"
#include "Editor.h"
#include "ResourceManager.h"
#include "AudioPlayer.h"
#include "Camera.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

int main() {

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Cubes", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    ResourceManager* pResourceManager = new ResourceManager();
    AudioPlayer* pAudioHandler = new AudioPlayer();

    pAudioHandler->Init();

    std::vector<std::string> skyFaces {
        "../resources/skybox/right.jpg",
        "../resources/skybox/left.jpg",
        "../resources/skybox/top.jpg",
        "../resources/skybox/bottom.jpg",
        "../resources/skybox/front.jpg",
        "../resources/skybox/back.jpg",
    };

    pResourceManager->AddCubeMap("Default", skyFaces);

    Renderer* pRenderer = new Renderer(SCREEN_WIDTH, SCREEN_HEIGHT, pResourceManager->GetCubeMap("Default"));
    Editor* pEditor = new Editor(window);

    Camera* pCamera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));

    srand(glfwGetTime());

    pRenderer->AddShape("PBR Shape", new Shape(pResourceManager, ShapeShading::PBR, "Sphere"));
    pRenderer->AddShape("Light Source", new Shape(pResourceManager, ShapeShading::LIGHT, "Sphere"));

    pRenderer->AddModel("Backpack", "../resources/objects/backpack/backpack.obj", pResourceManager);
    
    // Render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        pCamera->Update();
        pRenderer->Draw(SCREEN_WIDTH, SCREEN_HEIGHT, pCamera, pAudioHandler);
        pEditor->Update(pRenderer, pResourceManager, pAudioHandler, pCamera);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete pRenderer;
    delete pResourceManager;
    delete pEditor;
    delete pAudioHandler;
    delete pCamera;

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}