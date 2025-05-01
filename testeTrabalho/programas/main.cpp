#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "../lib/utils.h"

int win_width = 800;
int win_height = 600;

int program;
unsigned int VAO1;
unsigned int VBO1;

/** Cube x angle */
float cx_angle = 0.0f;
/** Cube x angle increment */
float cx_inc = 0.1f;
/** Cube y angle (orbit) */
float cy_angle = 0.0f;
/** Cube y angle increment */
float cy_inc = 0.3f;
/** Cube z angle */
float cz_angle = 0.0f;
/** Cube z angle increment */
float cz_inc = 0.2f;

// variaveis para o controle da posicao do objeto e da velocidade
glm::vec2 pos = glm::vec2(0.0f, 0.0f);    // posição do objeto
glm::vec2 vel = glm::vec2(0.01f, 0.012f); // velocidade (x, y)
float objeto_size = 0.2f;                 // "metade do tamanho" do objeto

/** Vertex shader. */
const char *vertex_code = "\n"
                          "#version 330 core\n"
                          "layout (location = 0) in vec3 position;\n"
                          "layout (location = 1) in vec3 normal;\n"
                          "\n"
                          "uniform mat4 model;\n"
                          "uniform mat4 view;\n"
                          "uniform mat4 projection;\n"
                          "\n"
                          "\n"
                          "out vec3 vNormal;\n"
                          "out vec3 fragPosition;\n"
                          "out vec3 vertexColor;\n"
                          "\n"
                          "void main()\n"
                          "{\n"
                          "    gl_Position = projection * view * model * vec4(position, 1.0);\n"
                          "    vNormal = mat3(transpose(inverse(model)))*normal;;\n"
                          "    fragPosition = vec3(model * vec4(position, 1.0));\n"
                          "    vertexColor = normal;\n"
                          "}\0";

/** Fragment shader. */
const char *fragment_code = "\n"
                            "#version 330 core\n"
                            "\n"
                            "in vec3 vNormal;\n"
                            "in vec3 fragPosition;\n"
                            "in vec3 vertexColor;\n"
                            "\n"
                            "out vec4 fragColor;\n"
                            "\n"
                            "uniform vec3 lightColor;\n"
                            "uniform vec3 lightPosition;\n"
                            "uniform vec3 cameraPosition;\n"
                            "\n"
                            "void main()\n"
                            "{\n"
                            "    float ka = 0.5;\n"
                            "    vec3 ambient = ka * lightColor;\n"
                            "\n"
                            "    float kd = 1.0;\n"
                            "    vec3 n = normalize(vNormal);\n"
                            "    vec3 l = normalize(lightPosition - fragPosition);\n"
                            "\n"
                            "    float diff = max(dot(n,l), 0.0);\n"
                            "    vec3 diffuse = kd * diff * lightColor;\n"
                            "\n"
                            "    float ks = 1.0;\n"
                            "    vec3 v = normalize(cameraPosition - fragPosition);\n"
                            "    vec3 r = reflect(-l, n);\n"
                            "\n"
                            "    float spec = pow(max(dot(v, r), 0.0), 3.0);\n"
                            "    vec3 specular = ks * spec * lightColor;\n"
                            "\n"
                            "    vec3 light = (ambient + diffuse + specular);\n"
                            "    fragColor = vec4(vertexColor * light, 1.0);\n"
                            "}\0";

void display(void);
void reshape(int, int);
void keyboard(unsigned char, int, int);
void idle(void);
void initData(void);
void initShaders(void);

void display()
{
    glClearColor(0.2, 0.3, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);

    // Define view matrix.
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

    // Send matrix to shader.
    unsigned int loc = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));

    // Define projection matrix.
    glm::mat4 projection = glm::perspective(glm::radians(52.0f), (win_width / (float)win_height), 0.1f, 100.0f);
    loc = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(projection));

    // Cube
    glBindVertexArray(VAO1);

    // Define model matrix.
    glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(objeto_size));
    glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), glm::radians(cx_angle), glm::vec3(3.0f, 0.0f, 0.0f));
    glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), glm::radians(cy_angle), glm::vec3(0.0f, 3.0f, 0.0f));
    glm::mat4 Rz = glm::rotate(glm::mat4(1.0f), glm::radians(cz_angle), glm::vec3(0.0f, 0.0f, 3.0f));
    glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
    glm::mat4 model = T * Ry * Rx * Rz * S;

    // Send model matrix to shader
    loc = glGetUniformLocation(program, "model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));

    // Set object color (you can change this color to any RGB value)
    loc = glGetUniformLocation(program, "objectColor");
    glUniform3f(loc, 1.0f, 0.0f, 0.0f); // Red color (example)

    // Light color.
    loc = glGetUniformLocation(program, "lightColor");
    glUniform3f(loc, 1.0, 1.0, 1.0);

    // Light position.
    loc = glGetUniformLocation(program, "lightPosition");
    glUniform3f(loc, 6.0, 0.0, 2.0);

    // Camera position.
    loc = glGetUniformLocation(program, "cameraPosition");
    glUniform3f(loc, 0.0, 0.0, 5.0);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glutSwapBuffers();
}


void reshape(int width, int height)
{
    win_width = width;
    win_height = height;
    glViewport(0, 0, width, height);
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        exit(0);
    case 'q':
    case 'Q':
        exit(0);
    case 'a': // aumenta o tamanho do cubo
        objeto_size = glm::min(2.0f, objeto_size + 0.05f);
        break;
    case 'd':
        objeto_size = glm::max(0.05f, objeto_size - 0.05f);
        break;
    }
}

void idle()
{
    cx_angle = ((cx_angle + cx_inc) < 360.0f) ? cx_angle + cx_inc : 360.0 - cx_angle + cx_inc;
    cy_angle = ((cy_angle + cy_inc) < 360.0f) ? cy_angle + cy_inc : 360.0 - cy_angle + cy_inc;
    cz_angle = ((cz_angle + cz_inc) < 360.0f) ? cz_angle + cz_inc : 360.0 - cz_angle + cz_inc;

    glutPostRedisplay();
}

void initData()
{
    // Set cube vertices.
    // float cube[] = {
    //     // Front face
    //     -0.5f, -0.5f, 0.5f, 1.1f, 0.1f, 0.1f,
    //     0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,
    //     0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f,

    //     -0.5f, -0.5f, 0.5f, 1.1f, 0.1f, 0.1f,
    //     0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f,
    //     -0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f,

    //     // Right face
    //     0.5f, -0.5f, 0.5f, 1.1f, 0.1f, 0.1f,
    //     0.5f, -0.5f, -0.5f, 1.1f, 0.1f, 0.1f,
    //     0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f,

    //     0.5f, -0.5f, 0.5f, 1.1f, 0.1f, 0.1f,
    //     0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
    //     0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f,

    //     // Back face
    //     0.5f, -0.5f, -0.5f, 1.1f, 0.1f, 0.1f,
    //     -0.5f, -0.5f, -0.5f, 1.1f, 0.1f, 0.1f,
    //     -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f,

    //     0.5f, -0.5f, -0.5f, 1.1f, 0.1f, 0.1f,
    //     -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
    //     0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f,

    //     // Left face
    //     -0.5f, -0.5f, -0.5f, 1.1f, 0.1f, 0.1f,
    //     -0.5f, -0.5f, 0.5f, 1.1f, 0.1f, 0.1f,
    //     -0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f,

    //     -0.5f, -0.5f, -0.5f, 1.1f, 0.1f, 0.1f,
    //     -0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f,
    //     -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f,

    //     // Top face
    //     -0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f,
    //     0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f,
    //     0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f,

    //     -0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f,
    //     0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
    //     -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f,

    //     // Bottom face
    //     -0.5f, -0.5f, 0.5f, 1.1f, 0.1f, 0.1f,
    //     -0.5f, -0.5f, -0.5f, 1.1f, 0.1f, 0.1f,
    //     0.5f, -0.5f, 0.5f, 1.1f, 0.1f, 0.1f,

    //     -0.5f, -0.5f, -0.5f, 1.1f, 0.1f, 0.1f,
    //     0.5f, -0.5f, -0.5f, 1.1f, 0.1f, 0.1f,
    //     0.5f, -0.5f, 0.5f, 1.1f, 0.1f, 0.1f};

    float cube[] = {
        //Front face first triangle.
            // coordinate       // color
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
            //Front face second triangle.
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
            // Right face first triangle.
             0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
            // Right face second triangle.
             0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
            // Back face first triangle.
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
            // Back face second triangle.
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
            // Left face first triangle.
            -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
            // Left face second triangle.
            -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
            // Top face first triangle.
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
            // Top face second triangle.
            -0.5f,  0.5f, 0.5f,  1.0f, 0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
            // Bottom face first triangle.
            -0.5f, -0.5f, 0.5f,  1.0f, 1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,
             0.5f, -0.5f, 0.5f,  1.0f, 1.0f, 1.0f,
            // Bottom face second triangle.
            -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f
        };

    // Vertex array.
    glGenVertexArrays(1, &VAO1);
    glBindVertexArray(VAO1);

    // Vertex buffer
    glGenBuffers(1, &VBO1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

    // Set attributes.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float))); // Envia as cores
    glEnableVertexAttribArray(2); // Habilita o uso das cores

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

void initShaders()
{
    // Request a program and shader slots from GPU
    program = createShaderProgram(vertex_code, fragment_code);
}

void update(int value)
{
    pos += vel;

    if (pos.x + objeto_size > 2.0f || pos.x - objeto_size < -2.0f)
        vel.x *= -1;

    if (pos.y + objeto_size > 1.5f || pos.y - objeto_size < -1.5f)
        vel.y *= -1;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // Chama novamente em ~16ms (~60fps)
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(win_width, win_height);
    glutCreateWindow("Teste Trabalho");
    glewExperimental = GL_TRUE;
    glewInit();

    initData();

    initShaders();

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);

    glutTimerFunc(0, update, 0);
    glutMainLoop();
}