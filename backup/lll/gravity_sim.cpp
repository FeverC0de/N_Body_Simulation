#include <iostream>
#include <C:\Users\Nick-Uni\Desktop\Sim Methods\GLFW\glfw3.h>
#include <cmath>
#include <vector>

GLFWwindow* StartGLFW();

float screenHeight = 6000.0f;
float screenWidth = 8000.0f;
float G_constant = 6.6742 * std::pow(10.0, -11.0);


class Object {
    public:

    std::vector<float> position;
    std::vector<float> velocity;
    float radius;
    float mass;
    int res = 100;

    Object(std::vector<float> position, std::vector<float> velocity, float radius, float mass){
        this->position = position;
        this->velocity = velocity;
        this->radius = radius;
        this->mass = mass;
    }
    void accelerate(float x, float y){
        this->velocity[0] += x;
        this->velocity[1] += y;
    };

    void updatePos(){
        this->position[0] += this->velocity[0];
        // std::cout << "Updated position "<< position[0] << std::endl;
        this->position[1] += this->velocity[1];
    };

    void DrawCircle(){
        glBegin(GL_TRIANGLE_FAN);
        glVertex2d(this->position[0], this->position[1]);
        for(int i = 0; i <= res; ++i){
            // std::cout << position[0] << std::endl;
            float angle = 2.0f * 3.141592653589f  * (static_cast<float>(i)/res);
            float x = position[0] + cos(angle) * radius;
            float y = position[1] + sin(angle) * radius;
            glVertex2d(x, y);
        }
        glEnd();
    }    
 };

bool detectCollision(Object &obj1, Object &obj2){
    float x_distance = obj1.position[0] - obj2.position[0];
    float y_distance = obj1.position[1] - obj2.position[1];
    
    // Calculate squared distance between centers
    float distance_squared = x_distance*x_distance + y_distance*y_distance;
    
    // Compare with squared sum of radii
    float radii_sum = obj1.radius + obj2.radius;
    float radii_sum_squared = radii_sum * radii_sum;
    
    return distance_squared < radii_sum_squared;
}

void handleCollision(Object &obj1, Object &obj2){
    std::cout << "Colidded!" << std::endl;

    // float fDistance = sqrtf((obj1.position[0] - obj2.position[0])*(obj1.position[0] - obj2.position[0]) + (obj1.position[1] - obj2.position[1])*(obj1.position[1] - obj2.position[1]));

    // float fOverlap = 0.5f * (fDistance - obj1.radius - obj2.radius);

    // //Static Collision
    // obj1.position[0] -= fOverlap * (obj1.position[0] - obj2.position[0]) / fDistance;
    // obj1.position[1] -= fOverlap * (obj1.position[1] - obj2.position[1]) / fDistance;
    // obj2.position[0] += fOverlap * (obj1.position[0] - obj2.position[0]) / fDistance;
    // obj2.position[1] += fOverlap * (obj1.position[1] - obj2.position[1]) / fDistance;

    // //Dynamic Collision
    // obj1.velocity[0] *= -1;
    // obj2.velocity[0] *= -1;
    // obj1.velocity[1] *= -1;
    // obj2.velocity[1] *= -1;

}

int main(){

    GLFWwindow* window = StartGLFW();

    glfwMakeContextCurrent(window);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, screenWidth, 0.0, screenHeight, -1.0, 1.0);
    float centerX = screenWidth /2.0f;
    float centerY = screenHeight/2.0f;

    std::vector<float> position1 = {centerX - 1500, centerY}; // Earth
    std::vector<float> position2 = {centerX , centerY}; //Sun
    std::vector<float> position3 = {centerX - 579 , centerY}; //Mercury
    // std::vector<float> position4 = {centerX - 2500 , centerY};


    std::vector<float> velocity1 = {0, -10};
    std::vector<float> velocity2 = {0, 0};


    std::vector<Object> objects = {
        Object(position1, velocity1, 45, 5.972f * std::pow(10, 24)/),
        Object(position2, velocity2, 90, 2.0f * std::pow(10, 30)/),
        Object(position3, velocity1, 10, 3.285f * std::pow(10, 23)/),
        // Object(position4, velocity1, 90, 2.0f * std::pow(10, 21)),


    };

    while(!glfwWindowShouldClose(window)){
        float screenHeight = 600.0f;
        float screenWidth = 800.0f;

        glClear(GL_COLOR_BUFFER_BIT);
        
    for(auto& object:objects){

            for(auto& object2:objects){
                if(&object2 != &object){

                    float distance_x = object2.position[0] - object.position[0];
                    float distance_y = object2.position[1] - object.position[1];
                    
                    std::cout<<"Distance x is: " << distance_x<<std::endl;
                    std::cout<<"Distance y is: " << distance_y<<std::endl;

                    
                    double distance_r = std::sqrt(distance_x * distance_x + distance_y * distance_y);

                    std::cout<<"Distance r is: " << distance_r<<std::endl;

                    if (distance_r == 0.0f) {
                        continue; // Skip this iteration to avoid division by zero
                    }

                    std::vector<double> direction = {distance_x/ distance_r, distance_y/distance_r};

                    distance_r;
                    // std::cout<<"Distance r is: " << distance_r<<std::endl;

                    double G_force = ((G_constant * object2.mass * object.mass)/(distance_r  * distance_r));
                    
                    std::cout << G_force << std::endl;
                    object.accelerate((G_force/object.mass) * direction[0], (G_force/object.mass) * direction[1]);

                }
            }
            object.updatePos();
            object.DrawCircle();

            //Top and bottom collision        
            // if(object.position[1] - object.radius < 0){
            //     std::cout << "collidded with surface" << std::endl;
            //     object.velocity[1] *= -0.95;
            //     object.position[1] = object.radius;
            // }
            // else if(object.position[1] + object.radius > screenHeight){
            //     object.velocity[1] *= -1.01;
            //     object.position[1] = screenHeight- object.radius;
            // }

            // //Side collision
            // if(object.position[0] - object.radius < 0){
            //     // std::cout << "collidded with surface" << std::endl;
            //     object.velocity[0] *= -1.01;
            //     object.position[0] = object.radius;
            // }
            // else if(object.position[0] + object.radius > screenHeight){
            //     object.velocity[0] *= 0.95;
            //     object.position[0] = screenHeight- object.radius;
            // }

            // //Object Collision
            // for(auto& object2:objects){
            //     if(&object != &object2 && detectCollision(object, object2)){
            //         handleCollision(object, object2);
            //     }
            // }
        }

        // DrawCircle(position[0], position[1], radius, res = 100);
        // DrawCircle(position2[0], position2[1], radius, res = 100);

        // position[1] += velocity[1];
        // velocity[1] += -9.81/20.0f;

        // if(position[1] < 0|| position[1] > screenHeight){
        //     velocity[1] *= -1.1;
        // }

        glfwSwapBuffers(window);
        glfwPollEvents();

    }
}

GLFWwindow* StartGLFW(){
    if(!glfwInit()){
        std::cerr << "Failed to initialize glfw, panic!"<<std::endl;
        return nullptr;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "gravity_sim", NULL, NULL);

    return window;
};
