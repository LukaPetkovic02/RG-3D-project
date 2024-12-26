// Autor: Luka Petkovic SV 16/2021
// Opis: Protiv-vazdušna odbrana Novog Sada 

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#define CRES 30
#define ROCKETS_LEFT 10
#define HELICOPTER_NUM 5
#define CAMERA_X_LOC 0.0f   //0.0f
#define CAMERA_Y_LOC 0.6f   //0.4f
#define CAMERA_Z_LOC -0.95f  //-0.65

#include "stb_image.h"
#include "TextRendering.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Za koptere
#include <ctime>
#include <random>
#include <chrono>
#include <thread>
#include <algorithm>

using namespace glm;
using namespace std;

struct ModelData {
    vector<vec3> vertices;
    vector<vec2> textureCoords;
    vector<vec3> normals;
};

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
float Xto2D(float a);
float Yto2D(float a);
void normalizeVector(float& x, float& y, float& z);
void selectHelicopter(int helIndex);
bool checkCollision(float x1, float y1, float z1, float x2, float y2, float z2);
void mouseCallback(GLFWwindow* window, int button, int action, int mods);
void setCircle(float  circle[64], float r, float xPomeraj, float yPomeraj);
static unsigned loadImageToTexture(const char* filePath);
void moveRocket(GLFWwindow* window, float& rocketX, float& rocketY, float rocketSpeed, unsigned int wWidth, unsigned int wHeight);
void generateHelicopterPositions(int number);
void moveHelicoptersTowardsCityCenter(float cityCenterX, float cityCenterY, float cityCenterZ, float speed);
bool isRocketOutsideScreen(float rocketX, float rocketY);

void renderClouds(unsigned int baseShader, unsigned int cloud1VAO, bool& hasTexture, int& colorLoc, unsigned int modelLocBase, ModelData& cloud1);
void renderMountain(unsigned int baseShader, unsigned int mountainVAO, unsigned int mapTexture, glm::mat4& model, unsigned int modelLocBase, ModelData& mountain);
void renderBase(unsigned int baseShader, unsigned int baseVAO, int& colorLoc, unsigned int modelLocBase, ModelData& base, vec3 translateV, float scaleV);

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
ModelData loadModel(const char* filePath);
void processMesh(aiMesh* mesh, const aiScene* scene, ModelData& modelData);
void processNode(aiNode* node, const aiScene* scene, ModelData& modelData);
void setupModelVAO(unsigned int& VAO, unsigned int& VBO, const ModelData& modelData);

float cameraAngle = 0.0f; // Ugao rotacije kamere oko Y ose (u radijanima)
const float rotationSpeed = 0.02f; // Brzina rotacije kamere
float zoomSpeed = 0.02f;

vec3 cameraPos(CAMERA_X_LOC, CAMERA_Y_LOC, CAMERA_Z_LOC); // Pozicija kamere
vec3 cameraTarget(-0.42f, 0.0f, 0.08f);                  // Tacka prema kojoj kamera gleda
vec3 cameraUp(0.0f, 1.0f, 0.0f);

vec3 rocketCameraPos(0.3f, 0.7f, -0.9f);
vec3 rocketCameraTarget(-0.3f, 0.0f, 0.08f);
vec3 rocketCameraUp(0.0f, 1.0f, 0.0f);


bool rotateLeft = false; // Da li kamera rotira ulevo (J)
bool rotateRight = false; // Da li kamera rotira udesno (L)
bool zoomIn = false, zoomOut = false;

bool initWait = false, gameOver = false;

struct Location {
    float x;
    float y;
    float z;
};
struct Rocket {
    float x, y, z;
    float dirX, dirY, dirZ;
    bool isFlying;
    int targetHelicopter;
    float distance;
};
Rocket rockets[10];

float rocketRadius = 0.0075, helicopterRadius = 0.0075;

int selectedHel = -1;
int helicoptersLeft = 5;

float helicopterSpeed = 0.001f;
float rocketSpeed = 0.004f;
bool isSpacePressed = false;
bool wasSpacePressed = false;
bool coptersOnScreen = true;
int numberOfCollied = 0;
bool isMapHidden = false;
Location helicopterPositions[HELICOPTER_NUM];
string helicopterStrings[5] = { "A","S","D","F","G" };
int helTracked = -1;
auto startTime = chrono::high_resolution_clock::now();

float baseCenterX = 0.0f, baseCenterZ = 0.0f, baseCenterY = 0.0f;
bool baseCenterSet = false;
float baseCenterRadius = 0.0175;
bool changedCameraPos = false;

bool cityCenterSet = false;
float cityCenterX = 0.0f, cityCenterZ = 0.0f, cityCenterY = 0.0f;
float cityCenterRadius = 0.008;
bool alreadyPlacedCity = false;
int cityHits = 0;
int countTo3 = 0;

bool activeRocketCam = false;
int main(void)
{
    float reflectorRadius = 3.0f;
    float reflectorSpeed = 0.0001f;
    float reflectorAngle = 0.5f;

    if (!glfwInit())
    {
        cout << "Greska pri ucitavanju GLFW biblioteke!\n";
        return 1;
    }

    for (int i = 0; i < 10; ++i) {
        rockets[i] = { baseCenterX, 0.0, baseCenterZ, 0.0f,0.0f,0.0f, false, -1, 0.0 };
    }

    generateHelicopterPositions(HELICOPTER_NUM);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    unsigned int wWidth = 1000;
    unsigned int wHeight = 1000;
    const char wTitle[] = "Protiv-vazdusna odbrana Novog Sada";
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    int xPos = (mode->width - wWidth) / 2;
    int yPos = (mode->height - wHeight) / 2;
    window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL);
    glfwSetWindowPos(window, xPos, yPos);

    if (window == NULL)
    {
        cout << "Greska pri formiranju prozora!\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window);


    if (glewInit() != GLEW_OK)
    {
        cout << "Greska pri ucitavanju GLEW biblioteke!\n";
        return 3;
    }

    glfwSetMouseButtonCallback(window, mouseCallback);

    unsigned int rocket2DShader = createShader("rocket2d.vert", "rocket2d.frag");
    unsigned int base2DShader = createShader("base2d.vert", "base2d.frag");
    unsigned int textureShader = createShader("texture.vert", "texture.frag");
    unsigned int baseShader = createShader("base.vert", "base.frag");
    unsigned int rocketShader = createShader("rocket.vert", "rocket.frag");
    unsigned int map2DShader = createShader("map_2d.vert", "map_2d.frag");
    unsigned int textShader = createShader("text.vert", "text.frag");
    loadFont("fonts/ariali.ttf");
    initTextRendering();

    int colorLoc = glGetUniformLocation(textureShader, "color");

    float vertices[] = {
   // X     Y      Z       S    T  
    -1.0, -0.01, -1.0,    0.0, 0.0,   0.0, 1.0, 0.0,
     1.0, -0.01, -1.0,    1.0, 0.0,   0.0, 1.0, 0.0,
    -1.0, -0.01,  1.0,    0.0, 1.0,   0.0, 1.0, 0.0,

     1.0, -0.01, -1.0,    1.0, 0.0,   0.0, 1.0, 0.0,
     1.0, -0.01,  1.0,    1.0, 1.0,   0.0, 1.0, 0.0
    };

    float map2DVertices[] = {
        // X     Y         S    T  
         -1.0,  0.5,     0.0, 1.0,     
         -0.5,  0.5,     1.0, 1.0,
         -0.5,   1.0,     1.0, 0.0, 

         -1.0,  0.5,     0.0, 1.0, 
         -0.5,   1.0,     1.0, 0.0,
         -1.0,   1.0,     0.0, 0.0
    };

    float noiseVertices[] = {
        // X     Y         S    T  
         0.5,  0.5,     0.0, 1.0,
         1.0,  0.5,     1.0, 1.0,
         1.0,   1.0,     1.0, 0.0,

         0.5,  0.5,     0.0, 1.0,
         1.0,   1.0,     1.0, 0.0,
         0.5,   1.0,     0.0, 0.0
    };
    // ********************************************** 2D OPISI ********************************************

    
    // Opis baze -----------------------------------------------------------------------
    float baseCircle[CRES * 2 + 4];
    setCircle(baseCircle, baseCenterRadius, 0.0, 0.0);
    unsigned int VAOBase2D, VBOBase2D;
    // VAO i VBO baze
    glGenVertexArrays(1, &VAOBase2D);
    glGenBuffers(1, &VBOBase2D);
    glBindVertexArray(VAOBase2D);
    glBindBuffer(GL_ARRAY_BUFFER, VBOBase2D);
    glBufferData(GL_ARRAY_BUFFER, sizeof(baseCircle), baseCircle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    int translationLocB = glGetUniformLocation(base2DShader, "translation");
    int applyTranslationLoc = glGetUniformLocation(base2DShader, "applyTranslation");

    // Opis centra Novog Sada ----------------------------------------------------------
    float cityCenterCircle2D[CRES * 2 + 4];
    setCircle(cityCenterCircle2D, cityCenterRadius, 0.0, 0.0);
    unsigned int VAOCity2D, VBOCity2D;
    // VAO i VBO baze
    glGenVertexArrays(1, &VAOCity2D);
    glGenBuffers(1, &VBOCity2D);
    glBindVertexArray(VAOCity2D);
    glBindBuffer(GL_ARRAY_BUFFER, VBOCity2D);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cityCenterCircle2D), cityCenterCircle2D, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);


    // Opis rakete ----------------------------------------------------------------------
    float blueCircle2D[CRES * 2 + 4];
    setCircle(blueCircle2D, rocketRadius, 0.0, 0.0);

    // VAO i VBO rakete
    unsigned int VAOBlue2D, VBOBlue2D;
    glGenVertexArrays(1, &VAOBlue2D);
    glGenBuffers(1, &VBOBlue2D);
    glBindVertexArray(VAOBlue2D);
    glBindBuffer(GL_ARRAY_BUFFER, VBOBlue2D);
    glBufferData(GL_ARRAY_BUFFER, sizeof(blueCircle2D), blueCircle2D, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ****************************************************************************************************

    // ********************************************** MODELI **********************************************
    // 
    // Planina --------------------------------------------------------------
    ModelData mountain = loadModel("res/mountain/Mountain.obj");
    unsigned int mountainVAO, mountainVBO;
    setupModelVAO(mountainVAO, mountainVBO, mountain);
    //cout << "PLANINA:" << mountain.vertices.size() << " " << mountain.normals.size() << " " << mountain.textureCoords.size() << endl;

    // Raketa -----------------------------------------------------------------
    ModelData rocket = loadModel("res/rocket/rocket.obj");
    unsigned int rocketVAO, rocketVBO;
    setupModelVAO(rocketVAO, rocketVBO, rocket);
    //cout << "RAKETA:" << rocket.vertices.size() << " " << rocket.normals.size() << " " << rocket.textureCoords.size() << endl;

    // Oblak ----------------------------------------------------------------
    ModelData cloud = loadModel("res/clouds/Cloud1.obj");//clouds/Cloud.obj
    unsigned int cloudVAO, cloudVBO;
    setupModelVAO(cloudVAO, cloudVBO, cloud);
    //cout << "OBLAK:" << cloud.vertices.size() << " " << cloud.normals.size() << " " << cloud.textureCoords.size() << endl;

    // Baza -----------------------------------------------------------------
    ModelData base = loadModel("res/base/Base.obj");
    unsigned int baseVAO, baseVBO;
    setupModelVAO(baseVAO, baseVBO, base);
    //cout << "BAZA:" << base.vertices.size() << " " << base.normals.size() << " " << base.textureCoords.size() << endl;


    // Helikopter -----------------------------------------------------------
    ModelData helicopter = loadModel("res/helicopter/Helicopter.obj");
    unsigned int helicopterVAO, helicopterVBO;
    setupModelVAO(helicopterVAO, helicopterVBO, helicopter);
    //cout << "Helikopter:" << helicopter.vertices.size() << " " << helicopter.normals.size() << " " << helicopter.textureCoords.size() << endl;

    // *****************************************************************************************************

    glUseProgram(0);


    unsigned int stride = (3 + 2 + 3) * sizeof(float);
    unsigned int map2DStride = (2 + 2) * sizeof(float);

    // Tekstura mape ------------------------------------------------------------
    unsigned map2DTexture = loadImageToTexture("res/novi_sad_flipped.png");

    unsigned int map2DVAO;
    unsigned int map2DVBO;

    glGenVertexArrays(1, &map2DVAO);
    glGenBuffers(1, &map2DVBO);

    glBindVertexArray(map2DVAO);
    glBindBuffer(GL_ARRAY_BUFFER, map2DVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(map2DVertices), map2DVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, map2DStride, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, map2DStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, map2DTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Tekstura suma ------------------------------------------------------------

    unsigned noise2DTexture = loadImageToTexture("res/noise.png");

    unsigned int noise2DVAO;
    unsigned int noise2DVBO;

    glGenVertexArrays(1, &noise2DVAO);
    glGenBuffers(1, &noise2DVBO);

    glBindVertexArray(noise2DVAO);
    glBindBuffer(GL_ARRAY_BUFFER, noise2DVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(noiseVertices), noiseVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, map2DStride, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, map2DStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, noise2DTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);





    // VAO i VBO teksture -------------------------------------------------------------   
    unsigned int VAO[2];
    glGenVertexArrays(2, VAO);
    unsigned int VBO[2];
    glGenBuffers(2, VBO);

    glGenVertexArrays(1, &VAO[0]);
    glBindVertexArray(VAO[0]);
    glGenBuffers(1, &VBO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Renderovanje teksture -----------------------------------------------------------
    unsigned mapTexture = loadImageToTexture("res/novi_sad.png");

    glBindTexture(GL_TEXTURE_2D, mapTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    unsigned uTexLoc = glGetUniformLocation(textureShader, "uTex");
    glUniform1i(uTexLoc, 0);


    int rocketsLeft = ROCKETS_LEFT;


    // VAO i VBO svetla ------------------------------------------------------------------
    unsigned int lightVAO, lightVBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);


    mat4 model = mat4(1.0f); //Matrica transformacija - mat4(1.0f) generise jedinicnu matricu
    unsigned int modelLocTex = glGetUniformLocation(textureShader, "uM");
    unsigned int modelLocRocket = glGetUniformLocation(rocketShader, "uM");
    unsigned int modelLocBase = glGetUniformLocation(baseShader, "uM");


    mat4 view; //Matrica pogleda (kamere)
    view = lookAt(cameraPos, cameraTarget, cameraUp); //2. vektor 0.0,0.0,0.0

    mat4 rocketView;
    rocketView = lookAt(rocketCameraPos, rocketCameraTarget, rocketCameraUp);
    unsigned int viewLocTex = glGetUniformLocation(textureShader, "uV");
    unsigned int viewLocRocket = glGetUniformLocation(rocketShader, "uV");
    unsigned int viewLocBase = glGetUniformLocation(baseShader, "uV");

    //promenio sa 90 na 45 stepeni
    mat4 projection = perspective(radians(45.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f); //Matrica perspektivne projekcije (FOV, Aspect Ratio, prednja ravan, zadnja ravan)
    unsigned int projectionLocTex = glGetUniformLocation(textureShader, "uP");
    unsigned int projectionLocRocket = glGetUniformLocation(rocketShader, "uP");
    unsigned int projectionLocBase = glGetUniformLocation(baseShader, "uP");

    unsigned int viewPosLoc = glGetUniformLocation(baseShader, "uViewPos");

    unsigned int lightPosLoc = glGetUniformLocation(baseShader, "uReflector.pos");
    unsigned int lightALoc = glGetUniformLocation(baseShader, "uReflector.kA");
    unsigned int lightDLoc = glGetUniformLocation(baseShader, "uReflector.kD");
    unsigned int lightSLoc = glGetUniformLocation(baseShader, "uReflector.kS");
    unsigned int lightCutoffLoc = glGetUniformLocation(baseShader, "uReflector.cutoff");
    unsigned int lightDirLoc = glGetUniformLocation(baseShader, "uReflector.dir");

    unsigned int materialShineLoc = glGetUniformLocation(baseShader, "uMaterial.shine");
    unsigned int materialALoc = glGetUniformLocation(baseShader, "uMaterial.kA");
    unsigned int materialDLoc = glGetUniformLocation(baseShader, "uMaterial.kD");
    unsigned int materialSLoc = glGetUniformLocation(baseShader, "uMaterial.kS");
    
    unsigned int viewPosLocTex = glGetUniformLocation(textureShader, "uViewPos");

    unsigned int materialShineLocTex = glGetUniformLocation(textureShader, "uMaterial.shine");
    unsigned int materialALocTex = glGetUniformLocation(textureShader, "uMaterial.kA");
    unsigned int materialDLocTex = glGetUniformLocation(textureShader, "uMaterial.kD");
    unsigned int materialSLocTex = glGetUniformLocation(textureShader, "uMaterial.kS");



    glUseProgram(baseShader);

    glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(model));
    glUniformMatrix4fv(viewLocBase, 1, GL_FALSE, value_ptr(view));
    glUniformMatrix4fv(projectionLocBase, 1, GL_FALSE, value_ptr(projection));

    glUniform3f(viewPosLoc, CAMERA_X_LOC, CAMERA_Y_LOC, CAMERA_Z_LOC); // Isto kao i pozicija kamere

    // Bela svetlost
    glUniform3f(lightPosLoc, -0.35f, -3.0f, 0.028f);
    glUniform3f(lightALoc, 0.2, 0.2, 0.2);
    glUniform3f(lightDLoc, 4.0, 4.0, 4.0);
    glUniform3f(lightSLoc, 4.0, 4.0, 4.0);
    glUniform1f(lightCutoffLoc, cos(radians(1.0f)));
    glUniform3f(lightDirLoc, 0.0, 1.0, 0.0);

    // Svojstva materijala
    glUniform1f(materialShineLoc, 132.0);      // Uglancanost (manja vrednost za slabiji sjaj)
    glUniform3f(materialALoc, 0.2, 0.2, 0.2);  // Ambijentalna refleksija materijala
    glUniform3f(materialDLoc, 0.5, 0.5, 0.5);  // Difuzna refleksija materijala
    glUniform3f(materialSLoc, 0.7, 0.7, 0.7);  // Spekularna refleksija materijala
    
    glUseProgram(textureShader);
    glUniform3f(viewPosLocTex, CAMERA_X_LOC, CAMERA_Y_LOC, CAMERA_Z_LOC); // Isto kao i pozicija kamere

    // Svojstva materijala teksture
    glUniform1f(materialShineLocTex, 132.0);      // Uglancanost
    glUniform3f(materialALocTex, 0.2, 0.2, 0.2);  // Ambijentalna refleksija materijala
    glUniform3f(materialDLocTex, 0.5, 0.5, 0.5);  // Difuzna refleksija materijala
    glUniform3f(materialSLocTex, 0.7, 0.7, 0.7);  // Spekularna refleksija materijala
    glUseProgram(baseShader);

    bool wasXpressed = false;
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);

    glfwSetKeyCallback(window, keyCallback);
    const double frameDuration = 1000.0 / 60.0;
    mat4 projectionText = ortho(0.0f, static_cast<float>(wWidth),
        0.0f, static_cast<float>(wHeight));
    glUseProgram(textShader);
    glUniformMatrix4fv(glGetUniformLocation(textShader, "projection"), 1, GL_FALSE, value_ptr(projectionText));
    while (!glfwWindowShouldClose(window))
    {
        auto frameStart = chrono::high_resolution_clock::now();
        glEnable(GL_DEPTH_TEST);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        {
            isMapHidden = true;
        }

        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        {
            isMapHidden = false;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            selectHelicopter(0);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            selectHelicopter(1);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            selectHelicopter(2);
        }

        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        {
            selectHelicopter(3);
        }
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        {
            selectHelicopter(4);
        }
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
        {
            cameraTarget = vec3(cityCenterX, cityCenterY, cityCenterZ);
            helTracked = -1;
        }
        
        if (rotateLeft)
        {
            cameraAngle = -rotationSpeed; // Negativni ugao za rotaciju ulevo
        }
        else if (rotateRight)
        {
            cameraAngle = rotationSpeed; // Pozitivni ugao za rotaciju udesno
        }
        else
        {
            cameraAngle = 0.0f; // Zaustavi rotaciju
        }
        if (zoomIn) {
            vec3 zoomDirection = normalize(cameraTarget - cameraPos);
            cameraPos += zoomDirection * zoomSpeed;
        }
        else if (zoomOut) {
            vec3 zoomDirection = normalize(cameraTarget - cameraPos);
            cameraPos -= zoomDirection * zoomSpeed;
        }
        vec3 direction = cameraTarget - cameraPos;
        mat4 rotationMatrix = rotate(mat4(1.0f), cameraAngle, vec3(0.0f, 1.0f, 0.0f));
        vec4 rotatedDirection = rotationMatrix * vec4(direction, 1.0f);

        cameraTarget = cameraPos + vec3(rotatedDirection);
        view = lookAt(cameraPos, cameraTarget, cameraUp);


        glClearColor(0.1, 0.1, 0.10023082, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //ISCRTAVANJE GLAVNOG DELA EKRANA
        glViewport(0, 0, wWidth, wHeight);
        
        model = mat4(1.0);
        glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(model));

        glUseProgram(textureShader);
        model[0] *= -1;
        glUniformMatrix4fv(modelLocTex, 1, GL_FALSE, value_ptr(model)); // (Adresa matrice, broj matrica koje saljemo, da li treba da se transponuju, pokazivac do matrica)
        glUniformMatrix4fv(viewLocTex, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projectionLocTex, 1, GL_FALSE, value_ptr(projection));
        glBindVertexArray(VAO[0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mapTexture);

        if (!isMapHidden)
        {
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 5);
        }
        glBindTexture(GL_TEXTURE_2D, 0);

        //// Renderovanje baze 3D------------------------------------------------------------------------------------
        if (baseCenterSet) {
            if (!changedCameraPos) {
                cameraPos = vec3(baseCenterX, 0.4, baseCenterZ);
                changedCameraPos = true;
            }
            renderBase(baseShader, baseVAO, colorLoc, modelLocBase, base, vec3(baseCenterX, 0.0, baseCenterZ), 1.0);
        }

        glCullFace(GL_FRONT);

        glCullFace(GL_BACK);

        glUseProgram(baseShader);

        glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(model)); // (Adresa matrice, broj matrica koje saljemo, da li treba da se transponuju, pokazivac do matrica)
        glUniformMatrix4fv(viewLocBase, 1, GL_FALSE, value_ptr(view));
        glUniformMatrix4fv(projectionLocBase, 1, GL_FALSE, value_ptr(projection));


        // Renderovanje centra Novog Sada ------------------------------------------------------------------------
        if (cityCenterSet) {
            renderBase(baseShader, baseVAO, colorLoc, modelLocBase, base, vec3(cityCenterX, cityCenterY, cityCenterZ),0.5);
            if (!alreadyPlacedCity) {
                cameraTarget = vec3(cityCenterX, cityCenterY, cityCenterZ);
                alreadyPlacedCity = true;
            }
            if (!initWait) {
                this_thread::sleep_for(chrono::seconds(3));
                initWait = true;
            }
            moveHelicoptersTowardsCityCenter(cityCenterX, cityCenterY, cityCenterZ, helicopterSpeed);
            activeRocketCam = true;
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !isSpacePressed) {
            if (selectedHel != -1 && rocketsLeft > 0 && !rockets[rocketsLeft - 1].isFlying) {
                rocketsLeft--;
                rockets[rocketsLeft].x = baseCenterX;
                rockets[rocketsLeft].y = baseCenterY;
                rockets[rocketsLeft].z = baseCenterZ;
                rockets[rocketsLeft].isFlying = true;
                rockets[rocketsLeft].targetHelicopter = selectedHel;



                // Postavi početni smer ka helikopteru
                float targetX = helicopterPositions[selectedHel].x;
                float targetY = helicopterPositions[selectedHel].y;
                float targetZ = helicopterPositions[selectedHel].z;
                rockets[rocketsLeft].dirX = -(targetX + baseCenterX);
                rockets[rocketsLeft].dirY = targetY - baseCenterY;
                rockets[rocketsLeft].dirZ = targetZ - baseCenterZ;
                normalizeVector(rockets[rocketsLeft].dirX, rockets[rocketsLeft].dirY, rockets[rocketsLeft].dirZ);

            }
            isSpacePressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
            isSpacePressed = false;
        }
        for (int i = 0; i < 10; ++i) {
            if (rockets[i].isFlying) {
                int targetHel = rockets[i].targetHelicopter;
                // Ažuriraj smer ka trenutnoj poziciji helikoptera
                float currentHelX = helicopterPositions[targetHel].x;
                float currentHelY = helicopterPositions[targetHel].y;
                float currentHelZ = helicopterPositions[targetHel].z;

                rockets[i].dirX = currentHelX - rockets[i].x;
                rockets[i].dirY = currentHelY - rockets[i].y;
                rockets[i].dirZ = currentHelZ - rockets[i].z;
                normalizeVector(rockets[i].dirX, rockets[i].dirY, rockets[i].dirZ);

                // Ažuriraj poziciju rakete
                rockets[i].x += rockets[i].dirX * rocketSpeed;
                rockets[i].y += rockets[i].dirY * rocketSpeed;
                rockets[i].z += rockets[i].dirZ * rocketSpeed;

                rockets[i].distance = sqrt(pow(rockets[i].x - currentHelX, 2) + pow(rockets[i].y - currentHelY, 2) + pow(rockets[i].z - currentHelZ, 2));

                if (checkCollision(rockets[i].x, rockets[i].y, rockets[i].z, currentHelX, currentHelY, currentHelZ)) {
                    random_device rd; // Nasumično seme
                    mt19937 gen(rd()); // Mersenne Twister generator
                    uniform_real_distribution<> dist(0.0, 1.0); // Interval [0, 1]

                    // Generišemo slučajan broj
                    double randomValue = dist(gen);

                    if (randomValue <= 0.75) {
                        cout << "POGODAK" << endl;
                        rockets[i].x = 1000.0;
                        rockets[i].y = 1000.0; // sklanjamo raketu sa scene
                        rockets[i].z = 1000.0;
                        helicopterPositions[targetHel].x = 1000.0;
                        helicopterPositions[targetHel].y = 1000.0; // sklanjamo helikopter sa scene
                        helicopterPositions[targetHel].z = 1000.0;
                        
                        if (rockets[i].targetHelicopter == selectedHel) { // vrati kameru na metu ako je ciljani helikopter pogodjen (i ako je bio trenutno izabran)
                            selectedHel = -1;
                            helTracked = -1;
                            cameraTarget = vec3(cityCenterX, cityCenterY, cityCenterZ);
                        }
                        rockets[i].isFlying = false; // Završava let rakete
                        rockets[i].targetHelicopter = -1; // Resetuj metu
                        for (int j = 0; j < 10; j++) { // unistavamo sve ostale rakete koje su ciljale isti taj helikopter
                            if (rockets[j].isFlying && rockets[j].targetHelicopter == targetHel) {
                                rockets[j].x = 1000.0;
                                rockets[j].y = 1000.0; // sklanjamo raketu sa scene
                                rockets[j].z = 1000.0;
                                rockets[j].isFlying = false; // Završava let rakete
                                rockets[j].targetHelicopter = -1; // Resetuj metu                                
                            }
                        }
                        
                        helicoptersLeft--;
                    }
                    else { // samo raketu sklanjamo
                        std::cout << "NEMAS SRECE" << std::endl;
                        rockets[i].x = 1000.0;
                        rockets[i].y = 1000.0; // sklanjamo raketu sa scene
                        rockets[i].z = 1000.0;
                        rockets[i].isFlying = false; // Završava let rakete
                        rockets[i].targetHelicopter = -1; // Resetuj metu
                    }
                }
                

                // Renderovanje 3D rakete
                glBindVertexArray(rocketVAO);
                mat4 model3D = mat4(1.0f);

                model3D = translate(model3D, vec3(rockets[i].x, rockets[i].y, rockets[i].z));
                
                //model3D = rotate(model3D, (float)radians(90.0f), vec3(1.0, 0.0, 0.0));
                model3D = scale(model3D, vec3(0.05f));
                glUniform3f(colorLoc, 1.0, 0.0, 0.0);
                glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(model3D));
                glDrawArrays(GL_TRIANGLES, 0, rocket.vertices.size());
                glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(mat4(1.0f)));
                glBindVertexArray(0);
            }
        }


        // Proteklo vreme
        auto currentTime = chrono::high_resolution_clock::now();
        float elapsedTime = chrono::duration_cast<chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

        // Renderovanje planine ------------------------------------------------------------------------------
        renderMountain(baseShader, mountainVAO, mapTexture, model, modelLocBase, mountain);

        // Renderovanje seta oblaka --------------------------------------------------------------------------
        bool hasTexture2 = false;
        renderClouds(baseShader, cloudVAO, hasTexture2, colorLoc, modelLocBase, cloud);

        // Renderovanje helikoptera --------------------------------------------------------------------------
        glUseProgram(baseShader);

        for (int i = 0; i < HELICOPTER_NUM; ++i) {
            if (helTracked == i) {
                cameraTarget = vec3(helicopterPositions[i].x, helicopterPositions[i].y, helicopterPositions[i].z);
            }
            glBindVertexArray(helicopterVAO);

            mat4 modelH = mat4(1.0f);
            modelH = translate(modelH, vec3(helicopterPositions[i].x, helicopterPositions[i].y, helicopterPositions[i].z));
            modelH = scale(modelH, vec3(0.01));

            glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(modelH));
            glUniform3f(colorLoc, 0.1, 0.4, 0.1);
            glDrawArrays(GL_TRIANGLES, 0, helicopter.vertices.size());

            glBindVertexArray(0);
            if (checkCollision(helicopterPositions[i].x, helicopterPositions[i].y, helicopterPositions[i].z, cityCenterX, cityCenterY, cityCenterZ)) {
                helicopterPositions[i].x = 1000.0f;
                helicopterPositions[i].y = 1000.0f;
                helicopterPositions[i].z = 1000.0f;
                cityHits++;
                helicoptersLeft--;
                for (int j = 0; j < 10; j++) {// nema vise smisla da raketa leti ka onom koji je vec pogodio grad
                    if (rockets[j].isFlying && rockets[j].targetHelicopter == i) {
                        rockets[j].isFlying = false;
                        rockets[j].x = 1000.0;
                        rockets[j].y = 1000.0;
                        rockets[j].z = 1000.0;
                    }
                }
                selectedHel = -1;
                helTracked = -1;
                if (cityHits >= 2) {
                    gameOver = true;
                }
            }
            if (helicoptersLeft <= 0) {
                gameOver = true;
            }
        }
        

        // Renderovanje mape ---------------------------------------------
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUseProgram(map2DShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, map2DTexture);
        glBindVertexArray(map2DVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        if (!activeRocketCam) {
            glUseProgram(map2DShader);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, noise2DTexture);
            glBindVertexArray(noise2DVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        
        glUseProgram(baseShader);


        // Renderovanje baze 2D ------------------------------------------------------------------------------------
        if (baseCenterSet) {
            glUseProgram(base2DShader);
            glUniform1i(applyTranslationLoc, true);
            glUniform2f(translationLocB, (-baseCenterX - 3) / 4, (baseCenterZ + 3) / 4);

            glBindVertexArray(VAOBase2D);
            colorLoc = glGetUniformLocation(base2DShader, "color");
            glUniform3f(colorLoc, 0.0, 0.0, 1.0);

            glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(baseCircle) / (2 * sizeof(float)));
        }

        if (cityCenterSet) {
            glUseProgram(base2DShader);
            glUniform1i(applyTranslationLoc, true);
            glUniform2f(translationLocB, (-cityCenterX - 3) / 4, (cityCenterZ + 3) / 4);

            glBindVertexArray(VAOCity2D);
            colorLoc = glGetUniformLocation(base2DShader, "color");
            glUniform3f(colorLoc, 1.0, 1.0, 0.0);

            glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(cityCenterCircle2D) / (2 * sizeof(float)));
        }
        for (int i = 0; i < 5; i++) {
            float dirX = Xto2D(-cityCenterX)  - Xto2D(-helicopterPositions[i].x);
            float dirY = Yto2D(cityCenterZ) - Yto2D(helicopterPositions[i].z);

            // Izraèunamo razdaljinu od koptera do centra - Pitagora
            float distance = sqrt(dirX * dirX + dirY * dirY);

            dirX /= distance;
            dirY /= distance;

            // Prilagodimo brzinu pulsiranja na osnovu udaljenosti od centra - osnovna brzina je 5.0f
            float pulseSpeed = 5.0f + 10.0f * (1.0f - distance);

            // Izraèunamo faktor pulsiranja na osnovu vremena i udaljenosti
            float pulseFactor = 0.5f + 0.5f * sin(elapsedTime * pulseSpeed);
            float redIntensity = 1.0;
            float greenIntensity = 1.0 - pulseFactor;
            float blueIntensity = 1.0 - pulseFactor;

            if (i == selectedHel) { // boji se samo ako ga niko trenutno ne gadja
                redIntensity = 1.0f;
                blueIntensity = 1.0f;
                greenIntensity = 0.0f;
            }


            glUseProgram(rocket2DShader);

            GLint scaleLoc = glGetUniformLocation(rocket2DShader, "uScale");

            // dinamički menjam veličinu kruga
            float dynamicScale = 1.0f+helicopterPositions[i].y*2;
            glUniform1f(scaleLoc, dynamicScale);

            glBindVertexArray(VAOBlue2D);
            GLint translationLoc = glGetUniformLocation(rocket2DShader, "uTranslation");
            glUniform2f(translationLoc, Xto2D(-helicopterPositions[i].x), Yto2D(helicopterPositions[i].z));
            colorLoc = glGetUniformLocation(rocket2DShader, "color");
            glUniform3f(colorLoc, redIntensity, greenIntensity, blueIntensity);
            glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(blueCircle2D) / (2 * sizeof(float)));
            renderText(textShader, helicopterStrings[i], Xto2D(-helicopterPositions[i].x)* wWidth / 2 + wWidth / 2 - 5, Yto2D(helicopterPositions[i].z)* wHeight / 2 + wHeight / 2 - 5, 0.2, glm::vec3(0.0f, 0.0f, 0.0f));

        }
        for (int i = 0; i < 10; i++) {
            if (rockets[i].isFlying) {
                renderText(textShader, to_string((int)(rockets[i].distance * 1000)) + "m", Xto2D(-rockets[i].x)* wWidth / 2 + wWidth / 2, Yto2D(rockets[i].z)* wHeight / 2 + wHeight / 2 - 20, 0.25, vec3(0.0f, 0.0f, 0.0f));
                glUseProgram(rocket2DShader);
                GLint scaleLoc = glGetUniformLocation(rocket2DShader, "uScale");

                // dinamički menjam veličinu kruga
                float dynamicScale = 1.0f + rockets[i].y * 2;
                glUniform1f(scaleLoc, dynamicScale);
                glBindVertexArray(VAOBlue2D);
                GLint translationLoc = glGetUniformLocation(rocket2DShader, "uTranslation");
                glUniform2f(translationLoc, Xto2D(-rockets[i].x), Yto2D(rockets[i].z));
                colorLoc = glGetUniformLocation(rocket2DShader, "color");
                glUniform3f(colorLoc, 0.0f, 1.0f, 0.0f);
                glDrawArrays(GL_TRIANGLE_FAN, 0, sizeof(blueCircle2D) / (2 * sizeof(float)));
            }
        }
        if (cityHits == 0) {
            renderText(textShader, "city: ACTIVE", 720, 200, 0.7, vec3(0.0, 1.0, 0.0));
        }
        else if (cityHits == 1) {
            renderText(textShader, "city: DAMAGED", 720, 200, 0.7, vec3(0.0, 1.0, 0.0));
        }
        else if (cityHits == 2) {
            renderText(textShader, "city: DESTROYED", 720, 200, 0.7, vec3(0.0, 1.0, 0.0));
        }
        renderText(textShader, "helicopters:" + to_string(helicoptersLeft), 720, 150, 0.7, vec3(0.0, 1.0, 0.0));
        renderText(textShader, "rockets:"+to_string(rocketsLeft), 720, 100, 0.7, vec3(0.0, 1.0, 0.0));
        if (gameOver && cityHits >= 2) {
            this_thread::sleep_for(chrono::seconds(1));
            renderText(textShader, "IZGUBILI STE", 200, 450, 1.5, vec3(1.0f, 0.0f, 0.0f));
            countTo3++;
            if (countTo3 >= 4) {// Zatvori aplikaciju
                exit(EXIT_FAILURE);
            }

        }
        else if (gameOver && helicoptersLeft <= 0) {
            this_thread::sleep_for(chrono::seconds(1));

            renderText(textShader, "ODBRANA JE USPJESNA!", 50, 450, 1.5, vec3(1.0f, 0.0f, 0.0f));
            countTo3++;
            if (countTo3 >= 4) {// Zatvori aplikaciju
                exit(EXIT_FAILURE);
            }
        }
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        // Pomeranje reflektora u krug - - - - - - - - - - - - - - - - - - - - - - - - -
        reflectorAngle += reflectorSpeed * elapsedTime;
        float reflectorX = 0.1f * reflectorRadius * cos(reflectorAngle);
        float reflectorZ = 0.1f * reflectorRadius * sin(reflectorAngle);

        glUseProgram(baseShader);
        glUniform3f(lightPosLoc, reflectorX, -3.0f, reflectorZ);


        //ISCRTAVANJE KAMERE RAKETE
        
        if (activeRocketCam) {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glViewport(wWidth * 0.75, wHeight * 0.75, wWidth / 4, wHeight / 4);

            model = mat4(1.0);
            glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(model));

            glUseProgram(textureShader);
            model[0] *= -1;
            glUniformMatrix4fv(modelLocTex, 1, GL_FALSE, value_ptr(model)); // (Adresa matrice, broj matrica koje saljemo, da li treba da se transponuju, pokazivac do matrica)
            glUniformMatrix4fv(viewLocTex, 1, GL_FALSE, value_ptr(rocketView));
            glUniformMatrix4fv(projectionLocTex, 1, GL_FALSE, value_ptr(projection));
            glBindVertexArray(VAO[0]);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mapTexture);

            if (!isMapHidden)
            {
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 5);
            }
            glBindTexture(GL_TEXTURE_2D, 0);


            glUniformMatrix4fv(viewLocTex, 1, GL_FALSE, value_ptr(rocketView));
            rocketView = lookAt(rocketCameraPos, rocketCameraTarget, rocketCameraUp);

            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
        }
        


        ///////////////////////////////////////////////////////////
        glfwSwapBuffers(window);
        glfwPollEvents();
        auto frameEnd = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> elapsed = frameEnd - frameStart;

        // Pauza ako je potrebno
        if (elapsed.count() < frameDuration) {
            this_thread::sleep_for(chrono::milliseconds(static_cast<int>(frameDuration - elapsed.count())));
        }
    }
    glDeleteBuffers(1, &VBOBase2D);
    glDeleteVertexArrays(1, &VAOBase2D);
    glDeleteTextures(1, &mapTexture);
    glDeleteTextures(1, &map2DTexture);
    glDeleteBuffers(1, &map2DVBO);
    glDeleteVertexArrays(1, &map2DVAO);
    glDeleteBuffers(2, VBO);
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(1, &mountainVBO);
    glDeleteVertexArrays(1, &mountainVAO);
    glDeleteBuffers(1, &rocketVBO);
    glDeleteVertexArrays(1, &rocketVAO);
    glDeleteBuffers(1, &cloudVBO);
    glDeleteVertexArrays(1, &cloudVAO);
    glDeleteBuffers(1, &baseVBO);
    glDeleteVertexArrays(1, &baseVAO);
    glDeleteBuffers(1, &helicopterVBO);
    glDeleteVertexArrays(1, &helicopterVAO);

    glDeleteProgram(textureShader);
    glDeleteProgram(baseShader);
    glDeleteProgram(rocket2DShader);
    glDeleteProgram(base2DShader);
    glDeleteProgram(rocketShader);
    glDeleteProgram(map2DShader);

    glfwTerminate();
    return 0;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_L)
    {
        if (action == GLFW_PRESS)
            rotateLeft = true;
        else if (action == GLFW_RELEASE)
            rotateLeft = false;
    }

    if (key == GLFW_KEY_J)
    {
        if (action == GLFW_PRESS)
            rotateRight = true;
        else if (action == GLFW_RELEASE)
            rotateRight = false;
    }
    if (key == GLFW_KEY_X)
    {
        if (action == GLFW_PRESS)
            zoomIn = true;
        else if (action == GLFW_RELEASE)
            zoomIn = false;
    }

    if (key == GLFW_KEY_Z)
    {
        if (action == GLFW_PRESS)
            zoomOut = true;
        else if (action == GLFW_RELEASE)
            zoomOut = false;
    }
}
void normalizeVector(float& x, float& y, float& z) {
    float length = sqrt(x * x + y * y + z*z);
    if (length != 0) {
        x /= length;
        y /= length;
        z /= length;
    }
}
void selectHelicopter(int helIndex) {
    if (helicopterPositions[helIndex].x < 100.0f) { // Proverava da li je helikopter na sceni
        selectedHel = helIndex;
        helTracked = helIndex;
    }
}
bool checkCollision(float x1, float y1, float z1, float x2, float y2, float z2) {
    float diff = 0.07;
    if (abs(x1 - x2) < diff && abs(y1 - y2) < diff && abs(z1 - z2) < diff) {
        return true;
    }
    return false;
}
float Xto2D(float a) {
    return (a - 3) / 4;
}
float Yto2D(float a) {
    return (a + 3) / 4;
}
void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // Dobijanje koordinata miša
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // Pretvaranje koordinata miša u OpenGL prostor
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // Konverzija iz piksela u normalized device coordinates (-1, 1)
        if (!baseCenterSet) {
            baseCenterX = (float)(2.0 * mouseX / width - 1.0);
            baseCenterZ = (float)(1.0 - 2.0 * mouseY / height);
            if (baseCenterX >= -1 && baseCenterX <= -0.5 && baseCenterZ >= 0.5 && baseCenterZ <= 1.0) { // samo ako smo kliknuli u gornji levi ugao ekrana
                baseCenterSet = true; // Centar je postavljen
                //baseCenterX = -baseCenterX;
            }
            baseCenterX = -(baseCenterX * 4 + 3);
            baseCenterZ = baseCenterZ * 4 - 3;
            cout << baseCenterX << " " << baseCenterZ << endl;
        }

        // Ako je baza već postavljena, postavi cityCenter
        else if (!cityCenterSet) {
            cityCenterX = (float)(2.0 * mouseX / width - 1.0);
            cityCenterZ = (float)(1.0 - 2.0 * mouseY / height);
            if (cityCenterX >= -1 && cityCenterX <= -0.5 && cityCenterZ >= 0.5 && cityCenterZ <= 1.0) {
                cityCenterSet = true;  // Postavljamo da je city center postavljen
                //cityCenterX = -cityCenterX;
            }
            cityCenterX = -(cityCenterX * 4 + 3);
            cityCenterZ = cityCenterZ * 4 - 3;
            cout << cityCenterX << " " << cityCenterZ << endl;
        }
    }
}

void renderBase(unsigned int baseShader, unsigned int baseVAO, int& colorLoc, unsigned int modelLocBase, ModelData& base, vec3 translateV, float scaleV)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(baseShader);
    glBindVertexArray(baseVAO);

    colorLoc = glGetUniformLocation(baseShader, "color");
    if (scaleV == 0.5) {//grad
        glUniform3f(colorLoc, 1.0, 1.0, 0.0);
    }
    else {//baza
        glUniform3f(colorLoc, 0.0, 0.0, 1.0);
    }
    

    mat4 modelB = mat4(1.0f);
    modelB = translate(modelB, translateV);
    modelB = scale(modelB, vec3(scaleV));
    glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(modelB));

    // Aktivacija teksture
    bool hasTexture = true;
    glUniform1i(glGetUniformLocation(baseShader, "useTexture"), hasTexture);

    glDrawArrays(GL_TRIANGLES, 0, base.vertices.size());
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}

void renderMountain(unsigned int baseShader, unsigned int mountainVAO, unsigned int mapTexture, glm::mat4& model, unsigned int modelLocBase, ModelData& mountain)
{
    glUseProgram(baseShader);
    glBindVertexArray(mountainVAO);

    // Uniforme teksture planine
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mapTexture);
    glUniform1i(glGetUniformLocation(baseShader, "uTex"), 0);

    GLint colorLoc = glGetUniformLocation(baseShader, "color");
    glUniform3f(colorLoc, 0.70, 0.58, 0.44);//0.82, 0.67, 0.46

    model = scale(model, vec3(0.1));
    model = translate(model, vec3(0.0, 0.0, -14.8));//0.0, 0.0, -12.8

    bool hasTexture = false;
    glUniform1i(glGetUniformLocation(baseShader, "useTexture"), hasTexture);
    glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(model));
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, mountain.vertices.size());
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
}

void renderClouds(unsigned int baseShader, unsigned int cloud1VAO, bool& hasTexture, int& colorLoc, unsigned int modelLocBase, ModelData& cloud1)
{
    // Renderovanje 1. oblaka ------------------------------------------------------------------------------
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(baseShader);
    glBindVertexArray(cloud1VAO);

    glUniform1i(glGetUniformLocation(baseShader, "useTexture"), hasTexture);
    colorLoc = glGetUniformLocation(baseShader, "color");
    glUniform3f(colorLoc, 0.7, 0.7, 0.7);
    GLuint alphaLoc = glGetUniformLocation(baseShader, "uAlpha");
    glUniform1f(alphaLoc, 0.5);
    mat4 model1 = mat4(1.0f);
    model1 = scale(model1, vec3(0.08));//0.1
    model1 = rotate(model1, (float)radians(180.0), vec3(1.0, 0.0, 0.0));/////
    model1 = translate(model1, vec3(0.0, -11.0, 0.0));//x=-2.0, y=6.0, z=1.0
    glUniformMatrix4fv(modelLocBase, 1, GL_FALSE, value_ptr(model1));

    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, cloud1.vertices.size());

    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glUniform1f(alphaLoc, 0.0);
    glEnable(GL_CULL_FACE);
}


void moveHelicoptersTowardsCityCenter(float cityCenterX, float cityCenterY, float cityCenterZ, float speed) {
    for (int i = 0; i < HELICOPTER_NUM; i++) {
        // Izracunamo vektor od helikoptera do centra
        float dirX = cityCenterX - helicopterPositions[i].x;
        float dirY = cityCenterY - helicopterPositions[i].y;
        float dirZ = cityCenterZ - helicopterPositions[i].z;

        // Izracunamo razdaljinu od koptera do centra
        float distance = sqrt(dirX * dirX + dirZ * dirZ + dirY * dirY);

        // Normalizujemo vektor
        dirX /= distance;
        dirY /= distance;
        dirZ /= distance;

        // Pomeramo helikopter ka centru odredjenom brzinom
        helicopterPositions[i].x += dirX * speed;
        helicopterPositions[i].y += dirY * speed;
        helicopterPositions[i].z += dirZ * speed;
    }
}

void generateHelicopterPositions(int number) {
    srand(static_cast<unsigned>(time(nullptr)));

    for (int i = 0; i < number; ++i) {
        int strana = rand() % 3;
        if (strana == 0) {                                      // Leva strana (zbog -x)
            helicopterPositions[i].x = 1;
            helicopterPositions[i].y = static_cast<float>(rand() % 101)/100;
            helicopterPositions[i].z = static_cast<float>(rand() % 101)/100;
        }
        else if (strana == 1) {                                 // Desna strana
            helicopterPositions[i].x = -1;
            helicopterPositions[i].y = static_cast<float>(rand() % 101)/100;
            helicopterPositions[i].z = static_cast<float>(rand() % 101)/100;
        }
        else {                                                  // Ispred nas
            helicopterPositions[i].x = static_cast<float>(rand() % 101)/100;
            helicopterPositions[i].y = static_cast<float>(rand() % 101)/100;
            helicopterPositions[i].z = 1;
        }
    }
}


bool isRocketOutsideScreen(float rocketX, float rocketY)
{
    return (rocketX < -1.0f || rocketX > 1.0f || rocketY < -1.0f || rocketY > 1.0f);
}

void moveRocket(GLFWwindow* window, float& rocketX, float& rocketY, float rocketSpeed, unsigned int wWidth, unsigned int wHeight)
{
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        rocketY += rocketSpeed;
        // Bilo nekad: rocketY = fmax(-1.0f, fmin(rocketY + rocketSpeed, 1.0f)); itd.
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        rocketY -= rocketSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        rocketX -= rocketSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        rocketX += rocketSpeed;
    }
}

void setCircle(float  circle[64], float r, float xPomeraj, float yPomeraj) {
    float centerX = 0.0;
    float centerY = 0.0;

    circle[0] = centerX + xPomeraj;
    circle[1] = centerY + yPomeraj;

    for (int i = 0; i <= CRES; i++) {
        circle[2 + 2 * i] = centerX + xPomeraj + r * cos((3.141592 / 180) * (i * 360 / CRES)); // Xi pomeren za xPomeraj
        circle[2 + 2 * i + 1] = centerY + yPomeraj + r * sin((3.141592 / 180) * (i * 360 / CRES)); // Yi pomeren za yPomeraj
    }
}
unsigned int compileShader(GLenum type, const char* source)
{
    string content = "";
    ifstream file(source);
    stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        cout << "Uspesno procitan fajl sa putanje \"" << source << "\"!" << endl;
    }
    else {
        ss << "";
        cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << endl;
    }
    string temp = ss.str();
    const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);

    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{

    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    program = glCreateProgram();

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);


    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        cout << "Objedinjeni sejder ima gresku! Greska: \n";
        cout << infoLog << endl;
    }

    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}
static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        cout << "Textura nije ucitana! Putanja texture: " << filePath << endl;
        stbi_image_free(ImageData);
        return 0;
    }
}
ModelData loadModel(const char* filePath) {
    ModelData modelData;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cerr << "Error loading model: " << importer.GetErrorString() << endl;
        return modelData;
    }

    processNode(scene->mRootNode, scene, modelData);

    return modelData;
}
void processMesh(aiMesh* mesh, const aiScene* scene, ModelData& modelData) {
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        vec3 vertex;
        vertex.x = mesh->mVertices[i].x;
        vertex.y = mesh->mVertices[i].y;
        vertex.z = mesh->mVertices[i].z;
        modelData.vertices.push_back(vertex);

        if (mesh->HasTextureCoords(0)) {
            vec2 texCoord;
            texCoord.x = mesh->mTextureCoords[0][i].x;
            texCoord.y = mesh->mTextureCoords[0][i].y;
            modelData.textureCoords.push_back(texCoord);
        }

        if (mesh->HasNormals()) {
            vec3 normal;
            normal.x = mesh->mNormals[i].x;
            normal.y = mesh->mNormals[i].y;
            normal.z = mesh->mNormals[i].z;
            modelData.normals.push_back(normal);
        }
        else {
            modelData.normals.push_back(vec3(0.0f, 0.0f, 0.0f));
        }
    }
}
void processNode(aiNode* node, const aiScene* scene, ModelData& modelData) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene, modelData);
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene, modelData);
    }
}
void setupModelVAO(unsigned int& VAO, unsigned int& VBO, const ModelData& modelData) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    size_t bufferSize = modelData.vertices.size() * sizeof(vec3) + modelData.textureCoords.size() * sizeof(vec2) + modelData.normals.size() * sizeof(vec3);
    vector<char> bufferData(bufferSize);

    memcpy(bufferData.data(), modelData.vertices.data(), modelData.vertices.size() * sizeof(vec3));
    memcpy(bufferData.data() + modelData.vertices.size() * sizeof(vec3), modelData.textureCoords.data(), modelData.textureCoords.size() * sizeof(vec2));
    memcpy(bufferData.data() + modelData.vertices.size() * sizeof(vec3) + modelData.textureCoords.size() * sizeof(vec2), modelData.normals.data(), modelData.normals.size() * sizeof(vec3));

    glBufferData(GL_ARRAY_BUFFER, bufferSize, bufferData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)(modelData.vertices.size() * sizeof(vec3)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)(modelData.vertices.size() * sizeof(vec3) + modelData.textureCoords.size() * sizeof(vec2)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}