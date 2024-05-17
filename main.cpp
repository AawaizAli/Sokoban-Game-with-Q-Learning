#include <iostream>
#include <vector>
#include <cstdlib> 
#include <ctime>   
#include <cmath>   
#include <algorithm> 

using namespace std;

// Function to print the current state of the board
void printBoardforplayer(const vector<vector<char>>& board, int playerX, int playerY) {
    for (int i = 0; i < board.size(); i++) {
        for (int j = 0; j < board[i].size(); j++) {
            if (i == playerX && j == playerY) {
                cout << 'P' << ' ';
            } else {
                cout << board[i][j] << ' ';
            }
        }
        cout << endl;
    }
}

// Function to check if the game is won
bool checkWin(const vector<vector<char>>& board) {
    for (int i = 1; i < board.size() - 1; i++) {
        for (int j = 1; j < board[i].size() - 1; j++) {
            // Check if there are targets not covered by boxes
            if (board[i][j] == 'T') return false;
        }
    }
    return true;
}

// Q-learning parameters
const double learningRate = 0.1;
const double discountFactor = 0.9;
const double explorationRate = 0.1;

// Function to update Q-values
void updateQValues(vector<vector<vector<double>>>& Q, int stateX, int stateY, char action, double reward, int nextStateX, int nextStateY) {
    int actionIndex;
    if (action == 'w') actionIndex = 0;
    else if (action == 's') actionIndex = 1;
    else if (action == 'a') actionIndex = 2;
    else if (action == 'd') actionIndex = 3;
    else return;

    double maxNextQ = *max_element(Q[nextStateX][nextStateY].begin(), Q[nextStateX][nextStateY].end());
    Q[stateX][stateY][actionIndex] += learningRate * (reward + discountFactor * maxNextQ - Q[stateX][stateY][actionIndex]);
}

// Function to calculate the reward based on the new state
double calculateReward(const vector<vector<char>>& board, int playerX, int playerY, int steps) {
    // Check if all boxes are on targets
    bool allBoxesOnTargets = true;
    for (int i = 1; i < board.size() - 1; i++) {
        for (int j = 1; j < board[i].size() - 1; j++) {
            if (board[i][j] == 'B' && board[i][j] != 'T') {
                allBoxesOnTargets = false;
                break;
            }
        }
        if (!allBoxesOnTargets) break;
    }
    if (allBoxesOnTargets) return 1000.0; // Large positive reward for completing the game

    // Small negative reward for each step
    return -0.1;
}

// Function to select an action using epsilon-greedy policy
char selectAction(const vector<vector<vector<double>>>& Q, int stateX, int stateY) {
    if ((double)rand() / RAND_MAX < explorationRate) {
        // Explore: Choose a random action
        int actionIndex = rand() % 4; // 4 possible actions (w/s/a/d)
        if (actionIndex == 0) return 'w';
        else if (actionIndex == 1) return 's';
        else if (actionIndex == 2) return 'a';
        else return 'd';
    } else {
        // Exploit: Choose the action with the highest Q-value
        int maxActionIndex = distance(Q[stateX][stateY].begin(), max_element(Q[stateX][stateY].begin(), Q[stateX][stateY].end()));
        if (maxActionIndex == 0) return 'w';
        else if (maxActionIndex == 1) return 's';
        else if (maxActionIndex == 2) return 'a';
        else return 'd';
    }
}


// Global variables for target and box positions | change these along with the maze to change the maze
int tX[] = {1, 3};  // Target X positions
int tY[] = {2, 1};  // Target Y positions
int bX[] = {3, 4};  // Box X positions
int bY[] = {3, 4};  // Box Y positions

// Function to initialize the maze/board
vector<vector<char>> getMaze() {
    vector<vector<char>> board(7, vector<char>(7, '#'));

    // Fill the board with empty spaces
    for (int i = 1; i < 6; i++) {
        for (int j = 1; j < 6; j++) {
            board[i][j] = '.';
        }
    }

    // Set targets on the board
    for (int i = 0; i < sizeof(tX) / sizeof(int); i++) {
        board[tX[i]][tY[i]] = 'T';
    }

    // Set boxes on the board
    for (int i = 0; i < sizeof(bX) / sizeof(int); i++) {
        board[bX[i]][bY[i]] = 'B';
    }

    // Set obstacles on the board
    board[5][5] = '#';
    board[5][4] = '#';
    board[5][3] = '#';
    board[1][5] = '#';
    board[2][5] = '#';
    board[1][4] = '#';
    board[2][4] = '#';
    board[1][3] = '#';
    board[2][3] = '#';
    board[6][2] = '#';
    board[6][6] = '#';

    return board;
}


// Function to play the game using Q-learning algorithm
void playGameWithQLearning(int playerX, int playerY) {
    srand(time(0)); // Seed for random number generation


    vector<vector<char>> board = getMaze();

    // Initialize Q-tables with zeros
    vector<vector<vector<double>>> Q(7, vector<vector<double>>(7, vector<double>(4, 0.0)));

    int initialPlayerX, initialPlayerY;
    initialPlayerX = playerX;
    initialPlayerY = playerY;
    int steps = 0; // Counter for total steps taken by the player
    vector<char> stepsTaken;


    // Ensure the player's position is valid
    if (initialPlayerX < 0 || initialPlayerX >= 7 || initialPlayerY < 0 || initialPlayerY >= 7 || board[initialPlayerX][initialPlayerY] == '#') {
        cerr << "Invalid starting position. Exiting..." << endl;
        return;
    }

    playerX = initialPlayerX;
    playerY = initialPlayerY;
    int resetCounter = 0;

    // Main game loop
    while (!checkWin(board) && resetCounter < 6000000) {
        while (steps < 100 && resetCounter < 6000000) {
            for (int i = 0 ; i < sizeof(tX)/sizeof(int) ; i++) {
                if (board[tX[i]][tY[i]] == '.') {
                    board[tX[i]][tY[i]] = 'T';
                }
            }

            char move = selectAction(Q, playerX, playerY);

            int nextX = playerX, nextY = playerY;
            bool validMove = true; // Flag to check if the move is valid

            // Process the move
            if (move == 'w') nextX--;
            else if (move == 's') nextX++;
            else if (move == 'a') nextY--;
            else if (move == 'd') nextY++;
            else validMove = false; // Invalid move

            // Check if the move is valid (not into a wall)
            if (validMove && nextX >= 0 && nextX < 7 && nextY >= 0 && nextY < 7 && board[nextX][nextY] != '#') {
                // Check if the move involves pushing a box against a wall
                if (board[nextX][nextY] == 'B') {
                    int beyondX = nextX + (nextX - playerX);
                    int beyondY = nextY + (nextY - playerY);
                    // Check if the beyond position is a wall
                    if (beyondX >= 0 && beyondX < 7 && beyondY >= 0 && beyondY < 7 && board[beyondX][beyondY] == '#') {
                        // If beyond position is not a target, don't record the move
                        if (board[nextX][nextY] != 'T') {
                            continue;
                        }
                    }
                }

                // Update player position
                if (board[nextX][nextY] == '.' || board[nextX][nextY] == 'T') {
                    playerX = nextX;
                    playerY = nextY;
                    steps++; // Increment step counter
                } else if (board[nextX][nextY] == 'B') {
                    // Move the box
                    int beyondX = nextX + (nextX - playerX);
                    int beyondY = nextY + (nextY - playerY);
                    // Check if both current and next positions are blocked by obstacles
                    if (board[nextX][nextY] == '#' && board[beyondX][beyondY] == '#') {
                        board[nextX][nextY] = 'B';  // Reset box to original position
                    } else {
                        // Move the box if the beyond position is valid
                        if (beyondX >= 0 && beyondX < 7 && beyondY >= 0 && beyondY < 7 &&
                            (board[beyondX][beyondY] == '.' || board[beyondX][beyondY] == 'T')) {
                            board[beyondX][beyondY] = 'B';
                            board[nextX][nextY] = '.';
                            playerX = nextX;
                            playerY = nextY;
                            steps++; // Increment step counter
                            if (board[beyondX][beyondY] == 'T') {
                                board[beyondX][beyondY] = 'B';  // Ensure box stays on target
                            }
                        }
                    }
                }

                // Only record valid moves (not into a wall) to stepsTaken
                stepsTaken.push_back(move); // Store the step taken
            }

            double reward = calculateReward(board, playerX, playerY, steps);
            // Update Q-values
            updateQValues(Q, playerX, playerY, move, reward, nextX, nextY);
            if (checkWin(board)) break;
        }

        if (!checkWin(board)) {
            for (int i = 1; i < 6; i++) {
                for (int j = 1; j < 6; j++) {
                    if (board[i][j] == 'P') board[i][j] = '.';
                    if (board[i][j] == 'B') board[i][j] = '.'; // Reset box to original position
                }
            }

            // Set targets on the board
            for (int i = 0; i < sizeof(tX) / sizeof(int); i++) {
                board[tX[i]][tY[i]] = 'T';
            }

            // Set boxes on the board
            for (int i = 0; i < sizeof(bX) / sizeof(int); i++) {
                board[bX[i]][bY[i]] = 'B';
            }

            playerX = initialPlayerX;
            playerY = initialPlayerY;
            steps = 0; // Reset step counter
            stepsTaken.clear(); // Clear steps taken
            resetCounter++;
        }
    }


    if (resetCounter < 6000000) {
    cout << "The Q-learning Algorithm has finished with " << steps + (resetCounter * 100) << " steps!" << endl;
    } else {
        cout << "The Q-learning Algorithm couldn't finish.You can have a go" << endl;
    }


    // Display the steps taken
    cout << "Steps taken: ";
    for (char step : stepsTaken) {
        cout << step << " ";
    }
    cout << endl;
}

// Function to play the game manually
void playGameManually(int playerX, int playerY) {
    cout << "YOu can play the game now, you can choose to follow the steps the Q-Learning algorithm used or sollve it yourself!" << endl;


    vector<vector<char>> board = getMaze();

    int steps = 0; // Counter for total steps taken by the player

    // Main game loop
    while (!checkWin(board)) {
        printBoardforplayer(board, playerX, playerY);

    for (int i = 0 ; i < sizeof(tX)/sizeof(int) ; i++) {
            if (board[tX[i]][tY[i]] == '.') {
                board[tX[i]][tY[i]] = 'T';
            }
        }

        cout << "Enter move (w/a/s/d): ";
        char move;
        cin >> move;

        int nextX = playerX, nextY = playerY;
        if (move == 'w') nextX--;
        else if (move == 's') nextX++;
        else if (move == 'a') nextY--;
        else if (move == 'd') nextY++;

        // Check boundaries and move the player, ensuring it doesn't move into walls
        if (nextX >= 0 && nextX < 7 && nextY >= 0 && nextY < 7 && board[nextX][nextY] != '#') {
            if (board[nextX][nextY] == '.' || board[nextX][nextY] == 'T') {
                playerX = nextX;
                playerY = nextY;
                steps++; // Increment step counter
            } else if (board[nextX][nextY] == 'B') {
                // Move the box
                int beyondX = nextX + (nextX - playerX);
                int beyondY = nextY + (nextY - playerY);
                if (beyondX >= 0 && beyondX < 7 && beyondY >= 0 && beyondY < 7 &&
                    (board[beyondX][beyondY] == '.' || board[beyondX][beyondY] == 'T')) {
                    board[beyondX][beyondY] = 'B';
                    board[nextX][nextY] = '.';
                    playerX = nextX;
                    playerY = nextY;
                    steps++; // Increment step counter
                }
            }
        }
    }

    printBoardforplayer(board, playerX, playerY);

    cout << "Congratulations! You've won in " << steps << " steps!" << endl;
}

int main() {
    cout << "The Algorithm Will Run For Atleast 6 million times if it couldn't solve it you can have Try to\n";

    vector<vector<char>> board = getMaze();
    printBoardforplayer(board, -1, -1);

    cout << "Enter the row and column (0-indexed) to start (e.g., 4 4): ";
    int playerX, playerY;
    cin >> playerX >> playerY;

    if (playerX < 0 || playerX >= 7 || playerY < 0 || playerY >= 7 || board[playerX][playerY] == '#') {
        cerr << "Invalid starting position. Exiting..." << endl;
        return 0;
    }

    playGameWithQLearning(playerX,playerY); // Let the algorithm play the game
    playGameManually(playerX,playerY); // Play the gamae manually

    return 0;
}
