// Core headers: I/O streams, string manipulation, containers, utilities, and C time
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <random>
#include <functional>
// POSIX headers for terminal control (raw input) and non-blocking I/O detection
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

static const auto program_start = chrono::steady_clock::now();
const string VERSION = "v0.7.0";

constexpr int CIN_IGNORE_MAX = 10000;
constexpr int GAME_SPEED_MS = 150;
constexpr int SNAKE_W = 20;
constexpr int SNAKE_H = 15;
constexpr int MINESWEEPER_W = 10;
constexpr int MINESWEEPER_H = 10;
constexpr int MINESWEEPER_MINES = 12;
constexpr int TICTACTOE_SIZE = 9;
constexpr int HANGMAN_ATTEMPTS = 6;
constexpr int RPS_WIN_TARGET = 4;
constexpr int SLEEP_MAX_SEC = 30;
constexpr int YES_COUNT = 100;
constexpr int HEAD_TAIL_LINES = 10;
constexpr int TRIVIA_COUNT = 5;
constexpr int ASCIIDASH_PADDING = 10;
constexpr int ASCIIDASH_WINDOW = 20;
constexpr int JUMP_FRAMES = 3;

struct Question {
    string q;
    vector<string> opts;
    int correct;
};

struct TerminalGuard {
    struct termios oldt;
    int oldf;
    bool active = false;
    TerminalGuard() {
        active = (tcgetattr(STDIN_FILENO, &oldt) == 0);
        if (active) {
            struct termios newt = oldt;
            newt.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &newt);
            oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
            fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        }
    }
    ~TerminalGuard() {
        if (active) {
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            fcntl(STDIN_FILENO, F_SETFL, oldf);
        }
    }
};

// Generate a human-readable timestamp string (e.g. "Jul 05 09:53") for VFS metadata
string get_timestamp() {
    time_t now = time(nullptr);
    tm* t = localtime(&now);
    char buf[20];
    strftime(buf, sizeof(buf), "%b %d %H:%M", t);
    return string(buf);
}

struct FSNode {
    bool is_dir;
    bool is_link;
    string content;
    size_t size;
    string created_at;
    string mode;
    string link_target;

    FSNode() : is_dir(false), is_link(false), content(""), size(0), created_at(""), mode("rw-r--r--"), link_target("") {}
    FSNode(bool d, string c, size_t = 0) : is_dir(d), is_link(false), content(c), size(c.length()), created_at(get_timestamp()),
        mode(d ? "rwxr-xr-x" : "rw-r--r--"), link_target("") {}
};

string resolved_path(map<string,FSNode>& fs, const string& path, int depth = 0) {
    if (depth > 8) return path;
    if (fs.find(path) != fs.end() && fs[path].is_link)
        return resolved_path(fs, fs[path].link_target, depth + 1);
    return path;
}

void pager(const string& content) {
    istringstream ss(content);
    string line;
    vector<string> lines;
    while (getline(ss, line)) lines.push_back(line);
    size_t i = 0;
    while (i < lines.size()) {
        for (int n = 0; n < 20 && i < lines.size(); n++, i++)
            cout << lines[i] << "\n";
        if (i < lines.size()) {
            cout << "\033[2m-- More -- (Enter=next, q=quit)\033[0m";
            string resp;
            getline(cin, resp);
            if (resp == "q" || resp == "Q") break;
        }
    }
}

string help_text(const string& cmd) {
    static const map<string,string> ht = {
        {"help","show help or describe a command"},{"man","display manual page"},
        {"ls","list directory contents"},{"cd","change directory"},
        {"mkdir","create directory"},{"touch","create empty file"},
        {"cat","print file contents"},{"echo","write content to file"},
        {"rm","remove file or directory"},{"cp","copy file"},
        {"mv","move or rename"},{"clear","clear screen"},
        {"exit","exit NoNameOS"},{"pwd","print working directory"},
        {"whoami","print current user"},{"date","print date and time"},
        {"history","show command history"},{"grep","search file for pattern"},
        {"find","find files by name"},{"locate","find files by pattern"},
        {"cfetch","system info"},{"ps","process list"},
        {"uname","system information"},{"uptime","system uptime"},
        {"cal","calendar"},{"rainbow","rainbow text"},
        {"yes","repeat text"},{"env","environment variables"},
        {"hostname","print hostname"},{"sleep","delay execution"},
        {"which","locate command"},{"alias","manage aliases"},
        {"unalias","remove alias"},{"users","list users"},
        {"banner","ASCII banner"},{"fortune","random quote"},
        {"factor","factorize number"},{"shuf","shuffle text"},
        {"head","first 10 lines"},{"tail","last 10 lines"},
        {"sort","sort lines"},{"wc","count lines/words/chars"},
        {"tee","write and display"},{"nano","line editor"},
        {"calc","calculator"},{"bc","better calculator"},
        {"play","AsciiDash game"},{"guess","number guessing game"},
        {"trivia","trivia quiz"},{"adventure","dungeon RPG"},
        {"snake","snake game"},{"minesweeper","minesweeper"},
        {"tictactoe","tic-tac-toe vs AI"},{"ttt","tic-tac-toe shortcut"},
        {"hangman","hangman game"},{"rps","rock paper scissors"},
        {"2048","2048 puzzle"},{"typing","typing speed test"},
        {"reaction","reaction time test"},{"nummem","number memory game"},
        {"tree","directory tree"},{"watch","run command repeatedly"},
        {"ping","simulated ping"},{"top","process snapshot"},
        {"df","VFS disk usage"},{"seq","print number sequence"},
        {"printenv","print environment"},{"todo","task manager"},
        {"notes","note manager"},{"stopwatch","stopwatch"},
        {"timer","countdown timer"},{"lolcat","rainbow gradient text"},
        {"cowsay","ASCII cow"},{"sl","steam locomotive"},
        {"train","steam locomotive"},{"su","switch user"},
        {"chmod","change permissions"},{"who","show logged in users"},
        {"useradd","add user"},{"userdel","remove user"},
        {"rev","reverse each line"},{"tr","replace characters"},
        {"cut","extract first N chars"},{"paste","merge files"},
        {"uniq","remove duplicate lines"},{"nl","number lines"},
        {"fold","wrap lines"},{"basename","strip directory"},
        {"dirname","extract directory"},{"free","memory usage"},
        {"dmesg","boot messages"},{"lscpu","CPU info"},
        {"lsusb","USB devices"},{"arch","print architecture"},
        {"nproc","number of CPUs"},{"ln","create symlink"},
        {"trash","manage trash"},{"du","disk usage"},
        {"pom","pomodoro timer"},{"alarm","set alarm"}
    };
    auto it = ht.find(cmd);
    return it != ht.end() ? it->second : "no description";
}

void boot_delay(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}

// Split raw input into a command token and its arguments string
pair<string, string> parse_command(const string& input) {
    size_t first_space = input.find(' ');
    if (first_space == string::npos) return {input, ""};
    return {input.substr(0, first_space), input.substr(first_space + 1)};
}

int kbhit() {
    TerminalGuard guard;
    int ch = getchar();
    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

string vfs_read(const string& path, map<string,FSNode>& fs, const string& cdir) {
    if (path.find("..") != string::npos) return "";
    string fp = resolved_path(fs, cdir + path);
    if (fs.find(fp) != fs.end() && !fs[fp].is_dir) return fs[fp].content;
    return "";
}

const set<string> ALL_COMMANDS = {"ls","cd","mkdir","touch","cat","echo","rm","clear","exit","play","cowsay",
    "pwd","whoami","date","history","grep","find","cfetch","ps","uname","uptime","cal","rainbow",
    "man","help","nano","calc","guess","trivia","adventure","snake","minesweeper","tictactoe","ttt",
    "hangman","rps","yes","env","hostname","sleep","which","head","tail","sort","wc","tee","alias",
    "users","banner","fortune","factor","shuf","cp","mv","chmod","su","unalias","tree","watch",
    "ping","top","df","seq","printenv","todo","notes","stopwatch","timer","lolcat","sl",
    "train","who","useradd","userdel","2048","typing","reaction","nummem","rev","tr","cut",
    "paste","uniq","nl","fold","basename","dirname","free","dmesg","lscpu","lsusb","arch",
    "nproc","du","locate","pom","alarm","bc","ln","trash"};

size_t edit_dist(const string& a, const string& b) {
    size_t n = a.size(), m = b.size();
    vector<size_t> prev(m + 1), cur(m + 1);
    for (size_t j = 0; j <= m; j++) prev[j] = j;
    for (size_t i = 1; i <= n; i++) {
        cur[0] = i;
        for (size_t j = 1; j <= m; j++)
            cur[j] = min({prev[j] + 1, cur[j-1] + 1, prev[j-1] + (a[i-1] == b[j-1] ? 0 : 1)});
        swap(prev, cur);
    }
    return prev[m];
}

string closest_cmd(const string& cmd) {
    string best; size_t best_d = 4;
    for (const auto& c : ALL_COMMANDS) {
        size_t d = edit_dist(cmd, c);
        if (d < best_d) { best_d = d; best = c; }
    }
    return best;
}

// --- TEXT PROCESSING TOOLS ---
void cmd_rev(const string& args, map<string,FSNode>& fs, const string& cdir) {
    string c = vfs_read(args, fs, cdir);
    if (c.empty()) { cout << "error: file not found.\n"; return; }
    istringstream ss(c); string line;
    while (getline(ss, line)) { reverse(line.begin(), line.end()); cout << line << "\n"; }
}

void cmd_tr(const string& args, map<string,FSNode>& fs, const string& cdir) {
    istringstream ss(args); string fn, f, r;
    ss >> fn >> f >> r;
    if (fn.empty() || f.empty() || r.empty()) { cout << "Usage: tr <file> <find> <replace>\n"; return; }
    string c = vfs_read(fn, fs, cdir);
    if (c.empty()) { cout << "error: file not found.\n"; return; }
    for (char& ch : c) if (ch == f[0]) ch = r[0];
    cout << c << "\n";
}

void cmd_cut(const string& args, map<string,FSNode>& fs, const string& cdir) {
    istringstream ss(args); string fn; int n;
    ss >> fn >> n;
    if (fn.empty() || n <= 0) { cout << "Usage: cut <file> <n>\n"; return; }
    string c = vfs_read(fn, fs, cdir);
    if (c.empty()) { cout << "error: file not found.\n"; return; }
    istringstream is(c); string line;
    while (getline(is, line)) cout << line.substr(0, (size_t)n) << "\n";
}

void cmd_paste(const string& args, map<string,FSNode>& fs, const string& cdir) {
    istringstream ss(args); string f1, f2;
    ss >> f1 >> f2;
    if (f1.empty() || f2.empty()) { cout << "Usage: paste <file1> <file2>\n"; return; }
    string c1 = vfs_read(f1, fs, cdir), c2 = vfs_read(f2, fs, cdir);
    if (c1.empty() || c2.empty()) { cout << "error: file not found.\n"; return; }
    vector<string> l1, l2; string line;
    { istringstream s1(c1); while (getline(s1, line)) l1.push_back(line); }
    { istringstream s2(c2); while (getline(s2, line)) l2.push_back(line); }
    for (size_t i = 0; i < max(l1.size(), l2.size()); i++) {
        if (i < l1.size()) cout << l1[i]; cout << "\t";
        if (i < l2.size()) cout << l2[i]; cout << "\n";
    }
}

void cmd_uniq(const string& args, map<string,FSNode>& fs, const string& cdir) {
    string c = vfs_read(args, fs, cdir);
    if (c.empty()) { cout << "error: file not found.\n"; return; }
    istringstream ss(c); string line, prev;
    while (getline(ss, line)) { if (line != prev) cout << line << "\n"; prev = line; }
}

void cmd_nl(const string& args, map<string,FSNode>& fs, const string& cdir) {
    string c = vfs_read(args, fs, cdir);
    if (c.empty()) { cout << "error: file not found.\n"; return; }
    istringstream ss(c); string line; int n = 1;
    while (getline(ss, line)) { cout << "  " << n << "\t" << line << "\n"; n++; }
}

void cmd_fold(const string& args, map<string,FSNode>& fs, const string& cdir) {
    istringstream ss(args); string fn; int n = 80;
    ss >> fn >> n;
    if (fn.empty()) { cout << "Usage: fold <file> [width]\n"; return; }
    string c = vfs_read(fn, fs, cdir);
    if (c.empty()) { cout << "error: file not found.\n"; return; }
    for (size_t i = 0; i < c.length(); i += (size_t)n) cout << c.substr(i, (size_t)n) << "\n";
}

void cmd_basename(const string& args) {
    if (args.empty()) { cout << "Usage: basename <path>\n"; return; }
    size_t pos = args.find_last_of('/');
    if (pos == string::npos) cout << args << "\n";
    else cout << args.substr(pos + 1) << "\n";
}

void cmd_dirname(const string& args) {
    if (args.empty()) { cout << "Usage: dirname <path>\n"; return; }
    size_t pos = args.find_last_of('/');
    if (pos == string::npos) cout << ".\n";
    else if (pos == 0) cout << "/\n";
    else cout << args.substr(0, pos) << "\n";
}

// --- SYSTEM TOOLS ---
void cmd_free() {
    cout << "              total        used        free\n";
    cout << "Mem:        32768        18234       14534\n";
    cout << "Swap:        8192          234        7958\n";
}

void cmd_dmesg() {
    vector<string> msgs = {
        "[    0.000000] NoNameOS " + VERSION + " booting on x86_64",
        "[    0.102304] CPU: NoNameCPU v1.0 @ 2.4GHz (4 cores)",
        "[    1.500000] Memory: 32768K available",
        "[    2.100000] VFS: Mounted root filesystem",
        "[    2.750000] Console: NonameSH terminal",
        "[    3.050000] System ready. User: root"
    };
    for (auto& m : msgs) cout << m << "\n";
}

void cmd_lscpu() {
    cout << "Architecture:        x86_64\n";
    cout << "CPU op-mode(s):     32-bit, 64-bit\n";
    cout << "Model name:         NoNameCPU v1.0\n";
    cout << "CPU(s):             4\n";
    cout << "CPU MHz:            2400.000\n";
    cout << "L1d cache:          32K\n";
    cout << "L1i cache:          32K\n";
    cout << "L2 cache:           256K\n";
    cout << "L3 cache:           4096K\n";
}

void cmd_lsusb() {
    cout << "Bus 001 Device 001: ID 1d6b:0001 NoName USB Keyboard\n";
    cout << "Bus 001 Device 002: ID 1d6b:0002 NoName USB Mouse\n";
    cout << "Bus 002 Device 001: ID 1d6b:0003 NoName Storage Device\n";
    cout << "Bus 002 Device 002: ID 1d6b:0004 NoName USB Hub\n";
}

void cmd_arch() { cout << "x86_64\n"; }
void cmd_nproc() { cout << "4\n"; }

// --- VFS ENHANCEMENTS ---
void cmd_du(const string& args, map<string,FSNode>& fs, const string& cdir) {
    string a = args;
    while (!a.empty() && a.back() == '/') a.pop_back();
    string dir = a.empty() ? cdir : cdir + a + "/";
    size_t total = 0;
    for (auto& [p, n] : fs) {
        if (p.rfind(dir, 0) == 0 && !n.is_dir) total += n.size;
    }
    cout << total << "\t" << (args.empty() ? "." : args) << "\n";
}

void cmd_locate(const string& args, map<string,FSNode>& fs) {
    if (args.empty()) { cout << "Usage: locate <pattern>\n"; return; }
    bool found = false;
    for (auto& [p, n] : fs) {
        if (p.find(args) != string::npos) { cout << p << "\n"; found = true; }
    }
    if (!found) cout << "error: no matches found.\n";
}

// --- PRODUCTIVITY ---
void cmd_pom() {
    const int FOCUS = 25, BREAK = 5, LONG_BREAK = 15;
    for (int cycle = 0; cycle < 4; cycle++) {
        cout << "\033[33mFocus round " << (cycle+1) << "/4\033[0m\n";
        for (int m = FOCUS; m >= 0; m--) {
            cout << "\r  " << (m < 10 ? " " : "") << m << ":00 remaining  [";
            int pos = (int)((float)(FOCUS - m) / FOCUS * 20);
            for (int i = 0; i < 20; i++) cout << (i < pos ? "\033[32m=\033[0m" : " ");
            cout << "]";
            cout.flush();
            this_thread::sleep_for(chrono::seconds(1));
        }
        cout << "\n\033[32mFocus complete!\033[0m\n";
        if (cycle < 3) {
            int blen = cycle == 2 ? LONG_BREAK : BREAK;
            cout << "\033[36mBreak for " << blen << " min\033[0m\n";
            for (int m = blen; m >= 0; m--) {
                cout << "\r  " << m << ":00  ";
                cout.flush();
                this_thread::sleep_for(chrono::seconds(1));
            }
            cout << "\n";
        }
    }
    cout << "\033[32mPomodoro complete! Great work.\033[0m\n";
}

void cmd_alarm(const string& args) {
    int sec = 0;
    for (char c : args) { if (c >= '0' && c <= '9') sec = sec * 10 + (c - '0'); }
    if (sec <= 0) { cout << "Usage: alarm <seconds>\n"; return; }
    for (int i = sec; i >= 0; i--) {
        cout << "\rAlarm in " << i << "s  ";
        cout.flush();
        if (i > 0) this_thread::sleep_for(chrono::seconds(1));
    }
    cout << "\n\a\033[31m*** ALARM! ***\033[0m\n";
}

void cmd_bc(const string& args, map<string,FSNode>&, const string&) {
    if (args.empty()) { cout << "Usage: bc <expression>\n"; return; }
    vector<double> nums;
    vector<char> ops;
    istringstream ss(args);
    double val; char op;
    if (ss >> val) {
        nums.push_back(val);
        while (ss >> op >> val) {
            if (op == '+' || op == '-' || op == '*' || op == '/' || op == '%' || op == '^') {
                ops.push_back(op); nums.push_back(val);
            } else { cout << "error: invalid operator.\n"; return; }
        }
    }
    if (nums.size() < 2) { cout << "error: need at least 2 values.\n"; return; }
    vector<double> pn = {nums[0]};
    vector<char> po;
    for (size_t i = 0; i < ops.size(); i++) {
        if (ops[i] == '*' || ops[i] == '/' || ops[i] == '%' || ops[i] == '^') {
            double last = pn.back(); pn.pop_back();
            if (ops[i] == '*') pn.push_back(last * nums[i+1]);
            else if (ops[i] == '/') { if (nums[i+1] == 0) { cout << "error: division by zero.\n"; return; } pn.push_back(last / nums[i+1]); }
            else if (ops[i] == '%') pn.push_back(fmod(last, nums[i+1]));
            else pn.push_back(pow(last, nums[i+1]));
        } else { pn.push_back(nums[i+1]); po.push_back(ops[i]); }
    }
    double result = pn[0];
    for (size_t i = 0; i < po.size(); i++) {
        if (po[i] == '+') result += pn[i+1]; else result -= pn[i+1];
    }
    cout << "= " << result << "\n";
}

// --- ASCIIDASH ENGINE ---
// A side-scrolling obstacle runner that renders frames using ANSI escape sequences
// Controls: SPACE or ENTER to jump over '^' obstacles
void play_asciidash(string map_data) {
    cout << "\033[2J\033[1;1H";
    cout << "INITIALIZING ASCIIDASH ENGINE...\n";
    boot_delay(1000);

    int player_y = 0;
    int jump_timer = 0;
    bool crashed = false;

    string pad(ASCIIDASH_PADDING, '_');
    map_data = pad + map_data + pad;

    for (size_t i = 0; i < map_data.length() - ASCIIDASH_PADDING / 2; i++) {
        if (kbhit()) {
            char c = getchar();
            if ((c == ' ' || c == '\n') && player_y == 0) {
                player_y = 1;
                jump_timer = JUMP_FRAMES;
            }
        }

        if (jump_timer > 0) {
            jump_timer--;
        } else {
            player_y = 0;
        }

        if (player_y == 0 && map_data[i + ASCIIDASH_PADDING / 2] == '^') {
            crashed = true;
            break;
        }

        cout << "\033[2J\033[1;1H";
        cout << "--- ASCIIDASH v1.0 --- (Press SPACE to Jump)\n\n";
        cout << (player_y == 1 ? "     ■\n" : "\n");
        cout << "     " << (player_y == 0 ? "■" : " ") << "\n";
        cout << map_data.substr(i, ASCIIDASH_WINDOW) << "\n";
        cout << "====================\n";

        this_thread::sleep_for(chrono::milliseconds(GAME_SPEED_MS));
    }

    while (kbhit()) (void)getchar();

    cout << "\n\n";
    if (crashed) {
        cout << "\033[31m[ x_x ] CRASHED! Attempt failed.\033[0m\n";
    } else {
        cout << "\033[32m[ ^_^ ] LEVEL COMPLETE! GG!\033[0m\n";
    }
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(CIN_IGNORE_MAX), '\n');
    cin.get();
}

void play_snake() {
    vector<pair<int,int>> snake = {{SNAKE_W/2, SNAKE_H/2}};
    int dx = 1, dy = 0;
    int food_x = rand() % SNAKE_W, food_y = rand() % SNAKE_H;
    int score = 0;
    bool game_over = false;

    while (!game_over) {
        if (kbhit()) {
            char c = getchar();
            if (c == 'w' && dy == 0) { dx = 0; dy = -1; }
            else if (c == 's' && dy == 0) { dx = 0; dy = 1; }
            else if (c == 'a' && dx == 0) { dx = -1; dy = 0; }
            else if (c == 'd' && dx == 0) { dx = 1; dy = 0; }
        }

        int nx = snake[0].first + dx;
        int ny = snake[0].second + dy;

        if (nx < 0 || nx >= SNAKE_W || ny < 0 || ny >= SNAKE_H) {
            game_over = true;
            break;
        }

        bool eating = (nx == food_x && ny == food_y);
        size_t check_len = eating ? snake.size() : snake.size() - 1;
        for (size_t i = 0; i < check_len; i++) {
            if (snake[i].first == nx && snake[i].second == ny) {
                game_over = true;
                break;
            }
        }
        if (game_over) break;

        snake.insert(snake.begin(), {nx, ny});

        if (eating) {
            score++;
            food_x = rand() % SNAKE_W;
            food_y = rand() % SNAKE_H;
        } else {
            snake.pop_back();
        }

        vector<vector<bool>> grid(SNAKE_H, vector<bool>(SNAKE_W, false));
        for (size_t i = 0; i < snake.size(); i++) {
            grid[snake[i].second][snake[i].first] = true;
        }

        cout << "\033[2J\033[1;1H";
        cout << "--- SNAKE v1.0 --- Score: " << score << " (WASD to move)\n\n";
        for (int y = 0; y < SNAKE_H; y++) {
            cout << "  ";
            for (int x = 0; x < SNAKE_W; x++) {
                if (grid[y][x]) {
                    cout << (snake[0].first == x && snake[0].second == y ? "\033[32mO\033[0m" : "\033[36mo\033[0m");
                } else if (x == food_x && y == food_y) {
                    cout << "\033[31m*\033[0m";
                } else {
                    cout << ".";
                }
            }
            cout << "\n";
        }
        cout << "\n  Score: " << score << "  |  Press Ctrl+C to quit\n";
        this_thread::sleep_for(chrono::milliseconds(GAME_SPEED_MS));
    }

    while (kbhit()) (void)getchar();
    cout << "\n\n\033[31m[ GAME OVER ] Final Score: " << score << "\033[0m\n";
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(CIN_IGNORE_MAX), '\n');
    cin.get();
}

void play_minesweeper() {
    vector<vector<char>> board(MINESWEEPER_H, vector<char>(MINESWEEPER_W, '.'));
    vector<vector<bool>> revealed(MINESWEEPER_H, vector<bool>(MINESWEEPER_W, false));
    vector<vector<bool>> mines(MINESWEEPER_H, vector<bool>(MINESWEEPER_W, false));
    int remaining = MINESWEEPER_W * MINESWEEPER_H - MINESWEEPER_MINES;
    bool game_over = false;
    bool won = false;

    int placed = 0;
    while (placed < MINESWEEPER_MINES) {
        int mx = rand() % MINESWEEPER_W, my = rand() % MINESWEEPER_H;
        if (!mines[my][mx]) { mines[my][mx] = true; placed++; }
    }

    auto count_adj = [&](int x, int y) {
        int cnt = 0;
        for (int dy = -1; dy <= 1; dy++)
            for (int dx = -1; dx <= 1; dx++) {
                int nx = x + dx, ny = y + dy;
                if (nx >= 0 && nx < MINESWEEPER_W && ny >= 0 && ny < MINESWEEPER_H && mines[ny][nx]) cnt++;
            }
        return cnt;
    };

    function<void(int,int)> reveal = [&](int x, int y) {
        if (x < 0 || x >= MINESWEEPER_W || y < 0 || y >= MINESWEEPER_H || revealed[y][x]) return;
        if (board[y][x] == 'F') return;
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
        cout << "--- MINESWEEPER v1.0 --- Mines: " << MINESWEEPER_MINES << "\n\n";
        cout << "    ";
        for (int x = 0; x < MINESWEEPER_W; x++) cout << x << " ";
        cout << "\n   +";
        for (int x = 0; x < MINESWEEPER_W; x++) cout << "--";
        cout << "\n";
        for (int y = 0; y < MINESWEEPER_H; y++) {
            cout << " " << y << " |";
            for (int x = 0; x < MINESWEEPER_W; x++) {
                if (!revealed[y][x]) {
                    if (board[y][x] == 'F') cout << "\033[33mF\033[0m ";
                    else cout << "\033[90m#\033[0m ";
                } else if (mines[y][x]) cout << "\033[31mX\033[0m ";
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
        if (!getline(cin, line)) break;
        if (line.empty()) continue;
        istringstream iss(line);
        string cmd; int fx, fy;
        iss >> cmd;
        if (cmd == "f" && (iss >> fx >> fy)) {
            if (fx >= 0 && fx < MINESWEEPER_W && fy >= 0 && fy < MINESWEEPER_H && !revealed[fy][fx]) {
                board[fy][fx] = (board[fy][fx] == 'F') ? '.' : 'F';
            }
        } else {
            istringstream iss2(line);
            if (iss2 >> fx >> fy) {
                if (fx >= 0 && fx < MINESWEEPER_W && fy >= 0 && fy < MINESWEEPER_H) {
                    if (mines[fy][fx]) {
                        game_over = true;
                        for (int y = 0; y < MINESWEEPER_H; y++)
                            for (int x = 0; x < MINESWEEPER_W; x++)
                                if (mines[y][x]) revealed[y][x] = true;
                    } else {
                        reveal(fx, fy);
                    }
                }
            }
        }
    }

    cout << "\033[2J\033[1;1H";
    cout << "--- MINESWEEPER v1.0 ---\n\n";
    cout << "    ";
    for (int x = 0; x < MINESWEEPER_W; x++) cout << x << " ";
    cout << "\n   +";
    for (int x = 0; x < MINESWEEPER_W; x++) cout << "--";
    cout << "\n";
    for (int y = 0; y < MINESWEEPER_H; y++) {
        cout << " " << y << " |";
        for (int x = 0; x < MINESWEEPER_W; x++) {
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
    cin.ignore(static_cast<std::streamsize>(CIN_IGNORE_MAX), '\n');
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

    function<int(vector<char>&, char)> minimax = [&](vector<char>& brd, char player) -> int {
        if (check_win('O')) return 10;
        if (check_win('X')) return -10;
        if (is_draw()) return 0;
        int best = (player == 'O') ? -1000 : 1000;
        for (int i = 0; i < TICTACTOE_SIZE; i++) {
            if (brd[i] == ' ') {
                brd[i] = player;
                int score = minimax(brd, (player == 'O') ? 'X' : 'O');
                brd[i] = ' ';
                best = (player == 'O') ? max(best, score) : min(best, score);
            }
        }
        return best;
    };

    auto ai_move = [&]() {
        int best_score = -1000, best_move = -1;
        for (int i = 0; i < TICTACTOE_SIZE; i++) {
            if (board[i] == ' ') {
                board[i] = 'O';
                int score = minimax(board, 'X');
                board[i] = ' ';
                if (score > best_score) { best_score = score; best_move = i; }
            }
        }
        if (best_move != -1) board[best_move] = 'O';
    };

    print_board();
    while (true) {
        if (turn == 0) {
            cout << "Your move (1-9): ";
            string line; getline(cin, line);
            if (line.empty()) continue;
            int pos = line[0] - '1';
            if (pos < 0 || pos > 8 || board[pos] != ' ') {
                cout << "\033[31merror:\033[0m invalid move.\n";
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
    cin.ignore(static_cast<std::streamsize>(CIN_IGNORE_MAX), '\n');
    cin.get();
}

void play_hangman() {
    vector<string> words = {"computer", "keyboard", "monitor", "programming", "terminal",
                            "software", "hardware", "network", "internet", "algorithm"};
    string word = words[rand() % words.size()];
    string guessed(word.length(), '_');
    int attempts = HANGMAN_ATTEMPTS;
    vector<char> wrong;

    while (attempts > 0 && guessed.find('_') != string::npos) {
        cout << "\033[2J\033[1;1H";
        cout << "--- HANGMAN v1.0 ---\n\n";

        cout << "  +---+\n";
        cout << "  |   " << (attempts < HANGMAN_ATTEMPTS ? "|" : "") << "\n";
        cout << "  " << (attempts < HANGMAN_ATTEMPTS - 1 ? "O" : "") << (attempts < HANGMAN_ATTEMPTS - 2 ? "  |" : "") << "\n";
        cout << " " << (attempts < HANGMAN_ATTEMPTS - 3 ? "/" : "") << (attempts < HANGMAN_ATTEMPTS - 4 ? "|" : "") << (attempts < HANGMAN_ATTEMPTS - 5 ? "\\" : "") << "\n";
        cout << " " << (attempts < HANGMAN_ATTEMPTS - 3 ? "/" : "") << (attempts < HANGMAN_ATTEMPTS - 5 ? " \\" : "") << "\n\n";

        cout << "  Word: ";
        for (char c : guessed) cout << c << " ";
        cout << "\n\n  Wrong: ";
        for (char c : wrong) cout << c << " ";
        cout << "\n  Attempts left: " << attempts << "\n\n";

        cout << "Guess a letter: ";
        string line;
        if (!getline(cin, line)) break;
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
    cin.ignore(static_cast<std::streamsize>(CIN_IGNORE_MAX), '\n');
    cin.get();
}

void play_rps() {
    const char* choices[] = {"Rock", "Paper", "Scissors"};
    int player_wins = 0, ai_wins = 0;

    while (player_wins < RPS_WIN_TARGET && ai_wins < RPS_WIN_TARGET) {
        cout << "\033[2J\033[1;1H";
        cout << "--- ROCK PAPER SCISSORS v1.0 --- (First to " << RPS_WIN_TARGET << " wins)\n\n";
        cout << "  \033[32mYou: " << player_wins << "\033[0m  \033[31mAI: " << ai_wins << "\033[0m\n\n";
        cout << "  1. Rock\n  2. Paper\n  3. Scissors\n";
        cout << "  Choice (1-3): ";

        string line;
        if (!getline(cin, line)) break;
        if (line.empty()) continue;
        int p = line[0] - '1';
        if (p < 0 || p > 2) { cout << "\033[31merror:\033[0m invalid choice.\n"; this_thread::sleep_for(chrono::milliseconds(500)); continue; }

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

    cout << "\n\033[1m" << (player_wins == RPS_WIN_TARGET ? "\033[32mYOU WIN THE SERIES!" : "\033[31mAI WINS THE SERIES!") << "\033[0m\n";
    cout << "Press Enter to return to NoNameOS...";
    cin.get();
}

vector<string> SYSTEM_USERS = {"root", "user", "guest", "admin"};

void show_progress(int current, int total, const string& label) {
    int bar_width = 30;
    float pct = total > 0 ? (float)current / total : 0.0f;
    int pos = (int)(bar_width * pct);
    cout << "\r" << label << " [";
    for (int i = 0; i < bar_width; i++) {
        if (i < pos) cout << "\033[32m=\033[0m";
        else if (i == pos) cout << "\033[33m>\033[0m";
        else cout << " ";
    }
    cout << "] " << int(pct * 100.0) << "%";
    if (current == total) cout << "\n";
    cout.flush();
}

void show_spinner(int frame) {
    const char* spinner = "|/-\\";
    cout << "\r" << spinner[frame % 4] << " ";
    cout.flush();
}

void print_cfetch_logo() {
    const char* colors[] = {"\033[31m", "\033[33m", "\033[32m", "\033[36m", "\033[34m", "\033[35m"};
    cout << "\n";
    for (int i = 0; i < 5; i++) {
        cout << "      ";
        for (int j = 0; j < 5 - i; j++) cout << " ";
        cout << colors[i % 6];
        for (int j = 0; j <= i; j++) cout << "/";
        cout << "\033[0m" << colors[(i + 2) % 6];
        for (int j = 0; j <= i; j++) cout << "\\";
        cout << "\033[0m\n";
    }
    for (int i = 0; i < 5; i++) {
        cout << "      ";
        for (int j = 0; j < i; j++) cout << " ";
        cout << colors[(i + 3) % 6];
        for (int j = 0; j < 5 - i; j++) cout << "\\";
        cout << "\033[0m" << colors[(i + 5) % 6];
        for (int j = 0; j < 5 - i; j++) cout << "/";
        cout << "\033[0m\n";
    }
    cout << "\n";
}

void play_2048() {
    vector<vector<int>> grid(4, vector<int>(4, 0));
    auto add_tile = [&]() {
        vector<pair<int,int>> cells;
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                if (grid[r][c] == 0) cells.push_back({r, c});
        if (cells.empty()) return;
        auto [r, c] = cells[rand() % cells.size()];
        grid[r][c] = (rand() % 10 < 9) ? 2 : 4;
    };
    add_tile();
    add_tile();
    auto slide_left = [&]() -> bool {
        bool moved = false;
        for (int r = 0; r < 4; r++) {
            vector<int> row;
            for (int c = 0; c < 4; c++) if (grid[r][c]) row.push_back(grid[r][c]);
            for (size_t i = 0; i + 1 < row.size(); i++) {
                if (row[i] == row[i+1]) {
                    row[i] *= 2;
                    row.erase(row.begin() + (int)i + 1);
                    moved = true;
                }
            }
            while ((int)row.size() < 4) row.push_back(0);
            for (int c = 0; c < 4; c++) {
                if (grid[r][c] != row[c]) moved = true;
                grid[r][c] = row[c];
            }
        }
        return moved;
    };
    auto rotate = [&]() {
        vector<vector<int>> ng(4, vector<int>(4));
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                ng[c][3 - r] = grid[r][c];
        grid = ng;
    };
    const char* tc[] = {"\033[0m", "\033[37m", "\033[33m", "\033[32m", "\033[36m", "\033[34m", "\033[35m", "\033[31m", "\033[37;41m", "\033[33;44m", "\033[37;42m"};
    while (true) {
        cout << "\033[2J\033[1;1H";
        cout << "--- 2048 v1.0 --- (WASD move, Q quit)\n\n";
        for (int r = 0; r < 4; r++) {
            cout << "  ";
            for (int c = 0; c < 4; c++) {
                int v = grid[r][c];
                if (v == 0) { cout << "    . "; continue; }
                int ci = 0, tmp = v;
                while (tmp > 1) { ci++; tmp /= 2; }
                int ti = min(ci, 10);
                cout << tc[ti] << "\033[1m";
                if (v < 10) cout << "   " << v << " \033[0m";
                else if (v < 100) cout << "  " << v << " \033[0m";
                else if (v < 1000) cout << " " << v << " \033[0m";
                else cout << v << " \033[0m";
            }
            cout << "\n\n";
        }
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                if (grid[r][c] == 2048) { cout << "\033[32mYOU WIN! 2048 reached!\033[0m\n"; goto end2048; }
        {
            bool can = false;
            for (int r = 0; r < 4; r++)
                for (int c = 0; c < 4; c++) {
                    if (grid[r][c] == 0) can = true;
                    if (r > 0 && grid[r][c] == grid[r-1][c]) can = true;
                    if (c > 0 && grid[r][c] == grid[r][c-1]) can = true;
                }
            if (!can) { cout << "\033[31mGame Over! No moves left.\033[0m\n"; break; }
        }
        cout << "Move: ";
        string ln; getline(cin, ln);
        if (ln.empty()) continue;
        char d = tolower(ln[0]);
        if (d == 'q') break;
        bool moved = false;
        if (d == 'a') moved = slide_left();
        else if (d == 'd') { rotate(); rotate(); moved = slide_left(); rotate(); rotate(); }
        else if (d == 'w') { rotate(); rotate(); rotate(); moved = slide_left(); rotate(); }
        else if (d == 's') { rotate(); moved = slide_left(); rotate(); rotate(); rotate(); }
        if (moved) add_tile();
    }
    end2048:
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(CIN_IGNORE_MAX), '\n');
    cin.get();
}

void play_typing_test() {
    vector<string> sentences = {
        "The quick brown fox jumps over the lazy dog.",
        "Pack my box with five dozen liquor jugs.",
        "How vexingly quick daft zebras jump.",
        "The five boxing wizards jump quickly.",
        "Sphinx of black quartz judge my vow.",
        "Two driven jocks help fax my big quiz.",
        "Farmer jack realized that big yellow quilts were expensive.",
        "NoNameOS is a fake operating system written in C++.",
        "The asciidash engine renders obstacles with ASCII art.",
        "Virtual file systems store data in memory instead of disk.",
        "The snake game grows longer as it eats food pellets.",
        "Minesweeper requires logic to reveal safe cells."
    };
    string sentence = sentences[rand() % sentences.size()];
    cout << "\033[2J\033[1;1H";
    cout << "--- Typing Test ---\n\n";
    cout << "Type this sentence:\n\n";
    cout << "\033[33m" << sentence << "\033[0m\n\n";
    cout << "Press Enter when ready...";
    cin.get();
    cout << "\033[2J\033[1;1H";
    cout << "Type now:\n\n";
    auto start = chrono::steady_clock::now();
    string input;
    getline(cin, input);
    auto end = chrono::steady_clock::now();
    double elapsed_s = chrono::duration_cast<chrono::milliseconds>(end - start).count() / 1000.0;
    int chars_typed = (int)input.length();
    double wpm = elapsed_s > 0 ? (chars_typed / 5.0) / (elapsed_s / 60.0) : 0.0;
    int correct = 0;
    for (size_t i = 0; i < min(input.length(), sentence.length()); i++)
        if (input[i] == sentence[i]) correct++;
    int maxlen = max(input.length(), sentence.length());
    double accuracy = maxlen > 0 ? 100.0 * correct / maxlen : 0.0;
    cout << "\nTime: " << elapsed_s << "s\n";
    cout << "WPM: " << (int)wpm << "\n";
    cout << "Accuracy: " << (int)accuracy << "%\n";
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(CIN_IGNORE_MAX), '\n');
    cin.get();
}

void play_reaction_time() {
    cout << "\033[2J\033[1;1H";
    cout << "--- Reaction Time Test ---\n\n";
    cout << "Press any key when you see NOW.\n";
    while (kbhit()) (void)getchar();
    double times[3];
    for (int r = 0; r < 3; r++) {
        int delay = 2000 + rand() % 3001;
        for (int i = delay / 100; i > 0; i--) {
            this_thread::sleep_for(chrono::milliseconds(100));
            if (kbhit()) { (void)getchar(); }
        }
        cout << "  \033[32mNOW!\033[0m" << flush;
        auto start = chrono::steady_clock::now();
        while (!kbhit()) this_thread::sleep_for(chrono::milliseconds(1));
        (void)getchar();
        auto end = chrono::steady_clock::now();
        times[r] = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        cout << "\r  " << times[r] << " ms\n";
        while (kbhit()) (void)getchar();
        if (r < 2) {
            cout << "Enter for round " << (r+2) << "...";
            while (!kbhit()) this_thread::sleep_for(chrono::milliseconds(50));
            while (kbhit()) (void)getchar();
        }
    }
    double avg = (times[0] + times[1] + times[2]) / 3.0;
    cout << "\nAverage: " << avg << " ms\n";
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(CIN_IGNORE_MAX), '\n');
    cin.get();
}

void play_number_memory() {
    cout << "\033[2J\033[1;1H";
    cout << "--- Number Memory ---\n\n";
    cout << "Remember the number shown, then type it back.\n";
    cout << "Press Enter to start...";
    cin.get();
    int max_digits = 0;
    for (int len = 3; len <= 20; len++) {
        string num;
        for (int i = 0; i < len; i++) num += '0' + (rand() % 10);
        cout << "\033[2J\033[1;1H";
        cout << "Level " << (len - 2) << " (" << len << " digits)\n\n";
        cout << "\033[1;36m" << num << "\033[0m\n\n";
        this_thread::sleep_for(chrono::seconds(2));
        cout << "\033[2J\033[1;1H";
        cout << "Type the number: ";
        string guess;
        getline(cin, guess);
        if (guess == num) {
            max_digits = len;
            cout << "\033[32mCorrect!\033[0m\n";
            cout << "Enter for next...";
            cin.get();
        } else {
            cout << "\033[31mWrong! Number was: " << num << "\033[0m\n";
            break;
        }
    }
    cout << "\nYou remembered " << max_digits << " digits!\n";
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(CIN_IGNORE_MAX), '\n');
    cin.get();
}

void cmd_tree(const string& root, const string& current_dir, const map<string,FSNode>& fs) {
    string dir = root.empty() ? current_dir : (root[0] == '/' ? root : current_dir + root + "/");
    if (fs.find(dir) == fs.end() && fs.find(dir + "/") != fs.end()) dir += "/";
    function<void(const string&, int)> rec = [&](const string& d, int depth) {
        vector<string> entries;
        for (auto& [path, node] : fs) {
            if (path == d) continue;
            if (path.rfind(d, 0) == 0) {
                string rel = path.substr(d.length());
                bool is_dir_node = node.is_dir;
                if (!is_dir_node && rel.find('/') == rel.length() - 1) is_dir_node = true;
                if (rel.find('/') == string::npos || (is_dir_node && rel.find('/') == rel.length() - 1)) {
                    entries.push_back(rel);
                }
            }
        }
        sort(entries.begin(), entries.end());
        for (const string& e : entries) {
            string full = d + e;
            bool is_d = (fs.find(full) != fs.end() && fs.at(full).is_dir);
            if (!is_d && fs.find(full + "/") != fs.end()) is_d = true;
            for (int i = 0; i < depth; i++) cout << "  ";
            cout << e << (is_d ? "/" : "") << "\n";
            if (is_d) {
                string sub = full;
                if (sub.back() != '/') sub += "/";
                rec(sub, depth + 1);
            }
        }
    };
    cout << dir << "\n";
    rec(dir, 1);
}

    int main() {
    cout << "\033[2J\033[1;1H";
    cout << "\033[36m  NoNameOS v0.7.0\033[0m\n\n";
    const vector<string> boot_steps = {
        "Initializing core", "Loading termios", "Preparing games",
        "Loading tools", "Starting VFS", "Setting up shell"
    };
    for (size_t i = 0; i < boot_steps.size(); i++) {
        show_progress((int)i + 1, (int)boot_steps.size(), "  " + boot_steps[i]);
        boot_delay(300 + (i % 3) * 50);
    }
    cout << "\n";

    srand(time(nullptr));
    map<string, FSNode> file_system;
    file_system["/"] = FSNode(true, "");

    // Default custom level mapping -- pre-load AsciiDash obstacle map into VFS
    file_system["/geometry/"] = FSNode(true, "");
    file_system["/geometry/jumper.gmd"] = FSNode(false, "_______^_______^^_______^___^^^___");

    string current_user = "root";
    string current_dir = "/";
    string input;
    vector<string> cmd_history;
    map<string, string> aliases;
    auto last_cmd_end = chrono::steady_clock::now();
    aliases["ll"] = "ls -l";
    aliases[".."] = "cd ..";
    aliases["ttt"] = "tictactoe";

    while (true) {
        {
            auto dur = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - last_cmd_end).count();
            if (cmd_history.size() > 0)
                cout << "\033[2m[" << dur << "ms]\033[0m ";
        }
        cout << "\033[32m" << current_user << "\033[37m@NoNameOS:\033[34m" << current_dir << "\033[0m# ";
        if (!getline(cin, input)) break;

        if (input.empty()) continue;
        cmd_history.push_back(input);
        auto [cmd, args] = parse_command(input);

        // Resolve aliases
        if (aliases.find(cmd) != aliases.end()) {
            string alias_cmd = aliases[cmd];
            if (!args.empty()) alias_cmd += " " + args;
            auto [resolved_cmd, resolved_args] = parse_command(alias_cmd);
            cmd = resolved_cmd;
            args = resolved_args;
        }

        if (cmd == "help") {
            if (!args.empty()) {
                cout << "\033[1m" << args << "\033[0m: " << help_text(args) << "\n";
                continue;
            }
            cout << "NoNameOS " << VERSION << " Commands:\n";
            cout << "\033[1;36m  General:\033[0m   help, man <cmd>, clear, exit\n";
            cout << "\033[1;32m  Filesystem:\033[0m ls, ls -l, cd, mkdir, touch, cat, echo, rm, pwd, grep, find, cp, mv, cp -r, rm -r\n";
            cout << "\033[1;32m  File View:\033[0m  head <f>, tail <f>, sort <f>, wc <f>, tee <f> <t>\n";
            cout << "\033[1;33m  System:\033[0m    whoami, date, uptime, history, cfetch, ps, uname, hostname, env, printenv\n";
            cout << "\033[1;35m  Tools:\033[0m     nano <f>, calc <x>, cowsay [m], cal, rainbow [m], yes, sleep, which\n";
            cout << "\033[1;35m  Tools:\033[0m     alias [x=y], unalias, su <user>, chmod, users, banner [m]\n";
            cout << "\033[1;35m  Tools:\033[0m     fortune, factor <n>, shuf <text>, tree, watch, ping, top, df, seq\n";
            cout << "\033[1;35m  Tools:\033[0m     todo, notes, stopwatch, timer, lolcat, sl (train), pom, alarm, bc\n";
            cout << "\033[1;35m  Text:\033[0m      rev, tr, cut, paste, uniq, nl, fold, basename, dirname\n";
            cout << "\033[1;32m  System:\033[0m    free, dmesg, lscpu, lsusb, arch, nproc\n";
            cout << "\033[1;32m  VFS:\033[0m       du, locate, ln -s, trash list/empty\n";
            cout << "\033[1;31m  Games:\033[0m     play [f], guess, trivia, adventure, snake, minesweeper\n";
            cout << "\033[1;31m  Games:\033[0m     tictactoe (ttt), hangman, rps, 2048, typing, reaction, nummem\n";
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
                            cout << type << node.mode << "  " << node.size << " " << node.created_at << " ";
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
            if (!args.empty()) {
                string path = current_dir;
                istringstream ss(args);
                string part;
                while (getline(ss, part, '/')) {
                    if (part.empty() || part == ".") continue;
                    if (part == "..") {
                        if (path != "/") {
                            string temp = path.substr(0, path.length() - 1);
                            path = temp.substr(0, temp.find_last_of('/') + 1);
                        }
                        continue;
                    }
                    path += part + "/";
                    if (file_system.find(path) == file_system.end()) {
                        file_system[path] = FSNode(true, "");
                    }
                }
            }
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
                    cout << "\033[31merror:\033[0m directory not found.\n";
                }
            }
        }
        else if (cmd == "echo") {
            size_t first_space = args.find(' ');
            if (first_space != string::npos) {
                string filename = args.substr(0, first_space);
                string content = args.substr(first_space + 1);
                if (content.front() == '"' && content.back() == '"') {
                    content = content.substr(1, content.length() - 2);
                }
                if (filename.find("..") != string::npos) {
                    cout << "\033[31merror:\033[0m path traversal not allowed.\n";
                } else {
                    file_system[current_dir + filename] = FSNode(false, content);
                }
            }
        }
        else if (cmd == "cat") {
            // Print the content of a VFS file to stdout
            if (file_system.find(current_dir + args) != file_system.end()) cout << file_system[current_dir + args].content << "\n";
            else cout << "\033[31merror:\033[0m file not found.\n";
        }
        else if (cmd == "rm") {
            string trash_dir = "/trash/";
            if (file_system.find(trash_dir) == file_system.end())
                file_system[trash_dir] = FSNode(true, "");
            string ts = to_string(chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count());
            if (args.rfind("-r ", 0) == 0) {
                string dirname = args.substr(3);
                string dpath = current_dir + dirname;
                if (dpath.back() != '/') dpath += "/";
                vector<string> to_erase;
                for (auto& [path, _] : file_system)
                    if (path.rfind(dpath, 0) == 0 || path == dpath.substr(0, dpath.length()-1))
                        to_erase.push_back(path);
                for (const string& p : to_erase) {
                    file_system[trash_dir + ts + "_" + p.substr(p.find_last_of('/') + 1)] = file_system[p];
                    file_system.erase(p);
                }
                cout << "\033[32mTrashed " << dirname << " recursively.\033[0m\n";
            } else {
                string src = current_dir + args;
                string dst = trash_dir + ts + "_" + args;
                vector<string> to_trash;
                if (file_system.find(src) != file_system.end()) {
                    to_trash.push_back(src);
                }
                string srcd = current_dir + args + "/";
                for (auto& [p, _] : file_system)
                    if (p.rfind(srcd, 0) == 0 || p == srcd.substr(0, srcd.length()-1))
                        to_trash.push_back(p);
                for (const string& p : to_trash) {
                    string name = p.substr(p.find_last_of('/') + 1);
                    file_system[trash_dir + ts + "_" + name] = file_system[p];
                    file_system.erase(p);
                }
            }
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
                    cout << "\033[31merror:\033[0m Map file not found in VFS.\n";
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
        // --- ADDITIONAL COMMANDS ---
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
                    cout << "\033[31merror:\033[0m file not found.\n";
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
            print_cfetch_logo();
            cout << "------------------------\n";
            cout << "\033[1;33mOS:\033[0m       NoNameOS " << VERSION << "\n";
            cout << "\033[1;33mKernel:\033[0m   C++ POSIX Sim\n";
            cout << "\033[1;33mShell:\033[0m    nonamesh\n";
            cout << "\033[1;33mVFS:\033[0m      " << file_system.size() << " nodes\n";
            cout << "\033[1;33mUptime:\033[0m   ";
            auto elapsed = chrono::duration_cast<chrono::seconds>(
                chrono::steady_clock::now() - program_start).count();
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
                cout << "NoNameOS nonameos " << VERSION << " C++ POSIX x86_64 GNU/C++\n";
            } else if (flag == "-r") cout << VERSION << "\n";
            else if (flag == "-s") cout << "NoNameOS\n";
            else if (flag == "-m") cout << "x86_64\n";
        }
        else if (cmd == "uptime") {
            auto elapsed = chrono::duration_cast<chrono::seconds>(
                chrono::steady_clock::now() - program_start).count();
            int days = elapsed / 86400;
            int hours = (elapsed % 86400) / 3600;
            int mins = (elapsed % 3600) / 60;
            cout << " up " << days << " day" << (days != 1 ? "s" : "")
                 << ", " << (hours < 10 ? "0" : "") << hours << ":" << (mins < 10 ? "0" : "") << mins
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
            for (int i = 0; i < YES_COUNT; i++) cout << text << " ";
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
            if (sec > 0 && sec <= SLEEP_MAX_SEC) {
                cout << "Sleeping for " << sec << " second" << (sec != 1 ? "s" : "") << "...\n";
                this_thread::sleep_for(chrono::seconds(sec));
            }
        }
        else if (cmd == "which") {
            if (args.empty()) cout << "Usage: which <command>\n";
            else if (ALL_COMMANDS.find(args) != ALL_COMMANDS.end()) cout << "/bin/" << args << "\n";
            else cout << "\033[31merror:\033[0m " << args << " not found\n";
        }
        else if (cmd == "alias") {
            if (args.empty()) {
                for (auto& [name, cmd_str] : aliases)
                    cout << "alias " << name << "='" << cmd_str << "'\n";
            } else {
                size_t eq = args.find('=');
                if (eq != string::npos) {
                    string name = args.substr(0, eq);
                    string val = args.substr(eq + 1);
                    if (val.front() == '\'' && val.back() == '\'')
                        val = val.substr(1, val.length() - 2);
                    aliases[name] = val;
                    cout << "Alias created: " << name << "='" << val << "'\n";
                }
            }
        }
        else if (cmd == "unalias") {
            if (aliases.find(args) != aliases.end()) {
                aliases.erase(args);
                cout << "Alias '" << args << "' removed.\n";
            } else {
                cout << "Alias '" << args << "' not found.\n";
            }
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
            if (sp == string::npos) cout << "Usage: cp [-r] <source> <dest>\n";
            else {
                string a = args.substr(0, sp);
                string b = args.substr(sp + 1);
                if (a == "-r") {
                    size_t sp2 = b.find(' ');
                    if (sp2 == string::npos) { cout << "Usage: cp -r <source> <dest>\n"; }
                    else {
                        string src = b.substr(0, sp2);
                        string dst = b.substr(sp2 + 1);
                        string sfull = current_dir + src;
                        string dfull = current_dir + dst;
                        if (sfull.back() != '/') sfull += "/";
                        if (dfull.back() != '/') dfull += "/";
                        for (auto& [path, node] : file_system) {
                            if (path.rfind(sfull, 0) == 0) {
                                string rel = path.substr(sfull.length());
                                file_system[dfull + rel] = node;
                            }
                        }
                        cout << "\033[32mCopied " << src << " -> " << dst << " recursively.\033[0m\n";
                    }
                } else {
                    string full_src = current_dir + a;
                    string full_dst = current_dir + b;
                    if (file_system.find(full_src) != file_system.end() && !file_system[full_src].is_dir) {
                        file_system[full_dst] = FSNode(false, file_system[full_src].content);
                        file_system[full_dst].mode = file_system[full_src].mode;
                        cout << "\033[32mCopied " << a << " -> " << b << "\033[0m\n";
                    } else cout << "\033[31merror:\033[0m source file not found.\n";
                }
            }
        }
        else if (cmd == "mv") {
            size_t sp = args.find(' ');
            if (sp == string::npos) cout << "Usage: mv <source> <dest>\n";
            else {
                string src = args.substr(0, sp);
                string dst = args.substr(sp + 1);
                string full_src = current_dir + src;
                string full_src_dir = full_src + "/";
                string full_dst = current_dir + dst;
                if (file_system.find(full_src) != file_system.end()) {
                    file_system[full_dst] = file_system[full_src];
                    file_system.erase(full_src);
                    cout << "\033[32mMoved " << src << " -> " << dst << "\033[0m\n";
                } else if (file_system.find(full_src_dir) != file_system.end()) {
                    file_system[full_dst + "/"] = file_system[full_src_dir];
                    file_system.erase(full_src_dir);
                    for (auto& [p, n] : file_system) {
                        if (p.rfind(full_src_dir, 0) == 0) {
                            string rel = p.substr(full_src_dir.length());
                            file_system[full_dst + "/" + rel] = n;
                            file_system.erase(p);
                        }
                    }
                    cout << "\033[32mMoved directory " << src << " -> " << dst << "\033[0m\n";
                } else cout << "\033[31merror:\033[0m source not found.\n";
            }
        }
        else if (cmd == "chmod") {
            size_t sp = args.find(' ');
            if (sp == string::npos) {
                cout << "Usage: chmod <mode> <file>\n";
            } else {
                string mode = args.substr(0, sp);
                string fn = args.substr(sp + 1);
                string fullpath = current_dir + fn;
                if (file_system.find(fullpath) != file_system.end()) {
                    if (mode.length() == 9) {
                        file_system[fullpath].mode = mode;
                        cout << "\033[32mMode changed.\033[0m\n";
                    } else cout << "\033[31merror:\033[0m invalid mode (use format rwxr-xr-x).\n";
                } else {
                    string dpath = fullpath + "/";
                    if (file_system.find(dpath) != file_system.end()) {
                        if (mode.length() == 9) {
                            file_system[dpath].mode = mode;
                            cout << "\033[32mMode changed.\033[0m\n";
                        } else cout << "\033[31merror:\033[0m invalid mode.\n";
                    } else cout << "\033[31merror:\033[0m file not found.\n";
                }
            }
        }
        else if (cmd == "su") {
            if (args.empty()) {
                cout << "Available users: ";
                for (size_t i = 0; i < SYSTEM_USERS.size(); i++) {
                    if (i > 0) cout << ", ";
                    cout << SYSTEM_USERS[i];
                }
                cout << "\n";
            } else {
                bool found = false;
                for (const string& u : SYSTEM_USERS) {
                    if (u == args) { found = true; break; }
                }
                if (found) {
                    current_user = args;
                    cout << "\033[32mSwitched to " << args << ".\033[0m\n";
                } else {
                    cout << "\033[31merror:\033[0m user '" << args << "' does not exist.\n";
                }
            }
        }
        else if (cmd == "who") {
            cout << "Logged in: " << current_user << "\n";
        }
        else if (cmd == "useradd") {
            if (args.empty()) {
                cout << "Usage: useradd <name>\n";
            } else {
                SYSTEM_USERS.push_back(args);
                cout << "\033[32mUser '" << args << "' added.\033[0m\n";
            }
        }
        else if (cmd == "userdel") {
            if (args.empty()) {
                cout << "Usage: userdel <name>\n";
            } else if (args == current_user) {
                cout << "\033[31merror:\033[0m cannot delete current user.\n";
            } else {
                auto it = find(SYSTEM_USERS.begin(), SYSTEM_USERS.end(), args);
                if (it != SYSTEM_USERS.end()) {
                    SYSTEM_USERS.erase(it);
                    cout << "\033[32mUser '" << args << "' removed.\033[0m\n";
                } else {
                    cout << "\033[31merror:\033[0m user not found.\n";
                }
            }
        }
        else if (cmd == "head") {
            string fn = args;
            string fullpath = current_dir + fn;
            if (file_system.find(fullpath) != file_system.end() && !file_system[fullpath].is_dir) {
                istringstream ss(file_system[fullpath].content);
                string line;
                for (int i = 0; i < HEAD_TAIL_LINES && getline(ss, line); i++) cout << line << "\n";
            } else cout << "\033[31merror:\033[0m file not found.\n";
        }
        else if (cmd == "tail") {
            string fn = args;
            string fullpath = current_dir + fn;
            if (file_system.find(fullpath) != file_system.end() && !file_system[fullpath].is_dir) {
                vector<string> lines;
                istringstream ss(file_system[fullpath].content);
                string line;
                while (getline(ss, line)) lines.push_back(line);
                int start = max(0, (int)lines.size() - HEAD_TAIL_LINES);
                for (int i = start; i < (int)lines.size(); i++) cout << lines[i] << "\n";
            } else cout << "\033[31merror:\033[0m file not found.\n";
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
            } else cout << "\033[31merror:\033[0m file not found.\n";
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
            } else cout << "\033[31merror:\033[0m file not found.\n";
        }
        else if (cmd == "tee") {
            size_t sp = args.find(' ');
            if (sp == string::npos) cout << "Usage: tee <file> <text>\n";
            else {
                string fn = args.substr(0, sp);
                string text = args.substr(sp + 1);
                string fullpath = current_dir + fn;
                string display = text;
                while (!display.empty() && display.back() == '\n') display.pop_back();
                file_system[fullpath] = FSNode(false, text + "\n");
                cout << display << "\n";
            }
        }
        else if (cmd == "man") {
            if (args.empty()) {
                cout << "Usage: man <command>\n";
            } else {
                static const map<string, string> manpages = {
                    {"ls", "LS(1)\t\t\tUser Commands\n\nNAME\n\tls - list directory contents\n\nSYNOPSIS\n\tls [-l]\n\nDESCRIPTION\n\tList information about VFS files. With -l, show sizes and timestamps."},
                    {"cd", "CD(1)\t\t\tUser Commands\n\nNAME\n\tcd - change the current working directory\n\nSYNOPSIS\n\tcd [dir]\n\nDESCRIPTION\n\tChange to the specified directory. Use '..' for parent or '/' for root."},
                    {"mkdir", "MKDIR(1)\t\tUser Commands\n\nNAME\n\tmkdir - create a directory\n\nSYNOPSIS\n\tmkdir <name>\n\nDESCRIPTION\n\tCreate a new empty directory in the virtual filesystem."},
                    {"touch", "TOUCH(1)\t\tUser Commands\n\nNAME\n\ttouch - create an empty file\n\nSYNOPSIS\n\ttouch <file>\n\nDESCRIPTION\n\tCreate an empty file in the VFS. No-op if file already exists."},
                    {"cat", "CAT(1)\t\t\tUser Commands\n\nNAME\n\tcat - concatenate and print files\n\nSYNOPSIS\n\tcat <file>\n\nDESCRIPTION\n\tDisplay the contents of a VFS file to the terminal."},
                    {"echo", "ECHO(1)\t\t\tUser Commands\n\nNAME\n\techo - write content to a file\n\nSYNOPSIS\n\techo <file> <content>\n\nDESCRIPTION\n\tWrite quoted or unquoted text to a VFS file."},
                    {"rm", "RM(1)\t\t\tUser Commands\n\nNAME\n\trm - remove files or directories\n\nSYNOPSIS\n\trm <name>\n\nDESCRIPTION\n\tRemove a file or directory from the VFS."},
                    {"grep", "GREP(1)\t\t\tUser Commands\n\nNAME\n\tgrep - search for patterns in a file\n\nSYNOPSIS\n\tgrep <pattern> <file>\n\nDESCRIPTION\n\tSearch for lines containing a pattern in a VFS file."},
                    {"find", "FIND(1)\t\t\tUser Commands\n\nNAME\n\tfind - search for files by name\n\nSYNOPSIS\n\tfind <name>\n\nDESCRIPTION\n\tRecursively search and print all VFS paths matching the given name."},
                    {"calc", "CALC(1)\t\t\tUser Commands\n\nNAME\n\tcalc - arithmetic calculator\n\nSYNOPSIS\n\tcalc <expression>\n\nDESCRIPTION\n\tEvaluate a mathematical expression (supports + - * / with operator precedence)."},
                    {"cowsay", "COWSAY(1)\t\tUser Commands\n\nNAME\n\tcowsay - ASCII cow with speech bubble\n\nSYNOPSIS\n\tcowsay [message]\n\nDESCRIPTION\n\tDisplay a talking ASCII cow with the given message."},
                    {"nano", "NANO(1)\t\t\tUser Commands\n\nNAME\n\tnano - built-in line editor\n\nSYNOPSIS\n\tnano <file>\n\nDESCRIPTION\n\tLine-by-line text editor. Enter an empty line to save and exit."},
                    {"play", "PLAY(1)\t\t\tGame Commands\n\nNAME\n\tplay - launch AsciiDash obstacle runner\n\nSYNOPSIS\n\tplay [file]\n\nDESCRIPTION\n\tRun the AsciiDash side-scrolling game. Optionally load a custom .gmd map file."},
                    {"guess", "GUESS(1)\t\tGame Commands\n\nNAME\n\tguess - number guessing game\n\nSYNOPSIS\n\tguess\n\nDESCRIPTION\n\tGuess a random number between 1 and 100. Unlimited attempts."},
                    {"trivia", "TRIVIA(1)\t\tGame Commands\n\nNAME\n\ttrivia - technology trivia quiz\n\nSYNOPSIS\n\ttrivia\n\nDESCRIPTION\n\tAnswer 5 multiple-choice questions about computers and technology."},
                    {"adventure", "ADVENTURE(1)\t\tGame Commands\n\nNAME\n\tadventure - dungeon RPG\n\nSYNOPSIS\n\tadventure\n\nDESCRIPTION\n\tExplore a dungeon, fight monsters, collect gold, and survive. Commands: left, right, rest, quit."},
                    {"snake", "SNAKE(1)\t\tGame Commands\n\nNAME\n\tsnake - terminal snake game\n\nSYNOPSIS\n\tsnake\n\nDESCRIPTION\n\tControl a snake with WASD. Eat food (*) to grow. Don't hit walls or yourself."},
                    {"minesweeper", "MINESWEEPER(1)\t\tGame Commands\n\nNAME\n\tminesweeper - terminal minesweeper\n\nSYNOPSIS\n\tminesweeper\n\nDESCRIPTION\n\tReveal cells by entering coordinates (x y). Avoid mines. Flag with 'f x y'."},
                    {"cfetch", "CFETCH(1)\t\tUser Commands\n\nNAME\n\tcfetch - system info display\n\nSYNOPSIS\n\tcfetch\n\nDESCRIPTION\n\tDisplay system information similar to neofetch."},
                    {"ps", "PS(1)\t\t\tUser Commands\n\nNAME\n\tps - list running processes\n\nSYNOPSIS\n\tps\n\nDESCRIPTION\n\tDisplay a snapshot of current simulated processes."},
                    {"uname", "UNAME(1)\t\tUser Commands\n\nNAME\n\tuname - system information\n\nSYNOPSIS\n\tuname [-a | -r | -s | -m]\n\nDESCRIPTION\n\tPrint system information. -a for all, -r for release, -s for OS on Name, -m for architecture."},
                    {"uptime", "UPTIME(1)\t\tUser Commands\n\nNAME\n\tuptime - system uptime\n\nSYNOPSIS\n\tuptime\n\nDESCRIPTION\n\tDisplay how long the system has been running."},
                    {"cal", "CAL(1)\t\t\tUser Commands\n\nNAME\n\tcal - display calendar\n\nSYNOPSIS\n\tcal\n\nDESCRIPTION\n\tDisplay the current month's calendar with today highlighted."},
                    {"rainbow", "RAINBOW(1)\t\tUser Commands\n\nNAME\n\trainbow - rainbow text\n\nSYNOPSIS\n\trainbow [message]\n\nDESCRIPTION\n\tPrint text with rainbow color cycling animation."},
                    {"man", "MAN(1)\t\t\tUser Commands\n\nNAME\n\tman - display manual pages\n\nSYNOPSIS\n\tman <command>\n\nDESCRIPTION\n\tDisplay the manual page for a given command."},
                    {"help", "HELP(1)\t\t\tUser Commands\n\nNAME\n\thelp - show available commands\n\nSYNOPSIS\n\thelp\n\nDESCRIPTION\n\tDisplay a categorized list of all available NoNameOS commands."},
                    {"clear", "CLEAR(1)\t\tUser Commands\n\nNAME\n\tclear - clear terminal screen\n\nSYNOPSIS\n\tclear\n\nDESCRIPTION\n\tClear the terminal display."},
                    {"exit", "EXIT(1)\t\t\tUser Commands\n\nNAME\n\texit - exit NoNameOS\n\nSYNOPSIS\n\texit\n\nDESCRIPTION\n\tExit the NoNameOS shell and return to the real terminal."},
                    {"whoami", "WHOAMI(1)\t\tUser Commands\n\nNAME\n\twhoami - print current user\n\nSYNOPSIS\n\twhoami\n\nDESCRIPTION\n\tDisplay the current logged-in user name."},
                    {"date", "DATE(1)\t\t\tUser Commands\n\nNAME\n\tdate - print system date and time\n\nSYNOPSIS\n\tdate\n\nDESCRIPTION\n\tDisplay the current system date and time."},
                    {"history", "HISTORY(1)\t\tUser Commands\n\nNAME\n\thistory - command history\n\nSYNOPSIS\n\thistory\n\nDESCRIPTION\n\tDisplay the list of previously entered commands with line numbers."},
                    {"pwd", "PWD(1)\t\t\tUser Commands\n\nNAME\n\tpwd - print working directory\n\nSYNOPSIS\n\tpwd\n\nDESCRIPTION\n\tPrint the absolute path of the current working directory."},
                    {"tictactoe", "TICTACTOE(1)\t\tGame Commands\n\nNAME\n\ttictactoe - play tic-tac-toe against AI\n\nSYNOPSIS\n\ttictactoe (or ttt)\n\nDESCRIPTION\n\tPlay tic-tac-toe on a 3x3 grid against a minimax AI. You are X, AI is O."},
                    {"ttt", "TTT(1)\t\t\tGame Commands\n\nNAME\n\tttt - alias for tictactoe\n\nSYNOPSIS\n\tttt\n\nDESCRIPTION\n\tSame as tictactoe."},
                    {"hangman", "HANGMAN(1)\t\tGame Commands\n\nNAME\n\thangman - classic word guessing game\n\nSYNOPSIS\n\thangman\n\nDESCRIPTION\n\tGuess letters to reveal a hidden word before the stick figure is complete."},
                    {"rps", "RPS(1)\t\t\tGame Commands\n\nNAME\n\trps - rock paper scissors best of 7\n\nSYNOPSIS\n\trps\n\nDESCRIPTION\n\tPlay Rock Paper Scissors against the AI. First to 4 wins the series."},
                    {"yes", "YES(1)\t\t\tUser Commands\n\nNAME\n\tyes - output text repeatedly\n\nSYNOPSIS\n\tyes [text]\n\nDESCRIPTION\n\tPrint text repeatedly (100 times) to stdout."},
                    {"env", "ENV(1)\t\t\tUser Commands\n\nNAME\n\tenv - print environment variables\n\nSYNOPSIS\n\tenv\n\nDESCRIPTION\n\tDisplay the current simulated environment variables."},
                    {"hostname", "HOSTNAME(1)\t\tUser Commands\n\nNAME\n\thostname - print system hostname\n\nSYNOPSIS\n\thostname\n\nDESCRIPTION\n\tDisplay the system's hostname."},
                    {"sleep", "SLEEP(1)\t\tUser Commands\n\nNAME\n\tsleep - delay execution\n\nSYNOPSIS\n\tsleep <seconds>\n\nDESCRIPTION\n\tPause the shell for the specified number of seconds (max 30)."},
                    {"which", "WHICH(1)\t\tUser Commands\n\nNAME\n\twhich - locate a command\n\nSYNOPSIS\n\twhich <command>\n\nDESCRIPTION\n\tShow the full path of a command if it exists in the shell."},
                    {"alias", "ALIAS(1)\t\tUser Commands\n\nNAME\n\talias - show or create command aliases\n\nSYNOPSIS\n\talias [name=command]\n\nDESCRIPTION\n\tDisplay all aliases or create a new alias. Created aliases resolve at the shell prompt."},
                    {"unalias", "UNALIAS(1)\t\tUser Commands\n\nNAME\n\tunalias - remove an alias\n\nSYNOPSIS\n\tunalias <name>\n\nDESCRIPTION\n\tRemove a previously defined alias by name."},
                    {"users", "USERS(1)\t\tUser Commands\n\nNAME\n\tusers - show logged in users\n\nSYNOPSIS\n\tusers\n\nDESCRIPTION\n\tDisplay the currently logged-in users."},
                    {"banner", "BANNER(1)\t\tUser Commands\n\nNAME\n\tbanner - print ASCII banner\n\nSYNOPSIS\n\tbanner [text]\n\nDESCRIPTION\n\tDisplay a large ASCII banner with the given text."},
                    {"fortune", "FORTUNE(1)\t\tUser Commands\n\nNAME\n\tfortune - random quote\n\nSYNOPSIS\n\tfortune\n\nDESCRIPTION\n\tDisplay a random programming quote."},
                    {"factor", "FACTOR(1)\t\tUser Commands\n\nNAME\n\tfactor - factorize a number\n\nSYNOPSIS\n\tfactor <number>\n\nDESCRIPTION\n\tDisplay the prime factors of a positive integer."},
                    {"shuf", "SHUF(1)\t\t\tUser Commands\n\nNAME\n\tshuf - shuffle text\n\nSYNOPSIS\n\tshuf <text>\n\nDESCRIPTION\n\tRandomly shuffle the characters of the given text."},
                    {"head", "HEAD(1)\t\t\tUser Commands\n\nNAME\n\thead - display first lines of a file\n\nSYNOPSIS\n\thead <file>\n\nDESCRIPTION\n\tDisplay the first 10 lines of a VFS file."},
                    {"tail", "TAIL(1)\t\t\tUser Commands\n\nNAME\n\ttail - display last lines of a file\n\nSYNOPSIS\n\ttail <file>\n\nDESCRIPTION\n\tDisplay the last 10 lines of a VFS file."},
                    {"sort", "SORT(1)\t\t\tUser Commands\n\nNAME\n\tsort - sort file contents\n\nSYNOPSIS\n\tsort <file>\n\nDESCRIPTION\n\tSort the lines of a VFS file alphabetically."},
                    {"wc", "WC(1)\t\t\tUser Commands\n\nNAME\n\twc - count lines, words, and characters\n\nSYNOPSIS\n\twc <file>\n\nDESCRIPTION\n\tDisplay line, word, and character counts for a VFS file."},
                    {"tee", "TEE(1)\t\t\tUser Commands\n\nNAME\n\ttee - write to file and display\n\nSYNOPSIS\n\ttee <file> <text>\n\nDESCRIPTION\n\tWrite text to a VFS file and also display it on stdout."},
                    {"cp", "CP(1)\t\t\tUser Commands\n\nNAME\n\tcp - copy files\n\nSYNOPSIS\n\tcp <source> <dest>\n\nDESCRIPTION\n\tCopy a VFS file from source to destination."},
                    {"mv", "MV(1)\t\t\tUser Commands\n\nNAME\n\tmv - move or rename files\n\nSYNOPSIS\n\tmv <source> <dest>\n\nDESCRIPTION\n\tMove or rename a file or directory in the VFS."},
                    {"chmod", "CHMOD(1)\t\tUser Commands\n\nNAME\n\tchmod - change file permissions\n\nSYNOPSIS\n\tchmod <mode> <file>\n\nDESCRIPTION\n\tNoNameOS VFS uses simulated permissions (rw-r--r--)."},
                    {"su", "SU(1)\t\t\tUser Commands\n\nNAME\n\tsu - switch user\n\nSYNOPSIS\n\tsu <user>\n\nDESCRIPTION\n\tSwitch to another user. Available: root, user."}
                };

                auto it = manpages.find(args);
                if (it != manpages.end()) {
                    cout << it->second << "\n";
                } else {
                    cout << "\033[31merror:\033[0m no manual entry for " << args << "\n";
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
            cout << "\033[33m--- Guess the Number ---\033[0m\n";
            cout << "I'm thinking of a number between 1 and 100.\n";
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
            vector<Question> questions = {
                {"What language is NoNameOS written in?", {"C++", "Python", "Rust", "Java"}, 0},
                {"What does VFS stand for?", {"Virtual File System", "Very Fast Server", "Video File Storage", "None"}, 0},
                {"How many legs does a cow have?", {"4", "2", "6", "8"}, 0},
                {"What does CPU stand for?", {"Central Processing Unit", "Computer Power Unit", "Central Program Utility", "Core Processing Unit"}, 0},
                {"What year was C++ created?", {"1979", "1990", "2001", "1965"}, 0}
            };
            shuffle(questions.begin(), questions.end(), default_random_engine(rand()));
            int score = 0;
            for (int i = 0; i < TRIVIA_COUNT; i++) {
                cout << "\nQ" << (i+1) << ": " << questions[i].q << "\n";
                for (int j = 0; j < 4; j++) {
                    cout << "  " << (j+1) << ". " << questions[i].opts[j] << "\n";
                }
                cout << "Answer (1-4): ";
                string line;
                if (!getline(cin, line)) break;
                int ans = 0;
                for (char c : line) {
                    if (c >= '0' && c <= '9') ans = ans * 10 + (c - '0');
                }
                if (ans == questions[i].correct + 1) {
                    cout << "\033[32mCorrect!\033[0m\n";
                    score++;
                } else {
                    cout << "\033[31mWrong! Answer was " << (questions[i].correct + 1) << "\033[0m\n";
                }
            }
            cout << "\nScore: " << score << "/" << TRIVIA_COUNT << "\n";
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
        // --- TEXT PROCESSING ---
        else if (cmd == "rev") { cmd_rev(args, file_system, current_dir); }
        else if (cmd == "tr") { cmd_tr(args, file_system, current_dir); }
        else if (cmd == "cut") { cmd_cut(args, file_system, current_dir); }
        else if (cmd == "paste") { cmd_paste(args, file_system, current_dir); }
        else if (cmd == "uniq") { cmd_uniq(args, file_system, current_dir); }
        else if (cmd == "nl") { cmd_nl(args, file_system, current_dir); }
        else if (cmd == "fold") { cmd_fold(args, file_system, current_dir); }
        else if (cmd == "basename") { cmd_basename(args); }
        else if (cmd == "dirname") { cmd_dirname(args); }
        // --- SYSTEM INFO ---
        else if (cmd == "free") { cmd_free(); }
        else if (cmd == "dmesg") { cmd_dmesg(); }
        else if (cmd == "lscpu") { cmd_lscpu(); }
        else if (cmd == "lsusb") { cmd_lsusb(); }
        else if (cmd == "arch") { cmd_arch(); }
        else if (cmd == "nproc") { cmd_nproc(); }
        // --- VFS ENHANCEMENTS ---
        else if (cmd == "ln") {
            if (args.rfind("-s ", 0) == 0) {
                istringstream ss(args.substr(3));
                string target, link;
                ss >> target >> link;
                if (target.empty() || link.empty()) { cout << "Usage: ln -s <target> <link>\n"; }
                else {
                    string lp = current_dir + link;
                    file_system[lp] = FSNode(false, "");
                    file_system[lp].is_link = true;
                    file_system[lp].link_target = target.find('/') == 0 ? target : current_dir + target;
                    cout << "\033[32mCreated symlink: " << link << " -> " << target << "\033[0m\n";
                }
            } else { cout << "Usage: ln -s <target> <link>\n"; }
        }
        else if (cmd == "trash") {
            string trash_dir = "/trash/";
            if (file_system.find(trash_dir) == file_system.end())
                file_system[trash_dir] = FSNode(true, "");
            if (args == "list") {
                bool empty = true;
                for (auto& [p, n] : file_system) {
                    if (p.rfind(trash_dir, 0) == 0 && !n.is_dir) {
                        string tn = p.substr(trash_dir.length());
                        size_t us = tn.find('_');
                        if (us != string::npos) tn = tn.substr(us + 1);
                        cout << tn << "\n";
                        empty = false;
                    }
                }
                if (empty) cout << "Trash is empty.\n";
            } else if (args == "empty") {
                vector<string> to_del;
                for (auto& [p, _] : file_system)
                    if (p.rfind(trash_dir, 0) == 0) to_del.push_back(p);
                for (auto& p : to_del) file_system.erase(p);
                cout << "\033[32mTrash emptied.\033[0m\n";
            } else {
                cout << "Usage: trash list | empty\n";
            }
        }
        else if (cmd == "du") { cmd_du(args, file_system, current_dir); }
        else if (cmd == "locate") { cmd_locate(args, file_system); }
        // --- PRODUCTIVITY ---
        else if (cmd == "pom") { cmd_pom(); }
        else if (cmd == "alarm") { cmd_alarm(args); }
        else if (cmd == "bc") { cmd_bc(args, file_system, current_dir); }
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
                cout << "\033[32mSaved\033[0m to " << args << " (" << content.length() << " bytes)\n";
            }
        }
        else if (cmd == "calc") {
            if (args.empty()) {
                cout << "Usage: calc <expr> (e.g. calc 2+3*4, calc sin(30), calc sqrt(144))\n";
            } else {
                string a = args;
                // Check for scientific functions
                auto sci_func = [&](const string& fname, auto func, bool deg) -> bool {
                    if (a.rfind(fname, 0) == 0) {
                        size_t ps = a.find('(');
                        size_t pe = a.find(')');
                        if (ps != string::npos && pe != string::npos) {
                            string numstr = a.substr(ps + 1, pe - ps - 1);
                            double arg = 0;
                            try { arg = stod(numstr); }
                            catch (...) { cout << "\033[31merror:\033[0m invalid number.\n"; return true; }
                            if (deg) arg = arg * acos(-1.0) / 180.0;
                            cout << "= " << func(arg) << "\n";
                            return true;
                        }
                    }
                    return false;
                };
                if (sci_func("sin", [](double x) { return sin(x); }, true)) {}
                else if (sci_func("cos", [](double x) { return cos(x); }, true)) {}
                else if (sci_func("tan", [](double x) { return tan(x); }, true)) {}
                else if (sci_func("sqrt", [](double x) { return sqrt(x); }, false)) {}
                else if (sci_func("log", [](double x) { return log(x); }, false)) {}
                else if (a.rfind("pow", 0) == 0) {
                    size_t ps = a.find('(');
                    size_t comma = a.find(',');
                    size_t pe = a.find(')');
                    if (ps != string::npos && comma != string::npos && pe != string::npos) {
                        try {
                            string base = a.substr(ps + 1, comma - ps - 1);
                            string exp = a.substr(comma + 1, pe - comma - 1);
                            double b = stod(base), e = stod(exp);
                            cout << "= " << pow(b, e) << "\n";
                        } catch (...) { cout << "\033[31merror:\033[0m invalid number.\n"; }
                    } else cout << "\033[31merror:\033[0m usage: pow(base,exp)\n";
                }
                else {
                    // Original arithmetic parser
                    vector<double> nums;
                    vector<char> ops;
                    istringstream ss(a);
                    double val;
                    char op;
                    if (ss >> val) {
                        nums.push_back(val);
                        while (ss >> op >> val) {
                            if (op == '+' || op == '-' || op == '*' || op == '/') {
                                ops.push_back(op);
                                nums.push_back(val);
                            } else {
                                cout << "\033[31merror:\033[0m invalid operator '" << op << "'\n";
                                break;
                            }
                        }
                    }
                    if (nums.size() < 2 || nums.size() != ops.size() + 1) {
                        cout << "\033[31merror:\033[0m invalid expression.\n";
                    } else {
                        vector<double> pn = {nums[0]};
                        vector<char> po;
                        for (size_t i = 0; i < ops.size(); i++) {
                            if (ops[i] == '*' || ops[i] == '/') {
                                double last = pn.back(); pn.pop_back();
                                if (ops[i] == '*') pn.push_back(last * nums[i+1]);
                                else {
                                    if (nums[i+1] == 0) { cout << "\033[31merror:\033[0m division by zero.\n"; break; }
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
        }
        // --- NEW GAMES ---
        else if (cmd == "2048") {
            play_2048();
        }
        else if (cmd == "typing") {
            play_typing_test();
        }
        else if (cmd == "reaction") {
            play_reaction_time();
        }
        else if (cmd == "nummem") {
            play_number_memory();
        }
        // --- NEW SHELL TOOLS ---
        else if (cmd == "tree") {
            cmd_tree(args, current_dir, file_system);
        }
        else if (cmd == "watch") {
            size_t sp = args.find(' ');
            int interval = 2;
            string subcmd;
            if (sp == string::npos) {
                subcmd = args;
            } else {
                string first = args.substr(0, sp);
                bool all_digits = true;
                for (char c : first) if (!(c >= '0' && c <= '9')) { all_digits = false; break; }
                if (all_digits && !first.empty()) {
                    interval = stoi(first);
                    subcmd = args.substr(sp + 1);
                } else {
                    subcmd = args;
                }
            }
            if (subcmd.empty()) {
                cout << "Usage: watch [interval] <command> [args]\n";
            } else {
                for (int i = 0; i < 5; i++) {
                    cout << "\033[2J\033[1;1H";
                    cout << "watch: " << subcmd << " (iteration " << (i+1) << "/5)\n\n";
                    istringstream ss(subcmd);
                    string wcmd, wargs;
                    ss >> wcmd;
                    getline(ss, wargs);
                    if (!wargs.empty() && wargs[0] == ' ') wargs = wargs.substr(1);
                    // Simulate running the command
                    cout << "Running: " << subcmd << "\n";
                    cout << "\033[32m[OK]\033[0m (simulated output)\n";
                    if (i < 4) this_thread::sleep_for(chrono::seconds(interval));
                }
            }
        }
        else if (cmd == "ping") {
            string host = args.empty() ? "localhost" : args;
            cout << "PING " << host << " (127.0.0.1) 56(84) bytes of data.\n";
            for (int i = 0; i < 4; i++) {
                int latency = 20 + rand() % 61;
                cout << "64 bytes from 127.0.0.1: icmp_seq=" << (i+1) << " ttl=64 time=" << latency << "." << (rand() % 100) << " ms\n";
                this_thread::sleep_for(chrono::milliseconds(500));
            }
            cout << "\n--- " << host << " ping statistics ---\n";
            cout << "4 packets transmitted, 4 received, 0% packet loss\n";
        }
        else if (cmd == "top") {
            cout << "\033[1m  PID USER      PR  NI  VIRT   RES   SHR S  CPU  MEM   TIME+   COMMAND\033[0m\n";
            cout << "    1 root      20   0  128M  4.5M  2.1M S  0.0  0.1  00:00:01 init\n";
            cout << "    2 root      20   0   64M  2.1M  1.0M S  0.0  0.0  00:00:00 nonamesh\n";
            cout << "    3 " << current_user << "      20   0  256M  8.2M  3.3M R  2.3  0.2  00:00:" << (cmd_history.size() % 60 < 10 ? "0" : "") << cmd_history.size() % 60 << " commands\n";
            cout << "    4 root      20   0   16M  0.8M  0.4M S  0.0  0.0  00:00:00 kworker\n";
            cout << "    5 " << current_user << "      20   0   32M  1.2M  0.6M S  0.3  0.0  00:00:00 logger\n";
        }
        else if (cmd == "df") {
            size_t total_bytes = 0;
            size_t total_nodes = file_system.size();
            for (auto& [_, node] : file_system) total_bytes += node.size;
            cout << "Filesystem      Size  Used  Avail  Use%  Mounted on\n";
            cout << "VFS           " << (total_bytes + 1024*1024) / 1024 << "K  " << total_bytes / 1024 << "K  " << (total_bytes + 1024*1024 - total_bytes) / 1024 << "K  " << (total_bytes * 100 / (total_bytes + 1)) << "%  /\n";
            cout << "Nodes: " << total_nodes << "\n";
        }
        else if (cmd == "seq") {
            int start = 1, end = 0;
            size_t sp = args.find(' ');
            if (sp == string::npos) {
                end = 0;
                for (char c : args) if (c >= '0' && c <= '9') end = end * 10 + (c - '0');
            } else {
                string s1 = args.substr(0, sp);
                string s2 = args.substr(sp + 1);
                start = 0; end = 0;
                for (char c : s1) if (c >= '0' && c <= '9') start = start * 10 + (c - '0');
                for (char c : s2) if (c >= '0' && c <= '9') end = end * 10 + (c - '0');
            }
            if (end == 0 && start > 0) { end = start; start = 1; }
            if (end > 0) {
                for (int i = start; i <= end; i++) {
                    if (i > start) cout << " ";
                    cout << i;
                }
                cout << "\n";
            }
        }
        else if (cmd == "printenv") {
            cout << "USER=" << current_user << "\n";
            cout << "SHELL=/bin/nonamesh\n";
            cout << "PWD=" << current_dir << "\n";
            cout << "HOME=/\n";
            cout << "OS=NoNameOS\n";
            cout << "TERM=" << (getenv("TERM") ? getenv("TERM") : "xterm-256color") << "\n";
        }
        // --- PRODUCTIVITY ---
        else if (cmd == "todo") {
            string todopath = "/todos";
            size_t sp = args.find(' ');
            string subcmd = sp == string::npos ? args : args.substr(0, sp);
            string rest = sp == string::npos ? "" : args.substr(sp + 1);
            if (subcmd == "clear") {
                file_system.erase(todopath);
                cout << "\033[32mAll todos cleared.\033[0m\n";
            } else if (subcmd == "list" || subcmd.empty()) {
                if (file_system.find(todopath) == file_system.end() || file_system[todopath].content.empty()) {
                    cout << "No todos.\n";
                } else {
                    istringstream ss(file_system[todopath].content);
                    string line;
                    int n = 1;
                    while (getline(ss, line)) {
                        if (line.empty()) continue;
                        bool done = (line[0] == 'x');
                        string text = done ? line.substr(1) : line;
                        cout << "  " << n << ". [" << (done ? "\033[32mx\033[0m" : " ") << "] " << text << "\n";
                        n++;
                    }
                }
            } else if (subcmd == "add") {
                if (rest.empty()) cout << "Usage: todo add <text>\n";
                else {
                    string cur = file_system[todopath].content;
                    if (!cur.empty() && cur.back() != '\n') cur += "\n";
                    cur += " " + rest + "\n";
                    file_system[todopath] = FSNode(false, cur);
                    cout << "\033[32mTodo added.\033[0m\n";
                }
            } else if (subcmd == "done") {
                int n = 0;
                for (char c : rest) if (c >= '0' && c <= '9') n = n * 10 + (c - '0');
                if (n < 1) cout << "Usage: todo done <n>\n";
                else {
                    if (file_system.find(todopath) != file_system.end()) {
                        istringstream ss(file_system[todopath].content);
                        vector<string> lines;
                        string line;
                        while (getline(ss, line)) lines.push_back(line);
                        if (n > 0 && n <= (int)lines.size()) {
                            if (lines[n-1][0] != 'x') lines[n-1] = "x" + lines[n-1].substr(1);
                            string newc;
                            for (size_t i = 0; i < lines.size(); i++) {
                                if (i > 0) newc += "\n";
                                newc += lines[i];
                            }
                            file_system[todopath].content = newc;
                            cout << "\033[32mTodo " << n << " marked done.\033[0m\n";
                        } else cout << "\033[31merror:\033[0m invalid todo number.\n";
                    } else cout << "\033[31merror:\033[0m no todos.\n";
                }
            } else cout << "Usage: todo <add|list|done|clear> [args]\n";
        }
        else if (cmd == "notes") {
            string notesdir = "/notes/";
            if (file_system.find(notesdir) == file_system.end())
                file_system[notesdir] = FSNode(true, "");
            size_t sp = args.find(' ');
            string subcmd = sp == string::npos ? args : args.substr(0, sp);
            string rest = sp == string::npos ? "" : args.substr(sp + 1);
            if (subcmd == "list" || (subcmd.empty() && rest.empty())) {
                bool any = false;
                for (auto& [path, node] : file_system) {
                    if (path.rfind(notesdir, 0) == 0 && path != notesdir && !node.is_dir) {
                        string name = path.substr(notesdir.length());
                        cout << "  " << name << " (" << node.size << " bytes)\n";
                        any = true;
                    }
                }
                if (!any) cout << "No notes.\n";
            } else if (subcmd == "rm") {
                if (rest.empty()) cout << "Usage: notes rm <name>\n";
                else {
                    bool found = false;
                    for (auto& [path, node] : file_system) {
                        if (path == notesdir + rest) {
                            file_system.erase(path);
                            cout << "\033[32mNote '" << rest << "' removed.\033[0m\n";
                            found = true;
                            break;
                        }
                    }
                    if (!found) cout << "\033[31merror:\033[0m note not found.\n";
                }
            } else {
                string name = args;
                string fullpath = notesdir + name;
                cout << "\033[33m--- Editing note: " << name << " ---\033[0m\n";
                if (file_system.find(fullpath) != file_system.end()) {
                    cout << "Current content:\n" << file_system[fullpath].content << "\n---\n";
                }
                cout << "Enter lines (empty line to save):\n";
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
                cout << "\033[32mNote '" << name << "' saved (" << content.length() << " bytes).\033[0m\n";
            }
        }
        else if (cmd == "stopwatch") {
            cout << "--- Stopwatch ---\n";
            cout << "Press Enter to start...";
            cin.get();
            auto start = chrono::steady_clock::now();
            cout << "Press Enter to stop...";
            cin.get();
            auto end = chrono::steady_clock::now();
            auto elapsed_ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
            cout << "Elapsed: " << (elapsed_ms / 1000) << "." << (elapsed_ms % 1000) << "s\n";
        }
        else if (cmd == "timer") {
            int sec = 0;
            for (char c : args) if (c >= '0' && c <= '9') sec = sec * 10 + (c - '0');
            if (sec < 1 || sec > 300) {
                cout << "Usage: timer <seconds> (1-300)\n";
            } else {
                cout << "Timer: " << sec << "s\n";
                for (int i = sec; i > 0; i--) {
                    cout << "\r\033[32m" << i << "s \033[0m" << flush;
                    this_thread::sleep_for(chrono::seconds(1));
                }
                cout << "\r\033[32mTime's up!\033[0m             \n";
            }
        }
        // --- FUN/NOVELTY ---
        else if (cmd == "lolcat") {
            string text = args.empty() ? "NoNameOS" : args;
            for (size_t i = 0; i < text.length(); i++) {
                int ci = (i * 4) % 36;
                int r = ci < 12 ? ci * 21 : (ci < 24 ? 255 - (ci - 12) * 21 : 0);
                int g = ci < 12 ? 0 : (ci < 24 ? (ci - 12) * 21 : 255 - (ci - 24) * 21);
                int b = ci < 12 ? 255 - ci * 21 : (ci < 24 ? 0 : (ci - 24) * 21);
                cout << "\033[38;2;" << r << ";" << g << ";" << b << "m" << text[i] << "\033[0m";
            }
            cout << "\n";
        }
        else if (cmd == "sl" || cmd == "train") {
            while (kbhit()) (void)getchar();
            const string train[] = {
                "      ====        ________                ___________ ",
                "  _- _~_   \\\\    |        |              |           |",
                " ( 0 0 0 )  \\\\   |  CHOO  |______________|  CHOO     |",
                " /'OO OO OO\\\\== ==O========O=============O===========O",
                "''''''''''''''''''''''''''''''''''''''''''''''''''''''''"
            };
            for (int offset = 50; offset >= -40; offset--) {
                cout << "\033[2J\033[1;1H";
                for (int r = 0; r < 5; r++) {
                    if (offset > 0) cout << string(offset, ' ');
                    else cout << string(0, ' ');
                    cout << train[r] << "\n";
                }
                this_thread::sleep_for(chrono::milliseconds(80));
            }
        }
        else {
            cout << "\033[31merror:\033[0m command not found: " << cmd << "\n";
            string sug = closest_cmd(cmd);
            if (!sug.empty()) cout << "  Did you mean \033[33m" << sug << "\033[0m?\n";
        }
        last_cmd_end = chrono::steady_clock::now();
    }
    return 0;
}
