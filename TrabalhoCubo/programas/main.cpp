#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "../lib/utils.h"

// Tamanho inicial da janela
int win_width = 800;
int win_height = 600;

int program;
unsigned int VAO1; // Vertex Array Object
unsigned int VBO1;
// Controla se a escala vai aumentar ou diminuir após a colisão
bool aumentarEscala = true;

// Variáveis de cores de fundo da tela
float bgColorR = 0.0f;
float bgColorG = 0.0f;
float bgColorB = 0.0f;

// Gera cores aleatórias para preenchimento do fundo da tela
void gerarCorFundoAleatoria() {
    bgColorR = static_cast<float>(rand()) / RAND_MAX;
    bgColorG = static_cast<float>(rand()) / RAND_MAX;
    bgColorB = static_cast<float>(rand()) / RAND_MAX;
}

float hLimit;   // limite horizontal (x)
float vLimit;   // limite vertical (y)

// Ângulo de rotação do cubo no eixo x
float cx_angle = 0.0f;
// Incremento no ângulo de rotação em x
float cx_inc = 0.1f;
// Ângulo de rotação do cubo no eixo y
float cy_angle = 0.0f;
// Incremento no ângulo de rotação em y
float cy_inc = 0.3f;
// Ângulo de rotação do cubo no eixo z
float cz_angle = 0.0f;
// Incremento no ângulo de rotação em z
float cz_inc = 0.2f;

// variaveis para o controle da posicao do objeto e da velocidade
glm::vec2 pos = glm::vec2(0.0f, 0.0f);    // posição do objeto
glm::vec2 vel = glm::vec2(0.01f, 0.012f); // velocidade (x, y)
float objeto_size = 0.2f;                 // "20% do tamanho" do objeto


// Shader de vértices
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

// Fragment shader: Calcula a cor final de cada pixel do cubo. Implementa o modelo de iluminação Phong (luz ambiente + luz difusa + luz especular)
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
                            "    vec3 light = (diffuse + specular + ambient);\n"
                            "    fragColor = vec4(vertexColor * light, 1.0);\n"
                            "}\0";

void display(void);
void reshape(int, int);
void keyboard(unsigned char, int, int);
void idle(void);
void initData(void);
void initShaders(void);

// Função de renderização principal do programa
void display()
{
    // Define a cor para “apagar” a tela antes de desenhar
    glClearColor(bgColorR, bgColorG, bgColorB, 1.0f);

    // Apaga/pinta a tela com a cor definida. Usada antes de desenhar a cena
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Ativa o programa de shaders
    glUseProgram(program);

    // Define a matriz de visualização (simula uma câmera olhando para a origem a partir da posição (0, 0, 3))
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

    // Envia a matriz view para o shader
    unsigned int loc = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));

    // Configura a matriz de projeção (define uma projeção em perspectiva com campo de visão de 52°)
    glm::mat4 projection = glm::perspective(glm::radians(52.0f), (win_width / (float)win_height), 0.1f, 100.0f);
    
    // Envia a matriz projection ao shader
    loc = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(projection));

    // Prepara o cubo para renderização (Ativa o VAO que contém os vértices do cubo)
    glBindVertexArray(VAO1);

    // Cria transformações: escala, rotação (em x, y e z), e translação com base na posição do cubo
    glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(objeto_size));
    glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), glm::radians(cx_angle), glm::vec3(3.0f, 0.0f, 0.0f));
    glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), glm::radians(cy_angle), glm::vec3(0.0f, 3.0f, 0.0f));
    glm::mat4 Rz = glm::rotate(glm::mat4(1.0f), glm::radians(cz_angle), glm::vec3(0.0f, 0.0f, 3.0f));
    glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
    
    // Combina as transformações para formar a matriz model
    glm::mat4 model = T * Ry * Rx * Rz * S;

    // Envia a matriz model para o shader
    loc = glGetUniformLocation(program, "model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));

    // loc = glGetUniformLocation(program, "objectColor");
    // glUniform3f(loc, 1.0f, 0.0f, 0.0f); 

    // Cor da luz
    loc = glGetUniformLocation(program, "lightColor");
    glUniform3f(loc, 1.0, 1.0, 1.0);

    // Posição da luz
    loc = glGetUniformLocation(program, "lightPosition");
    glUniform3f(loc, 0.0, 0.0, 0.0);

    // Posição da câmera
    loc = glGetUniformLocation(program, "cameraPosition");
    glUniform3f(loc, 0.0, 0.0, 0.0);

    // Desenha o cubo (tipo da primitiva=GL_TRIANGLES, início na posição 0 do array, 36 vértices a serem desenhados)
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Troca os buffers (double buffering) para exibir o frame atual
    glutSwapBuffers();
}

// Atualiza o viewport e limites de colisão com base no tamanho da janela
void reshape(int width, int height)
{
    // Atualiza os valores de altura e largura da janela
    win_width  = width;
    win_height = height;

    // Define a área da janela onde a imagem será desenhada. Começa no canto inferior esquerdo (0, 0) e vai até (width, height)
    glViewport(0, 0, width, height);

    // Determina até onde o cubo pode ir em X e Y antes de "bater na parede"
    // Recalcula limites de colisão
    float aspect = width / (float)height;
    // Distância da câmera à origem (usada na matriz view)
    const float camDist = 3.0f;
    // Calcula ângulo de visão vertical (em radianos)
    float vFov = glm::radians(52.0f);
    // Metade da altura visível no plano z=0
    vLimit = tan(vFov * 0.5f) * camDist;
    // Metade da largura visível no plano z=0
    hLimit = vLimit * aspect;

    // Força uma nova chamada à função display() para redesenhar a cena com a nova viewport
    glutPostRedisplay();
}

// Define o que fazer quando uma tecla é pressionada
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27: // (Esc) Encerra o pragrama
        exit(0);
    case 'q': // Encerra o programa
    case 'Q':
        exit(0);
    case 'a': // Aumenta o tamanho do cubo
        objeto_size = glm::min(2.0f, objeto_size + 0.05f);
        break;
    case 'd': // Diminui o tamanho do cubo
        objeto_size = glm::max(0.05f, objeto_size - 0.05f);
        break;
    }
}

// Função usada para animação
void idle()
{
    // Incrementa o ângulo de rotação (em x, y e z). Se ultrapassar 360°, "reinicia" o valor de forma suave
    cx_angle = ((cx_angle + cx_inc) < 360.0f) ? cx_angle + cx_inc : 360.0 - cx_angle + cx_inc;
    cy_angle = ((cy_angle + cy_inc) < 360.0f) ? cy_angle + cy_inc : 360.0 - cy_angle + cy_inc;
    cz_angle = ((cz_angle + cz_inc) < 360.0f) ? cz_angle + cz_inc : 360.0 - cz_angle + cz_inc;

    // Garante que a função display() seja chamada novamente para desenhar o cubo com os novos ângulos
    glutPostRedisplay();
}

// Prepara os dados necessários para renderizar o cubo
void initData()
{
    // Define o array de vértices do cubo
    float cube[] = {
        // Face frontal. Primeiro triângulo
            // coordenada       // cor
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, //1R
             0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, //2G
             0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, //3B
            // Face frontal. Segundo triângulo
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, //1R
             0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, //3B
            -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, //4Y
            // Face direita. Primeiro triângulo
             0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, //2G
             0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, //5Y
             0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f, //6R
            // Face direita. Segundo triângulo
             0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, //2G
             0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f, //6R
             0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, //3B
            // Face traseira. Primeiro triângulo
             0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, //5Y
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, //8B
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, //7G
            // Face traseira. Segundo triângulo
             0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, //5Y
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, //7G
             0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f, //6R
            // Face esquerda. Primeiro triângulo
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, //8B
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, //1R
            -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, //4Y
            // Face esquerda. Segundo triângulo
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, //8B
            -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, //4Y
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, //7G
            // Face superior. Primeiro triângulo
            -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, //4Y
             0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, //3B
             0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f, //6R
            // Face superior. Segundo triângulo
            -0.5f,  0.5f, 0.5f,  1.0f, 1.0f, 0.0f, //4Y
             0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f, //6R
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, //7G
            // Face inferior. Primeiro triângulo
            -0.5f, -0.5f, 0.5f,  1.0f, 0.0f, 0.0f, //1R
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, //8B
             0.5f, -0.5f, 0.5f,  0.0f, 1.0f, 0.0f, //2G
            // Face inferioir. Segundo triângulo
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, //8B
             0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, //5Y
             0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f //2G
        };

    // VAO guarda os estados/configurações dos atributos de vértice. Definido como os dados serão lidos da GPU
    glGenVertexArrays(1, &VAO1);
    glBindVertexArray(VAO1);

    // Cria um buffer (VBO) e envia para a GPU os dados do cubo. GL_STATIC_DRAW indica que os dados não serão modificados.
    glGenBuffers(1, &VBO1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

    // Define os atributos dos vértices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float))); 
    glEnableVertexAttribArray(2); 

    // Finaliza a configuração do VAO
    glBindVertexArray(0);

    // Permite que o OpenGL desenhe corretamente objetos 3D baseados na profundidade
    glEnable(GL_DEPTH_TEST);
}

// Função para compilar, linkar e ativar os shaders usados na renderização do cubo 3D com OpenGL
void initShaders()
{
    program = createShaderProgram(vertex_code, fragment_code);
}

// Move o cubo, detecta colisões, inverte direção, altera cor de fundo e tamanho do cubo
void update(int value)
{
    // Atualiza posição do cubo
    pos += vel;
    bool colisaoOcorreu = false;

    // Colisão lateral (eixo X)
    if ((vel.x > 0 && pos.x + objeto_size > hLimit) ||
        (vel.x < 0 && pos.x - objeto_size < -hLimit)) {
        vel.x *= -1;
        gerarCorFundoAleatoria();     // Chama mudança de cor
        colisaoOcorreu = true;
    }

    // Colisão vertical (eixo Y)
    if ((vel.y > 0 && pos.y + objeto_size > vLimit) ||
        (vel.y < 0 && pos.y - objeto_size < -vLimit)) {
        vel.y *= -1;
        gerarCorFundoAleatoria();
        colisaoOcorreu = true;
    }

    // Altera tamanho
    if (colisaoOcorreu) {
        if (aumentarEscala) {
            objeto_size *= 1.1f; // Aumenta 10%
            if (objeto_size >= 2.0f) {
                objeto_size = 2.0f;
                aumentarEscala = false; // Ao atingir o máximo, começa a diminuir
            }
        } else {
            objeto_size *= 0.9f; // Diminui 10%
            if (objeto_size <= 0.05f) {
                objeto_size = 0.05f;
                aumentarEscala = true; // Ao atingir o mínimo, começa a aumentar
            }
        }
        objeto_size = glm::clamp(objeto_size, 0.05f, 2.0f);
    }

    // Solicita redesenho da cena
    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // ~60fps
}

int main(int argc, char **argv)
{
    // Inicia a biblioteca GLUT
    glutInit(&argc, argv);
    // Informa que o contexto é compatível com a versão OpenGL 3.3
    glutInitContextVersion(3, 3);
    // Informa a compatibilidade do contexto
    glutInitContextProfile(GLUT_CORE_PROFILE);
    // Define opções de como uma cena deve ser renderizada
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    // Define a dimensão da janela que será mostrada
    glutInitWindowSize(win_width, win_height);
    // Cria e define o nome da janela
    glutCreateWindow("Trabalho Cubo");
    glewExperimental = GL_TRUE;
    // Inicia a compatibilidade de funções do OpenGL em diferentes sistemas operacionais
    glewInit();

    initData();

    initShaders();

    // Define a função para redimensionamento da janela
    glutReshapeFunc(reshape);
    // Define a função para desenho
    glutDisplayFunc(display);
    // Define a função para tratar entradas do teclado
    glutKeyboardFunc(keyboard);

    glutIdleFunc(idle);

    glutTimerFunc(0, update, 0);
    // Loop que executa e gerencia as funções/callbacks que devem ser chamadas para cada evento que ocorre no programa
    glutMainLoop();
}
