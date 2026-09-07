#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <cstdlib>
#include <fstream>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include <filesystem>

struct PipeDeleter {
    void operator()(FILE* fp) const {
        if (fp) {
            pclose(fp);
        }
    }
};

static std::string run_command(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, PipeDeleter> pipe(popen(cmd.c_str(), "r"));
    if (!pipe) {
        return "";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' ')) {
        result.pop_back();
    }
    return result;
}

static std::string get_default_from_mimeapps() {
    const char* home = std::getenv("HOME");
    if (!home) {
        return "";
    }

    std::string path = std::string(home) + "/.config/mimeapps.list";
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }

    std::string line;
    while(std::getline(file, line)) {
        if (line.rfind("x-scheme-handler/http=", 0) == 0 ||
            line.rfind("x-scheme-handler/https=", 0) == 0 ||
            line.rfind("text/html=", 0) == 0) {
            return line;
        }
    }
    return "";

}

static std::vector<std::string> get_installed_browsers() {
    std::vector<std::string> candidates = {"librewolf", "firefox", "google-chrome", "brave-browser", "chromium"};
    std::vector<std::string> installed;

    for (const auto& b : candidates) {
        if (!run_command("command -v " + b).empty()) {
            installed.push_back(b);
        }
    }
    return installed;
}

static int select_browser(const std::vector<std::string>& browsers) {
    std::cout << "\n Select a browser: \n";
    for (size_t i = 0; i < browsers.size(); i++) {
        std::cout << "[" << i << "] " << browsers[i] << '\n';
    }

    int choice = -1;
    std::cout << "Choice (0-" << browsers.size() - 1 << "): ";
    std::cin >> choice;

    if (choice >= 0 && choice < browsers.size()){
        return choice;
    }
    return -1;

}

int main(int argc, char* argv[]) {
    std::cout << "Checking browsers installed... \n";

    std::vector<std::string> browsers = get_installed_browsers();

    if (browsers.empty()) {
        std::cout << "No browsers found \n";
        return 1;
    }

    std::cout << "Found " << browsers.size() << " installed browsers(s): \n";
    for (size_t i = 0; i < browsers.size(); i++) {
        std::cout << "[" << i << "] " << browsers[i] << '\n';
    }

    int choice = select_browser(browsers);
    if (choice < 0) {
        std::cout << "Invalid \n";
        return 1;
    } 

    std::string browser = browsers[choice];
    std::cout << "Selected " << browser << '\n';



    return 0;
}
