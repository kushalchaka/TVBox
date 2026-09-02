#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <cstdlib>
#include <unistd.h>

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
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }
    return result;
}

static bool binary_exists(const std::string& bin) {
    std::string check = "command -v " + bin + " >/dev/null 2>&1";
    return (std::system(check.c_str()) == 0);
}

static std::string build_kiosk_command(const std::string& url) {
    
    std::string desktop_entry = run_command("xdg-settings get default-web-browser");

    if (desktop_entry.find("librewolf") != std::string::npos) {
        return "librewolf --kiosk \"" + url + "\" &";
    }
    
    if (desktop_entry.find("firefox") != std::string::npos) {
        return "firefox --kiosk \"" + url + "\" &";
    }

    if (desktop_entry.find("google-chrome") != std::string::npos) {
        return "google-chrome --kiosk --no-first-run \"" + url + "\" &";
    }

    if (desktop_entry.find("brave") != std::string::npos) {
        return "brave-browser --kiosk --no-first-run \"" + url + "\" &";
    }

    if (binary_exists("chromium")) {
        return "chromium --kiosk --no-first-run \"" + url + "\" &";
    }
    if (binary_exists("google-chrome-stable")) {
        return "google-chrome-stable --kiosk --no-first-run \"" + url + "\" &";
    }
    if (binary_exists("firefox")) {
        return "firefox --kiosk \"" + url + "\" &";
    }

    return "xdg-open \"" + url + "\" &";
}

int main(int argc, char* argv[]) {
    std::string url = "https://youtube.com";
    if (argc > 1) {
        url = argv[1];
    }

    std::string launch_cmd = build_kiosk_command(url);

    std::cout << "Launching browser in kiosk mode:\n";
    std::cout << "  Command: " << launch_cmd << "\n";

    int status = std::system(launch_cmd.c_str());
    if (status != 0) {
        std::cerr << "Failed to run command.\n";
        return 1;
    }

    return 0;
}
