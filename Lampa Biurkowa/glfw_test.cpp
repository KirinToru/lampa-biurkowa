#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include <GLFW/stb_image.h>

#define M_PI 3.14159265358979323846

using namespace std;

#define LICZBA_OB_TEXTUR 2
GLuint obiektyTextur[LICZBA_OB_TEXTUR];
const char* plikiTextur[] = {
"C://Users//Kirin//Projects//VS2022//Lampa Biurkowa//tex//gold.jpg",
"C://Users//Kirin//Projects//VS2022//Lampa Biurkowa//tex//wood.jpg"};

enum
{
    FULL_WINDOW,
    ASPECT_1_1,
    ORTO,
    FRUST
};

GLint skala = FULL_WINDOW;
GLint rzut = ORTO;

GLfloat zakres = 10.0f;
GLfloat blisko = 1.0f;
GLfloat daleko = 10.0f;

float xx = 0.0f, yy = 0.0f, zz = 0.0f;
float r1 = 0.0f, r2 = 0.0f, r3 = 0.0f;

float sceneScale = 1.0f;
float sceneAngelX = 0.0f;
float sceneAngelY = 0.0f;

bool iCull = false, iDepth = true, iOutline = false, iFill = false, iClock = false;

bool autoRotationEnabled = true;

const float scaleStep = 0.05f;
const float scaleAngel = 1.0f;

bool lightingEnabled = true;
bool light0Enabled = true;
bool light1Enabled = true;

GLfloat light0Position[4] = { 1.5f, 4.0f, 0.0f, 1.0f };
GLfloat light0Color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

GLfloat light1Position[] = { -2.0f, 4.0f, 2.0f, 1.0f };
GLfloat light1Color[] = { 1.0f, 0.5f, 0.5f, 1.0f };

int rotationMode = 0;
float deskAngle = 0.0f;

struct Vec3 {
    float x, y, z;
};

std::vector<Vec3> vertices_lamp;
std::vector<unsigned int> indices_lamp;

bool loadOBJ(const std::string& path, std::vector<Vec3>& out_vertices, std::vector<unsigned int>& out_indices) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Nie można otworzyć pliku: " << path << std::endl;
        return false;
    }

    std::vector<Vec3> temp_vertices;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            Vec3 vertex;
            ss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        }
        else if (prefix == "f") {
            unsigned int idx1, idx2, idx3;
            char slash;
            std::string v1, v2, v3;
            ss >> v1 >> v2 >> v3;

            auto parseIndex = [](const std::string& token) {
                std::istringstream s(token);
                unsigned int index;
                s >> index;
                return index;
                };

            idx1 = parseIndex(v1);
            idx2 = parseIndex(v2);
            idx3 = parseIndex(v3);

            out_indices.push_back(idx1 - 1);
            out_indices.push_back(idx2 - 1);
            out_indices.push_back(idx3 - 1);
        }
    }

    out_vertices = temp_vertices;
    return true;
}

void drawSphere(float R, int numTheta, int numPhi) {
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < numTheta; ++i) {
        float theta1 = (i * 2.0 * M_PI) / numTheta;
        float theta2 = ((i + 1) * 2.0 * M_PI) / numTheta;
        for (int j = 0; j < numPhi; ++j) {
            float phi1 = (j * M_PI) / numPhi;
            float phi2 = ((j + 1) * M_PI) / numPhi;
            float x1 = R * sin(phi1) * cos(theta1);
            float y1 = R * sin(phi1) * sin(theta1);
            float z1 = R * cos(phi1);
            float x2 = R * sin(phi2) * cos(theta1);
            float y2 = R * sin(phi2) * sin(theta1);
            float z2 = R * cos(phi2);
            float x3 = R * sin(phi2) * cos(theta2);
            float y3 = R * sin(phi2) * sin(theta2);
            float z3 = R * cos(phi2);
            float x4 = R * sin(phi1) * cos(theta2);
            float y4 = R * sin(phi1) * sin(theta2);
            float z4 = R * cos(phi1);
            // Pierwszy trójkąt
            glNormal3f(x1, y1, z1);
            glVertex3f(x1, y1, z1);
            glNormal3f(x2, y2, z2);
            glVertex3f(x2, y2, z2);
            glNormal3f(x3, y3, z3);
            glVertex3f(x3, y3, z3);
            // Drugi trójkąt
            glNormal3f(x1, y1, z1);
            glVertex3f(x1, y1, z1);
            glNormal3f(x3, y3, z3);
            glVertex3f(x3, y3, z3);
            glNormal3f(x4, y4, z4);
            glVertex3f(x4, y4, z4);
        }
    }
    glEnd();
}

void drawLamp()
{
    glBindTexture(GL_TEXTURE_2D, obiektyTextur[0]);

    glBegin(GL_TRIANGLES);

    for (unsigned int i = 0; i < indices_lamp.size(); i++) {
        Vec3& v = vertices_lamp[indices_lamp[i]];

        float u = (v.x + 1.0f) * 0.5f;
        float v_tex = (v.y + 1.0f) * 0.5f;

        glTexCoord2f(u, v_tex);
        glVertex3f(v.x, v.y, v.z);
    }

    glEnd();
}

void drawTable()
{
    glBindTexture(GL_TEXTURE_2D, obiektyTextur[1]);
    glBegin(GL_QUADS);

    // Front A1->B1->C1->D1
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0, 0.0); glVertex3f(-0.1, -1, 1.5);
    glTexCoord2f(0.0, 1.0); glVertex3f(-0.1, 1, 1.5);
    glTexCoord2f(1.0, 1.0); glVertex3f(0.1, 1, 1.5);
    glTexCoord2f(1.0, 0.0); glVertex3f(0.1, -1, 1.5);

    // Back A->B->C->D
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0, 0.0); glVertex3f(-0.1, -1, -1.5);
    glTexCoord2f(0.0, 1.0); glVertex3f(-0.1, 1, -1.5);
    glTexCoord2f(1.0, 1.0); glVertex3f(0.1, 1, -1.5);
    glTexCoord2f(1.0, 0.0); glVertex3f(0.1, -1, -1.5);

    // Left A->B->B1->A1
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0, 0.0); glVertex3f(-0.1, -1, -1.5);
    glTexCoord2f(0.0, 1.0); glVertex3f(-0.1, -1, 1.5);
    glTexCoord2f(1.0, 1.0); glVertex3f(-0.1, 1, 1.5);
    glTexCoord2f(1.0, 0.0); glVertex3f(-0.1, 1, -1.5);

    // Right C1->D1->D->C
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0, 0.0); glVertex3f(0.1, -1, -1.5);
    glTexCoord2f(0.0, 1.0); glVertex3f(0.1, -1, 1.5);
    glTexCoord2f(1.0, 1.0); glVertex3f(0.1, 1, 1.5);
    glTexCoord2f(1.0, 0.0); glVertex3f(0.1, 1, -1.5);

    // Top B->B1->D1->D
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0, 0.0); glVertex3f(-0.1, 1, -1.5);
    glTexCoord2f(0.0, 1.0); glVertex3f(-0.1, 1, 1.5);
    glTexCoord2f(1.0, 1.0); glVertex3f(0.1, 1, 1.5);
    glTexCoord2f(1.0, 0.0); glVertex3f(0.1, 1, -1.5);

    // Bottom A->A1->C1->C
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.0, 0.0); glVertex3f(-0.1, -1, -1.5);
    glTexCoord2f(0.0, 1.0); glVertex3f(-0.1, -1, 1.5);
    glTexCoord2f(1.0, 1.0); glVertex3f(0.1, -1, 1.5);
    glTexCoord2f(1.0, 0.0); glVertex3f(0.1, -1, -1.5);

    glEnd();
}

void display(float size) 
{
    if (autoRotationEnabled) {
        r1 += 0.01f;
        if (r1 > 360.0f) r1 -= 360.0f;
    }

    glPushMatrix();
    glRotated(r1, 0, 1, 0);
    glScalef(sceneScale, sceneScale, sceneScale);
    glRotatef(sceneAngelX, 0, 1, 0);
    glRotatef(sceneAngelY, 1, 0, 0);

    // --- Lampa ---
    glPushMatrix();

    // Zarowka
    if (light0Enabled) {
        GLfloat lightPos[] = { 0.0f, 1.0f, 0.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

        glPushMatrix();
        glTranslated(lightPos[0], lightPos[1], lightPos[2]);
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawSphere(0.2f, 20, 20);
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    glScalef(2.0f, 2.0f, 2.0f);

    glPushMatrix();
    glTranslated(0.0f, 0.4f, 0.0f);
    drawLamp();
    glPopMatrix();

    glPopMatrix();

    // --- Biurko ---
    glPushMatrix();
    glRotatef(deskAngle, 0, 1, 0);
    glColor3f(1.0f, 1.0f, 1.0f);

    // Pierwsza noga biurka
    glPushMatrix();
    glScalef(2.0f, 4.0f, 4.0f);
    glTranslated(2.7f, -1.0f, 0.0f);
    drawTable();
    glPopMatrix();

    // Druga noga biurka
    glPushMatrix();
    glScalef(2.0f, 4.0f, 4.0f);
    glTranslated(-1.4f, -1.0f, 0.0f);
    drawTable();
    glPopMatrix();

    // Trzecia noga biurka
    glPushMatrix();
    glScalef(2.0f, 4.0f, 4.0f);
    glTranslated(-3.1f, -1.0f, 0.0f);
    drawTable();
    glPopMatrix();

    // Gorna czesc biurka
    glPushMatrix();
    glScalef(4.0f, 4.0f, 6.0f);
    glRotated(90.0, 0, 1, 0);
    glRotated(90.0, 0, 0, 1);
    glTranslated(0.1f, 0.0f, -0.1f);
    drawTable();
    glPopMatrix();

    // Czesc biurka pod komputerem
    glPushMatrix();
    glScalef(1.5f, 1.8f, 4.0f);
    glRotated(90.0, 0, 1, 0);
    glRotated(90.0, 0, 0, 1);
    glRotated(90.0, 1, 0, 0);
    glTranslated(-4.0f, -3.0f, 0.0f);
    drawTable();
    glPopMatrix();

    // Czesc biurka nad komputerem
    glPushMatrix();
    glScalef(1.5f, 1.8f, 4.0f);
    glRotated(90.0, 0, 1, 0);
    glRotated(90.0, 0, 0, 1);
    glRotated(90.0, 1, 0, 0);
    glTranslated(-1.0f, -3.0f, 0.0f);
    drawTable();
    glPopMatrix();

    glPopMatrix();

    glPopMatrix();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_O:
            if (skala == FULL_WINDOW) {
                skala = ASPECT_1_1;
                cout << "Tryb: ASPECT_1_1 (zachowanie proporcji)" << endl;
            }
            else {
                skala = FULL_WINDOW;
                cout << "Tryb: FULL_WINDOW (z deformacja)" << endl;
            }
            break;
        case GLFW_KEY_LEFT:
            r1 += 5.0f;
            break;
        case GLFW_KEY_RIGHT:
            r1 -= 5.0f;
            break;
        case GLFW_KEY_R:
        {
            sceneScale = 1.0f;
            sceneAngelX = 0.0f;
            sceneAngelY = 0.0f;

            r1 = 0.0f;
            r2 = 0.0f;
            r3 = 0.0f;

            xx = 0.0f;
            yy = 0.0f;
            zz = 0.0f;

            light0Position[0] = 1.5f;
            light0Position[1] = 4.0f;
            light0Position[2] = 0.0f;
            light0Position[3] = 1.0f;

            skala = FULL_WINDOW;
            rzut = ORTO;

            lightingEnabled = true;
            light0Enabled = true;

            iCull = false;
            iDepth = true; 
            iOutline = false;
            iFill = false;
            iClock = false;

            cout << "Zresetowano wszystkie ustawienia" << endl;
            break;
        }
        case GLFW_KEY_W:
            sceneScale += scaleStep;
            break;
        case GLFW_KEY_S:
            sceneScale -= scaleStep;
            break;
        case GLFW_KEY_A:
            sceneAngelX += scaleAngel;
            break;
        case GLFW_KEY_D:
            sceneAngelX -= scaleAngel;
            break;
        case GLFW_KEY_SPACE:
            sceneAngelY += scaleAngel;
            break;
        case GLFW_KEY_LEFT_SHIFT:
            sceneAngelY -= scaleAngel;
            break;
        case GLFW_KEY_L:
            light0Enabled = !light0Enabled;
            cout << "Light0: " << (light1Enabled ? "wlaczone" : "wylaczone") << endl;
            break;
        case GLFW_KEY_K:
            light1Enabled = !light1Enabled;
            cout << "Light1: " << (light1Enabled ? "wlaczone" : "wylaczone") << endl;
            break;
        case GLFW_KEY_Z: rotationMode = 0; cout << "Obracanie: Lampa i biurko\n"; break;
        case GLFW_KEY_X: rotationMode = 1; cout << "Obracanie: Tylko lampa\n"; break;
        case GLFW_KEY_C: rotationMode = 2; cout << "Obracanie: Tylko biurko\n"; break;
        case GLFW_KEY_V: 
            autoRotationEnabled = !autoRotationEnabled;
            cout << "Obracanie: Automatyczne: " << (autoRotationEnabled ? "wlaczone" : "wylaczone") << endl;
            break;
        case GLFW_KEY_Q:
            if (rotationMode == 0 || rotationMode == 1) r1 += 5.0f;
            if (rotationMode == 0 || rotationMode == 2) deskAngle += 5.0f;
            break;
        case GLFW_KEY_E:
            if (rotationMode == 0 || rotationMode == 1) r1 -= 5.0f;
            if (rotationMode == 0 || rotationMode == 2) deskAngle -= 5.0f;
            break;
        case GLFW_KEY_1:
            iCull = !iCull;
            cout << "iCull: " << (iCull ? "ON" : "OFF") << endl;
            break;
        case GLFW_KEY_2:
            iDepth = !iDepth;
            cout << "iDepth: " << (iDepth ? "ON" : "OFF") << endl;
            break;
        case GLFW_KEY_3:
            iOutline = !iOutline;
            cout << "iOutline: " << (iOutline ? "ON" : "OFF") << endl;
            break;
        case GLFW_KEY_4:
            iFill = !iFill;
            cout << "iFill: " << (iFill ? "ON" : "OFF") << endl;
            break;
        case GLFW_KEY_5:
            iClock = !iClock;
            cout << "iClock: " << (iClock ? "ON" : "OFF") << endl;
            break;
        }
    }
}

void setupCull()
{
    if (iCull)
    {
        glEnable(GL_CULL_FACE);
    }

    else
    {
        glDisable(GL_CULL_FACE);
        return;
    }
}

void setupDepth()
{
    if (iDepth)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
        return;
    }
}

void setupOutline()
{
    if (iOutline)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        return;
    }

}

void setupFill()
{
    if (iFill)
        glShadeModel(GL_FLAT);
    else
    {
        glShadeModel(GL_SMOOTH);
        return;
    }

}

void setupClock()
{
    if (iClock)
        glFrontFace(GL_CW);
    else
    {
        glFrontFace(GL_CCW);
        return;
    }

}

void setupLights() {
    if (lightingEnabled)
        glEnable(GL_LIGHTING);
    else {
        glDisable(GL_LIGHTING);
        return;
    }
    if (light0Enabled) {
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Color);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light0Color);
        GLfloat ambient0[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
    }
    else {
        glDisable(GL_LIGHT0);
    }
    if (light1Enabled) {
        glEnable(GL_LIGHT1);
        glLightfv(GL_LIGHT1, GL_POSITION, light1Position);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Color);
        glLightfv(GL_LIGHT1, GL_SPECULAR, light1Color);
        GLfloat ambient1[] = { 0.1f, 0.05f, 0.05f, 1.0f };
        glLightfv(GL_LIGHT1, GL_AMBIENT, ambient1);
    }
    else glDisable(GL_LIGHT1);
}

int main(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(800, 600, "Lampa Biurkowa", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    loadOBJ("C:\\Users\\Kirin\\Projects\\VS2022\\Lampa Biurkowa\\lamp.obj", vertices_lamp, indices_lamp);

    glGenTextures(LICZBA_OB_TEXTUR, obiektyTextur);
    for (int i = 0; i < LICZBA_OB_TEXTUR; i++) {
        GLint iWidth, iHeight, nrChannels;
        glBindTexture(GL_TEXTURE_2D, obiektyTextur[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        unsigned char* data = stbi_load(plikiTextur[i], &iWidth, &iHeight, &nrChannels, 0);
        cout << iWidth << "\t" << obiektyTextur[i] << endl;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    GLfloat globalAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LESS);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        if (rzut == ORTO)
        {
            if (skala == FULL_WINDOW)
            {
                if (width < height && width > 0)
                    glOrtho(-zakres, zakres, -zakres * height / width, zakres * height / width, -zakres, zakres);
                else
                    if (width >= height && height > 0)
                        glOrtho(-zakres * width / height, zakres * width / height, -zakres, zakres, -zakres, zakres);
            }
            else
                glOrtho(-zakres, zakres, -zakres, zakres, -zakres, zakres);
        }

        if (rzut == FRUST)
        {
            if (skala == ASPECT_1_1)
            {
                if (width < height && width > 0)
                    glFrustum(-zakres, zakres, -zakres * height / width, zakres * height / width, blisko, daleko);
                else
                    if (width >= height && height > 0)
                        glFrustum(-zakres * width / height, zakres * width / height, -zakres, zakres, blisko, daleko);
            }
            else
                glFrustum(-zakres, zakres, -zakres, zakres, blisko, daleko);
        } 
        glMatrixMode(GL_MODELVIEW);

        setupLights();
        setupCull();
        setupDepth();
        setupOutline();
        setupFill();
        setupClock();

        float squareSize = static_cast<float>(std::min(width, height)) * 0.2f;
        display(squareSize);

        glClearColor(0.2, 0.2, 0.2, 1);
        glfwSwapBuffers(window);

        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        glEnable(GL_NORMALIZE);
        glEnable(GL_COLOR_MATERIAL);
        glEnable(GL_LIGHTING);

        glMaterialfv(GL_FRONT, GL_AMBIENT, new GLfloat[4]{ 0.5f, 0.5f, 0.5f, 1.0f });
        glMaterialfv(GL_FRONT, GL_DIFFUSE, new GLfloat[4]{ 0.9f, 0.9f, 0.9f, 1.0f });
        glMaterialfv(GL_FRONT, GL_SPECULAR, new GLfloat[4]{ 1.0f, 1.0f, 1.0f, 1.0f });
        glMaterialf(GL_FRONT, GL_SHININESS, 50.0f);

        glfwPollEvents();
    }
    glfwTerminate();
    glDeleteTextures(LICZBA_OB_TEXTUR, obiektyTextur);
    return 0;
}