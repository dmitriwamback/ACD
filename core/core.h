//
//  core.h
//  VACD
//
//  Created by Dmitri Wamback on 2025-03-13.
//

#ifndef core_h
#define core_h

#include <iostream>
#include <GL/glew.h>
#include <glfw3.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

GLFWwindow* window;

#include <fstream>
#include <sstream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "helper/thread_pool.h"
#include "object/camera.h"
#include "helper/raycast.h"
#include "acd/acd_util.h"
#include "object/shader.h"
#include "object/object.h"

#include "helper/noise.h"
#include "object/terrain.h"

#include "acd/acd.h"
#include "object/model.h"

void initialize() {
    if (!glfwInit()) {
        throw std::runtime_error("Couldn't initialize glfw");
    }
    
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    
    window = glfwCreateWindow(1200, 800, "ACD Algorithm", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    glewExperimental = GL_TRUE;
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    
    Shader shader = Shader::Create("/Users/dmitriwamback/Documents/Projects/GJK/GJK/shader/main");
    RObject* model = Model::Create("test");
    //RObject* terrain = Terrain::Create();
    
    camera.Initialize();
    
    while (!glfwWindowShouldClose(window)) {
        
        glm::vec4 movement = glm::vec4(0.0f);

        movement.z = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ?  0.05f : 0;
        movement.w = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? -0.05f : 0;
        movement.x = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ?  0.05f : 0;
        movement.y = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? -0.05f : 0;
        
        float up = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS ? 0.05f : 0;
        float down = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS ? -0.05f : 0;
        
        camera.Update(movement, up, down);
        std::cout << camera.position.x << " " << camera.position.y << " " << camera.position.z << "\n";
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.25, 0.25, 0.25, 0.0);
        
        shader.Use();
        shader.SetMatrix4("projection", camera.projection);
        shader.SetMatrix4("lookAt", camera.lookAt);
        
        model->Render(shader);
        //terrain->Render(shader);
        
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
}


#endif /* core_h */
