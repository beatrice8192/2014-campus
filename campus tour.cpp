// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GL/glfw.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/texture.hpp>
#include <common/text2D.hpp>

// print out matrix by rows
void printMat(glm::mat4  mat){
    int i,j;
    for (j=0; j<4; j++){
        for (i=0; i<4; i++){
            printf("%f ",mat[i][j]);
        }
        printf("\n");
    }
}

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	if( !glfwOpenWindow( 1024, 768, 0,0,0,0, 32,0, GLFW_WINDOW ) )
	{
		fprintf( stderr, "Failed to open GLFW window.\n" );
		glfwTerminate();
		return -1;
	}

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	glfwSetWindowTitle( "CMPT485 Project Part 5" );

	// Ensure we can capture the escape key being pressed below
	glfwEnable( GLFW_STICKY_KEYS );

	// Dark blue background
	glClearColor(0.87f, 0.93f, 1.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "VP");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

    
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	// Camera matrix
/*	glm::mat4 View       = glm::lookAt(
								glm::vec3(0,1,3), // Camera is at (4,3,-3), in World Space
								glm::vec3(0,0,0), // and looks at the origin
								glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
						   );
*/	// Our ModelViewProjection : multiplication of our 3 matrices
//	glm::mat4 VP        = Projection * View; // Remember, matrix multiplication is the other way around
    
    // Load the texture using any two methods
//	GLuint Texture = loadBMP_custom("silo.bmp");
//	GLuint Texture = loadDDS("uvtemplate.DDS");
    
    // Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

    std::vector<Model> models;
    loadModels("usaskcampus.campus", models);

    int numModels = models.size();
    
    GLuint vaoIDs[numModels];
    GLsizei numVertices[numModels];
    GLsizei numVertexIndices[numModels];
    GLuint textures[numModels];
    std::vector<glm::mat4> ModelMatrix;
    ModelMatrix.resize(numModels);
    
    glGenVertexArrays(numModels, vaoIDs);

    for (int i  = 0; i < numModels; i++) {

        // Read our .obj file
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
        std::vector<glm::ivec3> vertex_indices;
        std::vector<glm::ivec3> uv_indices;
        std::vector<glm::ivec3> normal_indices;

        bool loadSuccess = loadOBJ(models[i].objFilename.c_str(), vertices, uvs, normals);
        if (!loadSuccess) {
            return -1;
        }
        
        // need to keep track of vbo sizes for drawing later
        numVertices[i] = vertices.size();
        numVertexIndices[i] = vertex_indices.size();

        ModelMatrix[i] = glm::mat4(1.0f);
        ModelMatrix[i] = glm::scale(ModelMatrix[i], glm::vec3(models[i].sx, models[i].sy, models[i].sz));
        ModelMatrix[i] = glm::rotate(ModelMatrix[i], models[i].ra,glm::vec3(models[i].rx, models[i].ry, models[i].rz));
        ModelMatrix[i] = glm::translate(ModelMatrix[i], glm::vec3(models[i].tx, models[i].ty, models[i].tz));
        
        // printf("ModelMat%d\n", i);
        // printMat(ModelMatrix[i]);
        
        // Load vertices into a VBO
        GLuint vertexbuffer;
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, numVertices[i] * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
        
        // Load normals into a VBO
        GLuint normalsbuffer;
        glGenBuffers(1, &normalsbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

        // Load texture coords into a VBO
        GLuint uvbuffer;
        glGenBuffers(1, &uvbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec3), &uvs[0], GL_STATIC_DRAW);
        
        // bind VAO in order to save attribute state
        glBindVertexArray(vaoIDs[i]);
        
        // Bind our texture in Texture Unit 0
        
        // Load textures
//        glActiveTexture(i);
//        glBindTexture(GL_TEXTURE_2D, textures[i]);
        GLuint t = loadBMP_custom(models[i].textureFilename.c_str());
        textures[i] = t;

        // Set our "myTextureSampler" sampler to user Texture Unit 0
        glUniform1i(TextureID, i);
        
        // 1st attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
              0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
              3,                  // size
              GL_FLOAT,           // type
              GL_FALSE,           // normalized?
              0,                  // stride
              (void*)0            // array buffer offset
              );
        
        // 2nd attribute buffer : normals
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, normalsbuffer);
        glVertexAttribPointer(
              1,                  // attribute. No particular reason for 1, but must match the layout in the shader.
              3,                  // size
              GL_FLOAT,           // type
              GL_FALSE,           // normalized?
              0,                  // stride
              (void*)0            // array buffer offset
              );
        
        // 3rd attribute buffer : uvs
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
              2,                  // attribute. No particular reason for 2, but must match the layout in the shader.
              2,                  // size
              GL_FLOAT,           // type
              GL_FALSE,           // normalized?
              0,                  // stride
              (void*)0            // array buffer offset
              );



        // release VAO
        glBindVertexArray(0);
    }

    
    // Use our shader
    glUseProgram(programID);

    // Send our transformation to the currently bound shader,
    // in the "VP" uniform

    // Initialize our little text library with the Holstein font
	initText2D( "Holstein.tga" );

	do{

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        char text[256];
        sprintf(text,"YUQING TAN");
        printText2D(text, 500, 50, 30);

        float t=glfwGetTime();
        t=t*10;
        float u=0;
        float posx = 0;
        float posy = 0;
        float posz = 0;
        float lookx = 0;
        float looky = 0;
        float lookz = 0;
        float upx = 0;
        float upy = 1;
        float upz = 0;
        
        //project6 predefined camera path
        if (t<25){
            u=t-0;
            posx = 20+t;//=45
            posy = 5;
            posz = 35-0.05*(t-15)*(t-15);//=30
            lookx = 30;
            looky = 0;
            lookz = 10;
            if (t>24.5&&t<25) printf("24 %f %f %f\n",posx,posy,posz);
            
        }else if (t<45){
            posx = 50-0.05*(t-35)*(t-35);//=45
            posy = 5;
            posz = 30-(t-25)*1;//=10
            lookx = 30-(t-25)*0.5;//=20
            looky = 0;
            lookz = 10-(t-25)*0.3;//=4
            if (t>25&&t<25.5) printf("25 %f %f %f\n",posx,posy,posz);
            if (t>44.5&&t<45) printf("44 %f %f %f\n",posx,posy,posz);
            
        }else if (t<55){
            posx = 45-(t-45)*1.2;//=33
            posy = 5;
            posz = 5+0.05*(t-55)*(t-55);//=5
            lookx = 20-(t-45)*0.5;//=15
            looky = 0;
            lookz = 4-(t-45)*0.3;//=1
            if (t>45&&t<45.5) printf("45 %f %f %f\n",posx,posy,posz);
            if (t>54.5&&t<55) printf("54 %f %f %f\n",posx,posy,posz);
            
        }else if (t<75){
            posx = 33-(t-55)*1.2;//=9
            posy = 5-(t-55)*0.2;//=1
            posz = 5;
            lookx = 15-(t-55)*0.5;//=5
            looky = 0+(t-55)*0.03;//=0.6
            lookz = 1;
            if (t>55&&t<55.5) {
                printf("55 %f %f %f\n",posx,posy,posz);
                printf("   %f %f %f\n",lookx,looky,lookz);
            }
            if (t>74.5&&t<75) {
                printf("74 %f %f %f\n",posx,posy,posz);
                printf("   %f %f %f\n",lookx,looky,lookz);
            }
            
        }else if (t<80){
            posx = 9-(t-75)*1.2;//=3
            posy = 1-(t-75)*0.1;//=0.5
            posz = 5-0.1*(t-75)*(t-75);//=2.5
            lookx = 5-(t-75)*0.8;//=1
            looky = 0.6;
            lookz = 1-(t-75)*0.5;//=-1.5
            if (t>75&&t<75.5) {
                printf("75 %f %f %f\n",posx,posy,posz);
                printf("   %f %f %f\n",lookx,looky,lookz);
            }
            if (t>79.5&&t<80) {
                printf("79 %f %f %f\n",posx,posy,posz);
                printf("   %f %f %f\n",lookx,looky,lookz);
            }
            
        }else if (t<85){
            posx = 0.5+0.1*(t-85)*(t-85);//=0.5
            posy = 0.5;
            posz = 2.5-(t-80);//=-2.5
            lookx = 1;
            looky = 0.6;
            lookz = -1.5-(t-80)*0.5;//=-4
            if (t>80&&t<80.5) {
                printf("80 %f %f %f\n",posx,posy,posz);
                printf("   %f %f %f\n",lookx,looky,lookz);
            }
            if (t>84.5&&t<85) {
                printf("84 %f %f %f\n",posx,posy,posz);
                printf("   %f %f %f\n",lookx,looky,lookz);
            }
            
        }else if (t<100){
            posx = 0.5;
            posy = 0.5;
            posz = -2.5-(t-85);//=-17.5
            lookx = 1+(t-85)*0.2;//=4
            looky = 0.6;
            lookz = -4-(t-85)*2;//=-34
            if (t>85&&t<85.5) {
                printf("85 %f %f %f\n",posx,posy,posz);
                printf("   %f %f %f\n",lookx,looky,lookz);
            }
            if (t>99.5&&t<100) {
                printf("99 %f %f %f\n",posx,posy,posz);
                printf("   %f %f %f\n",lookx,looky,lookz);
            }
            
        }else if (t<120){
            posx = 0.5+0.04*(t-100)*(t-100);//=16.5,dx=0.8
            posy = 0.5+(t-100)*0.05;//=1.5
            posz = -17.5-(t-100);//=-37.5,dz=-1
            lookx = 4+(t-100)*0.25;//=9;
            looky = 0.6+(t-100)*0.045;//=1.6
            lookz = -34-(t-100);//=-54
            if (t>100&&t<100.5) {
                printf("100 %f %f %f\n",posx,posy,posz);
                printf("    %f %f %f\n",lookx,looky,lookz);
            }
            if (t>119.5&&t<120) {
                printf("119 %f %f %f\n",posx,posy,posz);
                printf("    %f %f %f\n",lookx,looky,lookz);
            }
            
        }else if (t<145){
            posx = 16.5+(t-120)*0.8;//=36.5
            posy = 1.5+(t-120)*0.1;//=4
            posz = -50+0.02*(t-145)*(t-145);//=-50
            lookx = 9+(t-120)*0.5;//=21.5;
            looky = 1.5+(t-120)*0.1;//=4
            lookz = -72+0.02*(t-150)*(t-150);//=-72
            if (t>120&&t<120.5) {
                printf("120 %f %f %f\n",posx,posy,posz);
                printf("    %f %f %f\n",lookx,looky,lookz);
            }
            if (t>144.5&&t<145) {
                printf("144 %f %f %f\n",posx,posy,posz);
                printf("    %f %f %f\n",lookx,looky,lookz);
            }
            
        }else if (t<170){
            posx = 36.5+(t-145)*0.8;//=56.5
            posy = 4;
            posz = -50+0.04*(t-145)*(t-145);//=-25,dz=2
            lookx = 22+(t-145)*1.5;//=59
            looky = 4;
            lookz = -72;
            if (t>145&&t<145.5) {
                printf("145 %f %f %f\n",posx,posy,posz);
                printf("    %f %f %f\n",lookx,looky,lookz);
            }
            if (t>169.5&&t<170) {
                printf("169 %f %f %f\n",posx,posy,posz);
                printf("    %f %f %f\n",lookx,looky,lookz);
            }
            
        }else if (t<210){
            posx = 56.5+(t-170)*0.8;//=88.5
            posy = 4;
            posz = 15-0.025*(t-210)*(t-210);//=-37.5
            lookx = 60+(t-170)*0.5;//80
            looky = 4;
            lookz = -72;
            if (t>170&&t<170.5) {
                printf("170 %f %f %f\n",posx,posy,posz);
                printf("    %f %f %f\n",lookx,looky,lookz);
            }
            if (t>209.5&&t<210) {
                printf("209 %f %f %f\n",posx,posy,posz);
                printf("    %f %f %f\n",lookx,looky,lookz);
            }
            
        }else if (t<260){
            posx = 88.5+(t-210)*0.8;//=128.5
            posy = 4;
            posz = 15-0.01*(t-210)*(t-210);//=-10,dz=-1
            lookx = 80-(t-210)*0.4;
            looky = 4;
            lookz = -72+(t-210)*0.4;//-52
            if (t>210&&t<210.5) {
                printf("210 %f %f %f\n",posx,posy,posz);
                printf("    %f %f %f\n",lookx,looky,lookz);
            }
            if (t>259.5&&t<260) {
                printf("259 %f %f %f\n",posx,posy,posz);
                printf("    %f %f %f\n",lookx,looky,lookz);
            }
            
        }else if (t<290){
            posx = 131.2-0.075*(t-266)*(t-266);//=88,dx=-3.6
            posy = 4;
            posz = -10-(t-260);//=-40,dz=-1
            lookx = 60-(t-260)*0.5;//=45
            looky = 4;
            lookz = -52+(t-260)*0.4;//-40
            if (t>260&&t<260.5) {
                printf("260 %f %f %f\n",posx,posy,posz);
                printf("    %f %f %f\n",lookx,looky,lookz);
            }
            if (t>289.5&&t<290) {
                printf("289 %f %f %f\n",posx,posy,posz);
                printf("    %f %f %f\n",lookx,looky,lookz);
            }
            
            }else if (t<330){
                posx = 7+0.04*(t-335)*(t-335);//=8,dx=-0.4
                posy = 4+(t-290)*((t-290)*0.01+0.5);//=24
                posz = -65+0.01*(t-340)*(t-340);//-64,dz=-0.2
                lookx = 45-(t-290)*0.5;//=25
                looky = 4;
                lookz = -40+(t-290)*0.4;//-24
                if (t>290&&t<290.5) {
                    printf("290 %f %f %f\n",posx,posy,posz);
                    printf("    %f %f %f\n",lookx,looky,lookz);
                }
                if (t>329.5&&t<330) {
                    printf("329 %f %f %f\n",posx,posy,posz);
                    printf("    %f %f %f\n",lookx,looky,lookz);
                }
                
            }else if (t<500){
                posx = 8-(t-330)*0.4;
                posy = 40+(t-330)*((t-330)*0.02+0.9);
                posz = -64-(t-330)*0.2;
                lookx = 25;
                looky = 4;
                lookz = -24;
                if (t>330&&t<330.5) {
                    printf("330 %f %f %f\n",posx,posy,posz);
                    printf("    %f %f %f\n",lookx,looky,lookz);
                }
                
            }
        
                glm::mat4 View = glm::lookAt(
                                           glm::vec3( posx, posy, posz ), // Camera is here
                                           glm::vec3( lookx, looky, lookz ), // and looks here
                                           glm::vec3( upx, upy, upz )  // Head is up (set to 0,-1,0 to look upside-down)
                                           );
                
                glm::mat4 VP        = Projection * View; // Remember, matrix
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &VP[0][0]);

                for (int i = 0; i < numModels; i++) {
                    
                    // bind VAO to recall VBOs state
                    glBindVertexArray(vaoIDs[i]);
                    glActiveTexture(i);
                glBindTexture(GL_TEXTURE_2D, textures[i]);
                // Set our "myTextureSampler" sampler to user Texture Unit 0
                glUniform1i(TextureID, 0);
                
                // Set our Model transform matrix
                glm::mat4 M = ModelMatrix[i];
                glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &M[0][0]);
                
                glDrawArrays(GL_TRIANGLES, 0, numVertices[i] );
                
                glBindVertexArray(0);
            }

            // Swap buffers
            glfwSwapBuffers();
            
        } // Check if the ESC key was pressed or the window was closed
        while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
              glfwGetWindowParam( GLFW_OPENED ) );
        
        // Cleanup VBO and shader
        glDeleteProgram(programID);
        for (int i = 0; i < numModels; i++) {
            glDeleteVertexArrays(1, &vaoIDs[i]);
        }
    
        // Delete the text's VBO, the shader and the texture
        cleanupText2D();

        // Close OpenGL window and terminate GLFW
        glfwTerminate();
        
        return 0;
    }
    
