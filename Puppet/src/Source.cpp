#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stack>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_glfw_gl3.h"

#include <iostream>

using namespace std;

const float pi = 3.14159265359f;

const glm::mat4 I = glm::mat4(1.0f);

struct ShaderProgramSource {//Estructura para regresar los shaders al leer un archivo
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath) { //Para obtener los shaders del documento
    std::fstream stream(filepath);

    enum class ShaderType {//Banderas para saber el shader con el que estamos trabajando
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2]; // Uno para cada shader
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line)) {//Leemos lineas
        if (line.find("#shader") != std::string::npos) { // Si encontramos alguna bandera para shader
            if (line.find("vertex") != std::string::npos) {// Si encontro la bandera para vertex
                type = ShaderType::VERTEX;//Especificamos que estaremos trabajando con un shader vertex
            }
            else if (line.find("fragment") != std::string::npos) {//Si encontro la bandera para fragment
                type = ShaderType::FRAGMENT;//Especificamso que estaremos trabajando con un shader fragment
            }
        }
        else {//A adiremos la linea al shader seg n el type que registramos anteriormente
            ss[(int)type] << line << "\n";
        }
    }

    return ShaderProgramSource{ ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source) { // compilar el shader

    unsigned int id = glCreateShader(type); // id para identificar nuestro shader, segun el type
    const char* src = source.c_str(); // apuntador al primer caracter de source (source especifica los detalles del shader, y se manda como un string de una funcion)
    glShaderSource(id, 1, &src, nullptr); // creacion del shader
    glCompileShader(id); // compilacion

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Error al compilar el " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader " << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }


    return id; // regresamos el id del shader creado
}

static int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) { // Trabajaremos con los source code
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader); //vertex shader
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader); //fragment shader

    // Adjuntaremos vs y fs a un solo programa
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    // Ya que unimos los shaders al programa, los eliminamos
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}


int main() {
    GLFWwindow* window;

    /* Inicializar la libretria, se regresa -1 (error) en caso contrario */
    if (!glfwInit())
        return -1;

    /* Crear una ventana de windows y su contexto OpenGl */
    window = glfwCreateWindow(1280, 1280, "Hello World", NULL, NULL);// Especificamos detalles para la ventana
    if (!window)//En caso de que no se haya creado la ventana, terminamos glfw y regresamos -1 (error)
    {
        glfwTerminate();
        return -1;
    }

    /* Actualizamos el contexto de la ventana */
    glfwMakeContextCurrent(window);

    //glewInit();
    if (glewInit() != GLEW_OK) {
        std::cout << "ERROR on GLEW" << std::endl;
    }
    std::cout << glGetString(GL_VERSION) << std::endl;


    /* Definimos un vertex buffer para un cubo unitario centrado en el origen */
    float positions[8 * 3] = { // Creamos un array para 8 vertices 3D
        // Los vertices fueron considerando que el eje Y apuntaba a la camara, pero en realidad apunta el eje Z
        // X      Y      Z
        // Vertices inferiores
        -0.5f,  -0.5f,  0.5f,
        0.5f,   -0.5f,  0.5f,
        0.5f,   -0.5f,  -0.5f,
        -0.5f,  -0.5f,  -0.5f,


        //Vertices superiores
        -0.5f,  0.5f,  0.5f,
        0.5f,   0.5f,  0.5f,
        0.5f,   0.5f,  -0.5f,
        -0.5f,  0.5f,  -0.5f
    };

    /* Definimos un index buffer para el cubo unitario centrado en el origen */
    unsigned int indices[6 * 2 * 3] = {
        // Cara 1
        3,2,1,  // Triangulo 1
        1,0,3,  // Triangulo 2

        // Cara 2
        1,2,6,  // Triangulo 1
        6,5,1,  // Triangulo 2

        // Cara 3
        2,3,7,  // Triangulo 1
        7,6,2,  // Triangulo 2

        // Cara 4
        3,0,4,  // Triangulo 1
        4,7,3,  // Triangulo 2

        // Cara 5
        0,1,5,  // Triangulo 1
        5,4,0,  // Triangulo 2

        // Cara 6
        4,5,6,  // Triangulo 1
        6,7,4   // Triangulo 2
    };

    /*  Matriz MVP  */
    //////////////////////////////////////////////////////////
    // Projection matrix: 45  Field of View, 4:3 ratio, display range: 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)640 / (float)640, 0.1f, 100.0f);

    // Camera matrix
    glm::mat4 View = glm::lookAt(
        glm::vec3(2*4, 2*3, 2*3), // Camera is at (4,3,3), in World Space
        glm::vec3(0, 0, 0), // and looks at the origin
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Model matrix: an identity matrix (model will be at the origin)
    glm::mat4 Model = I;

    // Our ModelViewProjection: multiplication of our 3 matrices
    glm::mat4 mvp_base = Projection * View * Model; // Remember, matrix multiplication is the other way around
    
    //Inicializacion de ImGui
    ImGui::CreateContext();
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsDark();

    /* Array Buffer */
    unsigned int buffer; // Para identificar el buffer
    glGenBuffers(1, &buffer); //Generamos 1 buffer
    glBindBuffer(GL_ARRAY_BUFFER, buffer); //Determina el proposito del buffer
    glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(float), positions, GL_STATIC_DRAW); // Especificamos: typo de la data del buffer, tama o, la data, el tipo, como se accedera al almacen de data de un objeto de buffer

    /*  Atributos de los vertices */
    glEnableVertexAttribArray(0); // Habilitar los atributos del vertice. Reciber como atributo el indice del atributo de vertice generico
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
    // Parametros: indice en el cual se empezaran a considerar los vertices, cantidad de elementos por vertice {1,2,3,4}, tipo del dato que proporcionamos, bandera si se quieren normalizar los datos, cantidad de bytes entre vertices, pointer para ayudar a dividir el vertice

    /* Creamos los Shaders */
    // Leeremos el .shader para obtener los shaders
    ShaderProgramSource source = ParseShader("res/Shaders/Basic.shader");// Usamos el path relativo al proyecto
    std::cout << "VERTEX\n" << source.VertexSource << std::endl;
    std::cout << "FRAGMENT\n" << source.FragmentSource << std::endl;



    /* Index Buffer */
    unsigned int ibo; // Para identificar el buffer
    glGenBuffers(1, &ibo); //Generamos 1 buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); //Determina el proposito del buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * 2 * 3 * sizeof(unsigned int), indices, GL_STATIC_DRAW); // Especificamos: typo de la data del buffer, tama o, la data, el tipo, como se accedera al almacen de data de un objeto de buffer


    // Tomamos un unsigned int que represente el shader que utilizaremos
    unsigned int shader_0 = CreateShader(source.VertexSource, source.FragmentSource);

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


    /////////// DEPTH ///////////
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Obtenemos el id de mvp en el shader, para dibujar los subojetos
    int MatrixID = glGetUniformLocation(shader_0, "mvp");

    // Declaramos stacks para las jerarquias del puppet
    std::stack<glm::mat4> PI;//Pierna izquierda
    std::stack<glm::mat4> PD;//Piernda derecha
    std::stack<glm::mat4> BI;//Brazo izquierdo
    std::stack<glm::mat4> BD;//Brazo derecho
    std::stack<glm::mat4> C;//Cuello

    //////////////////// Variables de dimensiones del puppet ////////////////////
    // Variables que nos indican las dimensiones del torso
    float torso_x = 1;
    float torso_y = 1;
    float torso_z = 0.5;

    // Dimensiones de los hombros
    float hombros_x = 1;
    float hombros_y = 0.3;
    float hombros_z = 0.5;

    // Dimensiones de los brazos
    float brazos_x = 0.8;
    float brazos_y = 0.2;
    float brazos_z = 0.3;

    // Dimensiones de las manos
    float manos_x = 0.45;
    float manos_y = 0.5 / 4;
    float manos_z = 0.5;
    
    // Dimensiones de la cadera
    float cadera_x = 1;
    float cadera_y = 0.3;
    float cadera_z = 0.5;
 
    // Dimensiones de las piernas
    float pierna_x = 0.45;
    float pierna_y = 0.95;
    float pierna_z = 0.5;

    // Dimensiones de los pies
    float pie_x = 0.45;
    float pie_y = 0.5/4;
    float pie_z = 0.7;

    // Dimensiones del cuello
    float cuello_x = 0.2;
    float cuello_y = 0.2;
    float cuello_z = 0.2;

    //Dimensiones de la cabeza
    float cabeza_x = 0.5;
    float cabeza_y = 0.5;
    float cabeza_z = 0.5;

    // Dimensiones de la nariz
    float nariz_x = 0.05;
    float nariz_y = 0.05;
    float nariz_z = 0.2;

    //////////////////// Variables para el movimiento de puppet ////////////////////
    // Traslacion de torso
    float torso_t_x = 0;
    float torso_t_y = 0;
    float torso_t_z = 0;
    // Rotacion de torso
    float torso_r_x = 0;
    float torso_r_y = 0;
    float torso_r_z = 0;

    // Rotacion hombro izquierdo
    float hombro_up_down_izq = 0; // [-pi/2, pi/2]
    float hombro_front_back_izq = 0; // [-5*pi/6, 0]
    float hombro_spin_izq = 0; // [-pi/2, pi/2]
    // Rotacion hombro derecho
    float hombro_up_down_der = 0; // [-pi/2, pi/2]
    float hombro_front_back_der = 0; // [0, 5*pi/6]
    float hombro_spin_der = 0; // [-pi/2, pi/2] 
    // Rotacion codo izquierdo
    float codo_abrir_cerrar_izq = 0; // [-5*pi/6,0]
    // Rotacion codo derecho
    float codo_abrir_cerrar_der = 0; // [0, 5*pi/6]
    //Rotacion mano izquierda
    float mano_front_back_izq = 0; // [-pi/4, pi/4]
    float mano_up_down_izq = 0; // [-pi/2, pi/2]
    //Rotacion mano derecha
    float mano_front_back_der = 0; // [-pi/4, pi/4]
    float mano_up_down_der = 0; // [-pi/2, pi/2]

    // Rotacion cadera izquierda
    float cadera_left_right_izq = 0; // [0, pi/4]
    float cadera_spin_izq = 0; // [0, pi/2]
    float cadera_back_front_izq = 0; // [-pi/4, pi/4]
    // Rotacion cadera derecha
    float cadera_left_right_der = 0; // [-pi/4, 0]
    float cadera_spin_der = 0; // [-pi/2, 0]
    float cadera_back_front_der = 0; // [-pi/4, pi/4]
    // Rotacion rodilla izquierda
    float rodilla_izq = 0; // [0, pi/2]
    // Rotacion rodilla derecha
    float rodilla_der = 0; // [0, pi/2]
    // Rotacion Pie izquierdo
    float pie_left_rigt_izq = 0; // [0,pi/4]
    float pie_up_down_izq = 0; // [-pi/5, pi/4]
    // Rotacion Pie derecho
    float pie_left_rigt_der = 0; // [-pi/4,0]
    float pie_up_down_der = 0; // [-pi/5, pi/4]

    // Rotacion Cuello
    float cuello_up_down = 0; // [-pi/4, pi/4]
    float cuello_left_right = 0; // [-pi/2, pi/2]
    float cuello_inclinacion = 0; // [-pi/4, pi/4]

    /* Generamos un loop hasta que la ventana se cierre */
    while (!glfwWindowShouldClose(window))
    {
        /* Render aqui */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // New Gui Frame
        ImGui_ImplGlfwGL3_NewFrame();

        //////////////////// Dibujo de los modelos ////////////////////

        //Generamos las matrices con las que iremos generando los subjetos, y las agregamos a sus respectivas stacks
        glm::mat4 Modelo_Torso = I;
        glm::mat4 Modelo_PI = I;
        glm::mat4 Modelo_PD = I;
        glm::mat4 Modelo_BI = I;
        glm::mat4 Modelo_BD = I;
        glm::mat4 Modelo_C = I;


        //////// Torso ////////
        {
            Modelo_Torso = glm::translate(Modelo_Torso, glm::vec3(torso_t_x, torso_t_y, torso_t_z));
            Modelo_Torso = glm::rotate(Modelo_Torso, torso_r_x, glm::vec3(1, 0, 0));
            Modelo_Torso = glm::rotate(Modelo_Torso, torso_r_y, glm::vec3(0, 1, 0));
            Modelo_Torso = glm::rotate(Modelo_Torso, torso_r_z, glm::vec3(0, 0, 1));

            // Generamos las matrices con las que iremos generando los sub-objetos, y las agregamos a sus respectivas stacks
            Modelo_PI = Modelo_Torso;
            Modelo_PD = Modelo_Torso;
            Modelo_BI = Modelo_Torso;
            Modelo_BD = Modelo_Torso;
            Modelo_C = Modelo_Torso;

            PI.push(Modelo_PI);
            PD.push(Modelo_PD);
            BI.push(Modelo_BI);
            BD.push(Modelo_BD);
            C.push(Modelo_C);

            // Escalamos
            Modelo_Torso = glm::scale(Modelo_Torso, glm::vec3(torso_x, torso_y, torso_z));

            // Dinujamos
            glm::mat4 mvp = mvp_base * Modelo_Torso;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);

            // El resto de modelos siguen una idea similar
        }

        //////// Hombros ////////
        {
            Modelo_BI = glm::translate(Modelo_BI, glm::vec3(0, (torso_y / 2) + (hombros_y / 2), 0));
            Modelo_BD = Modelo_BI;
            BI.push(Modelo_BI);
            BD.push(Modelo_BD);

            Modelo_BI = glm::scale(Modelo_BI, glm::vec3(hombros_x, hombros_y, hombros_z));

            glm::mat4 mvp = mvp_base * Modelo_BI;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }

        //////// Brazo Izquierdo ////////
        // Modelo Brazo izquierdo 1
        {
            Modelo_BI = BI.top();
            Modelo_BI = glm::translate(Modelo_BI, glm::vec3((brazos_x / 2), 0, 0));
            Modelo_BI = glm::rotate(Modelo_BI, hombro_spin_izq, glm::vec3(1, 0, 0));
            Modelo_BI = glm::rotate(Modelo_BI, hombro_front_back_izq, glm::vec3(0, 1, 0));
            Modelo_BI = glm::rotate(Modelo_BI, hombro_up_down_izq, glm::vec3(0, 0, 1));
            Modelo_BI = glm::translate(Modelo_BI, glm::vec3((torso_x / 2), 0, 0));

            BI.push(Modelo_BI);

            Modelo_BI = glm::scale(Modelo_BI, glm::vec3(brazos_x, brazos_y, brazos_z));
            glm::mat4 mvp = mvp_base * Modelo_BI;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }
        // Modelo Brazo Izquierdo 2
        {
            Modelo_BI = BI.top();
            Modelo_BI = glm::translate(Modelo_BI, glm::vec3(brazos_x / 2, 0, 0));
            Modelo_BI = glm::rotate(Modelo_BI, codo_abrir_cerrar_izq, glm::vec3(0, 1, 0));
            Modelo_BI = glm::translate(Modelo_BI, glm::vec3(brazos_x / 2, 0, 0));

            BI.push(Modelo_BI);

            Modelo_BI = glm::scale(Modelo_BI, glm::vec3(brazos_x, brazos_y, brazos_z));
            glm::mat4 mvp = mvp_base * Modelo_BI;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }
        //Modelo Brazo Izquierdo 3
        {
            Modelo_BI = BI.top();
            Modelo_BI = glm::translate(Modelo_BI, glm::vec3(brazos_x / 2, 0, 0));
            Modelo_BI = glm::rotate(Modelo_BI, mano_front_back_izq, glm::vec3(0, 1, 0));
            Modelo_BI = glm::rotate(Modelo_BI, mano_up_down_izq, glm::vec3(0, 0, 1));
            Modelo_BI = glm::translate(Modelo_BI, glm::vec3(manos_x / 2, 0, 0));

            BI.push(Modelo_BI);

            Modelo_BI = glm::scale(Modelo_BI, glm::vec3(manos_x, manos_y, manos_z));
            glm::mat4 mvp = mvp_base * Modelo_BI;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }


        //////// Brazo Derecho ////////
        // Modelo Brazo Derecho 1
        {
            Modelo_BD = BD.top();
            Modelo_BD = glm::translate(Modelo_BD, glm::vec3(-(brazos_x / 2), 0, 0));
            Modelo_BD = glm::rotate(Modelo_BD, hombro_spin_der, glm::vec3(1, 0, 0));
            Modelo_BD = glm::rotate(Modelo_BD, hombro_front_back_der, glm::vec3(0, 1, 0));
            Modelo_BD = glm::rotate(Modelo_BD, hombro_up_down_der, glm::vec3(0, 0, 1));
            Modelo_BD = glm::translate(Modelo_BD, glm::vec3(-(torso_x / 2), 0, 0));

            BD.push(Modelo_BD);

            Modelo_BD = glm::scale(Modelo_BD, glm::vec3(brazos_x, brazos_y, brazos_z));
            glm::mat4 mvp = mvp_base * Modelo_BD;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }
        // Modelo Brazo Derecho 2
        {
            Modelo_BD = BD.top();
            Modelo_BD = glm::translate(Modelo_BD, glm::vec3(-brazos_x / 2, 0, 0));
            Modelo_BD = glm::rotate(Modelo_BD, codo_abrir_cerrar_der, glm::vec3(0, 1, 0));
            Modelo_BD = glm::translate(Modelo_BD, glm::vec3(-brazos_x / 2, 0, 0));

            BD.push(Modelo_BD);

            Modelo_BD = glm::scale(Modelo_BD, glm::vec3(brazos_x, brazos_y, brazos_z));
            glm::mat4 mvp = mvp_base * Modelo_BD;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }
        //Modelo Brazo Derecho 3
        {
            Modelo_BD = BD.top();
            Modelo_BD = glm::translate(Modelo_BD, glm::vec3(-brazos_x / 2, 0, 0));
            Modelo_BD = glm::rotate(Modelo_BD, mano_front_back_der, glm::vec3(0, 1, 0));
            Modelo_BD = glm::rotate(Modelo_BD, mano_up_down_der, glm::vec3(0, 0, 1));
            Modelo_BD = glm::translate(Modelo_BD, glm::vec3(-manos_x / 2, 0, 0));

            BD.push(Modelo_BD);

            Modelo_BD = glm::scale(Modelo_BD, glm::vec3(manos_x, manos_y, manos_z));
            glm::mat4 mvp = mvp_base * Modelo_BD;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }

        //////// Cadera ////////
        {
            Modelo_PI = glm::translate(Modelo_PI, glm::vec3(0, -(torso_y / 2) - (cadera_y / 2), 0));
            Modelo_PD = Modelo_PI;
            PI.push(Modelo_PI);
            PD.push(Modelo_PD);

            Modelo_PI = glm::scale(Modelo_PI, glm::vec3(cadera_x, cadera_y, cadera_z));

            glm::mat4 mvp = mvp_base * Modelo_PI;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }

        //////// Pierna Izquierda ////////
        // Modelo Pierna Izquierda 1
        {
            Modelo_PI = PI.top();
            Modelo_PI = glm::translate(Modelo_PI, glm::vec3(cadera_x / 2 - pierna_x / 2, -cadera_y / 2, 0));
            Modelo_PI = glm::rotate(Modelo_PI, cadera_back_front_izq, glm::vec3(1, 0, 0));
            Modelo_PI = glm::rotate(Modelo_PI, cadera_spin_izq, glm::vec3(0, 1, 0));
            Modelo_PI = glm::rotate(Modelo_PI, cadera_left_right_izq, glm::vec3(0, 0, 1));
            Modelo_PI = glm::translate(Modelo_PI, glm::vec3(0, -pierna_y / 2, 0));

            PI.push(Modelo_PI);

            Modelo_PI = glm::scale(Modelo_PI, glm::vec3(pierna_x, pierna_y, pierna_z));

            glm::mat4 mvp = mvp_base * Modelo_PI;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }
        // Modelo Pierna Izquierda 2
        {
            Modelo_PI = PI.top();
            Modelo_PI = glm::translate(Modelo_PI, glm::vec3(0, -pierna_y / 2, 0));
            Modelo_PI = glm::rotate(Modelo_PI, rodilla_izq, glm::vec3(1, 0, 0));
            Modelo_PI = glm::translate(Modelo_PI, glm::vec3(0, -pierna_y / 2, 0));

            PI.push(Modelo_PI);

            Modelo_PI = glm::scale(Modelo_PI, glm::vec3(pierna_x, pierna_y, pierna_z));

            glm::mat4 mvp = mvp_base * Modelo_PI;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }
        // Modelo Pierna Izquierda 3
        {
            Modelo_PI = PI.top();
            Modelo_PI = glm::translate(Modelo_PI, glm::vec3(0, -pierna_y / 2, 0));
            Modelo_PI = glm::rotate(Modelo_PI, pie_up_down_izq, glm::vec3(1, 0, 0));
            Modelo_PI = glm::rotate(Modelo_PI, pie_left_rigt_izq, glm::vec3(0, 1, 0));
            Modelo_PI = glm::translate(Modelo_PI, glm::vec3(0, -pie_y / 2, (pie_z - pierna_z) / 2));

            PI.push(Modelo_PI);

            Modelo_PI = glm::scale(Modelo_PI, glm::vec3(pie_x, pie_y, pie_z));

            glm::mat4 mvp = mvp_base * Modelo_PI;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }

        //////// Pierna Derecha ////////
        // Modelo Pierna Derecha 1
        {
            Modelo_PD = PD.top();
            Modelo_PD = glm::translate(Modelo_PD, glm::vec3(pierna_x / 2  -cadera_x / 2, -cadera_y / 2, 0));
            Modelo_PD = glm::rotate(Modelo_PD, cadera_back_front_der, glm::vec3(1, 0, 0));
            Modelo_PD = glm::rotate(Modelo_PD, cadera_spin_der, glm::vec3(0, 1, 0));
            Modelo_PD = glm::rotate(Modelo_PD, cadera_left_right_der, glm::vec3(0, 0, 1));
            Modelo_PD = glm::translate(Modelo_PD, glm::vec3(0, -pierna_y / 2, 0));

            PD.push(Modelo_PD);

            Modelo_PD = glm::scale(Modelo_PD, glm::vec3(pierna_x, pierna_y, pierna_z));

            glm::mat4 mvp = mvp_base * Modelo_PD;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }
        // Modelo Pierna Derecha 2
        {
            Modelo_PD = PD.top();
            Modelo_PD = glm::translate(Modelo_PD, glm::vec3(0, -pierna_y / 2, 0));
            Modelo_PD = glm::rotate(Modelo_PD, rodilla_der, glm::vec3(1, 0, 0));
            Modelo_PD = glm::translate(Modelo_PD, glm::vec3(0, -pierna_y / 2, 0));

            PD.push(Modelo_PD);

            Modelo_PD = glm::scale(Modelo_PD, glm::vec3(pierna_x, pierna_y, pierna_z));

            glm::mat4 mvp = mvp_base * Modelo_PD;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }
        // Modelo Pierna Derecha 3
        {
            Modelo_PD = PD.top();
            Modelo_PD = glm::translate(Modelo_PD, glm::vec3(0, -pierna_y / 2, 0));
            Modelo_PD = glm::rotate(Modelo_PD, pie_up_down_der, glm::vec3(1, 0, 0));
            Modelo_PD = glm::rotate(Modelo_PD, pie_left_rigt_der, glm::vec3(0, 1, 0));
            Modelo_PD = glm::translate(Modelo_PD, glm::vec3(0, -pie_y / 2, (pie_z - pierna_z) / 2));

            PD.push(Modelo_PD);

            Modelo_PD = glm::scale(Modelo_PD, glm::vec3(pie_x, pie_y, pie_z));

            glm::mat4 mvp = mvp_base * Modelo_PD;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }

        //////// Cabeza ////////
        // Modelo Cuello
        {
            Modelo_C = glm::translate(Modelo_C, glm::vec3(0, (torso_y / 2) + (hombros_y) + (cuello_y / 2), 0));
            C.push(Modelo_C);

            Modelo_C = glm::scale(Modelo_C, glm::vec3(cuello_x, cuello_y, cuello_z));

            glm::mat4 mvp = mvp_base * Modelo_C;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }
        // Modelo Cabeza
        {
            Modelo_C = C.top();
            Modelo_C = glm::translate(Modelo_C, glm::vec3(0, cuello_y / 2, 0));
            Modelo_C = glm::rotate(Modelo_C, cuello_up_down, glm::vec3(1, 0, 0));
            Modelo_C = glm::rotate(Modelo_C, cuello_left_right, glm::vec3(0, 1, 0));
            Modelo_C = glm::rotate(Modelo_C, cuello_inclinacion, glm::vec3(0, 0, 1));
            Modelo_C = glm::translate(Modelo_C, glm::vec3(0, cabeza_y / 2, 0));

            C.push(Modelo_C);

            Modelo_C = glm::scale(Modelo_C, glm::vec3(cabeza_x, cabeza_y, cabeza_z));

            glm::mat4 mvp = mvp_base * Modelo_C;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }
        // Modelo Nariz
        {
            Modelo_C = C.top();
            Modelo_C = glm::translate(Modelo_C, glm::vec3(0, 0, cabeza_z / 2 + nariz_z / 2));
            
            C.push(Modelo_C);

            Modelo_C = glm::scale(Modelo_C, glm::vec3(nariz_x, nariz_y, nariz_z));

            glm::mat4 mvp = mvp_base * Modelo_C;
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
            glUseProgram(shader_0);
            glDrawElements(GL_TRIANGLES, 6 * 2 * 3, GL_UNSIGNED_INT, nullptr);
        }

        // Datos de la ventana de ImGui
        {
            ImGui::Begin("Parametros del Puppet");
            ////////// Torso ////////////
            ImGui::Text("Parametros de la rotacion del torso");
            ImGui::SliderFloat("Torso: Rotation x", &torso_r_x, 0, 2 * pi);
            ImGui::SliderFloat("Torso: Rotation y", &torso_r_y, 0, 2 * pi);
            ImGui::SliderFloat("Torso: Rotation z", &torso_r_z, 0, 2 * pi);
            ImGui::Text("Parametros de la traslacion del torso");
            ImGui::SliderFloat("Torso: Traslacion x", &torso_t_x, -10, 10);
            ImGui::SliderFloat("Torso: Traslacion y", &torso_t_y, -10, 10);
            ImGui::SliderFloat("Torso: Traslacion z", &torso_t_z, -10, 10);

            ////////// Brazo Izquierdo ////////////
            ImGui::Text("Parametros del hombro izquierdo");
            ImGui::SliderFloat("Hombro I: Rotation x", &hombro_spin_izq, -pi/2, pi/2);
            ImGui::SliderFloat("Hombro I: Rotation y", &hombro_front_back_izq, -5*pi/6, 0);
            ImGui::SliderFloat("Hombro I: Rotation z", &hombro_up_down_izq, -pi/2, pi/2);
            ImGui::Text("Parametros del codo izquierdo");
            ImGui::SliderFloat("Codo I", &codo_abrir_cerrar_izq, -5*pi / 6, 0);
            ImGui::Text("Parametros de la mano izquierda");
            ImGui::SliderFloat("Mano I: Rotation y", &mano_front_back_izq, -pi / 4, pi / 4);
            ImGui::SliderFloat("Mano I: Rotation z", &mano_up_down_izq, -pi / 2, pi / 2);

            ////////// Brazo Derecho ////////////
            ImGui::Text("Parametros del hombro derecho");
            ImGui::SliderFloat("Hombro D: Rotation x", &hombro_spin_der, -pi / 2, pi / 2);
            ImGui::SliderFloat("Hombro D: Rotation y", &hombro_front_back_der, 0, 5 * pi / 6);
            ImGui::SliderFloat("Hombro D: Rotation z", &hombro_up_down_der, -pi / 2, pi / 2);
            ImGui::Text("Parametros del codo derecho");
            ImGui::SliderFloat("Codo D", &codo_abrir_cerrar_der, 0, 5 * pi / 6);
            ImGui::Text("Parametros de la mano derecha");
            ImGui::SliderFloat("Mano D: Rotation y", &mano_front_back_der, -pi / 4, pi / 4);
            ImGui::SliderFloat("Mano D: Rotation z", &mano_up_down_der, -pi / 2, pi / 2);
            
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ////////// Pierna Izquierda ////////////
            ImGui::Text("Parametros de la cadera izquierda");
            ImGui::SliderFloat("Cadera I: Rotation x", &cadera_back_front_izq, -pi / 4, pi / 4);
            ImGui::SliderFloat("Cadera I: Rotation y", &cadera_spin_izq, 0, pi / 2);
            ImGui::SliderFloat("Cadera I: Rotation z", &cadera_left_right_izq, 0, pi / 4);
            ImGui::Text("Parametros de la rodilla izquierda");
            ImGui::SliderFloat("Rodilla I", &rodilla_izq, 0, pi / 2);
            ImGui::Text("Parametros del pie izquierdo");
            ImGui::SliderFloat("Pie I: Rotation x", &pie_up_down_izq, -pi / 5, pi / 4);
            ImGui::SliderFloat("Pie I: Rotation y", &pie_left_rigt_izq, 0, pi/4);


            ////////// Pierna Derecha ////////////
            ImGui::Text("Parametros de la cadera derecha");
            ImGui::SliderFloat("Cadera D: Rotation x", &cadera_back_front_der, -pi / 4, pi / 4);
            ImGui::SliderFloat("Cadera D: Rotation y", &cadera_spin_der, -pi / 2, 0);
            ImGui::SliderFloat("Cadera D: Rotation z", &cadera_left_right_der, -pi / 4, 0);
            ImGui::Text("Parametros de la rodilla derecha");
            ImGui::SliderFloat("Rodilla D", &rodilla_der, 0, pi / 2);
            ImGui::Text("Parametros del pie derecho");
            ImGui::SliderFloat("Pie D: Rotation x", &pie_up_down_der,  -pi / 5, pi / 4);
            ImGui::SliderFloat("Pie D: Rotation y", &pie_left_rigt_der, -pi/4, 0);


            ////////// Cuello ////////////
            ImGui::Text("Parametros del cuello");
            ImGui::SliderFloat("Cuello: Rotation x", &cuello_up_down, -pi / 4, pi / 4);
            ImGui::SliderFloat("Cuello: Rotation y", &cuello_left_right, -pi / 2, pi / 2);
            ImGui::SliderFloat("Cuello: Rotation z", &cuello_inclinacion, -pi / 4, pi / 4);

            if (ImGui::Button("Reset")) {

                hombro_up_down_izq = 0; // [-pi/2, pi/2]
                hombro_front_back_izq = 0; // [-5*pi/6, 0]
                hombro_spin_izq = 0; // [-pi/2, pi/2]
           
                hombro_up_down_der = 0; // [-pi/2, pi/2]
                hombro_front_back_der = 0; // [0, 5*pi/6]
                hombro_spin_der = 0; // [-pi/2, pi/2] 
  
                codo_abrir_cerrar_izq = 0; // [-5*pi/6,0]

                codo_abrir_cerrar_der = 0; // [0, 5*pi/6]
 
                mano_front_back_izq = 0; // [-pi/4, pi/4]
                mano_up_down_izq = 0; // [-pi/2, pi/2]

                mano_front_back_der = 0; // [-pi/4, pi/4]
                mano_up_down_der = 0; // [-pi/2, pi/2]

                cadera_left_right_izq = 0; // [0, pi/4]
                cadera_spin_izq = 0; // [0, pi/2]
                cadera_back_front_izq = 0; // [-pi/4, pi/4]

                cadera_left_right_der = 0; // [-pi/4, 0]
                cadera_spin_der = 0; // [-pi/2, 0]
                cadera_back_front_der = 0; // [-pi/4, pi/4]

                rodilla_izq = 0; // [0, pi/2]

                rodilla_der = 0; // [0, pi/2]

                pie_left_rigt_izq = 0; // [0,pi/4]
                pie_up_down_izq = 0; // [-pi/5, pi/4]

                pie_left_rigt_der = 0; // [-pi/4,0]
                pie_up_down_der = 0; // [-pi/5, pi/4]

                cuello_up_down = 0; // [-pi/4, pi/4]
                cuello_left_right = 0; // [-pi/2, pi/2]
                cuello_inclinacion = 0; // [-pi/4, pi/4]
            }

            ImGui::End();
        }
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

        /* Intercambiamos buffers frontales y traseros */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }


    glDeleteProgram(shader_0);

    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();// Terminamos glfw
    return 0;
}