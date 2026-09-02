#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <cstdlib>
#include <fstream>
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

static std::string get_browser_command() {
    std::string entry = get_default_from_mimeapps();

    if (entry.empty()) {
        entry = run_command("/usr/bin/xdg-settings get default-web-browser 2>/dev/null");
    }
    /*
    if (entry.find("librewolf") != std::string::npos) {
        return "librewolf";
    }
    */
    
    if (entry.find("firefox") != std::string::npos) {
        return "firefox";
    }
    if (entry.find("google-chrome") != std::string::npos) {
        return "google-chrome";
    }
    if (entry.find("brave") != std::string::npos) {
        return "brave";
    }
    if (entry.find("chromium") != std::string::npos) {
        return "chromium";
    }

    return "firefox";
}


int main(int argc, char* argv[]) {
    std::string url = "https://youtube.com";
    if (argc > 1) {
        url = argv[1];
    }

    std::string browser = get_browser_command();
    std::cout << "Identified browser: " << browser << '\n';

    if (browser.empty()) {
        std::cerr << "Could not identifty an installed browser \n";
        return 1;
    }

    std::string cmd = browser + " --kiosk \"" + url + "\" &";
    std::cout << "Running command: " << cmd << "\n";
    std::system(cmd.c_str());
    

    return 0;
}
