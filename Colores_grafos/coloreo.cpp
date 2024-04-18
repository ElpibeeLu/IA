#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <chrono>
#include <thread>

struct Nodo {
    std::string color;
    std::vector<std::string> adyacentes;
};

typedef std::map<std::string, Nodo> Grafo;

void inicializarGrafo1(Grafo& grafo) {
    grafo["A"] = {"", {"D", "E", "B"}};
    grafo["B"] = {"", {"A", "C"}};
    grafo["C"] = {"", {"D", "E", "B"}};
    grafo["D"] = {"", {"A", "C", "E"}};
    grafo["E"] = {"", {"A", "D", "C"}};
}  

void inicializarGrafo2(Grafo& grafo) {
    grafo["A"] = {"", {"B", "D"}};
    grafo["B"] = {"", {"A", "G", "C"}};
    grafo["C"] = {"", {"B", "D", "F"}};
    grafo["D"] = {"", {"A", "C", "F", "E"}};
    grafo["E"] = {"", {"D", "F", "G"}};
    grafo["F"] = {"", {"D", "C", "E", "G"}};
    grafo["G"] = {"", {"F", "B", "E"}};
}

void dibujarNodo(float x, float y, const std::string& nombre, const std::string& color) {
    int num_segments = 20;
    float radius = 0.05;

    glColor3f(1.0, 1.0, 1.0);
    if (color == "rojo") glColor3f(1.0, 0.0, 0.0);
    else if (color == "verde") glColor3f(0.0, 1.0, 0.0);
    else if (color == "amarillo") glColor3f(1.0, 1.0, 0.0);

    glBegin(GL_POLYGON);
    for (int i = 0; i < num_segments; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(num_segments);
        float cx = radius * cosf(theta);
        float cy = radius * sinf(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

void dibujarAristas(const Grafo& grafo, std::map<std::string, std::pair<float, float>>& posiciones) {
    glColor3f(0.5, 0.5, 0.5);
    for (const auto& nodo : grafo) {
        auto origen = posiciones[nodo.first];
        for (const auto& vecino : nodo.second.adyacentes) {
            auto destino = posiciones[vecino];
            glBegin(GL_LINES);
            glVertex2f(origen.first, origen.second);
            glVertex2f(destino.first, destino.second);
            glEnd();
        }
    }
}

void dibujarYEsperar(const Grafo& grafo, GLFWwindow* window, std::map<std::string, std::pair<float, float>>& posiciones, int delay) {
    glClear(GL_COLOR_BUFFER_BIT);

    for (const auto& nodo : grafo) {
        auto pos = posiciones[nodo.first];
        dibujarNodo(pos.first, pos.second, nodo.first, nodo.second.color);
    }

    dibujarAristas(grafo, posiciones);

    glfwSwapBuffers(window);
    glfwPollEvents();
    std::this_thread::sleep_for(std::chrono::seconds(delay));
}

bool esValido(const Grafo &grafo, const std::string &nodo, const std::string &color) {
    for (const auto &vecino : grafo.at(nodo).adyacentes) {
        if (grafo.at(vecino).color == color) {
            return false;
        }
    }
    return true;
}

std::string seleccionarSiguienteNodo(Grafo &grafo, int heuristica) {
    std::string nodoSeleccionado;
    int minColoresPosibles = std::numeric_limits<int>::max();
    int maxGrado = -1;

    for (auto& nodo : grafo) {
        if (!nodo.second.color.empty()) continue;
        std::vector<std::string> coloresPosibles = {"rojo", "verde", "amarillo"};
        for (const auto& vecino : nodo.second.adyacentes) {
            coloresPosibles.erase(
                std::remove(coloresPosibles.begin(), coloresPosibles.end(), grafo[vecino].color),
                coloresPosibles.end());
        }

        int grado = nodo.second.adyacentes.size();
        if (heuristica == 1 && coloresPosibles.size() < minColoresPosibles) {
            minColoresPosibles = coloresPosibles.size();
            nodoSeleccionado = nodo.first;
        }
        if (heuristica == 2 && grado > maxGrado) {
            maxGrado = grado;
            nodoSeleccionado = nodo.first;
        }
    }

    return nodoSeleccionado;
}

bool todosColoreados(const Grafo &grafo) {
    for (const auto &nodo : grafo) {
        if (nodo.second.color.empty()) {
            return false;
        }
    }
    return true;
}

bool colorearNodo(Grafo& grafo, const std::string& nodo, const std::vector<std::string>& colores, int heuristica, GLFWwindow* window, std::map<std::string, std::pair<float, float>>& posiciones, int delay) {
    if (nodo.empty()) {
        return todosColoreados(grafo);
    }

    for (const auto& color : colores) {
        if (esValido(grafo, nodo, color)) {
            grafo[nodo].color = color;
            dibujarYEsperar(grafo, window, posiciones, delay);
            std::string siguienteNodo = seleccionarSiguienteNodo(grafo, heuristica);
            if (colorearNodo(grafo, siguienteNodo, colores, heuristica, window, posiciones, delay)) {
                return true;
            }
            grafo[nodo].color = ""; // backtrack
        }
    }
    return false;
}

int main() {
    std::cout << "Seleccione el grafo a visualizar (1 o 2): ";
    int opcion;
    std::cin >> opcion;

    std::cout << "Seleccione la heuristica 1 para mas restringida y 2 para mas restrictiva: ";
    int heuristica;
    std::cin >> heuristica;

    std::vector<std::string> colores = {"rojo", "verde", "amarillo"};

    Grafo grafo;
    if (opcion == 1) {
        inicializarGrafo1(grafo);
    } else {
        inicializarGrafo2(grafo);
    }

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(640, 480, "Coloración de Grafos", NULL, NULL);
    if (!window) {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window." << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);

    std::map<std::string, std::pair<float, float>> posiciones;
    if (opcion == 1) {
        posiciones = {
            {"A", {-0.5, 0.5}},
            {"B", {-0.5, -0.5}},
            {"C", {0.5, -0.5}},
            {"D", {0.5, 0.5}},
            {"E", {0.9, 0.0}}
        };
    } else {
        posiciones = {
            {"A", {-0.9, 0.5}},
            {"B", {-0.9, -0.5}},
            {"C", {-0.4, -0.5}},
            {"D", {-0.4, 0.5}},
            {"E", {0.1, 0.5}},
            {"F", {0.1, -0.5}},
            {"G", {0.55, 0.0}}
        };
    }

    std::string initialNode = seleccionarSiguienteNodo(grafo, heuristica);
    if (!colorearNodo(grafo, initialNode, colores, heuristica, window, posiciones, 4)) {
        std::cerr << "No se encontró una solución de coloración válida." << std::endl;
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
