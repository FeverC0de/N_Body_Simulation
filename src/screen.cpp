#include <iostream>
#include <C:\Users\Nick-Uni\Desktop\Sim Methods\GLFW\glfw3.h>
#include <vector>
#include <random>
#include "object.h"
#include "tree.h"
#include <omp.h>
#include "morton.h"
#include <chrono>
#include "mortonkeys.h"
#include <algorithm> 
// #include <execution>


GLFWwindow* StartGLFW();
float WorldSpaceX = 1000000;
float WorldSpaceY = 1000000;
static bool  showQuadtree = false;

float dt = 0.1;

void DrawWorldBoundary() {
    glColor3f(1.0f, 1.0f, 1.0f);
    
    // Set line thickness
    glLineWidth(100.0f);
    
    glBegin(GL_LINE_LOOP);
        glVertex2f(-WorldSpaceX/2,-WorldSpaceY/2);          // Bottom-left corner
        glVertex2f(WorldSpaceX/2, -WorldSpaceY/2);   // Bottom-right corner
        glVertex2f(WorldSpaceX/2, WorldSpaceY/2); // Top-right corner
        glVertex2f(-WorldSpaceY/2, WorldSpaceY/2);   // Top-left corner
    glEnd();

}

float screenWidth = 5000;
float screenHeight = 5000;
float fOffsetX = 0.0f;
float fOffsetY = 0.0f;

GLFWwindow* StartGLFW(){
    if(!glfwInit()){
        std::cerr << "Failed to initialize glfw, panic!"<<std::endl;
        return nullptr;
    }
    GLFWwindow* window = glfwCreateWindow(2000, 2000, "gravity_sim", NULL, NULL);
    return window;
};

// Pan speed (units per second)
float panSpeed = 200.0f * screenWidth/1000;
// Flags to track which keys are pressed
bool keyW = false, keyS = false, keyA = false, keyD = false, keyU = false;

void UpdateProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(fOffsetX, fOffsetX + screenWidth, fOffsetY, fOffsetY + screenHeight, -1.0, 1.0);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Update key state flags based on press/release events
    if (key == GLFW_KEY_W) {
        keyW = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_S) {
        keyS = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_A) {
        keyA = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_D) {
        keyD = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_V && action == GLFW_PRESS){
        showQuadtree = !showQuadtree;
        std::cout << "Quad Tree Les GOO";
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // Zoom factor (adjust for preferred zoom speed)
    float zoomFactor = 0.1f;
    
    // Get current center of screen in world coordinates
    float centerX = fOffsetX + screenWidth / 2;
    float centerY = fOffsetY + screenHeight / 2;
    
    // Calculate new dimensions based on scroll direction
    if (yoffset > 0) {
        // Zoom in
        screenWidth *= (1.0f - zoomFactor);
        screenHeight *= (1.0f - zoomFactor);
    } else if (yoffset < 0) {
        // Zoom out
        screenWidth *= (1.0f + zoomFactor);
        screenHeight *= (1.0f + zoomFactor);
    }
    
    // Recalculate offset to keep the center point fixed
    fOffsetX = centerX - screenWidth / 2;
    fOffsetY = centerY - screenHeight / 2;
    panSpeed = 200.0f * screenWidth/1000;

    
    // Update the projection with new dimensions
    UpdateProjection();
}
void DrawQuadTree(const QuadTree* tree, int depth = 0) {
    if (!tree) return;
    
    const std::array<float, 2>& min = tree->GetMin();
    const std::array<float, 2>& max = tree->GetMax();
    
    // Set color based on depth (creates a nice hierarchical coloring)
    float r = (depth % 3 == 0) ? 0.8f : 0.2f;
    float g = (depth % 3 == 1) ? 0.8f : 0.2f;
    float b = (depth % 3 == 2) ? 0.8f : 0.2f;
    float alpha = 1.0f - std::min(0.9f, depth * 0.1f); // Fade out deeper levels
    
    glColor4f(r, g, b, alpha);
    
    // Set line width based on depth (thinner lines for deeper levels)
    glLineWidth(std::max(1.0f, 5.0f - depth));
    
    // Draw the quad boundary
    glBegin(GL_LINE_LOOP);
        glVertex2f(min[0], min[1]);
        glVertex2f(max[0], min[1]);
        glVertex2f(max[0], max[1]);
        glVertex2f(min[0], max[1]);
    glEnd();
    
    // Recursively draw child nodes
    for (int i = 0; i < 4; i++) {
        const QuadTree* child = tree->_quadNode[i];
        if (child) {
            DrawQuadTree(child, depth + 1);
        }
    }
}


void updatePanning(float deltaTime) {
    //Set keys to pan across x,y directions
    if (keyW) fOffsetY += panSpeed * deltaTime;
    if (keyS) fOffsetY -= panSpeed * deltaTime;
    if (keyA) fOffsetX -= panSpeed * deltaTime;
    if (keyD) fOffsetX += panSpeed * deltaTime;

    //Update based on key clicked.
    if (keyW || keyS || keyA || keyD) {
        UpdateProjection();
    }
}

int main(){
    // Define dt for the physics simulation
    GLFWwindow* window = StartGLFW();
    glfwMakeContextCurrent(window);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, screenWidth, 0.0, screenHeight, -1.0, 1.0);

    std::array<float, 2> pos = {screenHeight/2, screenWidth/2};

    morton<256, 0>::add_values(mortonkeyX);
	morton<256, 1>::add_values(mortonkeyY);

    Object ball(pos, {300,300}, 1, 1);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(screenWidth/2 -10000.0f, screenHeight/2 + 10000);

    std::vector<Object> bodies;
    bodies.push_back(ball);
    for (int i = 0; i < 50000; ++i) {
        std::array<float,2> pos = {dist(gen), dist(gen)};
        bodies.push_back(Object(pos, {0,0}, 10, 1));
    }


    float bounds = 24000;

    std::array<float, 2> min_bounds = {-bounds + screenWidth/2, -bounds + screenWidth/2};
    std::array<float, 2> max_bounds = {bounds + screenWidth/2, bounds + screenWidth/2};

    // std::cout << "Initial body count: " << bodies.size() << std::endl;
    // Morton morton;

    while(!glfwWindowShouldClose(window)){
    
        QuadTree tree(min_bounds, max_bounds, nullptr); 
        QuadTree* tree_pointer = &tree;
        std::cout << "running"<<std::endl;

        //Morton Sort
        std::vector<std::pair<uint64_t, Object>> mortonParticles;

        // std::cout << "running"<<std::endl;

        auto start_time_encode = std::chrono::high_resolution_clock::now();

        
            for(const auto& body: bodies){
                uint64_t code = encodeFloat(body.position[0], body.position[1], min_bounds[0] ,max_bounds[0]);
                // std::cout << "code is " << code;
                mortonParticles.push_back({code, body});
            }

        auto end_time_encode = std::chrono::high_resolution_clock::now();
        auto duration_encode = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_encode - start_time_encode).count();
        static int encode_frame_count = 0;
        if(encode_frame_count++ % 5 == 0){
            std::cout << "Encoding took " << duration_encode << " ms " << std::endl;
        }
        // std::cout << "running"<<std::endl;


        auto start_time_sort = std::chrono::high_resolution_clock::now();

        std::sort(mortonParticles.begin(), mortonParticles.end(),
                [](const auto& a, const auto& b) {
                    return a.first < b.first;
        });
        auto end_time_sort = std::chrono::high_resolution_clock::now();
        auto duration_sort = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_sort - start_time_sort).count();
        static int sort_frame_count = 0;
        if(sort_frame_count++ % 5 == 0){
            std::cout << "sorting took " << duration_sort << " ms " << std::endl;
        }


        // For 2D Morton keys, use only the first 2 bits to identify quadrants
        const int BLOCK_BIT_MASK = 28; 
        std::vector<size_t> quadrantBoundaries;
        
        quadrantBoundaries.push_back(0); // Start boundary
        uint64_t lastQuadrant = mortonParticles[0].first >> (64 - BLOCK_BIT_MASK);
        
        // Find where quadrants change
        for (size_t i = 1; i < mortonParticles.size(); i++) {
            uint64_t currentQuadrant = mortonParticles[i].first >> (64 - BLOCK_BIT_MASK);
            if (currentQuadrant != lastQuadrant) {
                quadrantBoundaries.push_back(i);
                lastQuadrant = currentQuadrant;
            }
        }
        quadrantBoundaries.push_back(mortonParticles.size()); // End boundary

        std::cout << "Quadrant Boundaries of size: " << quadrantBoundaries.size() << std::endl;
        for(int i  = 0 ; i < quadrantBoundaries.size(); ++i ){
            
            std::cout << (quadrantBoundaries[i]) << std::endl;
        }

        std::vector<std::vector<std::pair<uint64_t, Object>>> quadrantParticles(4);

        for (int i = 0; i < 4; i++) {
            size_t start = quadrantBoundaries[i];
            size_t end = quadrantBoundaries[i+1];
            
            // Reserve space for efficiency
            quadrantParticles[i].reserve(end - start);
            
            // Copy particles for this quadrant
            quadrantParticles[i].insert(
                quadrantParticles[i].end(),
                mortonParticles.begin() + start,
                mortonParticles.begin() + end
            );
        }
// Create dynamic trees that will persist

    auto start_time_insert = std::chrono::high_resolution_clock::now();

        std::vector<QuadTree*> quadrantTrees(4, nullptr);

        #pragma omp parallel for num_threads(4)
        for (int i = 0; i < 4; ++i) {
            // Skip empty quadrants
            if (quadrantParticles[i].empty()) {
                continue;
            }
            std::array<float, 2> quadMin = quadrantParticles[i][0].second.position;
            std::array<float, 2> quadMax = quadrantParticles[i][quadrantParticles.size()-1].second.position;

            quadrantTrees[i] = new QuadTree(quadMin, quadMax, nullptr);
            
            // Insert all particles for this quadrant
            for (const auto& particle : quadrantParticles[i]) {
                quadrantTrees[i]->Insert(particle.second, 0);
            }
            std::cout << "Tree " << i << " size is " << quadrantTrees[i]->GetNum() << std::endl;
            
        }

        // Clean up old quadrants and connect new ones
        for (int i = 0; i < 4; ++i) {
            if (tree._quadNode[i] != nullptr) {
                delete tree._quadNode[i]; // Prevent memory leak
            }
            
            if (quadrantTrees[i] != nullptr) {
                tree._quadNode[i] = quadrantTrees[i];
                tree._quadNode[i]->_parent = &tree; // Set parent relationship
            } else {
                tree._quadNode[i] = nullptr;
            }
        }
        std::cout << "Tree is "  << tree.GetNum() << std::endl;


        auto end_time_insert = std::chrono::high_resolution_clock::now();
        auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_insert - start_time_insert).count();

        static int insert_frame_count = 0;
        if (insert_frame_count++ % 5 == 0){
            std::cout << "Quadtree insertion time: " << duration2 << " ms" << std::endl;
        }











        // for (const auto& body : mortonParticles) {
        //     tree.Insert(body.second, 0); 
        // }



        auto start_time_md = std::chrono::high_resolution_clock::now();

        tree.ComputeMassDistribution();
        auto end_time_md = std::chrono::high_resolution_clock::now();
        auto duration_md = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_md - start_time_md).count();

        static int md_frame_count = 0;
        if (md_frame_count++ % 5 == 0){
            std::cout << "MD calc time: " << duration_md << " ms" << std::endl;
        }
        
        glClear(GL_COLOR_BUFFER_BIT);

        // ball.updatePos(dt);
        // ball.DrawCircle();

        

        auto start_time = std::chrono::high_resolution_clock::now();

        #pragma omp parallel for num_threads(8)
        for (size_t i = 0; i < bodies.size(); ++i) {
            std::array<float, 2> acc = tree.CalcForce(bodies[i]);
            
            bodies[i].accelerate(acc[0], acc[1], dt);
            bodies[i].updatePos(dt);
        }
            
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        
        static int frame_count = 0;
        if (frame_count++ % 5 == 0) {
            std::cout << "Physics calculation time: " << duration << " ms" << std::endl;
            // std::cout << tree.s_renegades.size();
        }

            for (size_t i = 0; i < bodies.size(); ++i) {
                bodies[i].DrawCircle();
            }

        
        if(showQuadtree){
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            DrawQuadTree(tree_pointer);
            glDisable(GL_BLEND);
        }   

        DrawWorldBoundary();
        float ball_x = ball.position[0];
        float ball_y = ball.position[1];

        glfwSetKeyCallback(window, keyCallback);
        glfwSetScrollCallback(window, scrollCallback);
        updatePanning(0.1);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    return 0;
}





