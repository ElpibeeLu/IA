#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath> 
#include <algorithm>
#define DEG2RAD (3.14159265358979323846 / 180.0)
int profundidad = 5;
int N; 
std::vector<std::vector<int>> TableroState; 
int currentPlayer = 1; // 1 para círculo, 2 para X 

std::pair<int, int> Busque_mejorMovi(std::vector<std::vector<int>>& Tablero, int player);
int minimax_alfa_beta(std::vector<std::vector<int>> Tablero, int profundidad, int alfa, int beta, bool esMaximizando);

bool verificarVictoria(int jugador) {
    // filas
    for (int i = 0; i < N; ++i) {
        bool filaCompleta = true;
        for (int j = 0; j < N; ++j) {
            if (TableroState[i][j] != jugador) {
                filaCompleta = false;
                break;
            }
        }
        if (filaCompleta) return true;
    }

    // columnas
    for (int j = 0; j < N; ++j) {
        bool columnaCompleta = true;
        for (int i = 0; i < N; ++i) {
            if (TableroState[i][j] != jugador) {
                columnaCompleta = false;
                break;
            }
        }
        if (columnaCompleta) return true;
    }

    // diagonales
    bool diagonalCompleta = true;
    for (int i = 0; i < N; ++i) {
        if (TableroState[i][i] != jugador) {
            diagonalCompleta = false;
            break;
        }
    }
    if (diagonalCompleta) return true;

    diagonalCompleta = true;
    for (int i = 0; i < N; ++i) {
        if (TableroState[i][N - 1 - i] != jugador) {
            diagonalCompleta = false;
            break;
        }
    }
    if (diagonalCompleta) return true;

    return false;
}

std::vector<std::pair<int, int>> Movimientos_Posibles(const std::vector<std::vector<int>>& Tablero) {
    std::vector<std::pair<int, int>> moves;
    for (int i = 0; i < Tablero.size(); ++i) {
        for (int j = 0; j < Tablero[i].size(); ++j) {
            if (Tablero[i][j] == 0) {
                moves.emplace_back(i, j);
            }
        }
    }
    return moves;
}

bool gameOver(const std::vector<std::vector<int>>& Tablero) {
    // vericamos si hay ganador
    if (verificarVictoria(1) || verificarVictoria(2)) {
        return true;
    }
    // verificamos si hay algun espacio libre
    for (const auto& row : Tablero) {
        for (int cell : row) {
            if (cell == 0) {
                return false; // si hay espacio le juego no termina
            }
        }
    }
    return true; // empate
}

std::vector<std::vector<int>> haz_movimiento(std::vector<std::vector<int>> Tablero, const std::pair<int, int>& move, int player) {
    if (Tablero[move.first][move.second] == 0) { 
        Tablero[move.first][move.second] = player; 
    }
    return Tablero;
}

void Movimiento_IA() {
    if (!gameOver(TableroState) && currentPlayer == 2) { // aseguramos de que es el turno de la IA
        auto move = Busque_mejorMovi(TableroState, currentPlayer); 
        TableroState = haz_movimiento(TableroState, move, currentPlayer);
        
        // verificamos el estado del juego
        if (verificarVictoria(currentPlayer)) {
            std::cout << "La IA gana!" << std::endl;
        } else if (gameOver(TableroState)) {
            std::cout << "Empate!" << std::endl;
        }
        
        currentPlayer = 1; // cambiamos el turno al jugador humano
    }
}

std::pair<int, int> Busque_mejorMovi(std::vector<std::vector<int>>& Tablero, int player) {
    int bestScore = std::numeric_limits<int>::min();
    std::pair<int, int> bestMove = {-1, -1};
    
    int alfa = std::numeric_limits<int>::min();
    int beta = std::numeric_limits<int>::max();
    
    auto possibleMoves = Movimientos_Posibles(Tablero);
    for (const auto& move : possibleMoves) {
        auto tempTablero = haz_movimiento(Tablero, move, player);
        int score = minimax_alfa_beta(tempTablero, profundidad, alfa, beta, false);
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }
    return bestMove;
}

int evaluarLinea(const std::vector<int>& linea, int player) {
    int score = 0;
    int opponent = player == 1 ? 2 : 1;
    int countPlayer = std::count(linea.begin(), linea.end(), player);
    int countOpponent = std::count(linea.begin(), linea.end(), opponent);

    if (countPlayer > 0 && countOpponent == 0) {
        // La línea aún es ganable para el jugador.
        score += std::pow(10, countPlayer); // Pondera más fuertemente las líneas con más fichas del jugador.
    } else if (countPlayer == 0 && countOpponent > 0) {
        // La línea podría ser bloqueada o ganada por el oponente.
        score -= std::pow(10, countOpponent);
    }

    return score;
}

int aptitud(const std::vector<std::vector<int>>& Tablero, int player) {
    int score = 0;

    // evaluamos filas y columnas
    for (int i = 0; i < N; ++i) {
        std::vector<int> fila(N);
        std::vector<int> columna(N);
        for (int j = 0; j < N; ++j) {
            fila[j] = Tablero[i][j];
            columna[j] = Tablero[j][i];
        }
        score += evaluarLinea(fila, player);
        score += evaluarLinea(columna, player);
    }

    // evaluamos diagonales
    std::vector<int> diagonalPrincipal(N), diagonalSecundaria(N);
    for (int i = 0; i < N; ++i) {
        diagonalPrincipal[i] = Tablero[i][i];
        diagonalSecundaria[i] = Tablero[i][N - 1 - i];
    }
    score += evaluarLinea(diagonalPrincipal, player);
    score += evaluarLinea(diagonalSecundaria, player);

    return score;
}

int minimax_alfa_beta(std::vector<std::vector<int>> Tablero, int profundidad, int alfa, int beta, bool esMaximizando) {
    
    if (profundidad == 0 || gameOver(Tablero)) {
        return aptitud(Tablero, currentPlayer);
    }

    if (esMaximizando) {
        
        int maxEval = std::numeric_limits<int>::min();
        for (auto& move : Movimientos_Posibles(Tablero)) {
            std::vector<std::vector<int>> newTablero = haz_movimiento(Tablero, move, 1); 
            int eval = minimax_alfa_beta(newTablero, profundidad - 1, alfa, beta, false); 
            maxEval = std::max(maxEval, eval);
            alfa = std::max(alfa, eval);
            if (beta <= alfa)
                break; // Poda beta
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        for (auto& move : Movimientos_Posibles(Tablero)) {
            std::vector<std::vector<int>> newTablero = haz_movimiento(Tablero, move, 2); 
            int eval = minimax_alfa_beta(newTablero, profundidad - 1, alfa, beta, true); 
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alfa)
                break; // Poda alfa
        }
        return minEval;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    static bool moveMade = false; // variable pa rastrear si se ha hecho un movimeinto en eese turno

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !moveMade) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        int gridX = static_cast<int>(N * xpos / width);
        int gridY = static_cast<int>(N * ypos / height); 
        if (gridX >= 0 && gridX < N && gridY >= 0 && gridY < N && TableroState[gridY][gridX] == 0) {
            TableroState[gridY][gridX] = currentPlayer; 

            // verificamos si hay una victoria despues del movimiento actual
            if (verificarVictoria(currentPlayer)) {
                std::cout << "Jugador " << currentPlayer << " gana!" << std::endl;
            }

            // verificmaos empate
            bool empate = true;
            for (const auto& fila : TableroState) {
                for (int celda : fila) {
                    if (celda == 0) {
                        empate = false;
                        break;
                    }
                }
                if (!empate) break;
            }
            if (empate) {
                std::cout << "Empate!" << std::endl;
            }

            currentPlayer = (currentPlayer % 2) + 1; // cambiamos el jugador actual
            moveMade = true; // marcamos si se realizo turno 
        }
    }

    if (!gameOver(TableroState) && currentPlayer == 2) {
        Movimiento_IA(); 
        currentPlayer = 1; 
        moveMade = false; 
    }

}

void drawCircle(float x, float y, float radius) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i++) {
        float degInRad = i * DEG2RAD;
        glVertex2f(x + cos(degInRad) * radius, y + sin(degInRad) * radius);
    }
    glEnd();
}

void drawX(float x, float y, float size) {
    glBegin(GL_LINES);
    glVertex2f(x - size, y - size);
    glVertex2f(x + size, y + size);
    glVertex2f(x - size, y + size);
    glVertex2f(x + size, y - size);
    glEnd();
}

void drawTablero() {
 
    float cellSize = 1.8f / N;
    float margin = (2.0f - 1.8f) / 2.0f;
    float radius = cellSize / 4; 
    float halfSize = cellSize / 4; 
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            float x = -1.0f + margin + i * cellSize + cellSize / 2;
            float y = 1.0f - margin - j * cellSize - cellSize / 2;
            if (TableroState[j][i] == 1) {
                drawCircle(x, y, radius);
            } else if (TableroState[j][i] == 2) {
                drawX(x, y, halfSize);
            }
        }
    }
}

int fun_aptitud(const std::vector<std::vector<int>>& Tablero, int player) {
    int Jugador_X = 0, Jugador_O = 0;
    int opponent = player == 1 ? 2 : 1;

    // verificamos filas y columnas
    for (int i = 0; i < N; ++i) {
        bool Jugador_filas_X = true, Jugador_filas_O = true;
        bool Jugador_columnas_X = true, Jugador_columnas_O = true;

        for (int j = 0; j < N; ++j) {
            // filas
            if (Tablero[i][j] == opponent) Jugador_filas_X = false;
            if (Tablero[i][j] == player) Jugador_filas_O = false;
            
            // columnas
            if (Tablero[j][i] == opponent) Jugador_columnas_X = false;
            if (Tablero[j][i] == player) Jugador_columnas_O = false;
        }

        Jugador_X += Jugador_filas_X + Jugador_columnas_X;
        Jugador_O += Jugador_filas_O + Jugador_columnas_O;
    }

    // verificamos diagonales
    bool Jugador_diagonales_X = true, Jugador_diagonales_O = true;
    bool Jugador_diagonales_X_1 = true, Jugador_diagonales_O_1 = true;

    for (int i = 0; i < N; ++i) {
        if (Tablero[i][i] == opponent) Jugador_diagonales_X = false;
        if (Tablero[i][i] == player) Jugador_diagonales_O = false;
        
        if (Tablero[i][N - 1 - i] == opponent) Jugador_diagonales_X_1 = false;
        if (Tablero[i][N - 1 - i] == player) Jugador_diagonales_O_1 = false;
    }

    Jugador_X += Jugador_diagonales_X + Jugador_diagonales_X_1;
    Jugador_O += Jugador_diagonales_O + Jugador_diagonales_O_1;

    return Jugador_X - Jugador_O;
}

int main() {
    std::cout << "Ingrese el tamano de N para el juego N en Raya: ";
    std::cin >> N; 
    TableroState = std::vector<std::vector<int>>(N, std::vector<int>(N, 0));



    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(640, 480, "N en Raya Mejorado", NULL, NULL);
    if (!window) {
        glfwTerminate();
        std::cerr << "Failed to create the window\n";
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window, mouseButtonCallback); 


    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        glColor3f(0.0f, 0.0f, 0.0f);
        glLineWidth(2.0f); 

        float cellSize = 1.8f / N; 
        float margin = (2.0f - 1.8f) / 2.0f; 

     
        glBegin(GL_LINES);
        for (int i = 1; i < N; ++i) {
            float linePos = -1.0f + margin + i * cellSize;
            
            // lina vertical
            glVertex2f(linePos, -1.0f + margin);
            glVertex2f(linePos, 1.0f - margin);
            // linea horizontal 
            glVertex2f(-1.0f + margin, linePos);
            glVertex2f(1.0f - margin, linePos);
        }
        glEnd();

        drawTablero(); 

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
