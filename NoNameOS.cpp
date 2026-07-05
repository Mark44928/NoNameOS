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
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

string get_timestamp() {
    time_t now = time(nullptr);
    tm* t = localtime(&now);
    char buf[20];
    strftime(buf, sizeof(buf), "%b %d %H:%M", t);
    return string(buf);
}

struct FSNode {
    bool is_dir;
    string content;
    size_t size;
    string created_at;
    FSNode() : is_dir(false), content(""), size(0), created_at("") {}
    FSNode(bool d, string c, size_t s = 0) : is_dir(d), content(c), size(c.length()), created_at(get_timestamp()) {}
};

void boot_delay(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}

pair<string, string> parse_command(const string& input) {
    size_t first_space = input.find(' ');
    if (first_space == string::npos) return {input, ""};
    return {input.substr(0, first_space), input.substr(first_space + 1)};
}

// POSIX Non-blocking keyboard hit detection
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
    cin.ignore(10000, '\n');
    cin.get();
}

int main() {
    cout << "\033[2J\033[1;1H";
    cout << "[    0.000000] Booting NoNameOS Core v0.4.0...\n";
    boot_delay(300);
    cout << "[    1.102304] Loading Termios Input Hijacker...\n";
    boot_delay(300);
    cout << "[    1.500000] AsciiDash Engine Ready.\n";
    boot_delay(200);
    cout << "[    1.700000] Loading Bovine Speech Synthesizer...\n";
    boot_delay(150);
    cout << "[    1.850000] Loading Game Engines (guess, trivia, adventure)...\n";
    boot_delay(150);
    cout << "[    2.000000] Loading System Tools (nano, calc)...\n\n";

    map<string, FSNode> file_system;
    file_system["/"] = FSNode(true, "");

    // Default custom level mapping
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
            cout << "NoNameOS v0.4.0 Commands:\n";
            cout << "\033[1;33m  Filesystem:\033[0m ls, ls -l, cd, mkdir, touch, cat, echo, rm, pwd, grep, find\n";
            cout << "\033[1;33m  System:\033[0m    whoami, date, history, clear, cfetch, exit\n";
            cout << "\033[1;33m  Tools:\033[0m     nano <file>, calc <expr>, cowsay [msg]\n";
            cout << "\033[1;33m  Games:\033[0m     play [file], guess, trivia, adventure\n";
        } 
        else if (cmd == "ls") {
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
            if (!args.empty()) file_system[current_dir + args + "/"] = FSNode(true, "");
        }
        else if (cmd == "cd") {
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
            if (file_system.find(current_dir + args) != file_system.end()) cout << file_system[current_dir + args].content << "\n";
            else cout << "File not found.\n";
        }
        else if (cmd == "rm") {
            file_system.erase(current_dir + args);
            file_system.erase(current_dir + args + "/");
        }
        else if (cmd == "clear") cout << "\033[2J\033[1;1H";
        else if (cmd == "exit") break;
        else if (cmd == "play") {
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
        // THE NEW COWSAY COMMAND
        else if (cmd == "cowsay") {
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
            cout << current_dir << "\n";
        }
        else if (cmd == "whoami") {
            cout << current_user << "\n";
        }
        else if (cmd == "date") {
            time_t now = time(nullptr);
            tm* t = localtime(&now);
            char buf[64];
            strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y", t);
            cout << buf << "\n";
        }
        else if (cmd == "history") {
            for (size_t i = 0; i < cmd_history.size(); i++) {
                cout << "  " << (i + 1) << "  " << cmd_history[i] << "\n";
            }
        }
        else if (cmd == "grep") {
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
            cout << "\033[1;33mOS:\033[0m       NoNameOS v0.4.0\n";
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
        else if (cmd == "touch") {
            if (!args.empty()) {
                string fullpath = current_dir + args;
                if (file_system.find(fullpath) == file_system.end()) {
                    file_system[fullpath] = FSNode(false, "");
                }
            }
        }
        // --- GAMES ---
        else if (cmd == "guess") {
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
        // --- SYSTEM TOOLS ---
        else if (cmd == "nano") {
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
                    if (c >= '0' && c <= '9' || c == '.') {
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
