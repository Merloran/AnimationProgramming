﻿#include "imgui.h"
#include "imgui_impl/imgui_impl_glfw.h"
#include "imgui_impl/imgui_impl_opengl3.h"

#include "Public/Shader.h"
#include "Public/Camera.h"

#include "Public/Texture.h"
#include "Public/CubeMap.h"
#include "Public/PBRManager.h"
#include "Public/BloomRenderer.h"

#include "Public/ParticleSystem.h"

#include "Public/Model.h"
#include "Public/InstancedModel.h"
#include "Public/Cube.h"
#include "Public/Quad.h"
#include "Public/Entity.h"

#include "Public/PointLight.h"
#include "Public/DirectionalLight.h"
#include "Public/SpotLight.h"
#include "Public/Shadow.h"

#include "Public/Box.h"

#include <stdio.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>  // Initialize with gladLoadGL()

#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions
#include <spdlog/spdlog.h>

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

const int WINDOW_WIDTH = 1366;
const int WINDOW_HEIGHT = 768;

float Zoom = 45.0f;
void MouseCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (Zoom >= 1.0f && Zoom <= 45.0f)
    {
        Zoom -= yoffset;
    }
    else if (Zoom < 1.0f)
    {
        Zoom = 1.0f;
    }
    else if (Zoom > 45.0f)
    {
        Zoom = 45.0f;
    }
}

bool isSpotlightOn = true;
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_E:
        {
            if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            else
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            break;
        }
        case GLFW_KEY_F:
        {
            isSpotlightOn = !isSpotlightOn;
            break;
        }
        case GLFW_KEY_ESCAPE:
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        }
        default:
            break;
        }
    }
}


int main(int, char**)
{
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); //Memory leak check
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        return 1;
    }

    // GL 4.3 + GLSL 430
    const char* glsl_version = "#version 460";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL Learning", NULL, NULL);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    if (window == NULL)
    {
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // Enable vsync


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        spdlog::error("Failed to initialize OpenGL loader!");
        return 1;
    }
    spdlog::info("Successfully initialized OpenGL loader!");

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup style
    ImGui::StyleColorsDark();

    unsigned int amount = 1000000U;
    std::vector<glm::mat4> modelMatrices;
    modelMatrices.reserve(amount);
    srand(glfwGetTime()); // zainicjuj losowe ziarno
    float radius = 80.0;
    float offset = 20.0f;
    for (unsigned int i = 0; i < amount; ++i)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1. translacja: przesuwaj po okręgu o "promieniu" w zakresie [-offset, offset]
        float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));

        // 2. Skala: przeskaluj od 0.05 do 0.25f
        float scale = (rand() % 20) / 100.0f + 0.05;
        model = glm::scale(model, glm::vec3(scale));

        // 3. rotation: dodaj losow¹ rotacjê wokó³ (pó³) losowo wybranego wektora osi obrotu
        float rotAngle = (rand() % 360);
        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

        // 4. teraz dodaj do listy macierzy
        modelMatrices.push_back(model);
    }

    glEnable(GL_DEPTH_TEST);
    // set depth function to less than AND equal for skybox depth trick.
    glDepthFunc(GL_LEQUAL);
    // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    Shader lightShader("res/shaders/MVP.vs", "res/shaders/LightGizmo.fs");
    Shader screenShader("res/shaders/Screen.vs", "res/shaders/Screen.fs");
    Shader skyBoxShader("res/shaders/SkyBox.vs", "res/shaders/SkyBox.fs");
    Shader normalShader("res/shaders/Normals.vs", "res/shaders/Normals.fs", "res/shaders/Normals.gs");
    Shader shadowShader("res/shaders/ShadowMap.vs", "res/shaders/ShadowMap.fs");
    Shader instanceShader("res/shaders/Instance.vs", "res/shaders/Instance.fs");
    Shader blurShader("res/shaders/Screen.vs", "res/shaders/Blur.fs");
    Shader particleShader("res/shaders/Particle.vert", "res/shaders/Particle.frag");
    Shader computeShader("res/shaders/Compute.comp");

    Shader PBRShader("res/shaders/PBR/PBR.vs", "res/shaders/PBR/PBR.fs");
    Shader equirectangularShader("res/shaders/PBR/CubeMap.vs", "res/shaders/PBR/Equirectangular.fs");
    Shader irradianceShader("res/shaders/PBR/CubeMap.vs", "res/shaders/PBR/IrradianceConvolution.fs");
    Shader prefilterShader("res/shaders/PBR/CubeMap.vs", "res/shaders/PBR/Prefilter.fs");
    Shader BRDFShader("res/shaders/PBR/BRDF.vs", "res/shaders/PBR/BRDF.fs");

    Model generator("res/models/generator/generator.obj");
    InstancedModel box("res/models/box/box.obj", modelMatrices);
    //Model Scene1 = Model("res/models/sponza/Sponza.gltf");
    Model Scene2 = Model("res/models/bistro/bistro.gltf");
    // pbr: load the HDR environment map
    // ---------------------------------
    Texture HDR("res/textures/Canyon/Canyon.hdr");

    // pbr: setup cubemap to render to and attach to framebuffer
    // ---------------------------------------------------------
    CubeMap Environment(512, 512);

    CubeMap Irradiance(32, 32);
    glBindTexture(GL_TEXTURE_CUBE_MAP, Irradiance.GetId());
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    CubeMap Prefilter(128, 128);

    Texture BRDF(512, 512, 2);

    PBRManager::GetInstance().SetupEquirectangular(equirectangularShader, HDR, Environment);
    PBRManager::GetInstance().SetupIrradiance(irradianceShader, Irradiance, Environment);
    PBRManager::GetInstance().SetupPrefilter(prefilterShader, Prefilter, Environment);
    PBRManager::GetInstance().SetupBRDF(BRDFShader, BRDF);

    BloomRenderer bloomRenderer(WINDOW_WIDTH, WINDOW_HEIGHT);

    Camera camera;

    std::vector<PointLight> pointLights =
    {
        PointLight(pointLightPositions[0], glm::vec3(1.0f, 0.078f, 0.576f), AttenuationDist::D_7),
        PointLight(pointLightPositions[1], glm::vec3(1.0f, 0.078f, 0.576f), AttenuationDist::D_7),
        PointLight(pointLightPositions[2], glm::vec3(1.0f, 0.078f, 0.576f), AttenuationDist::D_7),
        PointLight(pointLightPositions[3], glm::vec3(1.0f, 0.078f, 0.576f), AttenuationDist::D_7),
    };
    for (PointLight& light : pointLights)
    {
        light.SetIntensity(30.0f);
    }

    std::vector<DirectionalLight> dirLights =
    {
        DirectionalLight(glm::vec3(0.0f, -15.0f, 4.0f), glm::vec3(0.5647f, 0.7529f, 0.8745f))
    };
    dirLights[0].SetIntensity(2.0f);

    std::vector<SpotLight> spotLights =
    {
        SpotLight(camera.Position, camera.ForwardVector, glm::vec3(1.0f)),
        SpotLight(glm::vec3(-5.0f, 7.0f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f))
    };
    for (SpotLight& light : spotLights)
    {
        light.SetIntensity(100.0f);
    }
    spotLights[0].SetAttenuationParams(AttenuationDist::D_20);
    spotLights[1].SetAttenuationParams(AttenuationDist::D_7);

    std::vector<Light*> lights;
    lights.reserve(pointLights.size() + dirLights.size() + spotLights.size());
    for (Light& light : pointLights)
    {
        lights.push_back(&light);
    }
    for (Light& light : dirLights)
    {
        lights.push_back(&light);
    }
    for (Light& light : spotLights)
    {
        lights.push_back(&light);
    }


    Entity Root;
    //Root.AddChild(Scene1, "Sponza", PBRShader);
    //Root.children.back().get()->transform.SetLocalPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    //Root.children.back().get()->transform.SetLocalScale(glm::vec3(0.01f));

    Root.AddChild(Scene2, "Bistro", PBRShader);
    Root.children.back().get()->transform.SetLocalPosition(glm::vec3(0.0f, 0.0f, 0.0f));

    Root.AddChild(generator, "Generator", PBRShader);
    Root.children.back().get()->transform.SetLocalPosition(glm::vec3(-8.0f, 0.4f, 0.0f));

    Root.AddChild(pointLights[0], "PointLight1", lightShader);
    Root.AddChild(pointLights[1], "PointLight2", lightShader);
    Root.AddChild(pointLights[2], "PointLight3", lightShader);
    Root.AddChild(pointLights[3], "PointLight4", lightShader);
    Root.AddChild(dirLights[0], "DirectionalLight", lightShader);
    Root.AddChild(spotLights[1], "SpotLight", lightShader);

    Root.AddChild(box, "CubeRing", instanceShader);
    Root.children.back().get()->transform.SetLocalPosition(glm::vec3(0.0f, 20.0f, 0.0f));

    Root.UpdateSelfAndChildren();

    Shadow DirLightShadow(2048, 2048);


    // Framebuffer for post-processing
    GLuint FBO, CBO[2], RBO;
    GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    {
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        // Setup CBOs
        glGenTextures(2, CBO);
        for (unsigned int i = 0; i < 2; i++)
        {
            glBindTexture(GL_TEXTURE_2D, CBO[i]);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);

            // Bind CBO to FBO
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, CBO[i], 0);
        }

        glGenRenderbuffers(1, &RBO);
        glBindRenderbuffer(GL_RENDERBUFFER, RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO);
        // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        glDrawBuffers(2, attachments);
        // finally check if framebuffer is complete

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            fprintf(stderr, "ERROR::FRAMEBUFFER::Framebuffer is not complete!\n");
        }
        // Set default frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // ping-pong-framebuffer for blurring
    GLuint pingpongFBO[2], pingpongCBO[2];
    {
        glGenFramebuffers(2, pingpongFBO);
        glGenTextures(2, pingpongCBO);
        for (unsigned int i = 0; i < 2; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
            glBindTexture(GL_TEXTURE_2D, pingpongCBO[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongCBO[i], 0);
            // also check if framebuffers are complete (no need for depth buffer)
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                fprintf(stderr, "ERROR::FRAMEBUFFER::Framebuffer is not complete!\n");
            }
        }
    }

    GLuint UBO;
    {
        glGenBuffers(1, &UBO);

        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, 2 * sizeof(glm::mat4));
    }
    stbi_set_flip_vertically_on_load(true);

    glm::mat4 model(1.0f);
    glm::mat4 view(1.0f);
    glm::mat4 projection(1.0f);

    bool bIsWireMode = false;
    float parallax = 0.1f;
    bool isNormals = false;

    float ZoomOld = Zoom;
    camera.Position.x = -5.0f;
    glm::vec3 camRotOld(0.0f);
    glm::vec3 camPosOld(0.0f);

    float gamma = 2.2f;
    float exposure = 1.0f;
    float brightness = 2.0f;
    float bloomStrength = 0.5f;
    float filterRadius = 0.005f;
    int bloomType = 0;
    int bloomSamples = 5u;

    GLfloat deltaTime = 0.0f;
    GLfloat lastFrame = 0.0f;
    GLfloat currentFrame;

    Texture diffuseBox("res/textures/container.png", true);
    Texture specularBox("res/textures/container_specular.png", false);
    Texture transparentTex("res/textures/blending_transparent_window.png", false);
    Texture screenTex("res/textures/container.png", true);
    Texture particleTex("res/textures/Bolt.png", true);
    Texture planeTex("res/textures/plane.jpg", true);

    ParticleSystem Particles(particleTex, 500, 5.0f);

    skyBoxShader.Use();
    Environment.BindCubeMap(0);
    skyBoxShader.setInt("skybox", 0);

    screenShader.Use();
    screenShader.setInt("screenTexture", 0);
    screenShader.setInt("bloomBlur", 1);

    blurShader.Use();
    blurShader.setInt("image", 0);

    PBRShader.Use();
    Irradiance.BindCubeMap(6);
    PBRShader.setInt("irradianceMap", 6);
    Prefilter.BindCubeMap(7);
    PBRShader.setInt("prefilterMap", 7);
    BRDF.BindTexture(8);
    PBRShader.setInt("brdfLUT", 8);
    DirLightShadow.BindShadowMap(9);
    PBRShader.setInt("shadowMap", 9);

    int winWidth = WINDOW_WIDTH, winHeight = WINDOW_HEIGHT;

    glfwSetScrollCallback(window, MouseCallback);
    glfwSetKeyCallback(window, KeyCallback);

    // Setting shaders uniform block binding
    lightShader.setBlock("Matrixes", 0);
    skyBoxShader.setBlock("Matrixes", 0);
    normalShader.setBlock("Matrixes", 0);
    particleShader.setBlock("Matrixes", 0);
    PBRShader.setBlock("Matrixes", 0);


    Entity* ring = Root.FindByName("CubeRing");
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glfwGetWindowSize(window, &winWidth, &winHeight);
        glViewport(0, 0, winWidth, winHeight);
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Global settings menu window
        {
            ImGui::Begin("Global options");
            ImGui::Checkbox("WireFrame Mode", &bIsWireMode);
            if (bIsWireMode)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            ImGui::Checkbox("Normals", &isNormals);
            ImGui::SliderFloat("Gamma", &gamma, 0.0f, 20.0f);
            ImGui::SliderFloat("Exposure", &exposure, 0.0f, 20.0f);
            ImGui::SliderFloat("BloomStrength", &bloomStrength, 0.0f, 1.0f);
            ImGui::SliderFloat("FilterRadius", &filterRadius, 0.0f, 0.01f, "%.5f");
            ImGui::SliderInt("Bloom Samples", &bloomSamples, 0, 15);
            ImGui::Checkbox("Light Gizmos", &Light::isGizmosOn);

            ImGui::RadioButton("Physical based bloom", &bloomType, 0); ImGui::SameLine();
            ImGui::RadioButton("Gauss blur bloom", &bloomType, 1);


            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // Camera menu window
        {
            ImGui::Begin("Camera settings");

            ImGui::SliderFloat3("Position", &camera.Position[0], -1000.0f, 1000.0f);
            ImGui::SliderFloat("Yaw", &camera.Rotation.x, -89.0f, 89.0f);
            ImGui::SliderFloat("Pitch", &camera.Rotation.y, 0.0f, 360.0f);
            ImGui::SliderFloat("Speed", &camera.Speed, 0.0f, 10.0f);
            ImGui::SliderFloat("Zoom", &Zoom, 1.0f, 45.0f);

            ImGui::End();
        }

        // Scene graph window
        {
            ImGui::Begin("Scene Graph");

            Root.DrawGUITree();

            ImGui::Separator();
            if (Entity::GetSelectedEntity())
            {
                Entity::GetSelectedEntity()->DrawGUIEdit();
            }

            ImGui::End();
        }

        // Rendering
        ImGui::Render();

        glfwMakeContextCurrent(window);

        if (camPosOld != camera.Position || camRotOld != camera.Rotation || ZoomOld != Zoom)
        {
            view = glm::mat4(1.0f);
            view = glm::translate(view, camera.Position);
            camera.UpdateForwardVector();

            view = camera.LookAt(camera.Position, camera.Position + camera.ForwardVector);
            projection = glm::perspective(glm::radians(Zoom), (float)winWidth / (float)winHeight, 0.1f, 100.0f);

            camPosOld = camera.Position;
            camRotOld = camera.Rotation;
            ZoomOld = Zoom;
        }


        Shader::bindUniformData(UBO, 0, sizeof(glm::mat4), glm::value_ptr(view));
        Shader::bindUniformData(UBO, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));

        // DRAW SHADOWS
        DirLightShadow.SetupMap(shadowShader, dirLights[0], *Root.children.front().get());

        glViewport(0, 0, winWidth, winHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.01f, 0.1f, 0.1f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Camera Input
        {
            if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
            {
                camera.GetMouseInput(window); // Rotate
            }
            // Camera Movement
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            {
                camera.GetKeyboardInput(window, Move::FORWARD, deltaTime);
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            {
                camera.GetKeyboardInput(window, Move::BACKWARD, deltaTime);
            }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            {
                camera.GetKeyboardInput(window, Move::LEFT, deltaTime);
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            {
                camera.GetKeyboardInput(window, Move::RIGHT, deltaTime);
            }
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            {
                camera.GetKeyboardInput(window, Move::UP, deltaTime);
            }
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            {
                camera.GetKeyboardInput(window, Move::DOWN, deltaTime);
            }
        }


        //===============================***PBR***===============================
        PBRShader.Use();
        {
            PBRShader.setMat4("lightSpace", DirLightShadow.GetLightSpace());

            spotLights[0].SetIsOn(isSpotlightOn);
            spotLights[0].SetPosition(camera.Position);
            spotLights[0].SetDirection(camera.ForwardVector);

            for (Light* light : lights)
            {
                light->SetupShader(PBRShader);
            }

            PBRShader.setVec3("camPos", camera.Position);
        }
        Particles.Update(computeShader, deltaTime, *Root.FindByName("Generator"));
        particleShader.Use();
        particleShader.setMat4("model", Root.FindByName("Generator")->transform.GetModel());
        Particles.Draw(particleShader);
        Root.DrawSelfAndChildren();


        Root.UpdateSelfAndChildren();

        DirLightShadow.BindShadowMap(8U);

        //===============================GEOMETRY SHADER NORMALS===============================
        if (isNormals)
        {
            Root.DrawSelfAndChildren(normalShader);
        }
        glm::vec3 rot = ring->transform.GetLocalRotation();
        rot.x += 0.5f;
        rot.z += 0.2f;
        ring->transform.SetLocalRotation(rot);
        //===============================SKYBOX===============================
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyBoxShader.Use();
        // skybox cube
        Environment.BindCubeMap(0);
        //Irradiance.BindCubeMap(0);
        //Prefilter.BindCubeMap(0);
        glCullFace(GL_FRONT);
        Cube::GetInstance().Draw(skyBoxShader);
        glCullFace(GL_BACK);
        glDepthFunc(GL_LESS);


        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        bool horizontal = true, first_iteration = true;
        if (bloomType == 0)
        {
            bloomRenderer.RenderBloomTexture(CBO[1], filterRadius);
        }
        else if (bloomType == 1)
        {
            //===============================BLUR===============================
            blurShader.Use();
            for (unsigned int i = 0; i < bloomSamples * 2; i++)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
                blurShader.setInt("horizontal", horizontal);
                glBindTexture(GL_TEXTURE_2D, first_iteration ? CBO[1] : pingpongCBO[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)

                Quad::GetInstance().Draw(blurShader);

                horizontal = !horizontal;
                if (first_iteration)
                {
                    first_iteration = false;
                }
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        screenShader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, CBO[0]);
        glActiveTexture(GL_TEXTURE1);
        if (bIsWireMode)
        {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            if (bloomType == 0)
            {
                glBindTexture(GL_TEXTURE_2D, bloomRenderer.BloomTexture());
            }
            else if (bloomType == 1)
            {
                glBindTexture(GL_TEXTURE_2D, pingpongCBO[!horizontal]);
            }
        }
        screenShader.setFloat("exposure", exposure);
        screenShader.setFloat("gamma", gamma);
        screenShader.setFloat("bloomStrength", bloomStrength);

        Quad::GetInstance().Draw(screenShader);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteBuffers(1, &UBO);

    glDeleteFramebuffers(1, &FBO);
    glDeleteTextures(2, CBO);

    glDeleteFramebuffers(2, pingpongFBO);
    glDeleteTextures(2, pingpongCBO);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}