#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <limits>
#include <cmath>

int N; 
float porcentajeEliminar; 
std::vector<std::vector<bool>> nodosActivos;
std::pair<int, int> nodoInicial;    
std::pair<int, int> nodoFinal;
std::vector<std::pair<int, int>> movimientos = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
std::vector<std::vector<bool>> enCamino; 
int algoritmoSeleccionado;


void ejecutarBusqueda();

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // convierte las coordenadas del raton a coordenadas de la cuadricula
        int gridX = static_cast<int>(N * xpos / width);
        int gridY = N - 1 - static_cast<int>(N * ypos / height); // invertir porque GLFW tiene origen en la esquina superior izquierda

        static bool seleccionandoInicial = true;
        if (seleccionandoInicial) {
            nodoInicial = {gridX, gridY};
            std::cout << "Nodo inicial seleccionado en: " << gridX << ", " << gridY << std::endl;
        } else {
            nodoFinal = {gridX, gridY};
            std::cout << "Nodo final seleccionado en: " << gridX << ", " << gridY << std::endl;
            ejecutarBusqueda(); 
        }
        seleccionandoInicial = !seleccionandoInicial;
    }
}

void inicializarNodosActivos() {
    nodosActivos.resize(N, std::vector<bool>(N, true));
    int nodosAEliminar = static_cast<int>(N * N * (porcentajeEliminar / 100.0));

    srand(static_cast<unsigned int>(time(nullptr)));

    while (nodosAEliminar > 0) {
        int i = rand() % N;
        int j = rand() % N;
        // Evita eliminar el nodo inicial y el nodo final
        if ((i == nodoInicial.first && j == nodoInicial.second) || 
            (i == nodoFinal.first && j == nodoFinal.second)) {
            continue;
        }
        if (nodosActivos[i][j]) {
            nodosActivos[i][j] = false;
            --nodosAEliminar;
        }
    }
}

void dibujarGrafo() {
    glClear(GL_COLOR_BUFFER_BIT); // limpia la pantalla 
    
    glColor3f(0.0f, 0.0f, 0.0f); // Color negro para las aristas

    const float ESPACIO_ENTRE_NODOS = 2.0f / N;
    const float OFFSET = ESPACIO_ENTRE_NODOS / 2.0f - 1.0f;

    // dibuja las aristas de nodos activos
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (!nodosActivos[i][j]) continue;  // omitimos nodos inactivo

            float x = i * ESPACIO_ENTRE_NODOS + OFFSET;
            float y = j * ESPACIO_ENTRE_NODOS + OFFSET;

            // dibujamos aristas arriba ya bajo de nodos acctivos 
            if (i < N - 1 && nodosActivos[i + 1][j]) {
                glBegin(GL_LINES);
                glVertex2f(x, y);
                glVertex2f(x + ESPACIO_ENTRE_NODOS, y);
                glEnd();
            }
            if (j < N - 1 && nodosActivos[i][j + 1]) {
                glBegin(GL_LINES);
                glVertex2f(x, y);
                glVertex2f(x, y + ESPACIO_ENTRE_NODOS);
                glEnd();
            }
        }
    }

    // dibuja los nodos
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            float x = i * ESPACIO_ENTRE_NODOS + OFFSET;
            float y = j * ESPACIO_ENTRE_NODOS + OFFSET;

            glBegin(GL_POINTS);
            if (!nodosActivos[i][j]) continue; 

            // 
            if (i == nodoInicial.first && j == nodoInicial.second) {
                glColor3f(0.0f, 0.0f, 1.0f); //  azul para el nodo inicial
            } else if (i == nodoFinal.first && j == nodoFinal.second) {
                glColor3f(0.0f, 0.0f, 1.0f); // azul para el nodo final
            } else if (enCamino[i][j]) {
                glColor3f(1.0f, 0.0f, 0.0f); // rojo para los nodos ruta finalcl
            } else {
                glColor3f(0.0f, 0.0f, 0.0f); // negro pa los demas nodos 
            }
            glVertex2f(x, y);
            glEnd();
        }
    }

    for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
        if (!nodosActivos[i][j]) continue;
        
        float x = i * ESPACIO_ENTRE_NODOS + OFFSET;
        float y = j * ESPACIO_ENTRE_NODOS + OFFSET;

        // dibuja aristas diagonales
        if (i < N - 1 && j < N - 1 && nodosActivos[i + 1][j + 1]) {
            glBegin(GL_LINES);
            glVertex2f(x, y);
            glVertex2f(x + ESPACIO_ENTRE_NODOS, y + ESPACIO_ENTRE_NODOS);
            glEnd();
        }
        if (i < N - 1 && j > 0 && nodosActivos[i + 1][j - 1]) {
            glBegin(GL_LINES);
            glVertex2f(x, y);
            glVertex2f(x + ESPACIO_ENTRE_NODOS, y - ESPACIO_ENTRE_NODOS);
            glEnd();
        }
    }
}

}

bool DFS(std::pair<int, int> nodo, std::vector<std::vector<bool>>& visitado) {
    if (nodo == nodoFinal) {
        enCamino[nodo.first][nodo.second] = true;
        return true;
    }

    if (nodo.first < 0 || nodo.first >= N || nodo.second < 0 || nodo.second >= N || 
        !nodosActivos[nodo.first][nodo.second] || visitado[nodo.first][nodo.second]) {
        return false;
    }

    
    visitado[nodo.first][nodo.second] = true;

    // iteramos sobre todos los posibles movimientos desde el nodo actual 
    for (const auto& mov : movimientos) {
        std::pair<int, int> siguienteNodo = {nodo.first + mov.first, nodo.second + mov.second};

        // si dio true, encontro un camino 
        if (DFS(siguienteNodo, visitado)) {
            enCamino[nodo.first][nodo.second] = true;
            return true;
        }
    }

    // si la recursividad no dio true, no hay camino hacia nodo final 
    return false;
}


bool BFS(const std::pair<int, int>& nodoInicial, std::vector<std::vector<bool>>& nodosActivos) {
    std::vector<std::vector<bool>> visitado(N, std::vector<bool>(N, false));
    std::queue<std::pair<int, int>> cola;
    cola.push(nodoInicial);
    visitado[nodoInicial.first][nodoInicial.second] = true;

    std::vector<std::vector<std::pair<int, int>>> predecesor(N, std::vector<std::pair<int, int>>(N, {-1, -1}));
    bool encontrado = false;

    while (!cola.empty() && !encontrado) {
        std::pair<int, int> nodo = cola.front();
        cola.pop();

        if (nodo == nodoFinal) {
            encontrado = true;
            break;
        }

        for (const auto& mov : movimientos) {
            int ni = nodo.first + mov.first, nj = nodo.second + mov.second;
            if (ni >= 0 && ni < N && nj >= 0 && nj < N && nodosActivos[ni][nj] && !visitado[ni][nj]) {
                visitado[ni][nj] = true;
                cola.push({ni, nj});
                predecesor[ni][nj] = nodo;
            }
        }
    }

    if (encontrado) {
        for (std::pair<int, int> at = nodoFinal; at != nodoInicial; at = predecesor[at.first][at.second]) {
            enCamino[at.first][at.second] = true;
        }
        enCamino[nodoInicial.first][nodoInicial.second] = true;
    }

    return encontrado;
}


int heuristica(std::pair<int, int> nodo) {
    return std::abs(nodo.first - nodoFinal.first) + std::abs(nodo.second - nodoFinal.second);
}

bool AStar() {
    // representa el estado de cada nodo con su costo g, costo f y si ha sido visitado
    struct NodoEstado {
        int gCost = INT_MAX; // costo desde el inicio hasta este nodo
        int fCost = INT_MAX; // costo total estimado desde el inicio hasta el final pasando por este nodo
        bool visitado = false; 
        std::pair<int, int> predecesor = {-1, -1}; // para reconstruir el camino
    };

    // inicialización de los estados de todos los nodos
    std::vector<std::vector<NodoEstado>> estados(N, std::vector<NodoEstado>(N));

    // inicia el nodo inicial con gCost 0 y calcula su fCost
    estados[nodoInicial.first][nodoInicial.second].gCost = 0;
    estados[nodoInicial.first][nodoInicial.second].fCost = heuristica(nodoInicial);

    while (true) {
        // Encuentra el nodo no visitado con el menor fCost
        std::pair<int, int> nodoActual = {-1, -1};
        int minFCost = INT_MAX;
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                if (!estados[i][j].visitado && estados[i][j].fCost < minFCost) {
                    minFCost = estados[i][j].fCost;
                    nodoActual = {i, j};
                }
            }
        }

        // si no encontramos tal nodo, el algoritmo falla y no hay camino
        if (nodoActual.first == -1) {
            return false;
        }

        // marca el nodo actual como visitado
        estados[nodoActual.first][nodoActual.second].visitado = true;

        // si el nodo actual es el nodo final, reconstruye y marca el camino
        if (nodoActual == nodoFinal) {
            while (nodoActual != nodoInicial) {
                enCamino[nodoActual.first][nodoActual.second] = true;
                nodoActual = estados[nodoActual.first][nodoActual.second].predecesor;
            }
            enCamino[nodoInicial.first][nodoInicial.second] = true;
            return true;
        }

        // explora los vecinos del nodo actual
        for (const auto& mov : movimientos) {
            std::pair<int, int> vecino = {nodoActual.first + mov.first, nodoActual.second + mov.second};

            // verificamos si el vecino es un nodo valido y no ha sido visitado
            if (vecino.first >= 0 && vecino.first < N && vecino.second >= 0 && vecino.second < N && 
                nodosActivos[vecino.first][vecino.second] && !estados[vecino.first][vecino.second].visitado) {

                // calcula el gCost para este vecino
                int tentativoGCost = estados[nodoActual.first][nodoActual.second].gCost; 
                bool esDiagonal = std::abs(mov.first) == 1 && std::abs(mov.second) == 1;

                if (esDiagonal)
                {
                    tentativoGCost += std::sqrt(2)*10;
                }
                else
                {
                    tentativoGCost += 10;
                }
                if (tentativoGCost < estados[vecino.first][vecino.second].gCost) {
                    // actualizamos el predecesor gCost y fCost del vecino
                    estados[vecino.first][vecino.second].predecesor = nodoActual;
                    estados[vecino.first][vecino.second].gCost = tentativoGCost;
                    estados[vecino.first][vecino.second].fCost = tentativoGCost + heuristica(vecino);
                }
            }
        }
    }

    return false;
}

int distancia_2(std::pair<int, int> nodo, std::pair<int, int> objetivo) {
    return std::abs(nodo.first - objetivo.first) + std::abs(nodo.second - objetivo.second);
}

bool HillClimbing(std::vector<std::vector<bool>>& nodosActivos, std::vector<std::vector<bool>>& enCamino) {
    // inicializa la posición actual como el nodo inicial
    std::pair<int, int> actual = nodoInicial;
    enCamino[actual.first][actual.second] = true; // marca el inicio en el camino

    // heuristica distancia hasta el nodo final
    auto distancia_2 = [](std::pair<int, int> a, std::pair<int, int> b) -> int {
        return std::abs(a.first - b.first) + std::abs(a.second - b.second);
    };

    // revisa si hemos alcanzado el objetivo
    while (actual != nodoFinal) {
        std::pair<int, int> siguiente = {-1, -1};
        int distanciaMinima = std::numeric_limits<int>::max();

        // explora todos los movimientos posibles desde el nodo actual
        for (auto& mov : movimientos) {
            std::pair<int, int> vecino = {actual.first + mov.first, actual.second + mov.second};

            // verificamos si el vecino es una opción acepptble
            if (vecino.first >= 0 && vecino.first < N && vecino.second >= 0 && vecino.second < N && 
                nodosActivos[vecino.first][vecino.second] && !enCamino[vecino.first][vecino.second]) {
                
                int distancia = distancia_2(vecino, nodoFinal);
                // selecciona el vecino si ta mas cerca del objetivo
                if (distancia < distanciaMinima) {
                    siguiente = vecino;
                    distanciaMinima = distancia;
                }
            }
        }

        // si no se encontr un vecino válida termina el loop
        if (siguiente.first == -1) {
            break;
        }

        // mueve al nodo seleccionado y marca en el camino
        actual = siguiente;
        enCamino[actual.first][actual.second] = true;
    }

    
    return actual == nodoFinal;
}

void ejecutarBusqueda() {
    enCamino = std::vector<std::vector<bool>>(N, std::vector<bool>(N, false)); // reinicia el camino
    std::vector<std::vector<bool>> visitado(N, std::vector<bool>(N, false));

    bool encontrado = false;
    switch (algoritmoSeleccionado) {
        case 1:
            encontrado = DFS(nodoInicial, visitado);
            break;
        case 2:
            encontrado = BFS(nodoInicial, nodosActivos);
            break;
        case 3:
            encontrado = AStar();
            break;
        case 4:
            encontrado = HillClimbing(nodosActivos, enCamino);
            break;
        default:
            std::cout << "Opcion no valida.\n";
            return;
    }

    if (!encontrado) {
        std::cout << "No se encontro un camino valido.\n";
    } else {
        std::cout << "Camino encontrado.\n";
    }
}

int main(void) {
    std::cout << "Ingrese el tamano de la cuadricula (N): ";
    std::cin >> N;
    std::cout << "Ingrese el porcentaje de nodos a eliminar: ";
    std::cin >> porcentajeEliminar;

    std::cout << "Seleccione el algoritmo de busqueda:\n";
    std::cout << "1. DFS\n2. BFS\n3. A*\n4. Hill Climbing\n";
    std::cin >> algoritmoSeleccionado;

    GLFWwindow* window;
    if (!glfwInit()) return -1;

    window = glfwCreateWindow(640, 480, "Grafo NxN con Nodos Eliminados", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glfwSetMouseButtonCallback(window, mouseButtonCallback); 
    glPointSize(10);
    inicializarNodosActivos();
    enCamino.resize(N, std::vector<bool>(N, false)); 

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);   

        dibujarGrafo();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
