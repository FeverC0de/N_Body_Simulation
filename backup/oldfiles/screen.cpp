#include <iostream>
#include <C:\Users\Nick-Uni\Desktop\Sim Methods\GLFW\glfw3.h>
#include <vector>
#include <random>
#include <algorithm>
#include "object.h"
#include "tree.h"
#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <bitset>
#include "threadpool.h"
#include <cstddef>  // For size_t

#ifdef _WIN32
#include <Windows.h>
#endif


GLFWwindow* StartGLFW();
float WorldSpaceX = 1000000;
float WorldSpaceY = 1000000;
static bool showQuadtree = false;

// Make dt a constant since it doesn't change during execution
const float dt = 0.5;

void DrawWorldBoundary() {
    glColor3f(1.0f, 1.0f, 1.0f); // White color for boundary
    
    // Set line thickness
    glLineWidth(100.0f); // Adjust this value to change thickness
    
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

GLFWwindow* StartGLFW() {
    if(!glfwInit()){
        std::cerr << "Failed to initialize glfw, panic!"<<std::endl;
        return nullptr;
    }
    GLFWwindow* window = glfwCreateWindow(800, 600, "gravity_sim", NULL, NULL);
    return window;
}

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
        std::cout << "Toggle quadtree visibility\n";
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
    
    // Get the boundaries of this quad
    const std::vector<float>& min = tree->GetMin();
    const std::vector<float>& max = tree->GetMax();
    
    // Set color based on depth (creates a nice hierarchical coloring)
    float r = (depth % 3 == 0) ? 0.8f : 0.2f;
    float g = (depth % 3 == 1) ? 0.8f : 0.2f;
    float b = (depth % 3 == 2) ? 0.8f : 0.2f;
    float alpha = 1.0f - std::min<float>(0.9f, depth * 0.1f); // Fade out deeper levels
    
    glColor4f(r, g, b, alpha);
    
    // Set line width based on depth (thinner lines for deeper levels)
    glLineWidth(std::max<float>(1.0f, 5.0f - depth));
    
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
    // Apply smooth panning based on key states and time
    if (keyW) fOffsetY += panSpeed * deltaTime;
    if (keyS) fOffsetY -= panSpeed * deltaTime;
    if (keyA) fOffsetX -= panSpeed * deltaTime;
    if (keyD) fOffsetX += panSpeed * deltaTime;

    if (keyU) screenHeight += panSpeed * deltaTime;

    // Update projection if any movement occurred
    if (keyW || keyS || keyA || keyD || keyU) {
        UpdateProjection();
    }
}

void processBody(Object& obj, const QuadTree& tree, float dt_value) {
    std::vector<float> acc = tree.CalcForce(obj);
    obj.accelerate(acc[0], acc[1], dt_value);
    obj.updatePos(dt_value);
}

int main(){
    // Define dt for the physics simulation
    GLFWwindow* window = StartGLFW();
    glfwMakeContextCurrent(window);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, screenWidth, 0.0, screenHeight, -1.0, 1.0);

    std::vector<float> pos = {screenHeight/2, screenWidth/2};

    Object ball(pos, {0,0}, 1, 1);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(screenWidth/2 - 4000.0f, screenHeight/2 + 4000);

    std::vector<Object> bodies;
    bodies.push_back(ball);
    for (int i = 0; i < 100; ++i) {
        std::vector<float> pos = {dist(gen), dist(gen)};
        bodies.push_back(Object(pos, {0,0}, 10, 1));
    }

    std::vector<float> min_bounds = {-WorldSpaceX/70 + screenWidth/2, -WorldSpaceY/70 + screenWidth/2};
    std::vector<float> max_bounds = {WorldSpaceX/70 + screenWidth/2, WorldSpaceY/70 + screenWidth/2};
    std::cout << "Initial body count: " << bodies.size() << std::endl;

    // Create high-performance thread pool that automatically uses max threads
    HighPerformanceThreadPool pool;

    // Performance monitoring
    double lastTime = glfwGetTime();
    int frameCount = 0;
    
    // For Z-order sorting
    std::vector<ZOrderedBody> z_bodies;
    z_bodies.resize(bodies.size());
    
    while(!glfwWindowShouldClose(window)){
        // Measure frame time for performance monitoring
        double currentTime = glfwGetTime();
        frameCount++;
        if (currentTime - lastTime >= 1.0) { // Print every second
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            lastTime = currentTime;
        }
        
        // STEP 1: Sort bodies using Z-order curve for improved cache locality
        // This step parallelizes Z-order computation
        for (size_t i = 0; i < bodies.size(); ++i) {
            pool.enqueue([&bodies, &z_bodies, i, &min_bounds, &max_bounds, &pool] {
                pool.increment_active();
                z_bodies[i].body = &bodies[i];
                z_bodies[i].zcode = compute_zorder(
                    bodies[i].position[0], 
                    bodies[i].position[1], 
                    min_bounds, 
                    max_bounds
                );
                pool.decrement_active();
            });
        }
        
        // Wait for all Z-order computations to complete
        pool.wait_idle();
        
        // Sort bodies by their Z-order values (sequential - sorting is hard to parallelize efficiently)
        std::sort(z_bodies.begin(), z_bodies.end());

        // STEP 2: Create quadtree with batch insertions for better parallelism
        QuadTree tree(min_bounds, max_bounds, nullptr); 
        QuadTree* tree_pointer = &tree;
        
        // Divide bodies into chunks for parallel insertion
        const size_t chunk_size = 200; // Adjust based on testing
        const size_t num_chunks = (bodies.size() + chunk_size - 1) / chunk_size;
        
        // Process each chunk in parallel
        for (size_t chunk = 0; chunk < num_chunks; ++chunk) {
            pool.enqueue([&tree, &z_bodies, chunk, chunk_size, &pool] {
                pool.increment_active();
                size_t start = chunk * chunk_size;
                size_t end = std::min<size_t>(start + chunk_size, z_bodies.size());
                
                // Create a local batch of bodies to insert
                std::vector<Object*> batch;
                batch.reserve(end - start);
                
                // Collect bodies in this chunk
                for (size_t i = start; i < end; ++i) {
                    batch.push_back(z_bodies[i].body);
                }
                
                // Insert the batch into the tree
                tree.BatchInsert(batch);
                pool.decrement_active();
            });
        }
        
        // Wait for all tree insertions to complete
        pool.wait_idle();
        
        // STEP 3: Compute mass distribution in parallel
        // This requires an updated QuadTree class with parallel mass distribution computation
        tree.ComputeMassDistributionParallel(pool);
        
        // STEP 4: Process physics for all bodies using thread pool
        for (size_t i = 0; i < bodies.size(); ++i) {
            pool.enqueue([&bodies, &tree, i, &pool] {
                pool.increment_active();
                processBody(bodies[i], tree, dt);
                pool.decrement_active();
            });
        }
        
        // Wait for all physics calculations to complete
        pool.wait_idle();
        
        // All rendering must be done on the main thread since OpenGL is not thread-safe
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Update and draw the main ball
        ball.updatePos(dt);
        ball.DrawCircle();
        
        // Draw all bodies
        for (size_t i = 0; i < bodies.size(); ++i) {
            bodies[i].DrawCircle();
        }
        
        // Draw quadtree if enabled
        if(showQuadtree){
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            DrawQuadTree(tree_pointer);
            glDisable(GL_BLEND);
        }   

        DrawWorldBoundary();
        
        // Handle input and update camera
        glfwSetKeyCallback(window, keyCallback);
        glfwSetScrollCallback(window, scrollCallback);
        updatePanning(0.1);
        
        // Swap buffers and poll for events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    return 0;
}