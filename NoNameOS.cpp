#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

struct FSNode {
    bool is_dir;
    string content;
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
    cout << "[    0.000000] Booting NoNameOS Core v0.3.1...\n";
    boot_delay(300);
    cout << "[    1.102304] Loading Termios Input Hijacker...\n";
    boot_delay(300);
    cout << "[    1.500000] AsciiDash Engine Ready.\n";
    boot_delay(200);
    cout << "[    1.700000] Loading Bovine Speech Synthesizer...\n\n";

    map<string, FSNode> file_system; 
    file_system["/"] = {true, ""};
    
    // Default custom level mapping
    file_system["/geometry/"] = {true, ""};
    file_system["/geometry/jumper.gmd"] = {false, "_______^_______^^_______^___^^^___"};

    string current_user = "root";
    string current_dir = "/"; 
    string input;

    while (true) {
        cout << current_user << "@NoNameOS:" << current_dir << "# ";
        getline(cin, input);

        if (input.empty()) continue;
        auto [cmd, args] = parse_command(input);

        if (cmd == "help") {
            cout << "NoNameOS v0.3.1 Commands: ls, cd, mkdir, touch, cat, echo, rm, clear, exit\n";
            cout << "\033[33m  play [file]  - Launch AsciiDash Engine (reads .gmd files)\033[0m\n";
            cout << "\033[36m  cowsay [msg] - Generates an ASCII cow saying your message\033[0m\n";
        } 
        else if (cmd == "ls") {
            bool empty = true;
            for (auto const& [path, node] : file_system) {
                if (path == current_dir) continue; 
                if (path.rfind(current_dir, 0) == 0) { 
                    string relative = path.substr(current_dir.length());
                    size_t slash_pos = relative.find('/');
                    if (slash_pos == string::npos || (node.is_dir && slash_pos == relative.length() - 1)) {
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
            if (!args.empty()) file_system[current_dir + args + "/"] = {true, ""};
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
                file_system[current_dir + filename] = {false, content};
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
            // Strip quotes so the speech bubble looks clean
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
        else cout << "command not found: " << cmd << "\n";
    }
    return 0;
}
