/**************************************************
Scene.inl contains the definition of the scene graph
*****************************************************/
#include "Scene.h"
#include "Cube.h"
#include "Obj.h"

using namespace glm;

void Scene::init(void){
      // initialize the moment of inertia for M_model(time indenpendent)
      // assume M_model has the entries ordered 0<u1<u2<u3
      M_model = mat3(1.0f, 0.0f, 0.0f, 
                      0.0f, 1.5f, 0.0f,
                      0.0f, 0.0f, 2.0f);
    
      //angular velocity (world coordinates) 
      w_t = vec3(1.0f, 4.0f, 1.0f);


      //rotation matrix, initialize as identity matrix
      mat4 R_t = mat4(1.0f);                        

      //angular velocity (model coordinates) vector
      omega_t = inverse(mat3(R_t)) * w_t;
      
      //the angular velocity's matrix
      mat4 omegaMatrix = mat4(1.0f, 0.0f, 0.0f, 0.0f, 
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 
        omega_t.x, omega_t.y, omega_t.z, 1.0f);

      showEllip = true;

      //calculate the kinetic energy : 
      //Ekinetic = traverse(omega_t) * M_model* omega_t
      //since GLM no need to transpose a vector
      float Ekinetic = dot(omega_t, (M_model*omega_t));

      vec3 Ek_semiAxes = vec3(sqrt(Ekinetic/M_model[0][0]),sqrt(Ekinetic/M_model[1][1]),sqrt(Ekinetic/M_model[2][2]));
      
      //calculate the angular momentum's constant
      //constant = traverse(omega_t)* transpose(M_model) * M_model * omega_t
      float AngConstant = dot(omega_t, transpose(M_model) * M_model * omega_t);

      vec3 Ang_semiAxes = vec3(sqrt(AngConstant)/M_model[0][0], sqrt(AngConstant)/M_model[1][1], sqrt(AngConstant)/M_model[2][2]);

      // Create a geometry palette
      geometry["cube"] = new Cube;
      geometry["teapot"] = new Obj;
      geometry["sphere"] = new Obj;
      geometry["cube"] -> init();
      geometry["teapot"] -> init("models/teapot.obj");
      geometry["sphere"] -> init("models/sphere.obj");
      
      // Create a material palette
      material["dark"] = new Material;
      material["dark"] -> ambient = vec4(0.0f,0.0f,0.0f,0.0f);
      material["dark"] -> diffuse = vec4(0.0f,0.0f,0.0f,0.0f);
      material["dark"] -> specular = vec4(0.0f,0.0f,0.0f,0.0f);
      material["dark"] -> shininess = 5.0f;
      
      material["ceramic"] = new Material;
      material["ceramic"] -> ambient = vec4(0.02f, 0.07f, 0.2f, 1.0f);
      material["ceramic"] -> diffuse = vec4(0.1f, 0.25f, 0.7f, 1.0f);
      material["ceramic"] -> specular = vec4(0.9f, 0.9f, 0.9f, 1.0f);
      material["ceramic"] -> shininess = 150.0f;
  
      material["silver"] = new Material;
      material["silver"] -> ambient = vec4(0.1f, 0.1f, 0.1f, 1.0f);
      material["silver"] -> diffuse = vec4(0.2f, 0.2f, 0.2f, 1.0f);
      material["silver"] -> specular = vec4(0.9f, 0.9f, 0.9f, 1.0f);
      material["silver"] -> shininess = 50.0f;
      
      material["turquoise"] = new Material;
      material["turquoise"] -> ambient = vec4(0.1f, 0.2f, 0.17f, 1.0f);
      material["turquoise"] -> diffuse = vec4(0.2f, 0.375f, 0.35f, 1.0f);
      material["turquoise"] -> specular = vec4(0.3f, 0.3f, 0.3f, 1.0f);
      material["turquoise"] -> shininess = 100.0f;
      
      material["bulb"] = new Material;
      material["bulb"] -> ambient = vec4(0.0f, 0.0f, 0.0f, 1.0f);
      material["bulb"] -> diffuse = vec4(0.0f, 0.0f, 0.0f, 1.0f);
      material["bulb"] -> specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
      material["bulb"] -> emision = vec4(1.0f,0.2f,0.1f,1.0f);
      material["bulb"] -> shininess = 200.0f;

      
      //create the ellipsoids using the circle geometry
      //since we can strech the x,y,z
      model["EnerEllip"] = new Model;
      model["EnerEllip"] -> geometry = geometry["sphere"];
      model["EnerEllip"] -> material = material["silver"];
    
      model["AngEllip"] = new Model;
      model["AngEllip"] -> geometry = geometry["sphere"];
      model["AngEllip"] -> material = material["turquoise"];
    
      model["polhode"] = new Model;
      model["polhode"] -> geometry = geometry["sphere"];
      model["polhode"] -> material = material["dark"];
    
      model["teapot"] = new Model;
      model["teapot"] -> geometry = geometry["teapot"];
      model["teapot"] -> material = material["ceramic"];

      model["bulb"] = new Model;
      model["bulb"] -> geometry = geometry["cube"];
      model["bulb"] -> material = material["bulb"];
      
      // Create a light palette
      light["sun"] = new Light;
      light["sun"] -> position = vec4(3.0f,2.0f,1.0f,0.0f);
      light["sun"] -> color = 1.0f*vec4(1.0f,1.0f,1.0f,1.0f);
      
      light["bulb"] = new Light;
      light["bulb"] -> position = vec4(0.0f,2.0f,0.0f,0.0f);
      light["bulb"] -> color = 1.5f * vec4(1.0f,0.8f,0.5f,1.0f);
      
      // Build the scene graph

      //create the rigid body node
      node["rigidbody"] = new Node;
      node["rigidbody"] -> models.push_back( model["teapot"] );
      node["rigidbody"] -> modeltransforms.push_back(scale(vec3(2.0f)));

      //represent the epplipsoid of constant energy
      node["EnergyEllipsoid"] = new Node;
      node["EnergyEllipsoid"] -> models.push_back( model["EnerEllip"] );
      node["EnergyEllipsoid"] -> modeltransforms.push_back(scale(Ek_semiAxes));

      //represent the epllipsoid of constant length of angular momentum
      node["AngularMomentumEllipsoid"] = new Node;
      node["AngularMomentumEllipsoid"] -> models.push_back( model["AngEllip"] );
      node["AngularMomentumEllipsoid"] -> modeltransforms.push_back(scale(Ang_semiAxes));

      node["polhode"] = new Node;
      node["polhode"] -> models.push_back( model["polhode"] );
      node["polhode"] -> modeltransforms.push_back( scale(vec3(0.125f)) );

      
      
      node["world"] -> childnodes.push_back( node["rigidbody"] );
      node["world"] -> childtransforms.push_back(R_t);

      node["world"] -> childnodes.push_back( node["polhode"] );
      node["world"] -> childtransforms.push_back(R_t*omegaMatrix);


      //the four lines can be commented out to view the teapot
      if (showEllip){
        node["world"] -> childnodes.push_back( node["EnergyEllipsoid"] );
        node["world"] -> childtransforms.push_back(R_t);
        node["world"] -> childnodes.push_back( node["AngularMomentumEllipsoid"] );
        node["world"] -> childtransforms.push_back(R_t);
      }
      
      

      // Put a camera
      camera = new Camera;
      camera -> target_default = vec3( 0.0f, 0.0f, 0.0f );
      camera -> eye_default = vec3( 0.0f, 0.0f, 25.0f );
      camera -> up_default = vec3( 0.0f, 1.0f, 0.0f );
      camera -> reset();
      
      // Initialize shader
      shader = new SurfaceShader;
      shader -> read_source( "shaders/projective.vert", "shaders/lighting.frag" );
      shader -> compile();
      glUseProgram(shader -> program);
      shader -> initUniforms();
}

