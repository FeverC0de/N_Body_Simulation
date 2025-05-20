#include <iostream>
#include <C:\Users\Nick-Uni\Desktop\Sim Methods\GLFW\glfw3.h>
#include <cmath>
#include <vector>
#include <random>

GLFWwindow* StartGLFW();

float screen_scale = 1;

float screenHeight = 26000.0f * screen_scale;
float screenWidth = 26000.0f * screen_scale;
float G_constant = 1;
float num_bodies = 400;
float PI = 3.141592653589f;
float g_scale = 10000/4;
float bh_radius = 100;
float bh_vel = 50;
bool have_initial = true;
bool tracking = true;
bool add_bh = true;

bool collision = false;

float black_hole_mass = 16000;
float object_mass = 10;

float distribution_radius = 2000;
float softening = 0.1 * distribution_radius * 5 ;




class Object {
    public:

    std::vector<float> position;
    std::vector<float> velocity;
    float radius;
    float mass;
    int res = 100;


    std::vector<float> camera_position = position;

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

        camera_position = position;
    };

    void DrawCircle(){
        glBegin(GL_TRIANGLE_FAN);
        glVertex2d(this->camera_position[0], this->camera_position[1]);
        for(int i = 0; i <= res; ++i){
            // std::cout << position[0] << std::endl;
            float angle = 2.0f * 3.141592653589f  * (static_cast<float>(i)/res);
            float x = camera_position[0] + cos(angle) * radius; 
            float y = camera_position[1] + sin(angle) * radius;
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
    // std::cout << "Colidded!" << std::endl;

    float fDistance = sqrtf((obj1.position[0] - obj2.position[0])*(obj1.position[0] - obj2.position[0]) + (obj1.position[1] - obj2.position[1])*(obj1.position[1] - obj2.position[1]));

    float fOverlap = 0.5f * (fDistance - obj1.radius - obj2.radius);

    //Static Collision

    if(obj1.mass == obj2.mass){
        obj1.position[0] -= fOverlap * (obj1.position[0] - obj2.position[0]) / fDistance;
        obj1.position[1] -= fOverlap * (obj1.position[1] - obj2.position[1]) / fDistance;
        obj2.position[0] += fOverlap * (obj1.position[0] - obj2.position[0]) / fDistance;
        obj2.position[1] += fOverlap * (obj1.position[1] - obj2.position[1]) / fDistance;
    }else if (obj1.mass > obj2.mass)
    {
        obj2.position[0] += fOverlap * (obj1.position[0] - obj2.position[0]) / fDistance;
        obj2.position[1] += fOverlap * (obj1.position[1] - obj2.position[1]) / fDistance;
    }else{
        obj1.position[0] -= fOverlap * (obj1.position[0] - obj2.position[0]) / fDistance;
        obj1.position[1] -= fOverlap * (obj1.position[1] - obj2.position[1]) / fDistance;
    }
    
    // //Dynamic Collision
    // obj1.velocity[0] *= -1;
    // obj2.velocity[0] *= -1;
    // obj1.velocity[1] *= -1;
    // obj2.velocity[1] *= -1;
}

float RandFloat(float max){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribution(1.0f, max);
    return distribution(gen);
}

void populate_universe(std::vector<Object> &bodies, int num_bodies, float height, float width){

    std::random_device seed;
    std::mt19937 gen{seed()};

    // std::uniform_int_distribution<> gen_x {0, width};
    // std::uniform_int_distribution<> gen_y {0, width};
    std::uniform_int_distribution<> gen_xv {10, 20};
    std::uniform_int_distribution<> gen_yv {10, 20};

    int count = 1;
    float mass = object_mass;
    float radius = bh_radius/4;

    float centerX = width/2;
    float centerY = height/2;

    float pos_x = 0;
    float pos_y = 0;

    while(count < num_bodies){

        float distance = RandFloat(distribution_radius * 5); // Random distance within screen bounds
        float angle = RandFloat(distribution_radius * 5) * 2 * PI; // Random angle between 0 and 2π
        
        float pos_x = centerX + cos(angle) * distance ;
        float pos_y = centerY + sin(angle) * distance ;

        if(pos_x < centerX + distribution_radius && pos_x > centerX - distribution_radius){
            if(pos_y < centerY + distribution_radius && pos_y > centerY - distribution_radius){
                pos_x += distribution_radius;
                pos_y += distribution_radius;

        }
    }
        
        // float vel_x = float(gen_yv(gen));
        // float vel_y = float(gen_yv(gen));

        float vel_x = 0;
        float vel_y = 0;

        // std::cout << vel_y << std::endl;
        std::vector<float> pos_vector = {pos_x, pos_y};
        std::vector<float> vel_vector = {vel_x, vel_y};

        Object new_object(pos_vector, vel_vector, radius, mass);        
        bodies.push_back(new_object);
        count += 1;

    }
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
    std::vector<float> position3 = {centerX + 1079 , centerY}; //Mercury

    std::vector<float> velocity1 = {bh_vel, 0};
    std::vector<float> velocity2 = {0, -10};
    
    std::vector<Object> objects = {};

if(add_bh == true){
    objects.push_back(Object(position2, velocity1, bh_radius, black_hole_mass));
    // objects.push_back(Object(position1, velocity2, 30, 1));
}

    populate_universe(objects, num_bodies, screenHeight, screenWidth);

    while(!glfwWindowShouldClose(window)){
        // float screenHeight = 12000.0f;
        // float screenWidth = 16000.0f;

        glClear(GL_COLOR_BUFFER_BIT);
        
    for(auto& object:objects){
            for(auto& object2:objects){

                if(&object2 != &object){

                    float distance_x = object2.position[0] - object.position[0];
                    float distance_y = object2.position[1] - object.position[1];

                    float distance_r = std::sqrt(distance_x * distance_x + distance_y * distance_y);

                    if (distance_r <= 0.0f) {
                        continue;
                    }

                    std::vector<double> direction = {distance_x/ distance_r, distance_y/distance_r};

                    double G_force = ((G_constant * object2.mass * object.mass) / ((distance_r  * distance_r) + (softening * softening))) * g_scale;

                    // double G_force = ((G_constant * object2.mass * object.mass)/(distance_r  * distance_r) + (softening * softening)) * g_scale;


                    object.accelerate((G_force/object.mass) * direction[0], (G_force/object.mass) * direction[1]);

                }
                if(&object != &object2 && detectCollision(object, object2) && collision){
                    handleCollision(object, object2);
                }
            }
            
            object.updatePos();

        

            if(!tracking){
                object.DrawCircle();
            }
            //Top and bottom collision   
            if(!tracking){     
                if(object.position[1] - object.radius < 0){
                    // std::cout << "collidded with surface" << std::endl;
                    object.velocity[1] *= -0.015;
                    object.position[1] = object.radius;
                }
                else if(object.position[1] + object.radius > screenHeight){
                    object.velocity[1] *= -0.015;
                    object.position[1] = screenHeight- object.radius;
                }

                //Side collision
                if(object.position[0] - object.radius < 0){
                    // std::cout << "collidded with surface" << std::endl;
                    object.velocity[0] *= -0.015;
                    object.position[0] = object.radius;
                }
                else if(object.position[0] + object.radius > screenHeight){
                    object.velocity[0] *= -0.015;
                    object.position[0] = screenHeight- object.radius;
                }
            }



}


if(tracking){
    float blackHoleX = objects[0].position[0];
    float blackHoleY = objects[0].position[1];

    for(auto& object : objects) {
        if (&object == &objects[0]) {
            object.camera_position[0] = screenWidth / 2;
            object.camera_position[1] = screenHeight / 2;
        } else {
            object.camera_position[0] = object.position[0] - blackHoleX + (screenWidth / 2);
            object.camera_position[1] = object.position[1] - blackHoleY + (screenHeight / 2);
        }
        
        object.DrawCircle();
    }
}
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
