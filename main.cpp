#include <stdlib.h>
#include <iostream>
// OSX systems need their own headers
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif
// Use of degrees is deprecated. Use radians for GLM functions
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <cmath>
#include "Screenshot.h"
#include "Scene.h"

using namespace glm;

static const int width = 800;
static const int height = 600;
static const char* title = "Scene viewer";
static const vec4 background(0.1f, 0.2f, 0.3f, 1.0f);
static Scene scene;

// we keep the time dependent/independent variables as global
// time dependent variable have a '_t' suffix
static vec3 w_t;           //the angular velocity in world coord is time dependent
static vec4 omega_t;        //the angular velocity in model coord is time dependent
static mat3 M_model;       //moment of inertia in model coord is time independent
static mat3 M_world_t;     //moment of inertia in world coord is time dependent
static vec3 L;             //augular momentum in world coord is time independent
static mat4 R_t;             //rotation matrix
static bool showEllip;
void printHelp(){
    std::cout << R"(
    Available commands:
      press 'H' to print this message again.
      press Esc to quit.
      press 'O' to save a screenshot.
      press the arrow keys to rotate camera.
      press 'A'/'Z' to zoom.
      press 'R' to reset camera.
      press 'L' to turn on/off the lighting.
      press 'T' to toggle on/off the ellipsodes.
)";
}

void initialize(void){
    printHelp();
    glClearColor(background[0], background[1], background[2], background[3]); // background color
    glViewport(0,0,width,height);


    // Initialize scene
    scene.init();
    scene.shader -> enablelighting = true;
    showEllip = scene.showEllip;
    //initialize all the variables from the scene
    w_t = scene.w_t;
    R_t = scene.node["world"]->childtransforms[0];
    M_model = scene.M_model;
    M_world_t = mat3(R_t) * M_model * transpose(mat3(R_t));
    L = M_world_t * w_t;


    // Enable depth test
    glEnable(GL_DEPTH_TEST);
}


void display(void){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    scene.draw();
    
    glutSwapBuffers();
    glFlush();
    
}

void saveScreenShot(const char* filename = "test.png"){
    int currentwidth = glutGet(GLUT_WINDOW_WIDTH);
    int currentheight = glutGet(GLUT_WINDOW_HEIGHT);
    Screenshot imag = Screenshot(currentwidth,currentheight);
    imag.save(filename);
}

void keyboard(unsigned char key, int x, int y){
    switch(key){
        case 27: // Escape to quit
            exit(0);
            break;
        case 'h': // print help
            printHelp();
            break;
        case 'o': // save screenshot
            saveScreenShot();
            break;
        case 'r':
            scene.camera -> aspect_default = float(glutGet(GLUT_WINDOW_WIDTH))/float(glutGet(GLUT_WINDOW_HEIGHT));
            scene.camera -> reset();
            glutPostRedisplay();
            break;
        case 'a':
            scene.camera -> zoom(0.9f);
            glutPostRedisplay();
            break;
        case 'z':
            scene.camera -> zoom(1.1f);
            glutPostRedisplay();
            break;
        case 'l':
            scene.shader -> enablelighting = !(scene.shader -> enablelighting);
            glutPostRedisplay();
            break;
        case 't':
            scene.showEllip = !scene.showEllip;
            showEllip = scene.showEllip;
            if (showEllip == true){
                scene.node["world"] -> childnodes.push_back(scene.node["EnergyEllipsoid"] );
                scene.node["world"] -> childtransforms.push_back(R_t);
                scene.node["world"] -> childnodes.push_back( scene.node["AngularMomentumEllipsoid"] );
                scene.node["world"] -> childtransforms.push_back(R_t);
            }else{
                scene.node["world"] ->childnodes.pop_back();
                scene.node["world"] ->childnodes.pop_back();
                scene.node["world"] ->childtransforms.pop_back();
                scene.node["world"] ->childtransforms.pop_back();
                
            }
            glutPostRedisplay();
            break;
        default:
            glutPostRedisplay();
            break;
    }
}


void specialKey(int key, int x, int y){
    switch (key) {
        case GLUT_KEY_UP: // up
            scene.camera -> rotateUp(-10.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_DOWN: // down
            scene.camera -> rotateUp(10.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_RIGHT: // right
            scene.camera -> rotateRight(-10.0f);
            glutPostRedisplay();
            break;
        case GLUT_KEY_LEFT: // left
            scene.camera -> rotateRight(10.0f);
            glutPostRedisplay();
            break;
    }
}

// Buss's Algorithm
void bussAlgorithm(float dt) {
    
    w_t = inverse(M_world_t) * L;
    vec3 alpha = -1.0f * inverse(M_world_t) * cross(w_t, L);
    vec3 w_t_hat = w_t + (dt / 2.0f) * alpha + float(pow(dt,2))/12.0f * cross(alpha, w_t) ;

    //use glm rotate
    R_t = rotate(dt * length(w_t_hat), normalize(w_t_hat)) * R_t;

    //recompute moment of inertia in world coordinate
    M_world_t = mat3(R_t) * M_model * transpose(mat3(R_t));
 
}


// animation function will be called at every frame
void animation(void) {
    //initial Delta t. Number of milliseconds since first call to glutGet(GLUT_ELAPSED_TIME).
    static int t1 = glutGet(GLUT_ELAPSED_TIME);
    int t2 = glutGet(GLUT_ELAPSED_TIME);

    //make the delta time be in seconds, otherwise each frame would be triggered
    // in milisecond
    float dt = ((float) t2 - t1)/1000.0f;

    //update the t1 for the next iteration
    t1 = t2; 

    //trigger buss algorithm for the current frame iteration to get the current
    //rotation matrix
    bussAlgorithm(dt);
    
    //assign the new rotation transformation to the teapot
    scene.node["world"]->childtransforms[0] = R_t;

    //get the current angular velocity in model coordinate
    omega_t = inverse((R_t))*vec4(w_t,0.0);
    //assign the rotation * translation to the angular velocity point
    scene.node["world"]->childtransforms[1] = (R_t) *mat4(1.0f, 0.0f, 0.0f, 0.0f, 
                                                            0.0f, 1.0f, 0.0f, 0.0f,
                                                            0.0f, 0.0f, 1.0f, 0.0f, 
                                                            omega_t.x, omega_t.y, omega_t.z, 1.0f);
 
    if (showEllip){
        //this two line can be commented to view the rigid object(teapot)
        //assign the new rotation transformation to the two ellipsoids
        scene.node["world"]->childtransforms[2] = R_t; 
        scene.node["world"]->childtransforms[3] = R_t; 
    }
    
   
    
    glutPostRedisplay();

}


int main(int argc, char** argv)
{
    // BEGIN CREATE WINDOW
    glutInit(&argc, argv);
    
#ifdef __APPLE__
    glutInitDisplayMode( GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
#else
    glutInitContextVersion(3,1);
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
#endif
    glutInitWindowSize(width, height);
    glutCreateWindow(title);
#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    GLenum err = glewInit() ;
    if (GLEW_OK != err) {
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
    }
#endif
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    // END CREATE WINDOW
    
    initialize();
    glutDisplayFunc(display);

    // Since we want to have our OpenGL progeam to key computing and
    // update the screen at every frame, we set:
    glutIdleFunc(animation);

    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKey);
    
    glutMainLoop();
	return 0;   /* ANSI C requires main to return int. */
}