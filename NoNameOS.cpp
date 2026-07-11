// Core headers: I/O streams, string manipulation, containers, utilities, and C time
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <random>
// POSIX headers for terminal control (raw input) and non-blocking I/O detection
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

// Generate a human-readable timestamp string (e.g. "Jul 05 09:53") for VFS metadata
string get_timestamp() {
    time_t now = time(nullptr);
    tm* t = localtime(&now);
    char buf[20];
    strftime(buf, sizeof(buf), "%b %d %H:%M", t);
    return string(buf);
}

// Virtual File System (VFS) node -- represents either a directory or a file
struct FSNode {
    bool is_dir;              // true if this node is a directory
    string content;           // file content (unused for directories)
    size_t size;              // byte length of content
    string created_at;        // creation timestamp from get_timestamp()

    FSNode() : is_dir(false), content(""), size(0), created_at("") {}
    FSNode(bool d, string c, size_t = 0) : is_dir(d), content(c), size(c.length()), created_at(get_timestamp()) {}
};

// Boot-sequence animation delay
void boot_delay(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}

// Split raw input into a command token and its arguments string
pair<string, string> parse_command(const string& input) {
    size_t first_space = input.find(' ');
    if (first_space == string::npos) return {input, ""};
    return {input.substr(0, first_space), input.substr(first_space + 1)};
}

// POSIX non-blocking keyboard hit detection
// Temporarily puts stdin into raw non-blocking mode, peeks for a character, then restores the terminal
int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

// --- ASCIIDASH ENGINE ---
// A side-scrolling obstacle runner that renders frames using ANSI escape sequences
// Controls: SPACE or ENTER to jump over '^' obstacles
void play_asciidash(string map_data) {
    cout << "\033[2J\033[1;1H";
    cout << "INITIALIZING ASCIIDASH ENGINE...\n";
    boot_delay(1000);

    int player_y = 0; // 0 = floor, 1 = jumping
    int jump_timer = 0;
    bool crashed = false;
    
    // Add padding to map
    map_data = "__________" + map_data + "__________";

    for (size_t i = 0; i < map_data.length() - 10; i++) {
        // Handle input
        if (kbhit()) {
            char c = getchar();
            if ((c == ' ' || c == '\n') && player_y == 0) {
                player_y = 1;
                jump_timer = 3; // Stay in air for 3 frames
            }
        }

        // Gravity
        if (jump_timer > 0) {
            jump_timer--;
        } else {
            player_y = 0;
        }

        // Collision detection (checking the block exactly at player pos)
        if (player_y == 0 && map_data[i + 5] == '^') {
            crashed = true;
            break;
        }

        // Render frame
        cout << "\033[2J\033[1;1H"; // Clear screen fast
        cout << "--- ASCIIDASH v1.0 --- (Press SPACE to Jump)\n\n";
        
        // Render Sky
        cout << (player_y == 1 ? "     ■\n" : "\n");
        // Render Ground/Player
        cout << "     " << (player_y == 0 ? "■" : " ") << "\n";
        // Render Map sliding
        cout << map_data.substr(i, 20) << "\n";
        cout << "====================\n";

        this_thread::sleep_for(chrono::milliseconds(150)); // Game speed
    }

    // Flush leftover inputs
    while(kbhit()) (void)getchar();
    
    cout << "\n\n";
    if (crashed) {
        cout << "\033[31m[ x_x ] CRASHED! Attempt failed.\033[0m\n";
    } else {
        cout << "\033[32m[ ^_^ ] LEVEL COMPLETE! GG!\033[0m\n";
    }
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(10000), '\n');
    cin.get();
}

// --- SNAKE GAME ---
// Terminal-based Snake game. Use WASD to move, eat food (*) to grow.
void play_snake() {
    const int W = 20, H = 15;
    vector<pair<int,int>> snake = {{W/2, H/2}};
    int dx = 1, dy = 0;
    int food_x = rand() % W, food_y = rand() % H;
    int score = 0;
    bool game_over = false;

    while (!game_over) {
        // Input
        if (kbhit()) {
            char c = getchar();
            if (c == 'w' && dy == 0) { dx = 0; dy = -1; }
            else if (c == 's' && dy == 0) { dx = 0; dy = 1; }
            else if (c == 'a' && dx == 0) { dx = -1; dy = 0; }
            else if (c == 'd' && dx == 0) { dx = 1; dy = 0; }
        }

        // Move snake
        int nx = snake[0].first + dx;
        int ny = snake[0].second + dy;

        // Wall collision
        if (nx < 0 || nx >= W || ny < 0 || ny >= H) {
            game_over = true;
            break;
        }

        // Self collision
        bool hit_self = false;
        for (auto& seg : snake) {
            if (seg.first == nx && seg.second == ny) { hit_self = true; break; }
        }
        if (hit_self) { game_over = true; break; }

        snake.insert(snake.begin(), {nx, ny});

        // Check food
        if (nx == food_x && ny == food_y) {
            score++;
            food_x = rand() % W;
            food_y = rand() % H;
        } else {
            snake.pop_back();
        }

        // Render
        cout << "\033[2J\033[1;1H";
        cout << "--- SNAKE v1.0 --- Score: " << score << " (WASD to move)\n\n";
        for (int y = 0; y < H; y++) {
            cout << "  ";
            for (int x = 0; x < W; x++) {
                bool is_snake = false;
                for (size_t i = 0; i < snake.size(); i++) {
                    if (snake[i].first == x && snake[i].second == y) {
                        cout << (i == 0 ? "\033[32mO\033[0m" : "\033[36mo\033[0m");
                        is_snake = true;
                        break;
                    }
                }
                if (!is_snake) {
                    if (x == food_x && y == food_y) cout << "\033[31m*\033[0m";
                    else cout << ".";
                }
            }
            cout << "\n";
        }
        cout << "\n  Score: " << score << "  |  Press Ctrl+C to quit\n";
        this_thread::sleep_for(chrono::milliseconds(150));
    }

    while(kbhit()) (void)getchar();
    cout << "\n\n\033[31m[ GAME OVER ] Final Score: " << score << "\033[0m\n";
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(10000), '\n');
    cin.get();
}

// --- MINESWEEPER ---
// Terminal-based Minesweeper. Enter coordinates to reveal cells.
void play_minesweeper() {
    const int W = 10, H = 10, MINES = 12;
    vector<vector<char>> board(H, vector<char>(W, '.'));
    vector<vector<bool>> revealed(H, vector<bool>(W, false));
    vector<vector<bool>> mines(H, vector<bool>(W, false));
    int remaining = W * H - MINES;
    bool game_over = false;
    bool won = false;

    // Place mines
    int placed = 0;
    while (placed < MINES) {
        int mx = rand() % W, my = rand() % H;
        if (!mines[my][mx]) { mines[my][mx] = true; placed++; }
    }

    // Calculate numbers
    auto count_adj = [&](int x, int y) {
        int cnt = 0;
        for (int dy = -1; dy <= 1; dy++)
            for (int dx = -1; dx <= 1; dx++) {
                int nx = x + dx, ny = y + dy;
                if (nx >= 0 && nx < W && ny >= 0 && ny < H && mines[ny][nx]) cnt++;
            }
        return cnt;
    };

    // Flood fill reveal
    function<void(int,int)> reveal = [&](int x, int y) {
        if (x < 0 || x >= W || y < 0 || y >= H || revealed[y][x]) return;
        revealed[y][x] = true;
        remaining--;
        int adj = count_adj(x, y);
        board[y][x] = adj + '0';
        if (adj == 0) {
            for (int dy = -1; dy <= 1; dy++)
                for (int dx = -1; dx <= 1; dx++)
                    reveal(x + dx, y + dy);
        }
    };

    while (!game_over) {
        cout << "\033[2J\033[1;1H";
        cout << "--- MINESWEEPER v1.0 --- Mines: " << MINES << "\n\n";
        cout << "    ";
        for (int x = 0; x < W; x++) cout << x << " ";
        cout << "\n   +";
        for (int x = 0; x < W; x++) cout << "--";
        cout << "\n";
        for (int y = 0; y < H; y++) {
            cout << " " << y << " |";
            for (int x = 0; x < W; x++) {
                if (!revealed[y][x]) cout << "\033[90m#\033[0m ";
                else if (mines[y][x]) cout << "\033[31mX\033[0m ";
                else {
                    int n = board[y][x] - '0';
                    if (n == 0) cout << "  ";
                    else cout << "\033[3" << (n % 7 + 1) << "m" << n << "\033[0m ";
                }
            }
            cout << "\n";
        }
        cout << "\n  Cells left: " << remaining << "\n";

        if (remaining <= 0) { won = true; break; }

        cout << "\nEnter coordinates (x y) or 'f x y' to flag: ";
        string line;
        getline(cin, line);
        if (line.empty()) continue;
        istringstream iss(line);
        string cmd; int fx, fy;
        iss >> cmd;
        if (cmd == "f" && (iss >> fx >> fy)) {
            if (fx >= 0 && fx < W && fy >= 0 && fy < H && !revealed[fy][fx]) {
                board[fy][fx] = (board[fy][fx] == 'F') ? '.' : 'F';
            }
        } else {
            istringstream iss2(line);
            if (iss2 >> fx >> fy) {
                if (fx >= 0 && fx < W && fy >= 0 && fy < H) {
                    if (mines[fy][fx]) {
                        game_over = true;
                        for (int y = 0; y < H; y++)
                            for (int x = 0; x < W; x++)
                                if (mines[y][x]) revealed[y][x] = true;
                    } else {
                        reveal(fx, fy);
                    }
                }
            }
        }
    }

    // Final render
    cout << "\033[2J\033[1;1H";
    cout << "--- MINESWEEPER v1.0 ---\n\n";
    cout << "    ";
    for (int x = 0; x < W; x++) cout << x << " ";
    cout << "\n   +";
    for (int x = 0; x < W; x++) cout << "--";
    cout << "\n";
    for (int y = 0; y < H; y++) {
        cout << " " << y << " |";
        for (int x = 0; x < W; x++) {
            if (mines[y][x]) cout << "\033[31mX\033[0m ";
            else {
                int n = count_adj(x, y);
                if (n == 0) cout << "  ";
                else cout << "\033[3" << (n % 7 + 1) << "m" << n << "\033[0m ";
            }
        }
        cout << "\n";
    }

    if (won) cout << "\n\033[32m[ YOU WIN! ] Congratulations!\033[0m\n";
    else cout << "\n\033[31m[ BOOM! ] You hit a mine!\033[0m\n";
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(10000), '\n');
    cin.get();
}

// --- COLOR FUNCTION ---
string rainbow(const string& text) {
    string result;
    const char* colors[] = {"\033[31m", "\033[33m", "\033[32m", "\033[36m", "\033[34m", "\033[35m"};
    for (size_t i = 0; i < text.length(); i++) {
        result += colors[i % 6] + text[i] + string("\033[0m");
    }
    return result;
}

// --- TIC-TAC-TOE ---
// Play against an AI that uses minimax logic
void play_tictactoe() {
    vector<char> board(9, ' ');
    int turn = 0; // 0 = player (X), 1 = AI (O)

    auto print_board = [&]() {
        cout << "\033[2J\033[1;1H";
        cout << "--- TIC-TAC-TOE v1.0 --- (You: X, AI: O)\n\n";
        for (int r = 0; r < 3; r++) {
            cout << "     ";
            for (int c = 0; c < 3; c++) {
                int idx = r * 3 + c;
                char ch = board[idx];
                if (ch == 'X') cout << "\033[32mX\033[0m";
                else if (ch == 'O') cout << "\033[31mO\033[0m";
                else cout << (char)('1' + idx);
                if (c < 2) cout << " | ";
            }
            cout << "\n";
            if (r < 2) cout << "    ---+---+---\n";
        }
        cout << "\n";
    };

    auto check_win = [&](char p) -> bool {
        int wins[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
        for (auto& w : wins)
            if (board[w[0]] == p && board[w[1]] == p && board[w[2]] == p) return true;
        return false;
    };

    auto is_draw = [&]() {
        for (char c : board) if (c == ' ') return false;
        return !check_win('X') && !check_win('O');
    };

    // Minimax AI
    function<int(vector<char>&, char)> minimax = [&](vector<char>& b, char player) -> int {
        if (check_win('O')) return 10;
        if (check_win('X')) return -10;
        if (is_draw()) return 0;
        int best = (player == 'O') ? -1000 : 1000;
        for (int i = 0; i < 9; i++) {
            if (b[i] == ' ') {
                b[i] = player;
                int score = minimax(b, (player == 'O') ? 'X' : 'O');
                b[i] = ' ';
                best = (player == 'O') ? max(best, score) : min(best, score);
            }
        }
        return best;
    };

    auto ai_move = [&]() {
        int best_score = -1000, best_move = -1;
        for (int i = 0; i < 9; i++) {
            if (board[i] == ' ') {
                board[i] = 'O';
                int score = minimax(board, 'X');
                board[i] = ' ';
                if (score > best_score) { best_score = score; best_move = i; }
            }
        }
        board[best_move] = 'O';
    };

    print_board();
    while (true) {
        if (turn == 0) {
            cout << "Your move (1-9): ";
            string line; getline(cin, line);
            if (line.empty()) continue;
            int pos = line[0] - '1';
            if (pos < 0 || pos > 8 || board[pos] != ' ') {
                cout << "Invalid move. Press Enter.\n";
                cin.get();
                continue;
            }
            board[pos] = 'X';
        } else {
            ai_move();
            cout << "AI moved.\n";
        }

        print_board();

        if (check_win('X')) { cout << "\033[32mYou win!\033[0m\n"; break; }
        if (check_win('O')) { cout << "\033[31mAI wins!\033[0m\n"; break; }
        if (is_draw()) { cout << "\033[33mDraw!\033[0m\n"; break; }

        turn = 1 - turn;
    }
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(10000), '\n');
    cin.get();
}

// --- HANGMAN ---
void play_hangman() {
    vector<string> words = {"computer", "keyboard", "monitor", "programming", "terminal",
                            "software", "hardware", "network", "internet", "algorithm"};
    string word = words[rand() % words.size()];
    string guessed(word.length(), '_');
    int attempts = 6;
    vector<char> wrong;

    while (attempts > 0 && guessed.find('_') != string::npos) {
        cout << "\033[2J\033[1;1H";
        cout << "--- HANGMAN v1.0 ---\n\n";

        cout << "  +---+\n";
        cout << "  |   " << (attempts < 6 ? "|" : "") << "\n";
        cout << "  " << (attempts < 5 ? "O" : "") << (attempts < 4 ? "  |" : "") << "\n";
        cout << " " << (attempts < 3 ? "/" : "") << (attempts < 2 ? "|" : "") << (attempts < 1 ? "\\" : "") << "\n";
        cout << " " << (attempts < 3 ? "/" : "") << (attempts < 1 ? " \\" : "") << "\n\n";

        cout << "  Word: ";
        for (char c : guessed) cout << c << " ";
        cout << "\n\n  Wrong: ";
        for (char c : wrong) cout << c << " ";
        cout << "\n  Attempts left: " << attempts << "\n\n";

        cout << "Guess a letter: ";
        string line; getline(cin, line);
        if (line.empty()) continue;
        char g = tolower(line[0]);

        bool found = false;
        for (size_t i = 0; i < word.length(); i++) {
            if (word[i] == g && guessed[i] == '_') { guessed[i] = g; found = true; }
        }
        if (!found) {
            if (find(wrong.begin(), wrong.end(), g) == wrong.end())
                wrong.push_back(g);
            attempts--;
        }
    }

    cout << "\033[2J\033[1;1H";
    cout << "--- HANGMAN v1.0 ---\n\n";
    if (guessed.find('_') == string::npos) {
        cout << "\033[32mYou saved him! The word was: " << word << "\033[0m\n";
    } else {
        cout << "\033[31mHANGED! The word was: " << word << "\033[0m\n";
    }
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(10000), '\n');
    cin.get();
}

// --- ROCK PAPER SCISSORS ---
void play_rps() {
    const char* choices[] = {"Rock", "Paper", "Scissors"};
    int player_wins = 0, ai_wins = 0;

    while (player_wins < 4 && ai_wins < 4) {
        cout << "\033[2J\033[1;1H";
        cout << "--- ROCK PAPER SCISSORS v1.0 --- (First to 4 wins)\n\n";
        cout << "  \033[32mYou: " << player_wins << "\033[0m  \033[31mAI: " << ai_wins << "\033[0m\n\n";
        cout << "  1. Rock\n  2. Paper\n  3. Scissors\n";
        cout << "  Choice (1-3): ";

        string line; getline(cin, line);
        if (line.empty()) continue;
        int p = line[0] - '1';
        if (p < 0 || p > 2) { cout << "Invalid.\n"; this_thread::sleep_for(chrono::milliseconds(500)); continue; }

        int a = rand() % 3;
        cout << "  You: " << choices[p] << "  vs  AI: " << choices[a] << "\n";

        if (p == a) cout << "  \033[33mDraw!\033[0m\n";
        else if ((p == 0 && a == 2) || (p == 1 && a == 0) || (p == 2 && a == 1)) {
            cout << "  \033[32mYou win this round!\033[0m\n"; player_wins++;
        } else {
            cout << "  \033[31mAI wins this round!\033[0m\n"; ai_wins++;
        }
        cout << "\nPress Enter to continue...";
        cin.get();
    }

    cout << "\n\033[1m" << (player_wins == 4 ? "\033[32mYOU WIN THE SERIES!" : "\033[31mAI WINS THE SERIES!") << "\033[0m\n";
    cout << "Press Enter to return to NoNameOS...";
    cin.get();
}

    int main() {
    cout << "\033[2J\033[1;1H";
    cout << "[    0.000000] Booting NoNameOS Core v0.5.0...\n";
    boot_delay(300);
    cout << "[    1.102304] Loading Termios Input Hijacker...\n";
    boot_delay(300);
    cout << "[    1.500000] AsciiDash Engine Ready.\n";
    boot_delay(200);
    cout << "[    1.700000] Loading Bovine Speech Synthesizer...\n";
    boot_delay(150);
    cout << "[    1.850000] Loading Game Engines (guess, trivia, adventure)...\n";
    boot_delay(150);
    cout << "[    2.000000] Loading System Tools (nano, calc)...\n";
    boot_delay(150);
    cout << "[    2.150000] Loading Snake Engine...\n";
    boot_delay(150);
    cout << "[    2.300000] Loading Minesweeper Grid...\n";
    boot_delay(150);
    cout << "[    2.450000] Loading Color Themes...\n";
    boot_delay(150);
    cout << "[    2.600000] Loading Tic-Tac-Toe AI Engine...\n";
    boot_delay(150);
    cout << "[    2.750000] Loading Hangman Gallows...\n";
    boot_delay(150);
    cout << "[    2.900000] Loading RPS Arena...\n";
    boot_delay(150);
    cout << "[    3.050000] Loading Utility Commands...\n\n";

    map<string, FSNode> file_system;
    file_system["/"] = FSNode(true, "");

    // Default custom level mapping -- pre-load AsciiDash obstacle map into VFS
    file_system["/geometry/"] = FSNode(true, "");
    file_system["/geometry/jumper.gmd"] = FSNode(false, "_______^_______^^_______^___^^^___");

    string current_user = "root";
    string current_dir = "/";
    string input;
    vector<string> cmd_history;

    while (true) {
        cout << current_user << "@NoNameOS:" << current_dir << "# ";
        getline(cin, input);

        if (input.empty()) continue;
        cmd_history.push_back(input);
        auto [cmd, args] = parse_command(input);

        if (cmd == "help") {
            // Display available commands grouped by category
            cout << "NoNameOS v0.6.0 Commands:\n";
            cout << "\033[1;33m  Filesystem:\033[0m ls, ls -l, cd, mkdir, touch, cat, echo, rm, pwd, grep, find, cp, mv, head, tail\n";
            cout << "\033[1;33m  System:\033[0m    whoami, date, uptime, history, clear, cfetch, ps, uname, exit, hostname\n";
            cout << "\033[1;33m  Tools:\033[0m     nano <file>, calc <expr>, cowsay [msg], man <cmd>, cal, rainbow [msg]\n";
            cout << "\033[1;33m  Tools:\033[0m     sort <file>, wc <file>, tee <file> <text>, yes, env, sleep, which\n";
            cout << "\033[1;33m  Tools:\033[0m     alias, users, banner [msg], fortune, factor <n>, shuf <text>\n";
            cout << "\033[1;33m  Tools:\033[0m     chmod, su <user>\n";
            cout << "\033[1;33m  Games:\033[0m     play [file], guess, trivia, adventure, snake, minesweeper, ttt, hangman, rps\n";
        } 
        else if (cmd == "ls") {
            // List VFS entries under current_dir; long mode (-l) shows perms, size, and timestamp
            bool long_mode = (args == "-l");
            bool empty = true;
            for (auto const& [path, node] : file_system) {
                if (path == current_dir) continue;
                if (path.rfind(current_dir, 0) == 0) {
                    string relative = path.substr(current_dir.length());
                    size_t slash_pos = relative.find('/');
                    if (slash_pos == string::npos || (node.is_dir && slash_pos == relative.length() - 1)) {
                        if (long_mode) {
                            string type = node.is_dir ? "d" : "-";
                            string perms = "rw-r--r--";
                            cout << type << perms << "  " << node.size << " " << node.created_at << " ";
                        }
                        if (node.is_dir) cout << "\033[34m" << relative << "\033[0m   ";
                        else cout << relative << "   ";
                        empty = false;
                    }
                }
            }
            if (empty) cout << "(Empty)";
            cout << "\n";
        } 
        else if (cmd == "mkdir") {
            // Create a new directory node in the VFS
            if (!args.empty()) file_system[current_dir + args + "/"] = FSNode(true, "");
        }
        else if (cmd == "cd") {
            // Change the active working directory, handling absolute, parent, and child navigation
            if (args.empty() || args == "/") current_dir = "/";
            else if (args == "..") {
                if (current_dir != "/") {
                    string temp = current_dir.substr(0, current_dir.length() - 1);
                    current_dir = temp.substr(0, temp.find_last_of('/') + 1);
                }
            } else {
                string target_dir = current_dir + args + "/";
                if (file_system.find(target_dir) != file_system.end() && file_system[target_dir].is_dir) {
                    current_dir = target_dir;
                } else {
                    cout << "error: directory not found.\n";
                }
            }
        }
        else if (cmd == "echo") {
            // Write text content to a file in the VFS
            size_t first_space = args.find(' ');
            if (first_space != string::npos) {
                string filename = args.substr(0, first_space);
                string content = args.substr(first_space + 1);
                // Strip quotes if they exist
                if (content.front() == '"' && content.back() == '"') {
                    content = content.substr(1, content.length() - 2);
                }
                file_system[current_dir + filename] = FSNode(false, content);
            }
        }
        else if (cmd == "cat") {
            // Print the content of a VFS file to stdout
            if (file_system.find(current_dir + args) != file_system.end()) cout << file_system[current_dir + args].content << "\n";
            else cout << "File not found.\n";
        }
        else if (cmd == "rm") {
            // Remove a file or directory node from the VFS
            file_system.erase(current_dir + args);
            file_system.erase(current_dir + args + "/");
        }
        else if (cmd == "clear") cout << "\033[2J\033[1;1H";
        else if (cmd == "exit") break;
        else if (cmd == "play") {
            // Launch the AsciiDash obstacle runner; optionally load a custom map from VFS
            if (args.empty()) {
                cout << "Loading Default Map: Stereo Madness...\n";
                play_asciidash("_______^_______^_____^^_______^___");
            } else {
                string target_file = current_dir + args;
                if (file_system.find(target_file) != file_system.end() && !file_system[target_file].is_dir) {
                    cout << "Loading Custom Map: " << args << "...\n";
                    play_asciidash(file_system[target_file].content);
                } else {
                    cout << "error: Map file not found in VFS.\n";
                }
            }
        }
        // --- UTILITIES ---
        else if (cmd == "cowsay") {
            // Render an ASCII cow with a speech bubble containing the provided message
            string text = args.empty() ? "Moo." : args;
            if (text.length() >= 2 && text.front() == '"' && text.back() == '"') {
                text = text.substr(1, text.length() - 2);
            }
            string border(text.length() + 4, '-');
            cout << " " << border << "\n";
            cout << "< " << text << " >\n";
            cout << " " << border << "\n";
            cout << "        \\   ^__^\n";
            cout << "         \\  (oo)\\_______\n";
            cout << "            (__)\\       )\\/\\\n";
            cout << "                ||----w |\n";
            cout << "                ||     ||\n";
        }
        // --- NEW COMMANDS v0.4.0 ---
        else if (cmd == "pwd") {
            // Print the current working directory path
            cout << current_dir << "\n";
        }
        else if (cmd == "whoami") {
            // Display the current logged-in user
            cout << current_user << "\n";
        }
        else if (cmd == "date") {
            // Print the current system date and time
            time_t now = time(nullptr);
            tm* t = localtime(&now);
            char buf[64];
            strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y", t);
            cout << buf << "\n";
        }
        else if (cmd == "history") {
            // Display the current shell session's command history with line numbers
            for (size_t i = 0; i < cmd_history.size(); i++) {
                cout << "  " << (i + 1) << "  " << cmd_history[i] << "\n";
            }
        }
        else if (cmd == "grep") {
            // Search for a text pattern inside the content of a VFS file and print matching lines
            size_t sp = args.find(' ');
            if (sp == string::npos) {
                cout << "Usage: grep <pattern> <filename>\n";
            } else {
                string pattern = args.substr(0, sp);
                string filename = args.substr(sp + 1);
                string fullpath = current_dir + filename;
                if (file_system.find(fullpath) != file_system.end() && !file_system[fullpath].is_dir) {
                    istringstream ss(file_system[fullpath].content);
                    string line;
                    bool found = false;
                    while (getline(ss, line)) {
                        if (line.find(pattern) != string::npos) {
                            cout << line << "\n";
                            found = true;
                        }
                    }
                    if (!found) cout << "(no matches)\n";
                } else {
                    cout << "File not found.\n";
                }
            }
        }
        else if (cmd == "find") {
            // Recursively search VFS node paths and print any path containing the given name
            if (args.empty()) {
                cout << "Usage: find <name>\n";
            } else {
                bool found = false;
                for (auto const& [path, node] : file_system) {
                    if (path.find(args) != string::npos) {
                        cout << path << "\n";
                        found = true;
                    }
                }
                if (!found) cout << "(no results)\n";
            }
        }
        else if (cmd == "cfetch") {
            // Display system info block (OS version, kernel, shell, VFS node count, uptime, user)
            cout << "\033[36m";
            cout << "       ___          \n";
            cout << "      /   \\         \n";
            cout << "     / NoN \\        \n";
            cout << "    /  Name \\       \n";
            cout << "   /___  OS  \\      \n";
            cout << "       \\     /      \n";
            cout << "        \\___/       \n";
            cout << "\033[0m";
            cout << "------------------------\n";
            cout << "\033[1;33mOS:\033[0m       NoNameOS v0.6.0\n";
            cout << "\033[1;33mKernel:\033[0m   C++ POSIX Sim\n";
            cout << "\033[1;33mShell:\033[0m    nonamesh\n";
            cout << "\033[1;33mVFS:\033[0m      " << file_system.size() << " nodes\n";
            cout << "\033[1;33mUptime:\033[0m   ";
            auto elapsed = chrono::duration_cast<chrono::seconds>(
                chrono::steady_clock::now() - chrono::steady_clock::time_point{}).count();
            cout << (elapsed / 3600) << "h " << (elapsed % 3600) / 60 << "m\n";
            cout << "\033[1;33mUser:\033[0m     " << current_user << "\n";
            cout << "------------------------\n";
        }
        else if (cmd == "ps") {
            cout << "  PID TTY          TIME CMD\n";
            cout << "    1 ?        00:00:00 init\n";
            cout << "    2 ?        00:00:00 nonamesh\n";
            cout << "    3 ?        00:00:00 " << cmd_history.size() << " commands\n";
        }
        else if (cmd == "uname") {
            string flag = args;
            if (flag == "-a" || flag.empty()) {
                cout << "NoNameOS nonameos 0.5.0 C++ POSIX x86_64 GNU/C++\n";
            } else if (flag == "-r") cout << "0.5.0\n";
            else if (flag == "-s") cout << "NoNameOS\n";
            else if (flag == "-m") cout << "x86_64\n";
        }
        else if (cmd == "uptime") {
            auto elapsed = chrono::duration_cast<chrono::seconds>(
                chrono::steady_clock::now() - chrono::steady_clock::time_point{}).count();
            int days = elapsed / 86400;
            int hours = (elapsed % 86400) / 3600;
            int mins = (elapsed % 3600) / 60;
            cout << " up " << days << " day" << (days != 1 ? "s" : "")
                 << ", " << hours << ":" << (mins < 10 ? "0" : "") << mins
                 << ",  " << current_user << ",  load average: 0.00, 0.00, 0.00\n";
        }
        else if (cmd == "cal") {
            time_t now = time(nullptr);
            tm* t = localtime(&now);
            int year = t->tm_year + 1900;
            int month = t->tm_mon + 1;
            int today = t->tm_mday;

            tm first = {}; first.tm_year = year - 1900; first.tm_mon = month - 1; first.tm_mday = 1;
            time_t first_t = mktime(&first);
            tm* first_tm = localtime(&first_t);
            int start_day = first_tm->tm_wday;

            // Days in month
            int days_in_month[] = {31,28+(year%4==0&&(year%100!=0||year%400==0)),31,30,31,30,31,31,30,31,30,31};
            int total = days_in_month[month - 1];

            const char* months[] = {"January","February","March","April","May","June","July","August","September","October","November","December"};
            cout << "      " << months[month-1] << " " << year << "\n";
            cout << " Su Mo Tu We Th Fr Sa\n";
            for (int i = 0; i < start_day; i++) cout << "   ";
            for (int d = 1; d <= total; d++) {
                if (d == today) cout << "\033[7m" << (d < 10 ? " " : "") << d << "\033[0m ";
                else cout << (d < 10 ? " " : "") << d << " ";
                if ((start_day + d) % 7 == 0) cout << "\n";
            }
            cout << "\n";
        }
        else if (cmd == "rainbow") {
            if (args.empty()) cout << rainbow("🌈 NoNameOS " + current_user + "!") << "\n";
            else cout << rainbow(args) << "\n";
        }
        else if (cmd == "yes") {
            string text = args.empty() ? "y" : args;
            for (int i = 0; i < 100; i++) cout << text << " ";
            cout << "\n";
        }
        else if (cmd == "env") {
            cout << "USER=" << current_user << "\n";
            cout << "SHELL=/bin/nonamesh\n";
            cout << "PWD=" << current_dir << "\n";
            cout << "HOME=/\n";
            cout << "OS=NoNameOS\n";
            cout << "TERM=" << (getenv("TERM") ? getenv("TERM") : "xterm-256color") << "\n";
        }
        else if (cmd == "hostname") {
            cout << "nonameos\n";
        }
        else if (cmd == "sleep") {
            int sec = 0;
            for (char c : args) { if (c >= '0' && c <= '9') sec = sec * 10 + (c - '0'); }
            if (sec > 0 && sec <= 30) {
                cout << "Sleeping for " << sec << " second" << (sec != 1 ? "s" : "") << "...\n";
                this_thread::sleep_for(chrono::seconds(sec));
            }
        }
        else if (cmd == "which") {
            set<string> commands = {"ls","cd","mkdir","touch","cat","echo","rm","clear","exit","play","cowsay",
                "pwd","whoami","date","history","grep","find","cfetch","ps","uname","uptime","cal","rainbow",
                "man","help","nano","calc","guess","trivia","adventure","snake","minesweeper","tictactoe","ttt",
                "hangman","rps","yes","env","hostname","sleep","which","head","tail","sort","wc","tee","alias",
                "users","banner","fortune","factor","shuf","cp","mv","chmod","su","alias","unalias"};
            if (args.empty()) cout << "Usage: which <command>\n";
            else if (commands.find(args) != commands.end()) cout << "/bin/" << args << "\n";
            else cout << args << " not found\n";
        }
        else if (cmd == "alias") {
            cout << "alias ll='ls -l'\n";
            cout << "alias ..='cd ..'\n";
            cout << "alias ttt='tictactoe'\n";
        }
        else if (cmd == "users") {
            cout << current_user << "\n";
        }
        else if (cmd == "banner") {
            string text = args.empty() ? "NoNameOS" : args;
            string line(text.length() + 4, '#');
            cout << "\033[36m" << line << "\033[0m\n";
            cout << "\033[36m# \033[33m" << text << "\033[36m #\033[0m\n";
            cout << "\033[36m" << line << "\033[0m\n";
        }
        else if (cmd == "fortune") {
            vector<string> quotes = {
                "The best way to predict the future is to create it.",
                "In the middle of difficulty lies opportunity.",
                "Simplicity is the ultimate sophistication.",
                "Code is poetry in motion.",
                "A journey of a thousand miles begins with a single step.",
                "Debugging is twice as hard as writing the code in the first place.",
                "First, solve the problem. Then, write the code.",
                "Talk is cheap. Show me the code.",
                "Any fool can write code that a computer can understand.",
                "Make it work, make it right, make it fast."
            };
            cout << "\033[33m" << quotes[rand() % quotes.size()] << "\033[0m\n";
        }
        else if (cmd == "factor") {
            int n = 0;
            for (char c : args) { if (c >= '0' && c <= '9') n = n * 10 + (c - '0'); }
            if (n < 2) cout << "Enter a number >= 2.\n";
            else {
                cout << n << ":";
                for (int f = 2; f * f <= n; f++) {
                    while (n % f == 0) { cout << " " << f; n /= f; }
                }
                if (n > 1) cout << " " << n;
                cout << "\n";
            }
        }
        else if (cmd == "shuf") {
            if (args.empty()) {
                cout << "Usage: shuf <text>\n";
            } else {
                string s = args;
                shuffle(s.begin(), s.end(), default_random_engine(rand()));
                cout << s << "\n";
            }
        }
        else if (cmd == "cp") {
            size_t sp = args.find(' ');
            if (sp == string::npos) cout << "Usage: cp <source> <dest>\n";
            else {
                string src = args.substr(0, sp);
                string dst = args.substr(sp + 1);
                string full_src = current_dir + src;
                string full_dst = current_dir + dst;
                if (file_system.find(full_src) != file_system.end() && !file_system[full_src].is_dir) {
                    file_system[full_dst] = FSNode(false, file_system[full_src].content);
                    cout << "Copied " << src << " -> " << dst << "\n";
                } else cout << "Source file not found.\n";
            }
        }
        else if (cmd == "mv") {
            size_t sp = args.find(' ');
            if (sp == string::npos) cout << "Usage: mv <source> <dest>\n";
            else {
                string src = args.substr(0, sp);
                string dst = args.substr(sp + 1);
                string full_src = current_dir + src;
                string full_dst = current_dir + dst;
                if (file_system.find(full_src) != file_system.end()) {
                    file_system[full_dst] = file_system[full_src];
                    file_system.erase(full_src);
                    file_system.erase(full_src + "/");
                    cout << "Moved " << src << " -> " << dst << "\n";
                } else cout << "Source not found.\n";
            }
        }
        else if (cmd == "chmod") {
            cout << "chmod: NoNameOS VFS uses simulated permissions. (rw-r--r--)\n";
        }
        else if (cmd == "su") {
            if (args.empty() || args == "root") {
                current_user = "root";
                cout << "Switched to root.\n";
            } else if (args == "user") {
                current_user = "user";
                cout << "Switched to user.\n";
            } else {
                cout << "User '" << args << "' does not exist.\n";
            }
        }
        else if (cmd == "head") {
            string fn = args;
            string fullpath = current_dir + fn;
            if (file_system.find(fullpath) != file_system.end() && !file_system[fullpath].is_dir) {
                istringstream ss(file_system[fullpath].content);
                string line;
                for (int i = 0; i < 10 && getline(ss, line); i++) cout << line << "\n";
            } else cout << "File not found.\n";
        }
        else if (cmd == "tail") {
            string fn = args;
            string fullpath = current_dir + fn;
            if (file_system.find(fullpath) != file_system.end() && !file_system[fullpath].is_dir) {
                vector<string> lines;
                istringstream ss(file_system[fullpath].content);
                string line;
                while (getline(ss, line)) lines.push_back(line);
                int start = max(0, (int)lines.size() - 10);
                for (int i = start; i < (int)lines.size(); i++) cout << lines[i] << "\n";
            } else cout << "File not found.\n";
        }
        else if (cmd == "sort") {
            string fn = args;
            string fullpath = current_dir + fn;
            if (file_system.find(fullpath) != file_system.end() && !file_system[fullpath].is_dir) {
                vector<string> lines;
                istringstream ss(file_system[fullpath].content);
                string line;
                while (getline(ss, line)) lines.push_back(line);
                sort(lines.begin(), lines.end());
                for (auto& l : lines) cout << l << "\n";
            } else cout << "File not found.\n";
        }
        else if (cmd == "wc") {
            string fn = args;
            string fullpath = current_dir + fn;
            if (file_system.find(fullpath) != file_system.end() && !file_system[fullpath].is_dir) {
                string content = file_system[fullpath].content;
                int lines = 0, words = 0, chars = content.length();
                bool in_word = false;
                for (char c : content) {
                    if (c == '\n') lines++;
                    if (c == ' ' || c == '\n' || c == '\t') in_word = false;
                    else if (!in_word) { words++; in_word = true; }
                }
                if (!content.empty() && content.back() != '\n') lines++;
                cout << "  " << lines << "  " << words << "  " << chars << "  " << fn << "\n";
            } else cout << "File not found.\n";
        }
        else if (cmd == "tee") {
            size_t sp = args.find(' ');
            if (sp == string::npos) cout << "Usage: tee <file> <text>\n";
            else {
                string fn = args.substr(0, sp);
                string text = args.substr(sp + 1);
                string fullpath = current_dir + fn;
                file_system[fullpath] = FSNode(false, text + "\n");
                cout << text << "\n";
            }
        }
        else if (cmd == "man") {
            if (args.empty()) {
                cout << "Usage: man <command>\n";
            } else {
                map<string, string> manpages;
                manpages["ls"] = "LS(1)\t\t\tUser Commands\n\nNAME\n\tls - list directory contents\n\nSYNOPSIS\n\tls [-l]\n\nDESCRIPTION\n\tList information about VFS files. With -l, show sizes and timestamps.";
                manpages["cd"] = "CD(1)\t\t\tUser Commands\n\nNAME\n\tcd - change the current working directory\n\nSYNOPSIS\n\tcd [dir]\n\nDESCRIPTION\n\tChange to the specified directory. Use '..' for parent or '/' for root.";
                manpages["mkdir"] = "MKDIR(1)\t\tUser Commands\n\nNAME\n\tmkdir - create a directory\n\nSYNOPSIS\n\tmkdir <name>\n\nDESCRIPTION\n\tCreate a new empty directory in the virtual filesystem.";
                manpages["touch"] = "TOUCH(1)\t\tUser Commands\n\nNAME\n\ttouch - create an empty file\n\nSYNOPSIS\n\ttouch <file>\n\nDESCRIPTION\n\tCreate an empty file in the VFS. No-op if file already exists.";
                manpages["cat"] = "CAT(1)\t\t\tUser Commands\n\nNAME\n\tcat - concatenate and print files\n\nSYNOPSIS\n\tcat <file>\n\nDESCRIPTION\n\tDisplay the contents of a VFS file to the terminal.";
                manpages["echo"] = "ECHO(1)\t\t\tUser Commands\n\nNAME\n\techo - write content to a file\n\nSYNOPSIS\n\techo <file> <content>\n\nDESCRIPTION\n\tWrite quoted or unquoted text to a VFS file.";
                manpages["rm"] = "RM(1)\t\t\tUser Commands\n\nNAME\n\trm - remove files or directories\n\nSYNOPSIS\n\trm <name>\n\nDESCRIPTION\n\tRemove a file or directory from the VFS.";
                manpages["grep"] = "GREP(1)\t\t\tUser Commands\n\nNAME\n\tgrep - search for patterns in a file\n\nSYNOPSIS\n\tgrep <pattern> <file>\n\nDESCRIPTION\n\tSearch for lines containing a pattern in a VFS file.";
                manpages["find"] = "FIND(1)\t\t\tUser Commands\n\nNAME\n\tfind - search for files by name\n\nSYNOPSIS\n\tfind <name>\n\nDESCRIPTION\n\tRecursively search and print all VFS paths matching the given name.";
                manpages["calc"] = "CALC(1)\t\t\tUser Commands\n\nNAME\n\tcalc - arithmetic calculator\n\nSYNOPSIS\n\tcalc <expression>\n\nDESCRIPTION\n\tEvaluate a mathematical expression (supports + - * / with operator precedence).";
                manpages["cowsay"] = "COWSAY(1)\t\tUser Commands\n\nNAME\n\tcowsay - ASCII cow with speech bubble\n\nSYNOPSIS\n\tcowsay [message]\n\nDESCRIPTION\n\tDisplay a talking ASCII cow with the given message.";
                manpages["nano"] = "NANO(1)\t\t\tUser Commands\n\nNAME\n\tnano - built-in line editor\n\nSYNOPSIS\n\tnano <file>\n\nDESCRIPTION\n\tLine-by-line text editor. Enter an empty line to save and exit.";
                manpages["play"] = "PLAY(1)\t\t\tGame Commands\n\nNAME\n\tplay - launch AsciiDash obstacle runner\n\nSYNOPSIS\n\tplay [file]\n\nDESCRIPTION\n\tRun the AsciiDash side-scrolling game. Optionally load a custom .gmd map file.";
                manpages["guess"] = "GUESS(1)\t\tGame Commands\n\nNAME\n\tguess - number guessing game\n\nSYNOPSIS\n\tguess\n\nDESCRIPTION\n\tGuess a random number between 1 and 100. Unlimited attempts.";
                manpages["trivia"] = "TRIVIA(1)\t\tGame Commands\n\nNAME\n\ttrivia - technology trivia quiz\n\nSYNOPSIS\n\ttrivia\n\nDESCRIPTION\n\tAnswer 5 multiple-choice questions about computers and technology.";
                manpages["adventure"] = "ADVENTURE(1)\t\tGame Commands\n\nNAME\n\tadventure - dungeon RPG\n\nSYNOPSIS\n\tadventure\n\nDESCRIPTION\n\tExplore a dungeon, fight monsters, collect gold, and survive. Commands: left, right, rest, quit.";
                manpages["snake"] = "SNAKE(1)\t\tGame Commands\n\nNAME\n\tsnake - terminal snake game\n\nSYNOPSIS\n\tsnake\n\nDESCRIPTION\n\tControl a snake with WASD. Eat food (*) to grow. Don't hit walls or yourself.";
                manpages["minesweeper"] = "MINESWEEPER(1)\t\tGame Commands\n\nNAME\n\tminesweeper - terminal minesweeper\n\nSYNOPSIS\n\tminesweeper\n\nDESCRIPTION\n\tReveal cells by entering coordinates (x y). Avoid mines. Flag with 'f x y'.";
                manpages["cfetch"] = "CFETCH(1)\t\tUser Commands\n\nNAME\n\tcfetch - system info display\n\nSYNOPSIS\n\tcfetch\n\nDESCRIPTION\n\tDisplay system information similar to neofetch.";
                manpages["ps"] = "PS(1)\t\t\tUser Commands\n\nNAME\n\tps - list running processes\n\nSYNOPSIS\n\tps\n\nDESCRIPTION\n\tDisplay a snapshot of current simulated processes.";
                manpages["uname"] = "UNAME(1)\t\tUser Commands\n\nNAME\n\tuname - system information\n\nSYNOPSIS\n\tuname [-a | -r | -s | -m]\n\nDESCRIPTION\n\tPrint system information. -a for all, -r for release, -s for OS name, -m for architecture.";
                manpages["uptime"] = "UPTIME(1)\t\tUser Commands\n\nNAME\n\tuptime - system uptime\n\nSYNOPSIS\n\tuptime\n\nDESCRIPTION\n\tDisplay how long the system has been running.";
                manpages["cal"] = "CAL(1)\t\t\tUser Commands\n\nNAME\n\tcal - display calendar\n\nSYNOPSIS\n\tcal\n\nDESCRIPTION\n\tDisplay the current month's calendar with today highlighted.";
                manpages["rainbow"] = "RAINBOW(1)\t\tUser Commands\n\nNAME\n\trainbow - rainbow text\n\nSYNOPSIS\n\trainbow [message]\n\nDESCRIPTION\n\tPrint text with rainbow color cycling animation.";
                manpages["man"] = "MAN(1)\t\t\tUser Commands\n\nNAME\n\tman - display manual pages\n\nSYNOPSIS\n\tman <command>\n\nDESCRIPTION\n\tDisplay the manual page for a given command.";
                manpages["help"] = "HELP(1)\t\t\tUser Commands\n\nNAME\n\thelp - show available commands\n\nSYNOPSIS\n\thelp\n\nDESCRIPTION\n\tDisplay a categorized list of all available NoNameOS commands.";
                manpages["clear"] = "CLEAR(1)\t\tUser Commands\n\nNAME\n\tclear - clear terminal screen\n\nSYNOPSIS\n\tclear\n\nDESCRIPTION\n\tClear the terminal display.";
                manpages["exit"] = "EXIT(1)\t\t\tUser Commands\n\nNAME\n\texit - exit NoNameOS\n\nSYNOPSIS\n\texit\n\nDESCRIPTION\n\tExit the NoNameOS shell and return to the real terminal.";
                manpages["whoami"] = "WHOAMI(1)\t\tUser Commands\n\nNAME\n\twhoami - print current user\n\nSYNOPSIS\n\twhoami\n\nDESCRIPTION\n\tDisplay the current logged-in user name.";
                manpages["date"] = "DATE(1)\t\t\tUser Commands\n\nNAME\n\tdate - print system date and time\n\nSYNOPSIS\n\tdate\n\nDESCRIPTION\n\tDisplay the current system date and time.";
                manpages["history"] = "HISTORY(1)\t\tUser Commands\n\nNAME\n\thistory - command history\n\nSYNOPSIS\n\thistory\n\nDESCRIPTION\n\tDisplay the list of previously entered commands with line numbers.";
                manpages["pwd"] = "PWD(1)\t\t\tUser Commands\n\nNAME\n\tpwd - print working directory\n\nSYNOPSIS\n\tpwd\n\nDESCRIPTION\n\tPrint the absolute path of the current working directory.";
                manpages["tictactoe"] = "TICTACTOE(1)\t\tGame Commands\n\nNAME\n\ttictactoe - play tic-tac-toe against AI\n\nSYNOPSIS\n\ttictactoe (or ttt)\n\nDESCRIPTION\n\tPlay tic-tac-toe on a 3x3 grid against a minimax AI. You are X, AI is O.";
                manpages["ttt"] = "TTT(1)\t\t\tGame Commands\n\nNAME\n\tttt - alias for tictactoe\n\nSYNOPSIS\n\tttt\n\nDESCRIPTION\n\tSame as tictactoe.";
                manpages["hangman"] = "HANGMAN(1)\t\tGame Commands\n\nNAME\n\thangman - classic word guessing game\n\nSYNOPSIS\n\thangman\n\nDESCRIPTION\n\tGuess letters to reveal a hidden word before the stick figure is complete.";
                manpages["rps"] = "RPS(1)\t\t\tGame Commands\n\nNAME\n\trps - rock paper scissors best of 7\n\nSYNOPSIS\n\trps\n\nDESCRIPTION\n\tPlay Rock Paper Scissors against the AI. First to 4 wins the series.";
                manpages["yes"] = "YES(1)\t\t\tUser Commands\n\nNAME\n\tyes - output text repeatedly\n\nSYNOPSIS\n\tyes [text]\n\nDESCRIPTION\n\tPrint text repeatedly (100 times) to stdout.";
                manpages["env"] = "ENV(1)\t\t\tUser Commands\n\nNAME\n\tenv - print environment variables\n\nSYNOPSIS\n\tenv\n\nDESCRIPTION\n\tDisplay the current simulated environment variables.";
                manpages["hostname"] = "HOSTNAME(1)\t\tUser Commands\n\nNAME\n\thostname - print system hostname\n\nSYNOPSIS\n\thostname\n\nDESCRIPTION\n\tDisplay the system's hostname.";
                manpages["sleep"] = "SLEEP(1)\t\tUser Commands\n\nNAME\n\tsleep - delay execution\n\nSYNOPSIS\n\tsleep <seconds>\n\nDESCRIPTION\n\tPause the shell for the specified number of seconds (max 30).";
                manpages["which"] = "WHICH(1)\t\tUser Commands\n\nNAME\n\twhich - locate a command\n\nSYNOPSIS\n\twhich <command>\n\nDESCRIPTION\n\tShow the full path of a command if it exists in the shell.";
                manpages["alias"] = "ALIAS(1)\t\tUser Commands\n\nNAME\n\talias - show command aliases\n\nSYNOPSIS\n\talias\n\nDESCRIPTION\n\tDisplay defined command aliases (ll, .., ttt).";
                manpages["users"] = "USERS(1)\t\tUser Commands\n\nNAME\n\tusers - show logged in users\n\nSYNOPSIS\n\tusers\n\nDESCRIPTION\n\tDisplay the currently logged-in users.";
                manpages["banner"] = "BANNER(1)\t\tUser Commands\n\nNAME\n\tbanner - print ASCII banner\n\nSYNOPSIS\n\tbanner [text]\n\nDESCRIPTION\n\tDisplay a large ASCII banner with the given text.";
                manpages["fortune"] = "FORTUNE(1)\t\tUser Commands\n\nNAME\n\tfortune - random quote\n\nSYNOPSIS\n\tfortune\n\nDESCRIPTION\n\tDisplay a random programming quote.";
                manpages["factor"] = "FACTOR(1)\t\tUser Commands\n\nNAME\n\tfactor - factorize a number\n\nSYNOPSIS\n\tfactor <number>\n\nDESCRIPTION\n\tDisplay the prime factors of a positive integer.";
                manpages["shuf"] = "SHUF(1)\t\t\tUser Commands\n\nNAME\n\tshuf - shuffle text\n\nSYNOPSIS\n\tshuf <text>\n\nDESCRIPTION\n\tRandomly shuffle the characters of the given text.";
                manpages["head"] = "HEAD(1)\t\t\tUser Commands\n\nNAME\n\thead - display first lines of a file\n\nSYNOPSIS\n\thead <file>\n\nDESCRIPTION\n\tDisplay the first 10 lines of a VFS file.";
                manpages["tail"] = "TAIL(1)\t\t\tUser Commands\n\nNAME\n\ttail - display last lines of a file\n\nSYNOPSIS\n\ttail <file>\n\nDESCRIPTION\n\tDisplay the last 10 lines of a VFS file.";
                manpages["sort"] = "SORT(1)\t\t\tUser Commands\n\nNAME\n\tsort - sort file contents\n\nSYNOPSIS\n\tsort <file>\n\nDESCRIPTION\n\tSort the lines of a VFS file alphabetically.";
                manpages["wc"] = "WC(1)\t\t\tUser Commands\n\nNAME\n\twc - count lines, words, and characters\n\nSYNOPSIS\n\twc <file>\n\nDESCRIPTION\n\tDisplay line, word, and character counts for a VFS file.";
                manpages["tee"] = "TEE(1)\t\t\tUser Commands\n\nNAME\n\ttee - write to file and display\n\nSYNOPSIS\n\ttee <file> <text>\n\nDESCRIPTION\n\tWrite text to a VFS file and also display it on stdout.";
                manpages["cp"] = "CP(1)\t\t\tUser Commands\n\nNAME\n\tcp - copy files\n\nSYNOPSIS\n\tcp <source> <dest>\n\nDESCRIPTION\n\tCopy a VFS file from source to destination.";
                manpages["mv"] = "MV(1)\t\t\tUser Commands\n\nNAME\n\tmv - move or rename files\n\nSYNOPSIS\n\tmv <source> <dest>\n\nDESCRIPTION\n\tMove or rename a file or directory in the VFS.";
                manpages["chmod"] = "CHMOD(1)\t\tUser Commands\n\nNAME\n\tchmod - change file permissions\n\nSYNOPSIS\n\tchmod <mode> <file>\n\nDESCRIPTION\n\tNoNameOS VFS uses simulated permissions (rw-r--r--).";
                manpages["su"] = "SU(1)\t\t\tUser Commands\n\nNAME\n\tsu - switch user\n\nSYNOPSIS\n\tsu <user>\n\nDESCRIPTION\n\tSwitch to another user. Available: root, user.";

                if (manpages.find(args) != manpages.end()) {
                    cout << manpages[args] << "\n";
                } else {
                    cout << "No manual entry for " << args << "\n";
                }
            }
        }
        else if (cmd == "touch") {
            // Create an empty file node in the VFS (no-op if it already exists)
            if (!args.empty()) {
                string fullpath = current_dir + args;
                if (file_system.find(fullpath) == file_system.end()) {
                    file_system[fullpath] = FSNode(false, "");
                }
            }
        }
        // --- GAMES ---
        else if (cmd == "guess") {
            // Guess the Number: pick a random 1-100 target and loop until the user guesses it
            cout << "\033[33m--- Guess the Number ---\033[0m\n";
            cout << "I'm thinking of a number between 1 and 100.\n";
            srand(time(nullptr));
            int target = rand() % 100 + 1;
            int attempts = 0;
            while (true) {
                cout << "Your guess: ";
                string line;
                getline(cin, line);
                int guess = 0;
                bool valid = true;
                for (char c : line) {
                    if (c >= '0' && c <= '9') guess = guess * 10 + (c - '0');
                    else { valid = false; break; }
                }
                if (!valid || guess < 1 || guess > 100) {
                    cout << "Enter a number 1-100.\n";
                    continue;
                }
                attempts++;
                if (guess == target) {
                    cout << "\033[32mCorrect! Got it in " << attempts << " attempts.\033[0m\n";
                    break;
                } else if (guess < target) {
                    cout << "Too low!\n";
                } else {
                    cout << "Too high!\n";
                }
            }
        }
        else if (cmd == "trivia") {
            // Trivia Quiz: 5 multiple-choice questions about computers and technology
            cout << "\033[33m--- Trivia Quiz ---\033[0m\n";
            struct Question { string q; vector<string> opts; int correct; };
            vector<Question> questions;
            questions.push_back({"What language is NoNameOS written in?", {"C++", "Python", "Rust", "Java"}, 0});
            questions.push_back({"What does VFS stand for?", {"Virtual File System", "Very Fast Server", "Video File Storage", "None"}, 0});
            questions.push_back({"How many legs does a cow have?", {"4", "2", "6", "8"}, 0});
            questions.push_back({"What does CPU stand for?", {"Central Processing Unit", "Computer Power Unit", "Central Program Utility", "Core Processing Unit"}, 0});
            questions.push_back({"What year was C++ created?", {"1979", "1990", "2001", "1965"}, 0});
            srand(time(nullptr));
            int score = 0;
            for (int i = 0; i < 5; i++) {
                int idx = i;
                cout << "\nQ" << (i+1) << ": " << questions[idx].q << "\n";
                for (int j = 0; j < 4; j++) {
                    cout << "  " << (j+1) << ". " << questions[idx].opts[j] << "\n";
                }
                cout << "Answer (1-4): ";
                string line;
                getline(cin, line);
                int ans = 0;
                for (char c : line) {
                    if (c >= '0' && c <= '9') ans = ans * 10 + (c - '0');
                }
                if (ans == questions[idx].correct + 1) {
                    cout << "\033[32mCorrect!\033[0m\n";
                    score++;
                } else {
                    cout << "\033[31mWrong! Answer was " << (questions[idx].correct + 1) << "\033[0m\n";
                }
            }
            cout << "\nScore: " << score << "/5\n";
        }
        else if (cmd == "adventure") {
            // Text RPG dungeon crawler: explore, collect gold, fight monsters, manage HP
            cout << "\033[33m--- The Dungeon of NoNameOS ---\033[0m\n\n";
            cout << "You awaken in a dark dungeon.\n";
            cout << "A faint glow comes from two paths.\n\n";
            int hp = 100;
            int gold = 0;
            bool running = true;
            while (running) {
                cout << "\033[1;31mHP: " << hp << "\033[0m  \033[1;33mGold: " << gold << "\033[0m\n";
                cout << "What do you do? [left / right / rest / quit]\n> ";
                string choice;
                getline(cin, choice);
                if (choice == "left") {
                    int event = rand() % 3;
                    if (event == 0) {
                        cout << "\033[32mYou found a treasure chest! +25 gold\033[0m\n";
                        gold += 25;
                    } else if (event == 1) {
                        int dmg = 10 + rand() % 20;
                        cout << "\033[31mA slime attacks! -" << dmg << " HP\033[0m\n";
                        hp -= dmg;
                    } else {
                        cout << "It's a dead end. Nothing here.\n";
                    }
                } else if (choice == "right") {
                    int event = rand() % 3;
                    if (event == 0) {
                        cout << "\033[32mYou found a health potion! +30 HP\033[0m\n";
                        hp += 30;
                    } else if (event == 1) {
                        int dmg = 15 + rand() % 25;
                        cout << "\033[31mA skeleton strikes! -" << dmg << " HP\033[0m\n";
                        hp -= dmg;
                    } else {
                        int found = 10 + rand() % 20;
                        cout << "\033[33mYou found " << found << " gold on the ground!\033[0m\n";
                        gold += found;
                    }
                } else if (choice == "rest") {
                    int heal = 10 + rand() % 15;
                    cout << "\033[36mYou rest and recover " << heal << " HP.\033[0m\n";
                    hp += heal;
                } else if (choice == "quit") {
                    running = false;
                } else {
                    cout << "Invalid choice.\n";
                }
                if (hp <= 0) {
                    cout << "\n\033[31mYou have been defeated... Game Over.\033[0m\n";
                    cout << "Final gold: " << gold << "\n";
                    running = false;
                }
            }
            if (hp > 0) cout << "\nYou escaped with " << gold << " gold! GG!\n";
        }
        else if (cmd == "snake") {
            play_snake();
        }
        else if (cmd == "minesweeper") {
            play_minesweeper();
        }
        else if (cmd == "tictactoe" || cmd == "ttt") {
            play_tictactoe();
        }
        else if (cmd == "hangman") {
            play_hangman();
        }
        else if (cmd == "rps") {
            play_rps();
        }
        // --- SYSTEM TOOLS ---
        else if (cmd == "nano") {
            // Built-in line-by-line text editor that writes content back to the VFS
            if (args.empty()) {
                cout << "Usage: nano <filename>\n";
            } else {
                string fullpath = current_dir + args;
                cout << "\033[33m--- nano: " << args << " ---\033[0m\n";
                cout << "Type content line by line. Enter an empty line to save.\n";
                string content;
                while (true) {
                    cout << "> ";
                    string line;
                    getline(cin, line);
                    if (line.empty()) break;
                    if (!content.empty()) content += "\n";
                    content += line;
                }
                file_system[fullpath] = FSNode(false, content);
                cout << "Saved to " << args << " (" << content.length() << " bytes)\n";
            }
        }
        else if (cmd == "calc") {
            // Simple arithmetic calculator supporting +, -, *, and / on integer/decimal expressions
            if (args.empty()) {
                cout << "Usage: calc <expression>  (e.g. calc 2+3*4)\n";
            } else {
                // Simple calculator: parse numbers and operators (+ - * /)
                // Supports: number op number op number ...
                vector<double> nums;
                vector<char> ops;
                double current_num = 0;
                bool has_num = false;
                for (size_t i = 0; i < args.length(); i++) {
                    char c = args[i];
                    if (c >= '0' && c <= '9') {
                        current_num = current_num * 10 + (c - '0');
                        has_num = true;
                    } else if (c == '+' || c == '-' || c == '*' || c == '/') {
                        if (has_num) { nums.push_back(current_num); current_num = 0; has_num = false; }
                        ops.push_back(c);
                    }
                }
                if (has_num) nums.push_back(current_num);
                if (nums.size() < 2 || nums.size() != ops.size() + 1) {
                    cout << "Error: invalid expression.\n";
                } else {
                    // First pass: * and /
                    vector<double> pn = {nums[0]};
                    vector<char> po;
                    for (size_t i = 0; i < ops.size(); i++) {
                        if (ops[i] == '*' || ops[i] == '/') {
                            double last = pn.back(); pn.pop_back();
                            if (ops[i] == '*') pn.push_back(last * nums[i+1]);
                            else {
                                if (nums[i+1] == 0) { cout << "Error: division by zero.\n"; break; }
                                pn.push_back(last / nums[i+1]);
                            }
                        } else {
                            pn.push_back(nums[i+1]);
                            po.push_back(ops[i]);
                        }
                    }
                    double result = pn[0];
                    for (size_t i = 0; i < po.size(); i++) {
                        if (po[i] == '+') result += pn[i+1];
                        else result -= pn[i+1];
                    }
                    cout << "= " << result << "\n";
                }
            }
        }
        else cout << "command not found: " << cmd << "\n";
    }
    return 0;
}
