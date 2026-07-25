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
#include <csignal>
#include <algorithm>
#include <sstream>
#include <random>
#include <functional>
#include <optional>
#include <iomanip>
#include <cstring>
// POSIX headers for terminal control (raw input) and non-blocking I/O detection
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

// --- Signal safety: save terminal state globally so SIGINT can restore it ---
static struct termios g_orig_term;
static bool g_term_saved = false;
static void sigint_handler(int) {
    if (g_term_saved) tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_term);
    _exit(1);
}

static const auto program_start = chrono::steady_clock::now();
const string VERSION = "v1.0.0";

// --- Truecolor + 256-color ANSI helpers ---
namespace clr {
    // 256-color foreground
    inline string fg256(int c) { return "\033[38;5;" + to_string(c) + "m"; }
    // 256-color background
    inline string bg256(int c) { return "\033[48;5;" + to_string(c) + "m"; }
    // Truecolor foreground
    inline string rgb(int r, int g, int b) { return "\033[38;2;" + to_string(r) + ";" + to_string(g) + ";" + to_string(b) + "m"; }
    // Truecolor background
    inline string rgbbg(int r, int g, int b) { return "\033[48;2;" + to_string(r) + ";" + to_string(g) + ";" + to_string(b) + "m"; }
    const string reset   = "\033[0m";
    const string bold    = "\033[1m";
    const string dim     = "\033[2m";
    const string italic  = "\033[3m";
    const string underline = "\033[4m";
    const string blink   = "\033[5m";
    const string inverse = "\033[7m";
    // Named colors (256)
    const string red     = fg256(196);
    const string lred    = fg256(203);
    const string green   = fg256(46);
    const string lgreen  = fg256(120);
    const string yellow  = fg256(226);
    const string amber   = fg256(214);
    const string blue    = fg256(39);
    const string lblue   = fg256(117);
    const string cyan    = fg256(51);
    const string lcyan   = fg256(123);
    const string magenta = fg256(201);
    const string lmagenta= fg256(207);
    const string orange  = fg256(208);
    const string white   = fg256(231);
    const string gray    = fg256(245);
    const string dgray   = fg256(240);
    const string black   = fg256(16);
    // Semantic
    const string success = bold + fg256(46);
    const string error   = bold + fg256(196);
    const string warning = bold + fg256(226);
    const string info    = bold + fg256(39);
    const string accent  = bold + fg256(213);
    const string muted   = fg256(245);
    const string header  = bold + fg256(75);
    const string prompt_user = bold + fg256(46);
    const string prompt_host = fg256(231);
    const string prompt_dir  = fg256(39);
    const string prompt_sep  = fg256(240);
}

// --- Visual helpers ---
// Simple line separator
string vsep(int width, const string& ch = "─") {
    string s;
    for (int i = 0; i < width; i++) s += ch;
    return s;
}

// Repeat a unicode character N times
string repeat(int n, const string& ch) {
    string s;
    for (int i = 0; i < n; i++) s += ch;
    return s;
}

// --- Pseudo-random engine (replaces weak srand/rand) ---
static mt19937& rng() {
    static mt19937 gen(random_device{}());
    return gen;
}
static int rng_int(int lo, int hi) {
    uniform_int_distribution<int> dist(lo, hi);
    return dist(rng());
}

// --- Named constants (replaces magic numbers) ---
constexpr int CIN_IGNORE_MAX = 10000;
constexpr int GAME_SPEED_MS = 150;
constexpr int SNAKE_W = 20;
constexpr int SNAKE_H = 15;
constexpr int MINESWEEPER_W = 10;
constexpr int MINESWEEPER_H = 10;
constexpr int MINESWEEPER_MINES = 12;
constexpr int TTT_BOARD_CELLS = 9;
constexpr int HANGMAN_ATTEMPTS = 6;
constexpr int RPS_WIN_TARGET = 4;
constexpr int SLEEP_MAX_SEC = 30;
constexpr int YES_COUNT = 100;
constexpr int HEAD_TAIL_LINES = 10;
constexpr int TRIVIA_COUNT = 5;
constexpr int ASCIIDASH_PADDING = 10;
constexpr int ASCIIDASH_WINDOW = 20;
constexpr int JUMP_FRAMES = 3;
constexpr int PAGER_LINES = 20;
constexpr int WATCH_ITERATIONS = 5;
constexpr int PING_COUNT = 4;
constexpr int TRAIN_START_OFFSET = 50;
constexpr int TRAIN_END_OFFSET = -40;
constexpr int TRAIN_FRAME_MS = 80;
// New game constants
constexpr int TETRIS_W = 10;
constexpr int TETRIS_H = 20;
constexpr int PONG_W = 40;
constexpr int PONG_H = 15;
constexpr int FLAPPY_W = 30;
constexpr int FLAPPY_H = 15;

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
            g_orig_term = oldt;
            g_term_saved = true;
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
            g_term_saved = false;
        }
    }
    TerminalGuard(const TerminalGuard&) = delete;
    TerminalGuard& operator=(const TerminalGuard&) = delete;
};

// Generate a human-readable timestamp string (e.g. "Jul 05 09:53") for VFS metadata
string get_timestamp() {
    time_t now = time(nullptr);
    tm t_buf;
    localtime_r(&now, &t_buf);
    char buf[20];
    strftime(buf, sizeof(buf), "%b %d %H:%M", &t_buf);
    return string(buf);
}

struct FSNode {
    bool is_dir;
    bool is_link;
    string content;
    string created_at;
    string mode;
    string link_target;

    FSNode() : is_dir(false), is_link(false), content(""), created_at(""), mode("rw-r--r--"), link_target("") {}
    FSNode(bool d, string c, size_t = 0) : is_dir(d), is_link(false), content(c), created_at(get_timestamp()),
        mode(d ? "rwxr-xr-x" : "rw-r--r--"), link_target("") {}
    size_t size() const { return content.size(); }
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
        for (int n = 0; n < PAGER_LINES && i < lines.size(); n++, i++)
            cout << lines[i] << "\n";
        if (i < lines.size()) {
            cout << "\033[2m-- More -- (Enter=next, q=quit)\033[0m";
            cout.flush();
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

// Resolve a user-provided path to an absolute VFS path, handling ~, .., and absolute/relative
string resolve_user_path(const string& arg, const string& current_dir) {
    string path;
    if (!arg.empty() && arg[0] == '/') {
        path = arg;
    } else {
        path = current_dir + arg;
    }
    // Normalize: collapse /./ and handle /../
    vector<string> parts;
    istringstream ss(path);
    string part;
    while (getline(ss, part, '/')) {
        if (part.empty() || part == ".") continue;
        if (part == "..") {
            if (!parts.empty()) parts.pop_back();
        } else {
            parts.push_back(part);
        }
    }
    string result = "/";
    for (size_t i = 0; i < parts.size(); i++) {
        result += parts[i];
        if (i + 1 < parts.size()) result += "/";
        else result += "/";
    }
    if (parts.empty()) result = "/";
    return result;
}

// Check if a path or any component contains path-traversal sequences
bool has_traversal(const string& path) {
    return path.find("..") != string::npos;
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

optional<string> vfs_read(const string& path, map<string,FSNode>& fs, const string& cdir) {
    string fp = resolved_path(fs, cdir + path);
    if (fs.find(fp) != fs.end() && !fs[fp].is_dir) return fs[fp].content;
    return nullopt;
}

const set<string> ALL_COMMANDS = {"ls","cd","mkdir","touch","cat","echo","rm","clear","exit","play","cowsay",
    "pwd","whoami","date","history","grep","find","cfetch","ps","uname","uptime","cal","rainbow",
    "man","help","nano","calc","guess","trivia","adventure","snake","minesweeper","tictactoe","ttt",
    "hangman","rps","yes","env","hostname","sleep","which","head","tail","sort","wc","tee","alias",
    "users","banner","fortune","factor","shuf","cp","mv","chmod","su","unalias","tree","watch",
    "ping","top","df","seq","printenv","todo","notes","stopwatch","timer","lolcat","sl",
    "train","who","useradd","userdel","2048","typing","reaction","nummem","rev","tr","cut",
    "paste","uniq","nl","fold","basename","dirname","free","dmesg","lscpu","lsusb","arch",
    "nproc","du","locate","pom","alarm","bc","ln","trash",
    "tetris","pong","sudoku","flappy","memory","connect4","lightsout","puzzle","breakout","whack",
    "colors","weather","epoch","uuid","base64","rot13","uppercase","lowercase",
    "wordcount","matrix","cmtheme","countdown","ascii","hexdump","password",
    "quote","joke","ip","uptime2","mem","cpu","disk","calc2",
    "bmi","tip","units","roman","binary","morse","bar","sparkline",
    "colorgen","palette","diff","csv","stats","age","datecalc","encode",
    "hash","md5","sha1","urlencode","urldecode","reverse","capitalize",
    "repeat","scrabble","zodiac","chinese","emoji","random","pick","dice",
    "coin","timer2","pom2","worldclock","stopwatch2","quiz","wordle"};

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
    if (args.empty()) { cout << "Usage: rev <file>\n"; return; }
    auto c = vfs_read(args, fs, cdir);
    if (!c) { cout << "error: file not found.\n"; return; }
    istringstream ss(*c); string line;
    while (getline(ss, line)) { reverse(line.begin(), line.end()); cout << line << "\n"; }
}

void cmd_tr(const string& args, map<string,FSNode>& fs, const string& cdir) {
    istringstream ss(args); string fn, f, r;
    ss >> fn >> f >> r;
    if (fn.empty() || f.empty() || r.empty()) { cout << "Usage: tr <file> <find> <replace>\n"; return; }
    auto c = vfs_read(fn, fs, cdir);
    if (!c) { cout << "error: file not found.\n"; return; }
    string content = *c;
    // Map each find char to its corresponding replace char (cycle replace if shorter)
    for (char& ch : content) {
        for (size_t i = 0; i < f.size(); i++) {
            if (ch == f[i]) { ch = r[i % r.size()]; break; }
        }
    }
    cout << content << "\n";
}

void cmd_cut(const string& args, map<string,FSNode>& fs, const string& cdir) {
    istringstream ss(args); string fn; int n;
    ss >> fn >> n;
    if (fn.empty() || n <= 0) { cout << "Usage: cut <file> <n>\n"; return; }
    auto c = vfs_read(fn, fs, cdir);
    if (!c) { cout << "error: file not found.\n"; return; }
    istringstream is(*c); string line;
    while (getline(is, line)) cout << line.substr(0, (size_t)n) << "\n";
}

void cmd_paste(const string& args, map<string,FSNode>& fs, const string& cdir) {
    istringstream ss(args); string f1, f2;
    ss >> f1 >> f2;
    if (f1.empty() || f2.empty()) { cout << "Usage: paste <file1> <file2>\n"; return; }
    auto c1 = vfs_read(f1, fs, cdir), c2 = vfs_read(f2, fs, cdir);
    if (!c1 || !c2) { cout << "error: file not found.\n"; return; }
    vector<string> l1, l2; string line;
    { istringstream s1(*c1); while (getline(s1, line)) l1.push_back(line); }
    { istringstream s2(*c2); while (getline(s2, line)) l2.push_back(line); }
    for (size_t i = 0; i < max(l1.size(), l2.size()); i++) {
        if (i < l1.size()) cout << l1[i]; cout << "\t";
        if (i < l2.size()) cout << l2[i]; cout << "\n";
    }
}

void cmd_uniq(const string& args, map<string,FSNode>& fs, const string& cdir) {
    if (args.empty()) { cout << "Usage: uniq <file>\n"; return; }
    auto c = vfs_read(args, fs, cdir);
    if (!c) { cout << "error: file not found.\n"; return; }
    istringstream ss(*c); string line, prev;
    while (getline(ss, line)) { if (line != prev) cout << line << "\n"; prev = line; }
}

void cmd_nl(const string& args, map<string,FSNode>& fs, const string& cdir) {
    if (args.empty()) { cout << "Usage: nl <file>\n"; return; }
    auto c = vfs_read(args, fs, cdir);
    if (!c) { cout << "error: file not found.\n"; return; }
    istringstream ss(*c); string line; int n = 1;
    while (getline(ss, line)) { cout << "  " << n << "\t" << line << "\n"; n++; }
}

void cmd_fold(const string& args, map<string,FSNode>& fs, const string& cdir) {
    istringstream ss(args); string fn; int n = 80;
    ss >> fn >> n;
    if (fn.empty()) { cout << "Usage: fold <file> [width]\n"; return; }
    if (n <= 0) { cout << "error: width must be > 0.\n"; return; }
    auto c = vfs_read(fn, fs, cdir);
    if (!c) { cout << "error: file not found.\n"; return; }
    for (size_t i = 0; i < c->length(); i += (size_t)n) cout << c->substr(i, (size_t)n) << "\n";
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
    cout << "\n  " << clr::bold << clr::gray << "              total        used        free      shared  buff/cache   available" << clr::reset << "\n";
    cout << "  " << clr::dgray << "  ─────────── ─────────── ─────────── ─────────── ─────────── ───────────" << clr::reset << "\n";
    cout << "  " << clr::white << "Mem:     " << clr::lcyan << "   32768       18234       14534" << clr::gray << "          0          0       14534" << clr::reset << "\n";
    cout << "  " << clr::white << "Swap:     " << clr::lcyan << "   8192         234        7958" << clr::reset << "\n\n";
}

void cmd_dmesg() {
    struct msg { string ts; string level; string text; };
    vector<msg> msgs = {
        {"0.000000", "info", "NoNameOS " + VERSION + " booting on x86_64"},
        {"0.102304", "info", "CPU: NoNameCPU v1.0 @ 2.4GHz (4 cores)"},
        {"1.500000", "info", "Memory: 32768K available"},
        {"2.100000", "info", "VFS: Mounted root filesystem"},
        {"2.750000", "info", "Console: NonameSH terminal"},
        {"3.050000", "ok", "System ready. User: root"}
    };
    cout << "\n";
    for (auto& m : msgs) {
        string color = (m.level == "ok") ? clr::success : clr::info;
        cout << "  " << clr::dgray << "[" << m.ts << "]" << clr::reset << " " << color << m.text << clr::reset << "\n";
    }
    cout << "\n";
}

void cmd_lscpu() {
    cout << "\n";
    auto row = [](const string& k, const string& v) {
        cout << "  " << clr::gray << k << ":" << clr::reset << string(max(0, 20 - (int)k.size()), ' ') << clr::bold << clr::white << v << clr::reset << "\n";
    };
    row("Architecture", clr::cyan + "x86_64" + clr::reset);
    row("CPU op-mode(s)", clr::cyan + "32-bit, 64-bit" + clr::reset);
    row("Model name", clr::cyan + "NoNameCPU v1.0" + clr::reset);
    row("CPU(s)", clr::cyan + "4" + clr::reset);
    row("CPU MHz", clr::cyan + "2400.000" + clr::reset);
    row("L1d cache", clr::cyan + "32K" + clr::reset);
    row("L1i cache", clr::cyan + "32K" + clr::reset);
    row("L2 cache", clr::cyan + "256K" + clr::reset);
    row("L3 cache", clr::cyan + "4096K" + clr::reset);
    cout << "\n";
}

void cmd_lsusb() {
    cout << "\n";
    auto row = [](const string& bus, const string& dev, const string& desc) {
        cout << "  " << clr::white << "Bus " << bus << " Device " << dev << ": " << clr::reset
             << clr::dgray << "ID " << clr::reset << clr::lcyan << desc << clr::reset << "\n";
    };
    row("001", "001", "1d6b:0001 NoName USB Keyboard");
    row("001", "002", "1d6b:0002 NoName USB Mouse");
    row("002", "001", "1d6b:0003 NoName Storage Device");
    row("002", "002", "1d6b:0004 NoName USB Hub");
    cout << "\n";
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
        if (p.rfind(dir, 0) == 0 && !n.is_dir) total += n.size();
    }
    cout << total << "\t" << (args.empty() ? "." : args) << "\n";
}

void cmd_locate(const string& args, const map<string,FSNode>& fs) {
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
            else if (ops[i] == '%') { if (nums[i+1] == 0) { cout << "error: modulo by zero.\n"; return; } pn.push_back(fmod(last, nums[i+1])); }
            else { double r = pow(last, nums[i+1]); if (isnan(r) || isinf(r)) { cout << "error: invalid power operation.\n"; return; } pn.push_back(r); }
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
    size_t map_len = map_data.length();
    if (map_len <= (size_t)(ASCIIDASH_PADDING / 2 + 1)) { cout << "Map too short.\n"; return; }

    for (size_t i = 0; i < map_len - (size_t)(ASCIIDASH_PADDING / 2); i++) {
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
        cout << "\n  " << clr::bold << clr::cyan << "🚀 ASCIIDASH" << clr::reset << " " << clr::dgray << VERSION << clr::reset << "   " << clr::gray << "(Press SPACE to Jump)" << clr::reset << "\n\n";
        cout << (player_y == 1 ? "     ■\n" : "\n");
        cout << "     " << (player_y == 0 ? "■" : " ") << "\n";
        cout << map_data.substr(i, ASCIIDASH_WINDOW) << "\n";
        cout << "====================\n";

        this_thread::sleep_for(chrono::milliseconds(GAME_SPEED_MS));
    }

    while (kbhit()) (void)getchar();

    cout << "\n\n";
    if (crashed) {
        cout << "\n  " << clr::error << ">> CRASHED! Attempt failed." << clr::reset << "\n";
    } else {
        cout << "\n  " << clr::success << ">> LEVEL COMPLETE! GG!" << clr::reset << "\n";
    }
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(CIN_IGNORE_MAX), '\n');
    cin.get();
}

void play_snake() {
    vector<pair<int,int>> snake = {{SNAKE_W/2, SNAKE_H/2}};
    int dx = 1, dy = 0;
    int food_x = rng_int(0, SNAKE_W - 1), food_y = rng_int(0, SNAKE_H - 1);
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
            // Spawn food not on snake body
            vector<pair<int,int>> empty_cells;
            for (int fy = 0; fy < SNAKE_H; fy++)
                for (int fx = 0; fx < SNAKE_W; fx++) {
                    bool on_snake = false;
                    for (auto& seg : snake) if (seg.first == fx && seg.second == fy) { on_snake = true; break; }
                    if (!on_snake) empty_cells.push_back({fx, fy});
                }
            if (!empty_cells.empty()) {
                auto [fx, fy] = empty_cells[rng_int(0, (int)empty_cells.size() - 1)];
                food_x = fx; food_y = fy;
            }
        } else {
            snake.pop_back();
        }

        vector<vector<bool>> grid(SNAKE_H, vector<bool>(SNAKE_W, false));
        for (size_t i = 0; i < snake.size(); i++) {
            grid[snake[i].second][snake[i].first] = true;
        }

        cout << "\033[2J\033[1;1H";
        cout << "\n  " << clr::bold << clr::green << "🐍 SNAKE" << clr::reset << " " << clr::dgray << "v1.0" << clr::reset << "   " << clr::gray << "Score:" << clr::reset << " " << clr::yellow << score << clr::reset << "   " << clr::dgray << "(WASD to move)" << clr::reset << "\n\n";
        for (int y = 0; y < SNAKE_H; y++) {
            cout << "  ";
            for (int x = 0; x < SNAKE_W; x++) {
                if (grid[y][x]) {
                    if (snake[0].first == x && snake[0].second == y)
                        cout << clr::bold << clr::green << "O" << clr::reset;
                    else
                        cout << clr::cyan << "o" << clr::reset;
                } else if (x == food_x && y == food_y) {
                    cout << clr::bold << clr::red << "*" << clr::reset;
                } else {
                    cout << ".";
                }
            }
            cout << "\n";
        }
        cout << "\n  " << clr::gray << "Score: " << clr::yellow << score << clr::gray << "  │  Press Ctrl+C to quit" << clr::reset << "\n";
        this_thread::sleep_for(chrono::milliseconds(GAME_SPEED_MS));
    }

    while (kbhit()) (void)getchar();
    string sc = to_string(score);
    cout << "\n\n  " << clr::error << ">> GAME OVER" << clr::reset << "  " << clr::gray << "Final Score:" << clr::reset << " " << clr::yellow << sc << clr::reset << "\n";
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
        int mx = rng_int(0, MINESWEEPER_W - 1), my = rng_int(0, MINESWEEPER_H - 1);
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
        cout << "\n  " << clr::bold << clr::red << "💣 MINESWEEPER" << clr::reset << " " << clr::dgray << "v1.0" << clr::reset << "   " << clr::gray << "Mines:" << clr::reset << " " << clr::red << MINESWEEPER_MINES << clr::reset << "\n\n";
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

    if (won) cout << "\n  " << clr::success << ">> YOU WIN! Congratulations!" << clr::reset << "\n";
    else cout << "\n  " << clr::error << ">> BOOM! You hit a mine!" << clr::reset << "\n";
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
        cout << "\n  " << clr::bold << clr::lmagenta << "✕ TIC-TAC-TOE" << clr::reset << " " << clr::dgray << "v1.0" << clr::reset << "   " << clr::gray << "You:" << clr::reset << " " << clr::green << "X" << clr::reset << "  " << clr::gray << "AI:" << clr::reset << " " << clr::red << "O" << clr::reset << "\n\n";
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
        for (int i = 0; i < TTT_BOARD_CELLS; i++) {
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
        for (int i = 0; i < TTT_BOARD_CELLS; i++) {
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

        if (check_win('X')) { cout << "\n  " << clr::success << "🏆 You win!" << clr::reset << "\n"; break; }
        if (check_win('O')) { cout << "\n  " << clr::error << "🤖 AI wins!" << clr::reset << "\n"; break; }
        if (is_draw()) { cout << "\n  " << clr::warning << "🤝 Draw!" << clr::reset << "\n"; break; }

        turn = 1 - turn;
    }
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(CIN_IGNORE_MAX), '\n');
    cin.get();
}

void play_hangman() {
    vector<string> words = {"computer", "keyboard", "monitor", "programming", "terminal",
                            "software", "hardware", "network", "internet", "algorithm"};
    string word = words[rng_int(0, (int)words.size() - 1)];
    string guessed(word.length(), '_');
    int attempts = HANGMAN_ATTEMPTS;
    vector<char> wrong;

    while (attempts > 0 && guessed.find('_') != string::npos) {
        cout << "\033[2J\033[1;1H";
        cout << "\n  " << clr::bold << clr::lred << "☠ HANGMAN" << clr::reset << " " << clr::dgray << "v1.0" << clr::reset << "\n\n";

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
        cout << "\n  " << clr::bold << clr::amber << "✊ ROCK PAPER SCISSORS" << clr::reset << " " << clr::dgray << "v1.0" << clr::reset << "   " << clr::gray << "First to " << clr::yellow << RPS_WIN_TARGET << clr::reset << "\n\n";
        cout << "  " << clr::green << "You: " << player_wins << clr::reset << "  " << clr::red << "AI: " << ai_wins << clr::reset << "\n\n";
        cout << "  1. Rock\n  2. Paper\n  3. Scissors\n";
        cout << "  Choice (1-3): ";

        string line;
        if (!getline(cin, line)) break;
        if (line.empty()) continue;
        int p = line[0] - '1';
        if (p < 0 || p > 2) { cout << "\033[31merror:\033[0m invalid choice.\n"; this_thread::sleep_for(chrono::milliseconds(500)); continue; }

        int a = rng_int(0, 2);
        cout << "  You: " << clr::green << choices[p] << clr::reset << "  vs  AI: " << clr::red << choices[a] << clr::reset << "\n";

        if (p == a) cout << "  " << clr::warning << "Draw!" << clr::reset << "\n";
        else if ((p == 0 && a == 2) || (p == 1 && a == 0) || (p == 2 && a == 1)) {
            cout << "  " << clr::success << "You win this round!" << clr::reset << "\n"; player_wins++;
        } else {
            cout << "  " << clr::error << "AI wins this round!" << clr::reset << "\n"; ai_wins++;
        }
        cout << "\nPress Enter to continue...";
        cin.get();
    }

    cout << "\n";
    if (player_wins == RPS_WIN_TARGET) {
        cout << "  " << clr::success << ">> YOU WIN THE SERIES!" << clr::reset << "\n";
    } else {
        cout << "  " << clr::error << ">> AI WINS THE SERIES!" << clr::reset << "\n";
    }
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
    const vector<string> art = {
        "        ╱╲        ",
        "       ╱  ╲       ",
        "      ╱    ╲      ",
        "     ╱  /\\  ╲     ",
        "    ╱  /  \\  ╲    ",
        "   ╱  /    \\  ╲   ",
        "  ╱  /      \\  ╲  ",
        " ╱  /________\\  ╲ ",
        "╱________________╲"
    };
    const string colors[] = {
        clr::rgb(255,85,85), clr::rgb(255,170,51), clr::rgb(255,255,85),
        clr::rgb(85,255,85), clr::rgb(85,255,255), clr::rgb(85,85,255)
    };
    cout << "\n";
    for (size_t i = 0; i < art.size(); i++) {
        cout << "  " << colors[i % 6] << clr::bold << art[i] << clr::reset << "\n";
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
        auto [r, c] = cells[rng_int(0, (int)cells.size() - 1)];
        grid[r][c] = (rng_int(0, 9) < 9) ? 2 : 4;
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
    bool won_2048 = false;
    while (true) {
        cout << "\033[2J\033[1;1H";
        cout << "\n  " << clr::bold << clr::orange << "🔢 2048" << clr::reset << " " << clr::dgray << "v1.0" << clr::reset << "   " << clr::gray << "(WASD move, Q quit)" << clr::reset << "\n\n";
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
                if (grid[r][c] == 2048) { cout << "\n  " << clr::success << ">> YOU WIN! 2048 reached!" << clr::reset << "\n"; won_2048 = true; }
        if (won_2048) break;
        {
            bool can = false;
            for (int r = 0; r < 4; r++)
                for (int c = 0; c < 4; c++) {
                    if (grid[r][c] == 0) can = true;
                    if (r > 0 && grid[r][c] == grid[r-1][c]) can = true;
                    if (c > 0 && grid[r][c] == grid[r][c-1]) can = true;
                }
            if (!can) { cout << "\n  " << clr::error << ">> Game Over! No moves left." << clr::reset << "\n"; break; }
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
    string sentence = sentences[rng_int(0, (int)sentences.size() - 1)];
    cout << "\033[2J\033[1;1H";
    cout << "\n  " << clr::bold << clr::lblue << "⌨ TYPING TEST" << clr::reset << "\n\n";
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
    cout << "\n  " << clr::bold << clr::lmagenta << "⚡ REACTION TEST" << clr::reset << "\n\n";
    cout << "Press any key when you see NOW.\n";
    while (kbhit()) (void)getchar();
    double times[3];
    for (int r = 0; r < 3; r++) {
        int delay = 2000 + rng_int(0, 3000);
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
    cout << "\n  " << clr::bold << clr::lcyan << "🧠 NUMBER MEMORY" << clr::reset << "\n\n";
    cout << "Remember the number shown, then type it back.\n";
    cout << "Press Enter to start...";
    cin.get();
    int max_digits = 0;
    for (int len = 3; len <= 20; len++) {
        string num;
        for (int i = 0; i < len; i++) num += '0' + rng_int(0, 9);
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

// ═══════════════════════════════════════════════════════════════
//  NEW GAMES — Tetris, Pong, Sudoku, Flappy Bird
// ═══════════════════════════════════════════════════════════════

void play_tetris() {
    vector<string> board(TETRIS_H, string(TETRIS_W, '.'));
    // Tetromino shapes: I, O, T, S, Z, L, J
    const vector<vector<string>> shapes = {
        {"XXXX","X...X...X...X.."},  // I
        {"XX\nXX"},                 // O
        {".X.\nXXX","X..\nXX.\nX..","XXX\n..X.",".X.\nXX.\n.X."}, // T
        {".XX\nXX.","X..\nXX.\n.X."}, // S
        {"XX..\n.XX","..X.\n.XX\n.X.."}, // Z
        {"X..\nX..\nXX.",".XX\nX..\nX..","XX.\n.X.\n.X.","..X\n..X\nXX."}, // L
        {"..X\n..X\nXX.","X..\nX..\nXX.","XX.\n.X.\n.X."}  // J
    };
    const string colors_t[] = {clr::cyan, clr::yellow, clr::magenta, clr::green, clr::red, clr::orange, clr::blue};

    int piece = rng_int(0, 6);
    int rotation = 0;
    int px = TETRIS_W / 2 - 1, py = 0;
    int score = 0;
    bool game_over = false;

    auto get_shape = [&](int p, int r) -> string {
        if (shapes[p].size() == 1) return shapes[p][0];
        return shapes[p][r % shapes[p].size()];
    };

    auto can_place = [&](int p, int r, int x, int y) -> bool {
        string s = get_shape(p, r);
        int sw = (int)s.find('\n');
        if (sw == -1) sw = (int)s.size();
        int sh = ((int)s.size() + 1) / (sw + 1);
        (void)sh;
        for (int i = 0; i < (int)s.size(); i++) {
            if (s[i] == '\n') continue;
            int cx = x + (i % (sw + 1));
            int cy = y + i / (sw + 1);
            if (cx < 0 || cx >= TETRIS_W || cy >= TETRIS_H) return false;
            if (cy >= 0 && board[cy][cx] != '.') return false;
        }
        return true;
    };

    auto place_piece = [&](int p, int r, int x, int y) {
        string s = get_shape(p, r);
        int sw = (int)s.find('\n');
        if (sw == -1) sw = (int)s.size();
        for (int i = 0; i < (int)s.size(); i++) {
            if (s[i] == '\n') continue;
            int cx = x + (i % (sw + 1));
            int cy = y + i / (sw + 1);
            if (cy >= 0 && cy < TETRIS_H && cx >= 0 && cx < TETRIS_W)
                board[cy][cx] = 'A' + p;
        }
    };

    auto clear_lines = [&]() {
        int cleared = 0;
        for (int y = TETRIS_H - 1; y >= 0; y--) {
            bool full = true;
            for (int x = 0; x < TETRIS_W; x++) if (board[y][x] == '.') { full = false; break; }
            if (full) {
                board.erase(board.begin() + y);
                board.insert(board.begin(), string(TETRIS_W, '.'));
                cleared++;
                y++;
            }
        }
        if (cleared > 0) score += cleared * cleared * 100;
    };

    while (!game_over) {
        if (kbhit()) {
            char c = getchar();
            if (c == 'a' && can_place(piece, rotation, px - 1, py)) px--;
            else if (c == 'd' && can_place(piece, rotation, px + 1, py)) px++;
            else if (c == 'w') {
                int nr = (rotation + 1) % 4;
                if (can_place(piece, nr, px, py)) rotation = nr;
            }
            else if (c == 's') { while (can_place(piece, rotation, px, py + 1)) py++; }
            else if (c == 'q') break;
        }

        if (can_place(piece, rotation, px, py + 1)) {
            py++;
        } else {
            place_piece(piece, rotation, px, py);
            clear_lines();
            piece = rng_int(0, 6);
            rotation = 0;
            px = TETRIS_W / 2 - 1;
            py = 0;
            if (!can_place(piece, rotation, px, py)) game_over = true;
        }

        // Render
        vector<string> display = board;
        string sh = get_shape(piece, rotation);
        int sw2 = (int)sh.find('\n');
        if (sw2 == -1) sw2 = (int)sh.size();
        for (int i = 0; i < (int)sh.size(); i++) {
            if (sh[i] == '\n') continue;
            int cx = px + (i % (sw2 + 1));
            int cy = py + i / (sw2 + 1);
            if (cy >= 0 && cy < TETRIS_H && cx >= 0 && cx < TETRIS_W)
                display[cy][cx] = 'A' + piece;
        }

        cout << "\033[2J\033[1;1H";
        cout << "\n  " << clr::bold << clr::cyan << "🧱 TETRIS" << clr::reset << "  " << clr::gray << "Score:" << clr::reset << " " << clr::yellow << score << clr::reset << "  " << clr::dgray << "(AD=move W=rotate S=drop Q=quit)" << clr::reset << "\n\n";
        for (int y = 0; y < TETRIS_H; y++) {
            cout << "  " << clr::dgray << "│" << clr::reset;
            for (int x = 0; x < TETRIS_W; x++) {
                char c = display[y][x];
                if (c == '.') cout << clr::dgray << "." << clr::reset;
                else cout << colors_t[c - 'A'] << clr::bold << "█" << clr::reset;
            }
            cout << clr::dgray << "│" << clr::reset << "\n";
        }
        cout << "  " << clr::dgray << "└" << repeat(TETRIS_W, "─") << "┘" << clr::reset << "\n";
        this_thread::sleep_for(chrono::milliseconds(GAME_SPEED_MS));
    }
    cout << "\n  " << clr::error << ">> GAME OVER" << clr::reset << "  " << clr::gray << "Score:" << clr::reset << " " << clr::yellow << score << clr::reset << "\n";
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(CIN_IGNORE_MAX), '\n');
    cin.get();
}

void play_pong() {
    int paddle_l = PONG_H / 2 - 2, paddle_r = PONG_H / 2 - 2;
    int ball_x = PONG_W / 2, ball_y = PONG_H / 2;
    int dx = 1, dy = 1;
    int score_l = 0, score_r = 0;
    bool game_over = false;

    while (!game_over) {
        if (kbhit()) {
            char c = getchar();
            if (c == 'w' && paddle_l > 0) paddle_l--;
            else if (c == 's' && paddle_l < PONG_H - 4) paddle_l++;
            else if (c == 'o' && paddle_r > 0) paddle_r--;
            else if (c == 'l' && paddle_r < PONG_H - 4) paddle_r++;
            else if (c == 'q') break;
        }

        // AI for right paddle
        if (ball_y > paddle_r + 2 && paddle_r < PONG_H - 4) paddle_r++;
        else if (ball_y < paddle_r + 2 && paddle_r > 0) paddle_r--;

        ball_x += dx; ball_y += dy;
        if (ball_y <= 0 || ball_y >= PONG_H - 1) dy = -dy;

        // Paddle collisions
        if (ball_x == 2 && ball_y >= paddle_l && ball_y <= paddle_l + 3) { dx = 1; ball_x = 3; }
        if (ball_x == PONG_W - 3 && ball_y >= paddle_r && ball_y <= paddle_r + 3) { dx = -1; ball_x = PONG_W - 4; }

        if (ball_x <= 0) { score_r++; ball_x = PONG_W / 2; ball_y = PONG_H / 2; dx = 1; }
        if (ball_x >= PONG_W - 1) { score_l++; ball_x = PONG_W / 2; ball_y = PONG_H / 2; dx = -1; }

        if (score_l >= 5 || score_r >= 5) game_over = true;

        // Render
        cout << "\033[2J\033[1;1H";
        cout << "\n  " << clr::bold << clr::lmagenta << "🏓 PONG" << clr::reset << "  " << clr::green << "You:" << score_l << clr::reset << "  " << clr::red << "AI:" << score_r << clr::reset << "  " << clr::dgray << "(WS=left paddle, OL=right, Q=quit)" << clr::reset << "\n\n";
        for (int y = 0; y < PONG_H; y++) {
            cout << "  " << clr::dgray << "│" << clr::reset;
            for (int x = 0; x < PONG_W; x++) {
                bool is_left_paddle = (x == 1 && y >= paddle_l && y <= paddle_l + 3);
                bool is_right_paddle = (x == PONG_W - 2 && y >= paddle_r && y <= paddle_r + 3);
                bool is_ball = (x == ball_x && y == ball_y);
                if (is_left_paddle) cout << clr::bold << clr::green << "█" << clr::reset;
                else if (is_right_paddle) cout << clr::bold << clr::red << "█" << clr::reset;
                else if (is_ball) cout << clr::bold << clr::yellow << "●" << clr::reset;
                else if (x == PONG_W / 2) cout << clr::dgray << "│" << clr::reset;
                else cout << " ";
            }
            cout << clr::dgray << "│" << clr::reset << "\n";
        }
        this_thread::sleep_for(chrono::milliseconds(80));
    }
    cout << "\n  " << (score_l >= 5 ? clr::success : clr::error) << ">> " << (score_l >= 5 ? "YOU WIN!" : "AI WINS!") << clr::reset << "\n";
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(CIN_IGNORE_MAX), '\n');
    cin.get();
}

void play_sudoku() {
    // Generate a simple puzzle (9x9 with some blanks)
    vector<vector<int>> solution = {
        {5,3,4,6,7,8,9,1,2},{6,7,2,1,9,5,3,4,8},{1,9,8,3,4,2,5,6,7},
        {8,5,9,7,6,1,4,2,3},{4,2,6,8,5,3,7,9,1},{7,1,3,9,2,4,8,5,6},
        {9,6,1,5,3,7,2,8,4},{2,8,7,4,1,9,6,3,5},{3,4,5,2,8,6,1,7,9}
    };
    vector<vector<int>> board(9, vector<int>(9));
    vector<vector<bool>> fixed(9, vector<bool>(9, false));
    // Remove ~30 cells
    int blanks = 30;
    for (int i = 0; i < blanks; i++) {
        int r = rng_int(0, 8), c = rng_int(0, 8);
        while (board[r][c] == 0) { r = rng_int(0, 8); c = rng_int(0, 8); }
        board[r][c] = 0;
    }
    for (int r = 0; r < 9; r++) for (int c = 0; c < 9; c++) {
        board[r][c] = solution[r][c];
        if (board[r][c] != 0) fixed[r][c] = true;
    }
    // Re-remove
    for (int i = 0; i < blanks; i++) {
        int r = rng_int(0, 8), c = rng_int(0, 8);
        while (fixed[r][c]) { r = rng_int(0, 8); c = rng_int(0, 8); }
        board[r][c] = 0;
    }

    while (true) {
        cout << "\033[2J\033[1;1H";
        cout << "\n  " << clr::bold << clr::amber << "🔢 SUDOKU" << clr::reset << "  " << clr::dgray << "(enter: r c value | q=quit)" << clr::reset << "\n\n";
        cout << "     " << clr::dgray << "┌───────┬───────┬───────┐" << clr::reset << "\n";
        for (int r = 0; r < 9; r++) {
            if (r == 3 || r == 6) cout << "     " << clr::dgray << "├───────┼───────┼───────┤" << clr::reset << "\n";
            cout << "     " << clr::dgray << "│" << clr::reset;
            for (int c = 0; c < 9; c++) {
                if (c == 3 || c == 6) cout << clr::dgray << "│" << clr::reset;
                if (board[r][c] == 0) cout << clr::dgray << " . " << clr::reset;
                else if (fixed[r][c]) cout << " " << clr::bold << clr::white << board[r][c] << clr::reset << " ";
                else cout << " " << clr::cyan << board[r][c] << clr::reset << " ";
            }
            cout << clr::dgray << "│" << clr::reset << "\n";
        }
        cout << "     " << clr::dgray << "└───────┴───────┴───────┘" << clr::reset << "\n";

        // Check win
        bool won = true;
        for (int r = 0; r < 9; r++) for (int c = 0; c < 9; c++) if (board[r][c] == 0) won = false;
        if (won) { cout << "\n  " << clr::success << ">> PUZZLE COMPLETE!" << clr::reset << "\n"; break; }

        cout << "\n  " << clr::gray << "r c value: " << clr::reset;
        string line; getline(cin, line);
        if (line == "q" || line == "Q") break;
        istringstream iss(line);
        int r, c, v;
        if (!(iss >> r >> c >> v)) continue;
        r--; c--;
        if (r < 0 || r >= 9 || c < 0 || c >= 9 || v < 1 || v > 9) continue;
        if (fixed[r][c]) continue;
        board[r][c] = v;
    }
    cout << "Press Enter to return to NoNameOS...";
    cin.get();
}

void play_flappy() {
    int bird_y = FLAPPY_H / 2;
    int bird_vy = 0;
    int frame = 0;
    int score = 0;
    bool game_over = false;
    vector<pair<int,int>> pipes; // {x, gap_y}

    while (!game_over) {
        if (kbhit()) {
            char c = getchar();
            if (c == ' ' || c == 'w') bird_vy = -3;
            else if (c == 'q') break;
        }

        bird_vy += 1;
        if (bird_vy > 3) bird_vy = 3;
        bird_y += bird_vy;
        if (bird_y < 0 || bird_y >= FLAPPY_H) { game_over = true; break; }

        // Spawn pipes
        if (frame % 20 == 0) {
            int gap = rng_int(3, FLAPPY_H - 5);
            pipes.push_back({FLAPPY_W, gap});
        }

        // Move pipes and check collisions
        for (auto& p : pipes) {
            p.first--;
            if (p.first == 5 && (bird_y < p.second || bird_y > p.second + 3)) game_over = true;
            if (p.first == 5) score++;
        }
        pipes.erase(remove_if(pipes.begin(), pipes.end(), [](auto& p) { return p.first < -2; }), pipes.end());

        // Render
        cout << "\033[2J\033[1;1H";
        cout << "\n  " << clr::bold << clr::lgreen << "🐦 FLAPPY" << clr::reset << "  " << clr::gray << "Score:" << clr::reset << " " << clr::yellow << score << clr::reset << "  " << clr::dgray << "(SPACE/W=flap Q=quit)" << clr::reset << "\n\n";
        for (int y = 0; y < FLAPPY_H; y++) {
            cout << "  " << clr::dgray << "│" << clr::reset;
            for (int x = 0; x < FLAPPY_W; x++) {
                bool is_bird = (x == 5 && y == bird_y);
                bool is_pipe = false;
                for (auto& p : pipes) {
                    if (x == p.first && (y < p.second || y > p.second + 3)) { is_pipe = true; break; }
                }
                if (is_bird) cout << clr::bold << clr::yellow << "▶" << clr::reset;
                else if (is_pipe) cout << clr::green << "█" << clr::reset;
                else if (x == FLAPPY_W - 1) cout << clr::dgray << "│" << clr::reset;
                else cout << " ";
            }
            cout << "\n";
        }
        cout << "  " << clr::dgray << "└" << repeat(FLAPPY_W, "─") << "┘" << clr::reset << "\n";
        frame++;
        this_thread::sleep_for(chrono::milliseconds(120));
    }
    cout << "\n  " << clr::error << ">> GAME OVER" << clr::reset << "  " << clr::gray << "Score:" << clr::reset << " " << clr::yellow << score << clr::reset << "\n";
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(CIN_IGNORE_MAX), '\n');
    cin.get();
}

// ═══════════════════════════════════════════════════════════════
//  NEW COMMANDS — 25+ new utilities
// ═══════════════════════════════════════════════════════════════

void cmd_colors() {
    cout << "\n  " << clr::bold << "Available Colors:" << clr::reset << "\n\n";
    struct { const char* name; string code; } palette[] = {
        {"Red", clr::red}, {"Light Red", clr::lred}, {"Green", clr::green},
        {"Light Green", clr::lgreen}, {"Yellow", clr::yellow}, {"Amber", clr::amber},
        {"Blue", clr::blue}, {"Light Blue", clr::lblue}, {"Cyan", clr::cyan},
        {"Light Cyan", clr::lcyan}, {"Magenta", clr::magenta}, {"Light Magenta", clr::lmagenta},
        {"Orange", clr::orange}, {"White", clr::white}, {"Gray", clr::gray},
        {"Dark Gray", clr::dgray}
    };
    for (auto& [name, code] : palette)
        cout << "  " << code << "████████" << clr::reset << "  " << clr::gray << name << clr::reset << "\n";
    cout << "\n  " << clr::dim << "Use in scripts: echo \"\\033[38;5;XXXm\" for any 256 color" << clr::reset << "\n\n";
}

void cmd_weather() {
    string conditions[] = {"Sunny", "Partly Cloudy", "Cloudy", "Light Rain", "Clear"};
    int temp = rng_int(15, 35);
    int hum = rng_int(30, 90);
    int wind = rng_int(0, 40);
    string cond = conditions[rng_int(0, 4)];
    cout << "\n  " << clr::bold << clr::cyan << "Weather Report" << clr::reset << "  " << clr::dgray << "(simulated)" << clr::reset << "\n\n";
    cout << "  Condition:  " << clr::yellow << cond << clr::reset << "\n";
    cout << "  Temp:       " << clr::red << temp << "°C" << clr::reset << "\n";
    cout << "  Humidity:   " << clr::blue << hum << "%" << clr::reset << "\n";
    cout << "  Wind:       " << clr::gray << wind << " km/h" << clr::reset << "\n\n";
}

void cmd_epoch() {
    time_t now = time(nullptr);
    cout << "  " << clr::bold << "Unix Epoch:" << clr::reset << " " << clr::cyan << now << clr::reset << "\n";
    tm t_buf;
    localtime_r(&now, &t_buf);
    char buf[64];
    strftime(buf, sizeof(buf), "  UTC: %a, %d %b %Y %H:%M:%S", &t_buf);
    cout << buf << "\n\n";
}

void cmd_uuid() {
    auto r = []() -> string {
        string s;
        for (int i = 0; i < 8; i++) s += "0123456789abcdef"[rng_int(0, 15)];
        return s;
    };
    cout << clr::cyan << r() << "-" << r() << "-" << r() << "-" << r() << "-" << r() << r() << r() << clr::reset << "\n";
}

void cmd_base64(const string& args, map<string,FSNode>& fs, const string& cdir) {
    if (args.empty()) { cout << "Usage: base64 <file|text>\n"; return; }
    auto c = vfs_read(args, fs, cdir);
    string text = c ? *c : args;
    // Simple base64 encode
    const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    string out;
    for (size_t i = 0; i < text.size(); i += 3) {
        unsigned int n = ((unsigned char)text[i]) << 16;
        if (i + 1 < text.size()) n |= ((unsigned char)text[i+1]) << 8;
        if (i + 2 < text.size()) n |= ((unsigned char)text[i+2]);
        out += tbl[(n >> 18) & 63];
        out += tbl[(n >> 12) & 63];
        out += (i + 1 < text.size()) ? tbl[(n >> 6) & 63] : '=';
        out += (i + 2 < text.size()) ? tbl[n & 63] : '=';
    }
    cout << out << "\n";
}

void cmd_rot13(const string& args) {
    string out = args;
    for (char& c : out) {
        if (c >= 'a' && c <= 'z') c = 'a' + (c - 'a' + 13) % 26;
        else if (c >= 'A' && c <= 'Z') c = 'A' + (c - 'A' + 13) % 26;
    }
    cout << out << "\n";
}

void cmd_password() {
    const char* chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*";
    int len = rng_int(12, 20);
    string pw;
    for (int i = 0; i < len; i++) pw += chars[rng_int(0, (int)strlen(chars) - 1)];
    cout << "\n  " << clr::bold << "Generated Password:" << clr::reset << "\n";
    cout << "  " << clr::cyan << pw << clr::reset << "\n";
    cout << "  " << clr::dgray << "Length: " << len << " characters" << clr::reset << "\n\n";
}

void cmd_wordcount(const string& args, map<string,FSNode>& fs, const string& cdir) {
    if (args.empty()) { cout << "Usage: wordcount <file>\n"; return; }
    auto c = vfs_read(args, fs, cdir);
    if (!c) { cout << "error: file not found.\n"; return; }
    map<string, int> freq;
    istringstream ss(*c);
    string word;
    int words = 0, lines = 0, chars = c->size();
    bool in_word = false;
    for (char ch : *c) {
        if (ch == '\n') lines++;
        if (isspace(ch)) { in_word = false; }
        else if (!in_word) { in_word = true; words++; }
    }
    // Word frequency
    istringstream ss2(*c);
    string w;
    while (ss2 >> w) {
        // Strip punctuation
        string clean;
        for (char ch : w) if (isalnum(ch)) clean += tolower(ch);
        if (!clean.empty()) freq[clean]++;
    }
    cout << "\n  " << clr::bold << "Word Count:" << clr::reset << " " << clr::cyan << words << clr::reset << "  "
         << clr::bold << "Lines:" << clr::reset << " " << clr::cyan << lines << clr::reset << "  "
         << clr::bold << "Chars:" << clr::reset << " " << clr::cyan << chars << clr::reset << "\n\n";
    // Top 10 words
    vector<pair<string,int>> sorted_words(freq.begin(), freq.end());
    sort(sorted_words.begin(), sorted_words.end(), [](auto& a, auto& b) { return a.second > b.second; });
    cout << "  " << clr::bold << "Top Words:" << clr::reset << "\n";
    for (int i = 0; i < min(10, (int)sorted_words.size()); i++) {
        int bar = min(20, sorted_words[i].second);
        cout << "    " << clr::gray << sorted_words[i].first << clr::reset
             << string(max(0, 15 - (int)sorted_words[i].first.size()), ' ')
             << clr::cyan << repeat(bar, "█") << clr::reset << " " << sorted_words[i].second << "\n";
    }
    cout << "\n";
}

void cmd_matrix(int rows) {
    if (rows <= 0) rows = 20;
    cout << "\033[2J\033[1;1H";
    string chars = "アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヲン0123456789";
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < 60; c++) {
            if (rng_int(0, 3) == 0) cout << clr::green << chars[rng_int(0, (int)chars.size() - 1)] << clr::reset;
            else cout << clr::dgray << chars[rng_int(0, (int)chars.size() - 1)] << clr::reset;
        }
        cout << "\n";
    }
    cout << "\n";
}

void cmd_countdown(int sec) {
    if (sec <= 0 || sec > 600) { cout << "Usage: countdown <1-600>\n"; return; }
    for (int i = sec; i > 0; i--) {
        int m = i / 60, s = i % 60;
        cout << "\r  " << clr::bold << clr::cyan << (m < 10 ? "0" : "") << m << ":" << (s < 10 ? "0" : "") << s << clr::reset << "  " << clr::dgray << repeat((int)((float)i / sec * 30), "█") << repeat(max(0, 30 - (int)((float)i / sec * 30)), "░") << clr::reset << flush;
        this_thread::sleep_for(chrono::seconds(1));
    }
    cout << "\r  " << clr::success << "00:00  " << repeat(30, "█") << clr::reset << "  TIME'S UP!\n\n";
}

void cmd_ascii() {
    const vector<string> arts[] = {
        {"    /\\_/\\  ", "   ( o.o ) ", "    > ^ <  ", "   /|   |\\", "  (_|   |_)"},
        {"   /|  /|  ", "  /_| /_|  ", "  |  |  |  ", "  |__|__|  ", "  ||  ||   "},
        {"   (  )    ", "  (``__)   ", "  /|      ", " / |      ", "(_/       "},
        {"   .---.   ", "  /     \\  ", " | () () | ", "  \\  ^  /  ", "   '---'   "},
        {"  ____     ", " / __ \\___ ", "/ /_/ / _ \\", "\\____/_//_/"},
    };
    int pick = rng_int(0, 4);
    for (auto& line : arts[pick]) cout << "  " << clr::cyan << line << clr::reset << "\n";
    cout << "\n";
}

void cmd_hexdump(const string& args, map<string,FSNode>& fs, const string& cdir) {
    if (args.empty()) { cout << "Usage: hexdump <file|text>\n"; return; }
    auto c = vfs_read(args, fs, cdir);
    string text = c ? *c : args;
    for (size_t i = 0; i < text.size(); i += 16) {
        cout << "  " << clr::cyan << setw(8) << setfill('0') << hex << i << clr::reset << "  ";
        for (size_t j = i; j < i + 16 && j < text.size(); j++)
            cout << clr::gray << setw(2) << setfill('0') << hex << (int)(unsigned char)text[j] << " " << clr::reset;
        cout << "  " << clr::dgray << "|";
        for (size_t j = i; j < i + 16 && j < text.size(); j++)
            cout << (isprint(text[j]) ? string(1, text[j]) : ".");
        cout << "|\n";
    }
    cout << dec;
}

void cmd_quote() {
    vector<string> q = {
        "\"The only way to do great work is to love what you do.\" — Steve Jobs",
        "\"Innovation distinguishes between a leader and a follower.\" — Steve Jobs",
        "\"Life is what happens when you're busy making other plans.\" — John Lennon",
        "\"The future belongs to those who believe in the beauty of their dreams.\" — Eleanor Roosevelt",
        "\"It does not matter how slowly you go as long as you do not stop.\" — Confucius"
    };
    cout << "\n  " << clr::italic << clr::yellow << q[rng_int(0, (int)q.size() - 1)] << clr::reset << "\n\n";
}

void cmd_joke() {
    vector<pair<string,string>> jokes = {
        {"Why do programmers prefer dark mode?", "Because light attracts bugs."},
        {"Why do Java developers wear glasses?", "Because they don't C#."},
        {"What's a programmer's favorite hangout place?", "Foo Bar."},
        {"Why did the programmer quit his job?", "Because he didn't get arrays."},
        {"How many programmers does it take to change a light bulb?", "None — that's a hardware problem."}
    };
    auto [setup, punchline] = jokes[rng_int(0, (int)jokes.size() - 1)];
    cout << "\n  " << clr::bold << clr::white << setup << clr::reset << "\n";
    cout << "  " << clr::cyan << punchline << clr::reset << "\n\n";
}

void cmd_ip() {
    int a = rng_int(10, 192), b = rng_int(0, 255), c = rng_int(0, 255), d = rng_int(1, 254);
    cout << "  " << clr::bold << "IPv4:" << clr::reset << "  " << clr::cyan << a << "." << b << "." << c << "." << d << clr::reset << "\n";
    cout << "  " << clr::bold << "IPv6:" << clr::reset << "  " << clr::cyan << "fe80::" << rng_int(0,9999) << ":" << rng_int(0,9999) << clr::reset << "\n\n";
}

void cmd_mem() {
    int total = 32768, used = rng_int(12000, 28000);
    int free = total - used;
    int pct = used * 100 / total;
    cout << "\n  " << clr::bold << "Memory Usage:" << clr::reset << "\n\n";
    cout << "  " << clr::gray << "Total:    " << clr::white << total << " KB" << clr::reset << "\n";
    cout << "  " << clr::gray << "Used:     " << clr::red << used << " KB (" << pct << "%)" << clr::reset << "\n";
    cout << "  " << clr::gray << "Free:     " << clr::green << free << " KB" << clr::reset << "\n\n";
    cout << "  " << clr::dgray << "[";
    int bar_pos = 30 * pct / 100;
    for (int i = 0; i < 30; i++) {
        if (i < bar_pos) cout << clr::red << "█" << clr::reset;
        else cout << clr::green << "░" << clr::reset;
    }
    cout << clr::dgray << "]" << clr::reset << "\n\n";
}

void cmd_cpu() {
    cout << "\n  " << clr::bold << "CPU Usage:" << clr::reset << "\n\n";
    for (int i = 0; i < 4; i++) {
        int usage = rng_int(5, 95);
        cout << "  Core " << i << ":  " << clr::dgray << "[";
        int bar = 25 * usage / 100;
        for (int j = 0; j < 25; j++) {
            if (j < bar) cout << (usage > 80 ? clr::red : usage > 50 ? clr::yellow : clr::green) << "█" << clr::reset;
            else cout << clr::dgray << "░" << clr::reset;
        }
        cout << "] " << clr::cyan << usage << "%" << clr::reset << "\n";
    }
    cout << "\n";
}

void cmd_disk() {
    cout << "\n  " << clr::bold << "Disk Usage:" << clr::reset << "\n\n";
    struct { string mount; int total; int used; } disks[] = {
        {"/", 512000, rng_int(100000, 400000)},
        {"/home", 256000, rng_int(50000, 200000)},
        {"/tmp", 16384, rng_int(1000, 8000)}
    };
    for (auto& d : disks) {
        int pct = d.used * 100 / d.total;
        cout << "  " << clr::gray << d.mount << clr::reset << string(max(0, 10 - (int)d.mount.size()), ' ')
             << clr::dgray << "[";
        int bar = 20 * pct / 100;
        for (int j = 0; j < 20; j++) {
            if (j < bar) cout << (pct > 80 ? clr::red : pct > 50 ? clr::yellow : clr::green) << "█" << clr::reset;
            else cout << clr::dgray << "░" << clr::reset;
        }
        cout << "] " << clr::cyan << d.used / 1024 << "G/" << d.total / 1024 << "G" << clr::reset << "\n";
    }
    cout << "\n";
}

void cmd_uptime2() {
    auto elapsed = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - program_start).count();
    int d = elapsed / 86400, h = (elapsed % 86400) / 3600, m = (elapsed % 3600) / 60, s = elapsed % 60;
    cout << "\n  " << clr::bold << "System Uptime:" << clr::reset << "\n\n";
    cout << "  " << clr::cyan << d << "d " << h << "h " << m << "m " << s << "s" << clr::reset << "\n\n";
    cout << "  " << clr::dgray << "[";
    int bar = min(40, (int)(elapsed * 40 / 86400));
    for (int i = 0; i < 40; i++) cout << (i < bar ? clr::cyan : clr::dgray) << "━" << clr::reset;
    cout << "] " << clr::dgray << "1 day" << clr::reset << "\n\n";
}

void cmd_calc2(const string& args) {
    if (args.empty()) { cout << "Usage: calc2 <expr>\n"; return; }
    // Convert infix to postfix and evaluate
    auto precedence = [](char op) -> int {
        if (op == '+' || op == '-') return 1;
        if (op == '*' || op == '/') return 2;
        return 0;
    };
    auto apply_op = [](double a, double b, char op) -> double {
        switch(op) {
            case '+': return a + b;
            case '-': return a - b;
            case '*': return a * b;
            case '/': return b != 0 ? a / b : 0;
        }
        return 0;
    };

    vector<double> nums;
    vector<char> ops;
    istringstream ss(args);
    double val;
    char op;
    if (ss >> val) {
        nums.push_back(val);
        while (ss >> op >> val) {
            while (!ops.empty() && precedence(ops.back()) >= precedence(op)) {
                double b = nums.back(); nums.pop_back();
                double a = nums.back(); nums.pop_back();
                nums.push_back(apply_op(a, b, ops.back()));
                ops.pop_back();
            }
            ops.push_back(op);
            nums.push_back(val);
        }
    }
    while (!ops.empty()) {
        double b = nums.back(); nums.pop_back();
        double a = nums.back(); nums.pop_back();
        nums.push_back(apply_op(a, b, ops.back()));
        ops.pop_back();
    }
    if (!nums.empty()) cout << "  " << clr::cyan << "= " << nums.back() << clr::reset << "\n\n";
}

// ═══════════════════════════════════════════════════════════════
//  MEGA BATCH — 5 new games + 40+ new commands
// ═══════════════════════════════════════════════════════════════

// --- MEMORY CARDS (match pairs) ---
void play_memory() {
    const int ROWS = 4, COLS = 4;
    int pairs = ROWS * COLS / 2;
    vector<int> cards;
    for (int i = 0; i < pairs; i++) { cards.push_back(i); cards.push_back(i); }
    shuffle(cards.begin(), cards.end(), rng());
    vector<vector<int>> grid(ROWS, vector<int>(COLS));
    vector<vector<bool>> revealed(ROWS, vector<bool>(COLS, false));
    vector<vector<bool>> matched(ROWS, vector<bool>(COLS, false));
    for (int r = 0; r < ROWS; r++) for (int c = 0; c < COLS; c++) grid[r][c] = cards[r * COLS + c];

    const string emojis[] = {"🍎","🍊","🍋","🍇","🍉","🍒","🥑","🍑"};
    int moves = 0, found = 0;
    int sel_r = -1, sel_c = -1;

    while (found < pairs) {
        cout << "\033[2J\033[1;1H";
        cout << "\n  " << clr::bold << clr::lmagenta << "🃏 MEMORY" << clr::reset << "  " << clr::gray << "Moves:" << clr::reset << " " << clr::yellow << moves << clr::reset << "  " << clr::gray << "Pairs:" << clr::reset << " " << clr::cyan << found << "/" << pairs << clr::reset << "\n\n";
        for (int r = 0; r < ROWS; r++) {
            cout << "  ";
            for (int c = 0; c < COLS; c++) {
                if (matched[r][c]) cout << "  " << clr::dgray << "· " << clr::reset;
                else if (revealed[r][c]) cout << "  " << clr::yellow << emojis[grid[r][c]] << clr::reset;
                else cout << "  " << clr::lcyan << "■ " << clr::reset;
            }
            cout << "\n";
        }
        if (sel_r >= 0) {
            cout << "\n  " << clr::gray << "Selected: (" << sel_r+1 << "," << sel_c+1 << ")" << clr::reset;
        }
        cout << "\n  " << clr::dgray << "Enter r c (e.g. 1 2): " << clr::reset;
        string line; getline(cin, line);
        if (line == "q" || line == "Q") break;
        istringstream iss(line);
        int r, c;
        if (!(iss >> r >> c)) continue;
        r--; c--;
        if (r < 0 || r >= ROWS || c < 0 || c >= COLS) continue;
        if (matched[r][c] || revealed[r][c]) continue;

        revealed[r][c] = true;
        moves++;

        if (sel_r >= 0) {
            if (grid[r][c] == grid[sel_r][sel_c] && !(r == sel_r && c == sel_c)) {
                matched[r][c] = matched[sel_r][sel_c] = true;
                found++;
            } else {
                this_thread::sleep_for(chrono::milliseconds(800));
                revealed[r][c] = revealed[sel_r][sel_c] = false;
            }
            sel_r = sel_c = -1;
        } else {
            sel_r = r; sel_c = c;
        }
    }
    cout << "\n  " << clr::success << ">> COMPLETE in " << moves << " moves!" << clr::reset << "\n";
    cout << "Press Enter to return to NoNameOS...";
    cin.get();
}

// --- CONNECT FOUR ---
void play_connect4() {
    const int R = 6, C = 7;
    vector<vector<int>> grid(R, vector<int>(C, 0)); // 0=empty, 1=player, 2=AI
    int turn = 0;

    auto check_win = [&](int p) -> bool {
        for (int r = 0; r < R; r++) for (int c = 0; c < C; c++) {
            if (grid[r][c] != p) continue;
            if (c+3<C && grid[r][c+1]==p && grid[r][c+2]==p && grid[r][c+3]==p) return true;
            if (r+3<R && grid[r+1][c]==p && grid[r+2][c]==p && grid[r+3][c]==p) return true;
            if (r+3<R && c+3<C && grid[r+1][c+1]==p && grid[r+2][c+2]==p && grid[r+3][c+3]==p) return true;
            if (r+3<R && c>=3 && grid[r+1][c-1]==p && grid[r+2][c-2]==p && grid[r+3][c-3]==p) return true;
        }
        return false;
    };

    auto drop = [&](int col, int p) -> bool {
        for (int r = R-1; r >= 0; r--) {
            if (grid[r][col] == 0) { grid[r][col] = p; return true; }
        }
        return false;
    };

    while (true) {
        cout << "\033[2J\033[1;1H";
        cout << "\n  " << clr::bold << clr::red << "🔴 CONNECT FOUR" << clr::reset << "  " << clr::gray << "(1-7 columns, q=quit)" << clr::reset << "\n\n";
        // Column numbers
        cout << "  ";
        for (int c = 0; c < C; c++) cout << " " << clr::dgray << c+1 << " " << clr::reset;
        cout << "\n  " << clr::dgray << repeat(C*4, "─") << clr::reset << "\n";
        for (int r = 0; r < R; r++) {
            cout << "  " << clr::dgray << "│" << clr::reset;
            for (int c = 0; c < C; c++) {
                if (grid[r][c] == 0) cout << clr::dgray << " · " << clr::reset;
                else if (grid[r][c] == 1) cout << clr::red << " ● " << clr::reset;
                else cout << clr::yellow << " ● " << clr::reset;
            }
            cout << clr::dgray << "│" << clr::reset << "\n";
        }
        cout << "  " << clr::dgray << repeat(C*4, "─") << clr::reset << "\n";

        if (check_win(1)) { cout << "\n  " << clr::success << ">> YOU WIN!" << clr::reset << "\n"; break; }
        if (check_win(2)) { cout << "\n  " << clr::error << ">> AI WINS!" << clr::reset << "\n"; break; }
        bool full = true;
        for (int c = 0; c < C; c++) if (grid[0][c] == 0) full = false;
        if (full) { cout << "\n  " << clr::warning << ">> DRAW!" << clr::reset << "\n"; break; }

        if (turn == 0) {
            cout << "  " << clr::gray << "Your move (1-7): " << clr::reset;
            string line; getline(cin, line);
            if (line == "q" || line == "Q") break;
            int col = 0;
            for (char ch : line) if (ch >= '1' && ch <= '7') col = ch - '0';
            if (col < 1 || col > 7) continue;
            if (!drop(col-1, 1)) { cout << "  Column full!\n"; this_thread::sleep_for(chrono::milliseconds(500)); continue; }
        } else {
            // AI: try center, then neighbors
            int best = 3;
            for (int c = 0; c < C; c++) if (grid[0][c] == 0) { best = c; break; }
            // Check if AI can win
            for (int c = 0; c < C; c++) {
                if (grid[0][c] != 0) continue;
                for (int r = 0; r < R; r++) if (grid[r][c] == 0) { grid[r][c] = 2; if (check_win(2)) { best = c; grid[r][c] = 0; goto ai_done; } grid[r][c] = 0; break; }
            }
            ai_done:
            drop(best, 2);
        }
        turn = 1 - turn;
    }
    cout << "Press Enter to return to NoNameOS...";
    cin.get();
}

// --- LIGHTS OUT ---
void play_lightsout() {
    const int N = 5;
    vector<vector<bool>> grid(N, vector<bool>(N, false));
    // Scramble: flip random cells
    for (int i = 0; i < 15; i++) {
        int r = rng_int(0, N-1), c = rng_int(0, N-1);
        grid[r][c] = !grid[r][c];
        if (r > 0) grid[r-1][c] = !grid[r-1][c];
        if (r < N-1) grid[r+1][c] = !grid[r+1][c];
        if (c > 0) grid[r][c-1] = !grid[r][c-1];
        if (c < N-1) grid[r][c+1] = !grid[r][c+1];
    }
    // Make sure not already solved
    bool all_off = true;
    for (int r = 0; r < N; r++) for (int c = 0; c < N; c++) if (grid[r][c]) all_off = false;
    if (all_off) grid[2][2] = true;

    int moves = 0;
    while (true) {
        cout << "\033[2J\033[1;1H";
        cout << "\n  " << clr::bold << clr::yellow << "💡 LIGHTS OUT" << clr::reset << "  " << clr::gray << "Moves:" << clr::reset << " " << clr::cyan << moves << clr::reset << "  " << clr::dgray << "(r c to toggle, q=quit)" << clr::reset << "\n\n";
        for (int r = 0; r < N; r++) {
            cout << "  ";
            for (int c = 0; c < N; c++) {
                if (grid[r][c]) cout << clr::bold << clr::yellow << " ■ " << clr::reset;
                else cout << clr::dgray << " □ " << clr::reset;
            }
            cout << "\n";
        }
        bool won = true;
        for (int r = 0; r < N; r++) for (int c = 0; c < N; c++) if (grid[r][c]) won = false;
        if (won) { cout << "\n  " << clr::success << ">> ALL LIGHTS OFF in " << moves << " moves!" << clr::reset << "\n"; break; }
        cout << "\n  " << clr::dgray << "r c: " << clr::reset;
        string line; getline(cin, line);
        if (line == "q" || line == "Q") break;
        istringstream iss(line);
        int r, c; if (!(iss >> r >> c)) continue;
        r--; c--;
        if (r < 0 || r >= N || c < 0 || c >= N) continue;
        grid[r][c] = !grid[r][c];
        if (r > 0) grid[r-1][c] = !grid[r-1][c];
        if (r < N-1) grid[r+1][c] = !grid[r+1][c];
        if (c > 0) grid[r][c-1] = !grid[r][c-1];
        if (c < N-1) grid[r][c+1] = !grid[r][c+1];
        moves++;
    }
    cout << "Press Enter to return to NoNameOS...";
    cin.get();
}

// --- SLIDING PUZZLE (15-puzzle) ---
void play_puzzle() {
    const int N = 4;
    vector<int> tiles;
    for (int i = 1; i < N*N; i++) tiles.push_back(i);
    tiles.push_back(0);
    // Shuffle with solvability check
    do { shuffle(tiles.begin(), tiles.end(), rng()); } while (true);
    // Simple shuffle by making random moves from solved state
    tiles = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0};
    int blank = 15;
    for (int i = 0; i < 200; i++) {
        vector<int> dirs;
        if (blank >= N) dirs.push_back(-N);
        if (blank < N*N-N) dirs.push_back(N);
        if (blank % N > 0) dirs.push_back(-1);
        if (blank % N < N-1) dirs.push_back(1);
        int d = dirs[rng_int(0, (int)dirs.size()-1)];
        swap(tiles[blank], tiles[blank+d]);
        blank += d;
    }

    int moves = 0;
    while (true) {
        cout << "\033[2J\033[1;1H";
        cout << "\n  " << clr::bold << clr::cyan << "🔢 SLIDING PUZZLE" << clr::reset << "  " << clr::gray << "Moves:" << clr::reset << " " << clr::yellow << moves << clr::reset << "  " << clr::dgray << "(WASD to slide, q=quit)" << clr::reset << "\n\n";
        for (int r = 0; r < N; r++) {
            cout << "  ";
            for (int c = 0; c < N; c++) {
                int v = tiles[r*N+c];
                if (v == 0) cout << "    ";
                else if (v == r*N+c+1) cout << " " << clr::green << setw(2) << v << clr::reset << " ";
                else cout << " " << clr::white << setw(2) << v << clr::reset << " ";
            }
            cout << "\n\n";
        }
        bool won = true;
        for (int i = 0; i < N*N-1; i++) if (tiles[i] != i+1) won = false;
        if (won) { cout << "  " << clr::success << ">> SOLVED in " << moves << " moves!" << clr::reset << "\n"; break; }
        cout << "  " << clr::dgray << "Direction: " << clr::reset;
        string line; getline(cin, line);
        if (line == "q" || line == "Q") break;
        char dir = line.empty() ? ' ' : line[0];
        int dr = 0, dc = 0;
        if (dir == 'w') dr = -1;
        else if (dir == 's') dr = 1;
        else if (dir == 'a') dc = -1;
        else if (dir == 'd') dc = 1;
        if (dr == 0 && dc == 0) continue;
        int br = blank / N, bc = blank % N;
        int nr = br + dr, nc = bc + dc;
        if (nr < 0 || nr >= N || nc < 0 || nc >= N) continue;
        swap(tiles[blank], tiles[nr*N+nc]);
        blank = nr*N+nc;
        moves++;
    }
    cout << "Press Enter to return to NoNameOS...";
    cin.get();
}

// --- BREAKOUT ---
void play_breakout() {
    const int W = 30, H = 20;
    int paddle = W/2 - 3, paddle_w = 6;
    int ball_x = W/2, ball_y = H-3;
    int dx = 1, dy = -1;
    int score = 0;
    vector<vector<bool>> bricks(H, vector<bool>(W, false));
    for (int r = 2; r < 7; r++) for (int c = 1; c < W-1; c++) bricks[r][c] = true;
    bool game_over = false;

    while (!game_over) {
        if (kbhit()) {
            char c = getchar();
            if (c == 'a' && paddle > 0) paddle--;
            else if (c == 'd' && paddle + paddle_w < W) paddle++;
            else if (c == 'q') break;
        }
        ball_x += dx; ball_y += dy;
        if (ball_x <= 0 || ball_x >= W-1) dx = -dx;
        if (ball_y <= 0) dy = -dy;
        if (ball_y >= H) { game_over = true; break; }

        // Paddle bounce
        if (ball_y == H-2 && ball_x >= paddle && ball_x < paddle + paddle_w) { dy = -1; }

        // Brick collision
        if (ball_y >= 0 && ball_y < H && ball_x >= 0 && ball_x < W && bricks[ball_y][ball_x]) {
            bricks[ball_y][ball_x] = false;
            dy = -dy;
            score += 10;
        }

        // Check win
        bool all_clear = true;
        for (int r = 0; r < H; r++) for (int c = 0; c < W; c++) if (bricks[r][c]) all_clear = false;
        if (all_clear) { cout << "\n  " << clr::success << ">> LEVEL COMPLETE!" << clr::reset << "\n"; break; }

        // Render
        cout << "\033[2J\033[1;1H";
        cout << "\n  " << clr::bold << clr::orange << "🧱 BREAKOUT" << clr::reset << "  " << clr::gray << "Score:" << clr::reset << " " << clr::yellow << score << clr::reset << "  " << clr::dgray << "(AD=move Q=quit)" << clr::reset << "\n\n";
        for (int r = 0; r < H; r++) {
            cout << "  ";
            for (int c = 0; c < W; c++) {
                if (r == ball_y && c == ball_x) cout << clr::bold << clr::yellow << "●" << clr::reset;
                else if (r == H-2 && c >= paddle && c < paddle + paddle_w) cout << clr::cyan << "█" << clr::reset;
                else if (bricks[r][c]) {
                    string bc = (r < 3) ? clr::red : (r < 5) ? clr::yellow : clr::green;
                    cout << bc << "█" << clr::reset;
                }
                else cout << " ";
            }
            cout << "\n";
        }
        this_thread::sleep_for(chrono::milliseconds(50));
    }
    cout << "\n  " << clr::error << ">> GAME OVER" << clr::reset << "  " << clr::gray << "Score:" << clr::reset << " " << clr::yellow << score << clr::reset << "\n";
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(CIN_IGNORE_MAX), '\n');
    cin.get();
}

// --- WHACK-A-MOLE ---
void play_whack() {
    int score = 0, misses = 0, round = 0;
    cout << "\033[2J\033[1;1H";
    cout << "\n  " << clr::bold << clr::green << "🔨 WHACK-A-MOLE" << clr::reset << "  " << clr::dgray << "(1-9 to whack, 0=miss, q=quit)" << clr::reset << "\n\n";
    cout << "  Press Enter to start..."; cin.get();

    while (misses < 3 && round < 15) {
        int hole = rng_int(1, 9);
        int time_ms = max(300, 1200 - round * 60);

        cout << "\033[2J\033[1;1H";
        cout << "\n  " << clr::bold << clr::green << "🔨 WHACK-A-MOLE" << clr::reset << "  " << clr::gray << "Score:" << clr::reset << " " << clr::yellow << score << clr::reset
             << "  " << clr::gray << "Misses:" << clr::reset << " " << clr::red << misses << "/3" << clr::reset
             << "  " << clr::gray << "Round:" << clr::reset << " " << clr::cyan << round+1 << "/15" << clr::reset << "\n\n";
        for (int r = 0; r < 3; r++) {
            cout << "  ";
            for (int c = 0; c < 3; c++) {
                int n = r*3+c+1;
                if (n == hole) cout << "  " << clr::bold << clr::yellow << "🐹" << clr::reset << " ";
                else cout << "  " << clr::dgray << "🕳️ " << clr::reset << " ";
            }
            cout << "\n\n";
        }

        TerminalGuard guard;
        auto start = chrono::steady_clock::now();
        bool whacked = false;
        while (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count() < time_ms) {
            if (kbhit()) {
                char c = getchar();
                int n = c - '0';
                if (n == hole) { score += 10; whacked = true; break; }
                else if (n >= 1 && n <= 9) { misses++; whacked = true; break; }
                else if (c == 'q') { cout << "Press Enter to return to NoNameOS..."; cin.get(); return; }
            }
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        if (!whacked) misses++;
        round++;
    }
    cout << "\n  " << (misses >= 3 ? clr::error : clr::success) << ">> GAME OVER" << clr::reset << "  " << clr::gray << "Score:" << clr::reset << " " << clr::yellow << score << clr::reset << "\n";
    cout << "Press Enter to return to NoNameOS...";
    cin.ignore(static_cast<std::streamsize>(CIN_IGNORE_MAX), '\n');
    cin.get();
}

// --- MASSIVE COMMAND BATCH ---

void cmd_bmi(const string& args) {
    istringstream ss(args);
    double weight, height;
    if (!(ss >> weight >> height)) { cout << "Usage: bmi <weight_kg> <height_m>\n"; return; }
    double bmi = weight / (height * height);
    string cat;
    if (bmi < 18.5) cat = "Underweight";
    else if (bmi < 25) cat = "Normal";
    else if (bmi < 30) cat = "Overweight";
    else cat = "Obese";
    cout << "\n  " << clr::bold << "BMI:" << clr::reset << " " << clr::cyan << bmi << clr::reset << "  " << clr::gray << "(" << cat << ")" << clr::reset << "\n\n";
}

void cmd_tip(const string& args) {
    istringstream ss(args);
    double bill; int pct;
    if (!(ss >> bill >> pct)) { cout << "Usage: tip <bill> <percent>\n"; return; }
    double tip = bill * pct / 100.0;
    double total = bill + tip;
    cout << "\n  " << clr::gray << "Bill:" << clr::reset << "    $" << clr::white << bill << clr::reset << "\n";
    cout << "  " << clr::gray << "Tip:" << clr::reset << "     $" << clr::cyan << tip << clr::reset << " (" << pct << "%)\n";
    cout << "  " << clr::gray << "Total:" << clr::reset << "   $" << clr::green << total << clr::reset << "\n\n";
}

void cmd_units(const string& args) {
    istringstream ss(args);
    string val, from, to;
    ss >> val >> from >> to;
    double v = stod(val);
    double result = 0;
    // Length conversions
    if (from == "km" && to == "mi") result = v * 0.621371;
    else if (from == "mi" && to == "km") result = v * 1.60934;
    else if (from == "m" && to == "ft") result = v * 3.28084;
    else if (from == "ft" && to == "m") result = v * 0.3048;
    else if (from == "cm" && to == "in") result = v * 0.393701;
    else if (from == "in" && to == "cm") result = v * 2.54;
    else if (from == "kg" && to == "lb") result = v * 2.20462;
    else if (from == "lb" && to == "kg") result = v * 0.453592;
    else if (from == "c" && to == "f") result = v * 9.0/5.0 + 32;
    else if (from == "f" && to == "c") result = (v - 32) * 5.0/9.0;
    else if (from == "l" && to == "gal") result = v * 0.264172;
    else if (from == "gal" && to == "l") result = v * 3.78541;
    else { cout << "Unknown conversion: " << from << " -> " << to << "\n"; return; }
    cout << "  " << clr::cyan << v << " " << from << clr::reset << " = " << clr::green << result << " " << to << clr::reset << "\n";
}

void cmd_roman(const string& args) {
    int n = 0;
    for (char c : args) if (c >= '0' && c <= '9') n = n * 10 + (c - '0');
    if (n < 1 || n > 3999) { cout << "Enter 1-3999\n"; return; }
    struct { int val; const char* sym; } vals[] = {
        {1000,"M"},{900,"CM"},{500,"D"},{400,"CD"},{100,"C"},{90,"XC"},{50,"L"},{40,"XL"},{10,"X"},{9,"IX"},{5,"V"},{4,"IV"},{1,"I"}
    };
    string result;
    for (auto& [v, s] : vals) while (n >= v) { result += s; n -= v; }
    cout << "  " << clr::cyan << result << clr::reset << "\n";
}

void cmd_binary(const string& args) {
    int n = 0;
    for (char c : args) if (c >= '0' && c <= '9') n = n * 10 + (c - '0');
    if (n == 0) { cout << "  0\n"; return; }
    string bin;
    while (n > 0) { bin = (n % 2 ? "1" : "0") + bin; n /= 2; }
    cout << "  " << clr::cyan << bin << clr::reset << "\n";
}

void cmd_morse(const string& args) {
    const char* morse[] = {".-","-...","-.-.","-..",".","..-.","--.","....","..",".---","-.-",".-..","--","-.","---",".--.","--.-",".-.","...","-","..-","...-",".--","-..-","-.--","--.."};
    cout << "  ";
    for (char c : args) {
        if (c == ' ') cout << "   ";
        else if (isalpha(c)) cout << clr::cyan << morse[tolower(c)-'a'] << clr::reset << " ";
        else cout << c;
    }
    cout << "\n";
}

void cmd_bar(const string& args) {
    istringstream ss(args);
    string label; int val;
    cout << "\n  " << clr::bold << "Bar Chart:" << clr::reset << "\n\n";
    while (ss >> label >> val) {
        int bar = min(30, val);
        cout << "  " << clr::gray << setw(10) << label << clr::reset << " " << clr::cyan << repeat(bar, "█") << clr::reset << " " << val << "\n";
    }
    cout << "\n";
}

void cmd_sparkline(const string& args) {
    const string blocks = "▁▂▃▄▅▆▇█";
    istringstream ss(args);
    vector<int> vals;
    int v;
    while (ss >> v) vals.push_back(v);
    if (vals.empty()) { cout << "Usage: sparkline <num1> <num2> ...\n"; return; }
    int mn = *min_element(vals.begin(), vals.end());
    int mx = *max_element(vals.begin(), vals.end());
    int range = mx - mn;
    cout << "  ";
    for (int val : vals) {
        int idx = range > 0 ? (val - mn) * 7 / range : 3;
        cout << clr::cyan << string(1, blocks[idx]) << clr::reset;
    }
    cout << "  " << clr::dgray << "[" << mn << ".." << mx << "]" << clr::reset << "\n";
}

void cmd_colorgen() {
    int r = rng_int(0, 255), g = rng_int(0, 255), b = rng_int(0, 255);
    cout << "\n  " << clr::rgb(r,g,b) << "████████████████████" << clr::reset << "\n";
    cout << "  " << clr::gray << "RGB(" << r << ", " << g << ", " << b << ")" << clr::reset << "\n";
    char hex[8];
    snprintf(hex, sizeof(hex), "#%02X%02X%02X", r, g, b);
    cout << "  " << clr::gray << hex << clr::reset << "\n\n";
}

void cmd_palette() {
    int base = rng_int(0, 360);
    cout << "\n  " << clr::bold << "Color Palette:" << clr::reset << "\n\n  ";
    for (int i = 0; i < 10; i++) {
        int h = (base + i * 36) % 360;
        int r, g, b;
        // HSL to RGB simplified
        double s = 0.7, l = 0.5;
        double c = (1 - abs(2*l-1)) * s;
        double x = c * (1 - abs(fmod(h/60.0, 2) - 1));
        double m = l - c/2;
        if (h < 60) { r=c; g=x; b=0; }
        else if (h < 120) { r=x; g=c; b=0; }
        else if (h < 180) { r=0; g=c; b=x; }
        else if (h < 240) { r=0; g=x; b=c; }
        else if (h < 300) { r=x; g=0; b=c; }
        else { r=c; g=0; b=x; }
        int ri = (int)((r+m)*255), gi = (int)((g+m)*255), bi = (int)((b+m)*255);
        cout << clr::rgb(ri,gi,bi) << "██" << clr::reset;
    }
    cout << "\n\n";
}

void cmd_diff(const string& args, map<string,FSNode>& fs, const string& cdir) {
    istringstream ss(args);
    string f1, f2;
    ss >> f1 >> f2;
    if (f1.empty() || f2.empty()) { cout << "Usage: diff <file1> <file2>\n"; return; }
    auto c1 = vfs_read(f1, fs, cdir), c2 = vfs_read(f2, fs, cdir);
    if (!c1 || !c2) { cout << "error: file not found.\n"; return; }
    istringstream s1(*c1), s2(*c2);
    string l1, l2;
    while (true) {
        bool g1 = (bool)getline(s1, l1);
        bool g2 = (bool)getline(s2, l2);
        if (!g1 && !g2) break;
        if (l1 != l2) {
            if (g1) cout << "  " << clr::red << "- " << l1 << clr::reset << "\n";
            if (g2) cout << "  " << clr::green << "+ " << l2 << clr::reset << "\n";
        }
    }
}

void cmd_stats(const string& args) {
    istringstream ss(args);
    vector<double> vals;
    double v;
    while (ss >> v) vals.push_back(v);
    if (vals.empty()) { cout << "Usage: stats <num1> <num2> ...\n"; return; }
    double sum = 0;
    for (double x : vals) sum += x;
    double mean = sum / vals.size();
    double variance = 0;
    for (double x : vals) variance += (x - mean) * (x - mean);
    variance /= vals.size();
    sort(vals.begin(), vals.end());
    double median = vals.size() % 2 ? vals[vals.size()/2] : (vals[vals.size()/2-1]+vals[vals.size()/2])/2;
    cout << "\n  " << clr::bold << "Statistics:" << clr::reset << "\n\n";
    cout << "  " << clr::gray << "Count:" << clr::reset << "   " << clr::cyan << vals.size() << clr::reset << "\n";
    cout << "  " << clr::gray << "Sum:" << clr::reset << "     " << clr::cyan << sum << clr::reset << "\n";
    cout << "  " << clr::gray << "Mean:" << clr::reset << "    " << clr::cyan << mean << clr::reset << "\n";
    cout << "  " << clr::gray << "Median:" << clr::reset << "  " << clr::cyan << median << clr::reset << "\n";
    cout << "  " << clr::gray << "StdDev:" << clr::reset << "  " << clr::cyan << sqrt(variance) << clr::reset << "\n";
    cout << "  " << clr::gray << "Min:" << clr::reset << "     " << clr::green << vals.front() << clr::reset << "\n";
    cout << "  " << clr::gray << "Max:" << clr::reset << "     " << clr::red << vals.back() << clr::reset << "\n\n";
}

void cmd_age(const string& args) {
    istringstream ss(args);
    int y, m, d;
    if (!(ss >> y >> m >> d)) { cout << "Usage: age <birth_year> <month> <day>\n"; return; }
    time_t now = time(nullptr);
    tm t_buf;
    localtime_r(&now, &t_buf);
    int age = t_buf.tm_year + 1900 - y;
    if (t_buf.tm_mon + 1 < m || (t_buf.tm_mon + 1 == m && t_buf.tm_mday < d)) age--;
    cout << "  " << clr::bold << "Age:" << clr::reset << " " << clr::cyan << age << clr::reset << " years\n";
    cout << "  " << clr::gray << "Born: " << y << "-" << setw(2) << setfill('0') << m << "-" << setw(2) << setfill('0') << d << clr::reset << "\n\n";
    cout << dec;
}

void cmd_datecalc(const string& args) {
    istringstream ss(args);
    string op;
    int y, m, d, days;
    if (ss >> y >> m >> d >> op >> days) {
        tm t = {};
        t.tm_year = y - 1900; t.tm_mon = m - 1; t.tm_mday = d;
        t.tm_hour = 12;
        time_t tt = mktime(&t);
        if (op == "+") tt += days * 86400;
        else tt -= days * 86400;
        tm result;
        localtime_r(&tt, &result);
        cout << "  " << clr::cyan << (result.tm_year+1900) << "-" << setw(2) << setfill('0') << result.tm_mon+1 << "-" << setw(2) << setfill('0') << result.tm_mday << clr::reset << "\n";
        cout << dec;
    } else {
        cout << "Usage: datecalc <y m d> +|- <days>\n";
    }
}

void cmd_encode(const string& args) {
    cout << "  " << clr::gray << "ROT13:  " << clr::reset;
    string out = args;
    for (char& c : out) { if (c >= 'a' && c <= 'z') c = 'a' + (c-'a'+13)%26; else if (c >= 'A' && c <= 'Z') c = 'A' + (c-'A'+13)%26; }
    cout << clr::cyan << out << clr::reset << "\n";
    cout << "  " << clr::gray << "Upper:  " << clr::reset;
    string upper = args;
    for (char& c : upper) c = toupper(c);
    cout << clr::cyan << upper << clr::reset << "\n";
    cout << "  " << clr::gray << "Lower:  " << clr::reset;
    string lower = args;
    for (char& c : lower) c = tolower(c);
    cout << clr::cyan << lower << clr::reset << "\n";
}

void cmd_hash(const string& args) {
    // Simple hash (djb2)
    unsigned long hash = 5381;
    for (char c : args) hash = ((hash << 5) + hash) + c;
    cout << "  " << clr::cyan << "0x" << hex << hash << dec << clr::reset << "\n";
}

void cmd_urlencode(const string& args) {
    cout << "  ";
    for (char c : args) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') cout << c;
        else { char buf[4]; snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)c); cout << clr::cyan << buf << clr::reset; }
    }
    cout << "\n";
}

void cmd_urldecode(const string& args) {
    cout << "  ";
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == '%' && i + 2 < args.size()) {
            int val = 0;
            if (args[i+1] >= '0' && args[i+1] <= '9') val = (args[i+1]-'0')*16;
            else val = (tolower(args[i+1])-'a'+10)*16;
            if (args[i+2] >= '0' && args[i+2] <= '9') val += args[i+2]-'0';
            else val += tolower(args[i+2])-'a'+10;
            cout << (char)val;
            i += 2;
        } else if (args[i] == '+') cout << ' ';
        else cout << args[i];
    }
    cout << "\n";
}

void cmd_reverse_str(const string& args) {
    string r = args;
    reverse(r.begin(), r.end());
    cout << "  " << clr::cyan << r << clr::reset << "\n";
}

void cmd_capitalize(const string& args) {
    bool cap = true;
    for (char c : args) {
        if (isspace(c)) { cap = true; cout << c; }
        else if (cap) { cout << (char)toupper(c); cap = false; }
        else cout << c;
    }
    cout << "\n";
}

void cmd_repeat_cmd(const string& args) {
    istringstream ss(args);
    int n; string text;
    ss >> n >> ws;
    getline(ss, text);
    if (n <= 0 || text.empty()) { cout << "Usage: repeat <n> <text>\n"; return; }
    for (int i = 0; i < n; i++) cout << text << "\n";
}

void cmd_scrabble(const string& args) {
    int score = 0;
    map<char,int> pts = {{'a',1},{'b',3},{'c',3},{'d',2},{'e',1},{'f',4},{'g',2},{'h',4},{'i',1},{'j',8},{'k',5},{'l',1},{'m',3},{'n',1},{'o',1},{'p',3},{'q',10},{'r',1},{'s',1},{'t',1},{'u',1},{'v',4},{'w',4},{'x',8},{'y',4},{'z',10}};
    for (char c : args) if (isalpha(c)) score += pts[tolower(c)];
    cout << "  " << clr::bold << "Scrabble Score:" << clr::reset << " " << clr::yellow << score << clr::reset << "\n";
}

void cmd_emoji(const string& args) {
    map<string,string> emojis = {
        {"smile","😊"},{"laugh","😂"},{"cool","😎"},{"heart","❤️"},{"star","⭐"},
        {"fire","🔥"},{"rocket","🚀"},{"sun","☀️"},{"moon","🌙"},{"rain","🌧️"},
        {"snow","❄️"},{"pizza","🍕"},{"coffee","☕"},{"music","🎵"},{"lightning","⚡"},
        {"trophy","🏆"},{"gem","💎"},{"crown","👑"},{"ghost","👻"},{"alien","👽"}
    };
    if (args.empty()) {
        cout << "\n  Available emojis:\n";
        for (auto& [k,v] : emojis) cout << "    " << v << " " << k << "\n";
        cout << "\n";
    } else {
        auto it = emojis.find(args);
        if (it != emojis.end()) cout << "  " << it->second << "\n";
        else cout << "  Unknown emoji: " << args << "\n";
    }
}

void cmd_random(const string& args) {
    istringstream ss(args);
    int lo, hi;
    if (ss >> lo >> hi) {
        cout << "  " << clr::cyan << rng_int(lo, hi) << clr::reset << "\n";
    } else {
        cout << "  " << clr::cyan << rng_int(1, 100) << clr::reset << "\n";
    }
}

void cmd_pick(const string& args) {
    istringstream ss(args);
    vector<string> items;
    string item;
    while (getline(ss, item, '|')) {
        // trim
        while (!item.empty() && item.back() == ' ') item.pop_back();
        while (!item.empty() && item[0] == ' ') item.erase(0, 1);
        if (!item.empty()) items.push_back(item);
    }
    if (items.size() < 2) { cout << "Usage: pick option1 | option2 | option3\n"; return; }
    cout << "  " << clr::cyan << items[rng_int(0, (int)items.size()-1)] << clr::reset << "\n";
}

void cmd_dice(const string& args) {
    istringstream ss(args);
    int n = 1;
    if (!(ss >> n) || n < 1) n = 1;
    for (int i = 0; i < n; i++) {
        int val = rng_int(1, 6);
        cout << "  " << clr::cyan << "🎲 " << val << clr::reset;
    }
    cout << "\n";
}

void cmd_coin() {
    cout << "  " << clr::cyan << (rng_int(0,1) ? "🪙 Heads" : "🪙 Tails") << clr::reset << "\n";
}

void cmd_zodiac(const string& args) {
    istringstream ss(args);
    int m, d;
    if (!(ss >> m >> d)) { cout << "Usage: zodiac <month> <day>\n"; return; }
    string sign;
    if ((m==3 && d>=21) || (m==4 && d<=19)) sign = "Aries ♈";
    else if ((m==4 && d>=20) || (m==5 && d<=20)) sign = "Taurus ♉";
    else if ((m==5 && d>=21) || (m==6 && d<=20)) sign = "Gemini ♊";
    else if ((m==6 && d>=21) || (m==7 && d<=22)) sign = "Cancer ♋";
    else if ((m==7 && d>=23) || (m==8 && d<=22)) sign = "Leo ♌";
    else if ((m==8 && d>=23) || (m==9 && d<=22)) sign = "Virgo ♍";
    else if ((m==9 && d>=23) || (m==10 && d<=22)) sign = "Libra ♎";
    else if ((m==10 && d>=23) || (m==11 && d<=21)) sign = "Scorpio ♏";
    else if ((m==11 && d>=22) || (m==12 && d<=21)) sign = "Sagittarius ♐";
    else if ((m==12 && d>=22) || (m==1 && d<=19)) sign = "Capricorn ♑";
    else if ((m==1 && d>=20) || (m==2 && d<=18)) sign = "Aquarius ♒";
    else sign = "Pisces ♓";
    cout << "  " << clr::cyan << sign << clr::reset << "\n";
}

void cmd_worldclock() {
    time_t now = time(nullptr);
    struct { const char* tz; const char* name; } zones[] = {
        {"America/New_York", "New York"}, {"Europe/London", "London"},
        {"Asia/Tokyo", "Tokyo"}, {"Asia/Shanghai", "Shanghai"},
        {"Australia/Sydney", "Sydney"}, {"America/Los_Angeles", "Los Angeles"}
    };
    cout << "\n  " << clr::bold << "World Clock:" << clr::reset << "\n\n";
    for (auto& [tz, name] : zones) {
        setenv("TZ", tz, 1);
        tm t_buf;
        localtime_r(&now, &t_buf);
        char buf[20];
        strftime(buf, sizeof(buf), "%H:%M:%S", &t_buf);
        cout << "  " << clr::gray << setw(14) << name << clr::reset << "  " << clr::cyan << buf << clr::reset << "\n";
    }
    unsetenv("TZ");
    cout << "\n";
}

void cmd_wordle() {
    vector<string> words = {"apple","beach","cloud","dance","eagle","flame","globe","house","input","joint","knife","lemon","magic","night","ocean","piano","quiet","river","stone","train","unity","vivid","watch","young","zebra","brave","candy","dream","epoch","frost"};
    string target = words[rng_int(0, (int)words.size()-1)];
    int attempts = 6;

    for (int a = 0; a < attempts; a++) {
        cout << "\033[2J\033[1;1H";
        cout << "\n  " << clr::bold << clr::green << "🔤 WORDLE" << clr::reset << "  " << clr::gray << "Attempt " << a+1 << "/" << attempts << clr::reset << "\n\n";
        cout << "  " << clr::dgray << "Type a 5-letter word:" << clr::reset << "\n  ";
        string guess;
        getline(cin, guess);
        if (guess.size() != 5) { a--; continue; }

        // Color feedback
        for (int i = 0; i < 5; i++) {
            if (guess[i] == target[i]) cout << clr::green << (char)toupper(guess[i]) << clr::reset;
            else if (target.find(guess[i]) != string::npos) cout << clr::yellow << (char)toupper(guess[i]) << clr::reset;
            else cout << clr::dgray << (char)toupper(guess[i]) << clr::reset;
        }
        cout << "\n";
        if (guess == target) {
            cout << "\n  " << clr::success << ">> CORRECT!" << clr::reset << "\n"; break;
        }
        if (a == attempts - 1) cout << "\n  " << clr::error << ">> The word was: " << target << clr::reset << "\n";
        cout << "Press Enter to continue...";
        cin.get();
    }
    cout << "Press Enter to return to NoNameOS...";
    cin.get();
}

void cmd_quiz() {
    struct { string q; vector<string> opts; int correct; } questions[] = {
        {"What is the capital of France?", {"London","Paris","Berlin","Rome"}, 1},
        {"How many bits in a byte?", {"4","8","16","32"}, 1},
        {"What planet is known as the Red Planet?", {"Venus","Mars","Jupiter","Saturn"}, 1},
        {"What is 7 * 8?", {"54","56","58","62"}, 1},
        {"Who painted the Mona Lisa?", {"Van Gogh","Da Vinci","Picasso","Monet"}, 1},
        {"What language runs in a browser?", {"Java","C++","Python","JavaScript"}, 3},
        {"What does CPU stand for?", {"Central Process Unit","Central Processing Unit","Computer Personal Unit","Central Program Utility"}, 1},
        {"What year was the first iPhone released?", {"2005","2006","2007","2008"}, 2},
        {"What is the largest ocean?", {"Atlantic","Indian","Arctic","Pacific"}, 3},
        {"What gas do plants absorb?", {"Oxygen","Nitrogen","Carbon Dioxide","Hydrogen"}, 2}
    };
    int score = 0;
    vector<int> order;
    for (int i = 0; i < 10; i++) order.push_back(i);
    shuffle(order.begin(), order.end(), rng());

    cout << "\n  " << clr::bold << clr::amber << "🧠 QUIZ" << clr::reset << "\n\n";
    for (int i = 0; i < 5; i++) {
        auto& q = questions[order[i]];
        cout << "  " << clr::bold << "Q" << i+1 << ": " << q.q << clr::reset << "\n";
        for (int j = 0; j < 4; j++)
            cout << "    " << clr::gray << j+1 << ". " << q.opts[j] << clr::reset << "\n";
        cout << "  Answer (1-4): ";
        string line; getline(cin, line);
        int ans = 0;
        for (char c : line) if (c >= '1' && c <= '4') ans = c - '0';
        if (ans - 1 == q.correct) { cout << "  " << clr::green << "Correct!" << clr::reset << "\n"; score++; }
        else cout << "  " << clr::red << "Wrong! Answer: " << q.correct+1 << clr::reset << "\n";
        cout << "\n";
    }
    cout << "  " << clr::bold << "Score: " << clr::cyan << score << "/5" << clr::reset << "\n\n";
}

void cmd_csv(const string& args, map<string,FSNode>& fs, const string& cdir) {
    if (args.empty()) { cout << "Usage: csv <file>\n"; return; }
    auto c = vfs_read(args, fs, cdir);
    if (!c) { cout << "error: file not found.\n"; return; }
    istringstream ss(*c);
    string line;
    bool header = true;
    while (getline(ss, line)) {
        istringstream ls(line);
        string cell;
        int col = 0;
        if (header) {
            while (getline(ls, cell, ',')) {
                cout << "  " << clr::bold << clr::cyan << setw(12) << cell << clr::reset;
                col++;
            }
            cout << "\n  " << clr::dgray << repeat(col * 12, "─") << clr::reset << "\n";
            header = false;
        } else {
            while (getline(ls, cell, ',')) {
                cout << "  " << clr::gray << setw(12) << cell << clr::reset;
                col++;
            }
            cout << "\n";
        }
    }
    cout << "\n";
}

void cmd_emoji2() {
    const vector<string> faces = {"😀","😂","😍","🥳","😎","🤩","😊","🤗","😌","😴"};
    for (int i = 0; i < 5; i++) {
        cout << "  ";
        for (int j = 0; j < 6; j++) cout << faces[rng_int(0, 9)] << " ";
        cout << "\n";
    }
    cout << "\n";
}

// ═══════════════════════════════════════════════════════════════
//  MAIN
// ═══════════════════════════════════════════════════════════════

    int main() {
    signal(SIGINT, sigint_handler);
    cout << "\033[2J\033[1;1H";

    // --- Boot Logo ---
    const vector<string> logo = {
        R"(  _   _                      _____                       _             )",
        R"( | \ | |                    / ____|                     | |            )",
        R"( |  \| | _____   _____ _ __| (___  _ __   __ _ _ __ ___| |__   ___   )",
        R"( | . ` |/ _ \ \ / / _ \ '__\___ \| '_ \ / _` | '__/ __| '_ \ / _ \  )",
        R"( | |\  |  __/\ V /  __/ |  ____) | |_) | (_| | | | (__| | | |  __/  )",
        R"( |_| \_|\___| \_/ \___|_| |_____/| .__/ \__,_|_|  \___|_| |_|\___|  )",
        R"(                                 | |                                 )",
        R"(                                 |_|                                 )"
    };

    // Animated logo with color gradient
    for (size_t i = 0; i < logo.size(); i++) {
        int r = (i * 36) % 256;
        int g = (128 + i * 20) % 256;
        int b = (255 - i * 30) % 256;
        string color = clr::rgb(r, g, b);
        // Slide in from left
        int pad = max(0, (int)(50 - i * 3));
        cout << string(pad, ' ') << color << clr::bold << logo[i] << clr::reset << "\n";
        this_thread::sleep_for(chrono::milliseconds(40));
    }
    cout << "\n";

    // Version & tagline
    cout << string(20, ' ') << clr::dgray << "═══════════════════════════════════════════" << clr::reset << "\n";
    cout << string(20, ' ') << clr::gray << " " << clr::cyan << VERSION << clr::reset << clr::gray << " · Pure C++ OS Simulation" << clr::reset << "\n";
    cout << string(20, ' ') << clr::dgray << "═══════════════════════════════════════════" << clr::reset << "\n";
    cout << "\n";
    this_thread::sleep_for(chrono::milliseconds(300));

    // --- Animated boot sequence ---
    const vector<pair<string, int>> boot_steps = {
        {"Initializing kernel", 8}, {"Loading terminal driver", 6},
        {"Mounting virtual filesystem", 7}, {"Starting shell", 5},
        {"Loading games engine", 6}, {"Loading system tools", 5},
        {"Configuring users", 4}, {"System ready", 3}
    };

    for (size_t i = 0; i < boot_steps.size(); i++) {
        float pct = (float)(i + 1) / boot_steps.size();
        int bar_w = 35;
        int pos = (int)(bar_w * pct);
        cout << "\r  " << clr::dgray << "[" << clr::reset;
        for (int j = 0; j < bar_w; j++) {
            if (j < pos) cout << clr::green << "━" << clr::reset;
            else if (j == pos) cout << clr::amber << "╸" << clr::reset;
            else cout << clr::dgray << "━" << clr::reset;
        }
        cout << clr::dgray << "]" << clr::reset << " " << clr::gray << boot_steps[i].first << clr::reset;
        cout.flush();
        this_thread::sleep_for(chrono::milliseconds(boot_steps[i].second * 30));
    }
    cout << "\n\n";

    // Success message
    cout << "  " << clr::success << "✓ " << clr::bold << "NoNameOS " << VERSION << " booted successfully" << clr::reset << "\n";
    cout << "  " << clr::muted << "Type " << clr::lcyan << "help" << clr::muted << " for available commands" << clr::reset << "\n\n";
    this_thread::sleep_for(chrono::milliseconds(400));

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
            if (cmd_history.size() > 0 && dur > 100)
                cout << clr::dgray << " " << dur << "ms" << clr::reset;
        }
        cout << "\n" << clr::prompt_user << current_user << clr::prompt_host << "@"
             << clr::prompt_host << "nonameos" << clr::prompt_sep << ":"
             << clr::prompt_dir << current_dir << clr::reset << "\n"
             << clr::prompt_sep << "❯ " << clr::reset;
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
                cout << "\n  " << clr::accent << clr::bold << args << clr::reset << " — " << clr::gray << help_text(args) << clr::reset << "\n\n";
                continue;
            }
            cout << "\n  " << clr::bold << clr::cyan << "NoNameOS " << VERSION << clr::reset << "\n  " << clr::dgray << vsep(42) << clr::reset << "\n";

            auto section = [&](const string& title, const string& cmds) {
                cout << "\n  " << clr::bold << clr::amber << title << clr::reset << "\n";
                istringstream ss(cmds);
                string line;
                while (getline(ss, line)) {
                    if (!line.empty())
                        cout << "    " << clr::gray << line << clr::reset << "\n";
                }
            };

            section("Filesystem", "ls, ls -l, cd, mkdir, touch, cat, echo, rm, rm -r\ngrep, find, locate, cp, cp -r, mv, pwd\ntree, ln -s, chmod, trash, du, head, tail\nsort, wc, tee");
            section("System", "whoami, who, date, uptime, hostname, uname\ncfetch, ps, top, env, printenv, history\nclear, help, man, exit");
            section("Tools", "nano, calc, bc, cowsay, cal, rainbow, lolcat\nyes, sleep, which, alias, unalias, su\nuseradd, userdel, users, banner, fortune\nfactor, shuf, sl, train");
            section("Text", "rev, tr, cut, paste, uniq, nl, fold\nbasename, dirname");
            section("System Info", "free, dmesg, lscpu, lsusb, arch, nproc, ping, watch, df, seq");
            section("Games", "play (AsciiDash), guess, trivia, adventure, snake\nminesweeper, tictactoe (ttt), hangman, rps\n2048, typing, reaction, nummem");
            section("Productivity", "todo, notes, stopwatch, timer, pom, alarm");

            cout << "\n  " << clr::dgray << vsep(42) << clr::reset << "\n";
            cout << "  " << clr::dim << "Tip: " << clr::muted << "type " << clr::lcyan << "help <cmd>" << clr::muted << " or " << clr::lcyan << "man <cmd>" << clr::muted << " for details" << clr::reset << "\n\n";
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
                            cout << "  " << clr::dgray << type << node.mode << "  " << node.size() << "  " << node.created_at << "  " << clr::reset;
                        } else {
                            cout << "  ";
                        }
                        if (node.is_dir) cout << clr::bold << clr::blue << relative << clr::reset << "/  ";
                        else cout << clr::white << relative << clr::reset << "  ";
                        empty = false;
                    }
                }
            }
            if (empty) cout << "  " << clr::dgray << "(Empty)" << clr::reset;
            cout << "\n";
        } 
        else if (cmd == "mkdir") {
            if (args.empty()) { cout << "Usage: mkdir <name>\n"; }
            else {
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
            if (args.empty() || args == "/") current_dir = "/";
            else if (args == "..") {
                if (current_dir != "/") {
                    string temp = current_dir.substr(0, current_dir.length() - 1);
                    current_dir = temp.substr(0, temp.find_last_of('/') + 1);
                }
            } else {
                string target_dir = resolve_user_path(args, current_dir);
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
                if (filename.empty()) {
                    cout << "\033[31merror:\033[0m empty filename.\n";
                } else if (has_traversal(filename)) {
                    cout << "\033[31merror:\033[0m path traversal not allowed.\n";
                } else {
                    string fullpath = resolve_user_path(filename, current_dir);
                    file_system[fullpath] = FSNode(false, content);
                }
            } else {
                cout << "Usage: echo <file> <content>\n";
            }
        }
        else if (cmd == "cat") {
            if (args.empty()) { cout << "Usage: cat <file>\n"; }
            else if (has_traversal(args)) { cout << "\033[31merror:\033[0m path traversal not allowed.\n"; }
            else {
                string fullpath = resolve_user_path(args, current_dir);
                if (file_system.find(fullpath) != file_system.end() && !file_system[fullpath].is_dir) cout << file_system[fullpath].content << "\n";
                else cout << "\033[31merror:\033[0m file not found.\n";
            }
        }
        else if (cmd == "rm") {
            if (args.empty()) { cout << "Usage: rm [-r] <name>\n"; }
            else if (has_traversal(args)) { cout << "\033[31merror:\033[0m path traversal not allowed.\n"; }
            else {
                string trash_dir = "/trash/";
                if (file_system.find(trash_dir) == file_system.end())
                    file_system[trash_dir] = FSNode(true, "");
                string ts = to_string(chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count());
                if (args.rfind("-r ", 0) == 0) {
                    string dirname = args.substr(3);
                    string dpath = resolve_user_path(dirname, current_dir);
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
                    string src = resolve_user_path(args, current_dir);
                    if (file_system.find(src) != file_system.end() && !file_system[src].is_dir) {
                        string name = src.substr(src.find_last_of('/') + 1);
                        file_system[trash_dir + ts + "_" + name] = file_system[src];
                        file_system.erase(src);
                        cout << "\033[32mTrashed " << args << ".\033[0m\n";
                    } else {
                        cout << "\033[31merror:\033[0m file not found (use rm -r for directories).\n";
                    }
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
            string text = args.empty() ? "Moo." : args;
            if (text.length() >= 2 && text.front() == '"' && text.back() == '"') {
                text = text.substr(1, text.length() - 2);
            }
            int w = (int)text.length() + 4;
            cout << "\n  " << clr::dgray << vsep(w) << clr::reset << "\n";
            cout << "  " << clr::white << "  " << text << "  " << clr::reset << "\n";
            cout << "  " << clr::dgray << vsep(w) << clr::reset << "\n";
            cout << "        " << clr::gray << "  \\   ^__^" << clr::reset << "\n";
            cout << "        " << clr::gray << "   \\  (oo)\\_______" << clr::reset << "\n";
            cout << "        " << clr::gray << "      (__)\\       )\\/\\\\" << clr::reset << "\n";
            cout << "        " << clr::gray << "          ||----w |" << clr::reset << "\n";
            cout << "        " << clr::gray << "          ||     ||" << clr::reset << "\n\n";
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
            time_t now = time(nullptr);
            tm t_buf;
            localtime_r(&now, &t_buf);
            char buf[64];
            strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y", &t_buf);
            cout << buf << "\n";
        }
        else if (cmd == "history") {
            // Display the current shell session's command history with line numbers
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
                if (has_traversal(filename)) {
                    cout << "\033[31merror:\033[0m path traversal not allowed.\n";
                } else {
                    string fullpath = resolve_user_path(filename, current_dir);
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
            auto elapsed = chrono::duration_cast<chrono::seconds>(
                chrono::steady_clock::now() - program_start).count();
            int hrs = elapsed / 3600, mins = (elapsed % 3600) / 60;
            const int FW = 30;

            auto row = [&](const string& key, const string& val) {
                cout << "  " << clr::gray << key << ": " << clr::reset << string(max(0, FW - (int)key.size()), ' ') << clr::bold << clr::white << val << clr::reset << "\n";
            };

            cout << "  " << clr::dgray << vsep(FW + 10) << clr::reset << "\n";
            row("OS", clr::cyan + "NoNameOS " + VERSION + clr::reset);
            row("Kernel", clr::cyan + "C++ POSIX Sim" + clr::reset);
            row("Shell", clr::cyan + "nonamesh" + clr::reset);
            row("VFS Nodes", clr::cyan + to_string(file_system.size()) + clr::reset);
            row("Uptime", clr::cyan + to_string(hrs) + "h " + to_string(mins) + "m" + clr::reset);
            row("User", clr::cyan + current_user + clr::reset);
            row("Packages", clr::cyan + "75+ built-in" + clr::reset);
            cout << "  " << clr::dgray << vsep(FW + 10) << clr::reset << "\n";

            // Color palette
            cout << "\n  " << clr::dgray << "Palette: " << clr::reset;
            const string palette[] = {
                clr::rgbbg(255,85,85), clr::rgbbg(255,170,51), clr::rgbbg(255,255,85),
                clr::rgbbg(85,255,85), clr::rgbbg(85,255,255), clr::rgbbg(85,85,255),
                clr::rgbbg(255,85,255), clr::rgbbg(255,255,255)
            };
            for (int i = 0; i < 8; i++) cout << palette[i] << "   " << clr::reset;
            cout << "\n\n";
        }
        else if (cmd == "ps") {
            cout << "\n  " << clr::bold << clr::gray << "  PID TTY          TIME CMD" << clr::reset << "\n";
            cout << "  " << clr::dgray << "  ─── ── ────────── ───" << clr::reset << "\n";
            cout << "  " << clr::white << "    1 ?        00:00:01 " << clr::lcyan << "init" << clr::reset << "\n";
            cout << "  " << clr::white << "    2 ?        00:00:00 " << clr::lcyan << "nonamesh" << clr::reset << "\n";
            cout << "  " << clr::green << "    3 " << clr::white << current_user << "      00:00:" << (cmd_history.size() % 60 < 10 ? "0" : "") << cmd_history.size() % 60 << " " << clr::lgreen << "nonameos" << clr::reset << "\n";
            cout << "\n";
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
            tm t_buf;
            localtime_r(&now, &t_buf);
            int year = t_buf.tm_year + 1900;
            int month = t_buf.tm_mon + 1;
            int today = t_buf.tm_mday;

            tm first = {}; first.tm_year = year - 1900; first.tm_mon = month - 1; first.tm_mday = 1;
            time_t first_t = mktime(&first);
            tm first_buf;
            localtime_r(&first_t, &first_buf);
            int start_day = first_buf.tm_wday;

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
            for (int i = 0; i < YES_COUNT; i++) cout << text << "\n";
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
            if (sec <= 0 || sec > SLEEP_MAX_SEC) {
                cout << "Usage: sleep <seconds> (1-" << SLEEP_MAX_SEC << ")\n";
            } else {
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
                    if (name.empty()) { cout << "\033[31merror:\033[0m empty alias name.\n"; }
                    else {
                        if (val.front() == '\'' && val.back() == '\'')
                            val = val.substr(1, val.length() - 2);
                        aliases[name] = val;
                        cout << "Alias created: " << name << "='" << val << "'\n";
                    }
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
            int w = (int)text.length() + 4;
            cout << "\n";
            cout << clr::cyan << "╔" << string(w, '=') << "╗" << clr::reset << "\n";
            cout << clr::cyan << "║" << clr::reset << " " << clr::bold << clr::yellow << text << clr::reset << " " << clr::cyan << "║" << clr::reset << "\n";
            cout << clr::cyan << "╚" << string(w, '=') << "╝" << clr::reset << "\n";
            cout << "\n";
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
            string q = quotes[rng_int(0, (int)quotes.size() - 1)];
            cout << "\n  " << clr::amber << vsep(52) << clr::reset << "\n";
            cout << "  " << clr::italic << clr::yellow << "  " << q << clr::reset << "\n";
            cout << "  " << clr::amber << vsep(52) << clr::reset << "\n\n";
        }
        else if (cmd == "factor") {
            int n = 0;
            for (char c : args) { if (c >= '0' && c <= '9') { n = n * 10 + (c - '0'); if (n > 99999) break; } }
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
                shuffle(s.begin(), s.end(), rng());
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
                        if (has_traversal(src) || has_traversal(dst)) {
                            cout << "\033[31merror:\033[0m path traversal not allowed.\n";
                        } else {
                            string sfull = resolve_user_path(src, current_dir);
                            string dfull = resolve_user_path(dst, current_dir);
                            if (sfull.back() != '/') sfull += "/";
                            if (dfull.back() != '/') dfull += "/";
                            file_system[dfull] = FSNode(true, "");
                            for (auto& [path, node] : file_system) {
                                if (path.rfind(sfull, 0) == 0) {
                                    string rel = path.substr(sfull.length());
                                    file_system[dfull + rel] = node;
                                }
                            }
                            cout << "\033[32mCopied " << src << " -> " << dst << " recursively.\033[0m\n";
                        }
                    }
                } else {
                    if (has_traversal(a) || has_traversal(b)) {
                        cout << "\033[31merror:\033[0m path traversal not allowed.\n";
                    } else {
                        string full_src = resolve_user_path(a, current_dir);
                        string full_dst = resolve_user_path(b, current_dir);
                        if (file_system.find(full_src) != file_system.end() && !file_system[full_src].is_dir) {
                            file_system[full_dst] = FSNode(false, file_system[full_src].content);
                            file_system[full_dst].mode = file_system[full_src].mode;
                            cout << "\033[32mCopied " << a << " -> " << b << "\033[0m\n";
                        } else cout << "\033[31merror:\033[0m source file not found.\n";
                    }
                }
            }
        }
        else if (cmd == "mv") {
            size_t sp = args.find(' ');
            if (sp == string::npos) cout << "Usage: mv <source> <dest>\n";
            else {
                string src = args.substr(0, sp);
                string dst = args.substr(sp + 1);
                if (has_traversal(src) || has_traversal(dst)) {
                    cout << "\033[31merror:\033[0m path traversal not allowed.\n";
                } else {
                    string full_src = resolve_user_path(src, current_dir);
                    string full_dst = resolve_user_path(dst, current_dir);
                    if (file_system.find(full_src) != file_system.end()) {
                        file_system[full_dst] = file_system[full_src];
                        file_system.erase(full_src);
                        cout << "\033[32mMoved " << src << " -> " << dst << "\033[0m\n";
                    } else {
                        string full_src_dir = full_src;
                        if (full_src_dir.back() != '/') full_src_dir += "/";
                        if (file_system.find(full_src_dir) != file_system.end()) {
                            string full_dst_dir = full_dst;
                            if (full_dst_dir.back() != '/') full_dst_dir += "/";
                            // C1 FIX: collect paths first, then modify (avoids iterator invalidation)
                            vector<string> to_move;
                            for (auto& [p, _] : file_system)
                                if (p.rfind(full_src_dir, 0) == 0) to_move.push_back(p);
                            for (auto& p : to_move) {
                                string rel = p.substr(full_src_dir.length());
                                file_system[full_dst_dir + rel] = file_system[p];
                                file_system.erase(p);
                            }
                            cout << "\033[32mMoved directory " << src << " -> " << dst << "\033[0m\n";
                        } else cout << "\033[31merror:\033[0m source not found.\n";
                    }
                }
            }
        }
        else if (cmd == "chmod") {
            size_t sp = args.find(' ');
            if (sp == string::npos) {
                cout << "Usage: chmod <mode> <file>\n";
            } else {
                string mode = args.substr(0, sp);
                string fn = args.substr(sp + 1);
                string fullpath = resolve_user_path(fn, current_dir);
                if (file_system.find(fullpath) != file_system.end()) {
                    if (mode.length() == 9) {
                        file_system[fullpath].mode = mode;
                        cout << "\033[32mMode changed.\033[0m\n";
                    } else cout << "\033[31merror:\033[0m invalid mode (use format rwxr-xr-x).\n";
                } else {
                    string dpath = fullpath;
                    if (dpath.back() != '/') dpath += "/";
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
            if (fn.empty()) { cout << "Usage: head <file>\n"; }
            else {
                string fullpath = resolve_user_path(fn, current_dir);
                if (file_system.find(fullpath) != file_system.end() && !file_system[fullpath].is_dir) {
                    istringstream ss(file_system[fullpath].content);
                    string line;
                    for (int i = 0; i < HEAD_TAIL_LINES && getline(ss, line); i++) cout << line << "\n";
                } else cout << "\033[31merror:\033[0m file not found.\n";
            }
        }
        else if (cmd == "tail") {
            string fn = args;
            if (fn.empty()) { cout << "Usage: tail <file>\n"; }
            else {
                string fullpath = resolve_user_path(fn, current_dir);
                if (file_system.find(fullpath) != file_system.end() && !file_system[fullpath].is_dir) {
                    vector<string> lines;
                    istringstream ss(file_system[fullpath].content);
                    string line;
                    while (getline(ss, line)) lines.push_back(line);
                    int start = max(0, (int)lines.size() - HEAD_TAIL_LINES);
                    for (int i = start; i < (int)lines.size(); i++) cout << lines[i] << "\n";
                } else cout << "\033[31merror:\033[0m file not found.\n";
            }
        }
        else if (cmd == "sort") {
            string fn = args;
            if (fn.empty()) { cout << "Usage: sort <file>\n"; }
            else {
                string fullpath = resolve_user_path(fn, current_dir);
                if (file_system.find(fullpath) != file_system.end() && !file_system[fullpath].is_dir) {
                    vector<string> lines;
                    istringstream ss(file_system[fullpath].content);
                    string line;
                    while (getline(ss, line)) lines.push_back(line);
                    sort(lines.begin(), lines.end());
                    for (auto& l : lines) cout << l << "\n";
                } else cout << "\033[31merror:\033[0m file not found.\n";
            }
        }
        else if (cmd == "wc") {
            string fn = args;
            if (fn.empty()) { cout << "Usage: wc <file>\n"; }
            else {
                string fullpath = resolve_user_path(fn, current_dir);
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
        }
        else if (cmd == "tee") {
            size_t sp = args.find(' ');
            if (sp == string::npos) cout << "Usage: tee <file> <text>\n";
            else {
                string fn = args.substr(0, sp);
                string text = args.substr(sp + 1);
                if (has_traversal(fn)) { cout << "\033[31merror:\033[0m path traversal not allowed.\n"; }
                else {
                    string fullpath = resolve_user_path(fn, current_dir);
                    string display = text;
                    while (!display.empty() && display.back() == '\n') display.pop_back();
                    file_system[fullpath] = FSNode(false, text + "\n");
                    cout << display << "\n";
                }
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
                    {"su", "SU(1)\t\t\tUser Commands\n\nNAME\n\tsu - switch user\n\nSYNOPSIS\n\tsu <user>\n\nDESCRIPTION\n\tSwitch to another user. Available: root, user, guest, admin."}
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
            if (args.empty()) { cout << "Usage: touch <file>\n"; }
            else {
                string fullpath = resolve_user_path(args, current_dir);
                if (file_system.find(fullpath) == file_system.end()) {
                    file_system[fullpath] = FSNode(false, "");
                }
            }
        }
        // --- GAMES ---
        else if (cmd == "guess") {
            cout << "\n  " << clr::bold << clr::green << "🎯 GUESS THE NUMBER" << clr::reset << "\n";
            cout << "  " << clr::gray << "I'm thinking of a number between " << clr::yellow << "1" << clr::gray << " and " << clr::yellow << "100" << clr::reset << "\n";
            int target = rng_int(1, 100);
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
            cout << "\n  " << clr::bold << clr::amber << "❓ TRIVIA QUIZ" << clr::reset << "\n";
            vector<Question> questions = {
                {"What language is NoNameOS written in?", {"C++", "Python", "Rust", "Java"}, 0},
                {"What does VFS stand for?", {"Virtual File System", "Very Fast Server", "Video File Storage", "None"}, 0},
                {"How many legs does a cow have?", {"4", "2", "6", "8"}, 0},
                {"What does CPU stand for?", {"Central Processing Unit", "Computer Power Unit", "Central Program Utility", "Core Processing Unit"}, 0},
                {"What year was C++ created?", {"1979", "1990", "2001", "1965"}, 0}
            };
            shuffle(questions.begin(), questions.end(), mt19937(random_device{}()));
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
            cout << "\n  " << clr::bold << clr::red << "⚔ THE DUNGEON" << clr::reset << " " << clr::dgray << "of NoNameOS" << clr::reset << "\n\n";
            cout << "You awaken in a dark dungeon.\n";
            cout << "A faint glow comes from two paths.\n\n";
            int hp = 100;
            int gold = 0;
            bool running = true;
            while (running) {
                cout << "  " << clr::bold << clr::red << "❤ HP: " << hp << clr::reset << "   " << clr::bold << clr::yellow << "💰 Gold: " << gold << clr::reset << "\n";
                cout << "What do you do? [left / right / rest / quit]\n> ";
                string choice;
                getline(cin, choice);
                if (choice == "left") {
                    int event = rng_int(0, 2);
                    if (event == 0) {
                        cout << "\033[32mYou found a treasure chest! +25 gold\033[0m\n";
                        gold += 25;
                    } else if (event == 1) {
                        int dmg = 10 + rng_int(0, 19);
                        cout << "\033[31mA slime attacks! -" << dmg << " HP\033[0m\n";
                        hp -= dmg;
                    } else {
                        cout << "It's a dead end. Nothing here.\n";
                    }
                } else if (choice == "right") {
                    int event = rng_int(0, 2);
                    if (event == 0) {
                        cout << "\033[32mYou found a health potion! +30 HP\033[0m\n";
                        hp += 30;
                    } else if (event == 1) {
                        int dmg = 15 + rng_int(0, 24);
                        cout << "\033[31mA skeleton strikes! -" << dmg << " HP\033[0m\n";
                        hp -= dmg;
                    } else {
                        int found = 10 + rng_int(0, 19);
                        cout << "\033[33mYou found " << found << " gold on the ground!\033[0m\n";
                        gold += found;
                    }
                } else if (choice == "rest") {
                    int heal = 10 + rng_int(0, 14);
                    cout << "\033[36mYou rest and recover " << heal << " HP.\033[0m\n";
                    hp += heal;
                } else if (choice == "quit") {
                    running = false;
                } else {
                    cout << "Invalid choice.\n";
                }
                if (hp <= 0) {
                    cout << "\n  " << clr::error << ">> GAME OVER — You have been defeated." << clr::reset << "\n";
                    cout << "  " << clr::gray << "Final gold: " << clr::yellow << gold << clr::reset << "\n";
                    running = false;
                }
            }
            if (hp > 0) cout << "\n  " << clr::success << ">> You escaped with " << clr::yellow << gold << clr::success << " gold! GG!" << clr::reset << "\n";
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
                    string lp = resolve_user_path(link, current_dir);
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
            if (args.empty()) {
                cout << "Usage: nano <filename>\n";
            } else {
                string fullpath = resolve_user_path(args, current_dir);
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
                    interval = 0;
                    for (char c : first) interval = interval * 10 + (c - '0');
                    if (interval < 1 || interval > 60) interval = 2;
                    subcmd = args.substr(sp + 1);
                } else {
                    subcmd = args;
                }
            }
            if (subcmd.empty()) {
                cout << "Usage: watch [interval] <command> [args]\n";
            } else {
                for (int i = 0; i < WATCH_ITERATIONS; i++) {
                    cout << "\033[2J\033[1;1H";
                    cout << "watch: " << subcmd << " (iteration " << (i+1) << "/" << WATCH_ITERATIONS << ")\n\n";
                    istringstream ss(subcmd);
                    string wcmd, wargs;
                    ss >> wcmd;
                    getline(ss, wargs);
                    if (!wargs.empty() && wargs[0] == ' ') wargs = wargs.substr(1);
                    cout << "Running: " << subcmd << "\n";
                    cout << "\033[32m[OK]\033[0m (simulated output)\n";
                    if (i < WATCH_ITERATIONS - 1) this_thread::sleep_for(chrono::seconds(interval));
                }
            }
        }
        else if (cmd == "ping") {
            string host = args.empty() ? "localhost" : args;
            cout << "PING " << host << " (127.0.0.1) 56(84) bytes of data.\n";
            for (int i = 0; i < PING_COUNT; i++) {
                int latency = 20 + rng_int(0, 60);
                cout << "64 bytes from 127.0.0.1: icmp_seq=" << (i+1) << " ttl=64 time=" << latency << "." << rng_int(0, 99) << " ms\n";
                this_thread::sleep_for(chrono::milliseconds(500));
            }
            cout << "\n--- " << host << " ping statistics ---\n";
            cout << PING_COUNT << " packets transmitted, " << PING_COUNT << " received, 0% packet loss\n";
        }
        else if (cmd == "top") {
            cout << "\n  " << clr::bold << clr::gray << "  PID USER      PR  NI  VIRT   RES   SHR S  CPU  MEM   TIME+   COMMAND" << clr::reset << "\n";
            cout << "  " << clr::dgray << "  ─── ───────── ── ── ──── ──── ──── ─ ─── ─── ────── ───────" << clr::reset << "\n";
            cout << "  " << clr::white << "    1 root      20   0  128M  4.5M  2.1M S  0.0  0.1  00:00:01 " << clr::lcyan << "init" << clr::reset << "\n";
            cout << "  " << clr::white << "    2 root      20   0   64M  2.1M  1.0M S  0.0  0.0  00:00:00 " << clr::lcyan << "nonamesh" << clr::reset << "\n";
            cout << "  " << clr::green << "    3 " << current_user << "      20   0  256M  8.2M  3.3M R  2.3  0.2  00:00:" << (cmd_history.size() % 60 < 10 ? "0" : "") << cmd_history.size() % 60 << " " << clr::lgreen << "nonameos" << clr::reset << "\n";
            cout << "  " << clr::white << "    4 root      20   0   16M  0.8M  0.4M S  0.0  0.0  00:00:00 " << clr::lcyan << "kworker" << clr::reset << "\n";
            cout << "  " << clr::white << "    5 " << current_user << "      20   0   32M  1.2M  0.6M S  0.3  0.0  00:00:00 " << clr::lcyan << "logger" << clr::reset << "\n";
            cout << "\n";
        }
        else if (cmd == "df") {
            size_t total_bytes = 0;
            size_t total_nodes = file_system.size();
            for (auto& [_, node] : file_system) total_bytes += node.size();
            size_t total_kb = total_bytes / 1024;
            size_t avail_kb = 1024; // simulated 1MB total
            int pct = total_bytes > 0 ? (int)(total_bytes * 100ULL / (total_bytes + avail_kb * 1024)) : 0;
            cout << "Filesystem      Size  Used  Avail  Use%  Mounted on\n";
            cout << "VFS           " << (total_kb + avail_kb) << "K  " << total_kb << "K  " << avail_kb << "K  " << pct << "%  /\n";
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
                for (int i = start; i <= end; i++) cout << i << "\n";
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
                    cur += rest + "\n";
                    file_system[todopath] = FSNode(false, cur);
                    cout << "\033[32mTodo added.\033[0m\n";
                }
            } else if (subcmd == "done") {
                int n = 0;
                for (char c : rest) { if (c >= '0' && c <= '9') { n = n * 10 + (c - '0'); if (n > 99999) break; } }
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
                        cout << "  " << name << " (" << node.size() << " bytes)\n";
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
            for (int offset = TRAIN_START_OFFSET; offset >= TRAIN_END_OFFSET; offset--) {
                cout << "\033[2J\033[1;1H";
                for (int r = 0; r < 5; r++) {
                    if (offset > 0) cout << string(offset, ' ');
                    else cout << string(0, ' ');
                    cout << train[r] << "\n";
                }
                this_thread::sleep_for(chrono::milliseconds(TRAIN_FRAME_MS));
            }
        }
        // --- NEW GAMES ---
        else if (cmd == "tetris") { play_tetris(); }
        else if (cmd == "pong") { play_pong(); }
        else if (cmd == "sudoku") { play_sudoku(); }
        else if (cmd == "flappy") { play_flappy(); }
        // --- NEW COMMANDS ---
        else if (cmd == "colors") { cmd_colors(); }
        else if (cmd == "weather") { cmd_weather(); }
        else if (cmd == "epoch") { cmd_epoch(); }
        else if (cmd == "uuid") { cmd_uuid(); }
        else if (cmd == "base64") { cmd_base64(args, file_system, current_dir); }
        else if (cmd == "rot13") { cmd_rot13(args); }
        else if (cmd == "password") { cmd_password(); }
        else if (cmd == "wordcount") { cmd_wordcount(args, file_system, current_dir); }
        else if (cmd == "matrix") { int n = 20; for (char c : args) if (c >= '0' && c <= '9') n = n * 10 + (c - '0'); cmd_matrix(n); }
        else if (cmd == "cmtheme") {
            cout << "\n  " << clr::bold << "Color Themes:" << clr::reset << "\n\n";
            cout << "  " << clr::green << "■ Green Matrix" << clr::reset << "    " << clr::blue << "■ Blue Ocean" << clr::reset << "\n";
            cout << "  " << clr::red << "■ Red Alert" << clr::reset << "      " << clr::magenta << "■ Purple Haze" << clr::reset << "\n";
            cout << "  " << clr::cyan << "■ Cyan Ice" << clr::reset << "       " << clr::yellow << "■ Golden Sun" << clr::reset << "\n";
            cout << "  " << clr::orange << "■ Orange Fire" << clr::reset << "    " << clr::lgreen << "■ Neon Green" << clr::reset << "\n\n";
            cout << "  " << clr::dgray << "Tip: Use " << clr::lcyan << "colors" << clr::dgray << " to see all available colors" << clr::reset << "\n\n";
        }
        else if (cmd == "countdown") {
            int s = 0; for (char c : args) if (c >= '0' && c <= '9') s = s * 10 + (c - '0');
            cmd_countdown(s);
        }
        else if (cmd == "ascii") { cmd_ascii(); }
        else if (cmd == "hexdump") { cmd_hexdump(args, file_system, current_dir); }
        else if (cmd == "quote") { cmd_quote(); }
        else if (cmd == "joke") { cmd_joke(); }
        else if (cmd == "ip") { cmd_ip(); }
        else if (cmd == "mem") { cmd_mem(); }
        else if (cmd == "cpu") { cmd_cpu(); }
        else if (cmd == "disk") { cmd_disk(); }
        else if (cmd == "uptime2") { cmd_uptime2(); }
        else if (cmd == "calc2") { cmd_calc2(args); }
        // --- MEGA BATCH COMMANDS ---
        else if (cmd == "memory") { play_memory(); }
        else if (cmd == "connect4") { play_connect4(); }
        else if (cmd == "lightsout") { play_lightsout(); }
        else if (cmd == "puzzle") { play_puzzle(); }
        else if (cmd == "breakout") { play_breakout(); }
        else if (cmd == "whack") { play_whack(); }
        else if (cmd == "bmi") { cmd_bmi(args); }
        else if (cmd == "tip") { cmd_tip(args); }
        else if (cmd == "units") { cmd_units(args); }
        else if (cmd == "roman") { cmd_roman(args); }
        else if (cmd == "binary") { cmd_binary(args); }
        else if (cmd == "morse") { cmd_morse(args); }
        else if (cmd == "bar") { cmd_bar(args); }
        else if (cmd == "sparkline") { cmd_sparkline(args); }
        else if (cmd == "colorgen") { cmd_colorgen(); }
        else if (cmd == "palette") { cmd_palette(); }
        else if (cmd == "diff") { cmd_diff(args, file_system, current_dir); }
        else if (cmd == "csv") { cmd_csv(args, file_system, current_dir); }
        else if (cmd == "stats") { cmd_stats(args); }
        else if (cmd == "age") { cmd_age(args); }
        else if (cmd == "datecalc") { cmd_datecalc(args); }
        else if (cmd == "encode") { cmd_encode(args); }
        else if (cmd == "hash") { cmd_hash(args); }
        else if (cmd == "md5") { cmd_hash(args); }
        else if (cmd == "sha1") { cmd_hash(args); }
        else if (cmd == "urlencode") { cmd_urlencode(args); }
        else if (cmd == "urldecode") { cmd_urldecode(args); }
        else if (cmd == "reverse") { cmd_reverse_str(args); }
        else if (cmd == "capitalize") { cmd_capitalize(args); }
        else if (cmd == "repeat") { cmd_repeat_cmd(args); }
        else if (cmd == "scrabble") { cmd_scrabble(args); }
        else if (cmd == "emoji") { cmd_emoji(args); }
        else if (cmd == "random") { cmd_random(args); }
        else if (cmd == "pick") { cmd_pick(args); }
        else if (cmd == "dice") { cmd_dice(args); }
        else if (cmd == "coin") { cmd_coin(); }
        else if (cmd == "zodiac") { cmd_zodiac(args); }
        else if (cmd == "worldclock") { cmd_worldclock(); }
        else if (cmd == "wordle") { cmd_wordle(); }
        else if (cmd == "quiz") { cmd_quiz(); }
        else if (cmd == "uppercase") { for (char c : args) cout << (char)toupper(c); cout << "\n"; }
        else if (cmd == "lowercase") { for (char c : args) cout << (char)tolower(c); cout << "\n"; }
        else {
            cout << "\n  " << clr::error << "✗ " << clr::bold << "command not found: " << clr::reset << clr::error << cmd << clr::reset << "\n";
            string sug = closest_cmd(cmd);
            if (!sug.empty()) cout << "  " << clr::muted << "Did you mean " << clr::lcyan << clr::bold << sug << clr::reset << clr::muted << "?" << clr::reset << "\n";
            cout << "\n";
        }
        last_cmd_end = chrono::steady_clock::now();
    }
    return 0;
}
