#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <random>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>

std::string CalcStandby(int millisecondsToAdd) {
    auto now = std::chrono::system_clock::now();
    auto future = now + std::chrono::milliseconds(millisecondsToAdd);

    std::time_t future_c = std::chrono::system_clock::to_time_t(future);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&future_c), "%H:%M:%S"); // suppress the error or use localtime_s

    return ss.str();
}

void MoveCursor(HWND hwnd) {
    RECT rect;
    POINT currentPos;
    GetClientRect(hwnd, &rect);
    GetCursorPos(&currentPos);

    int x = rect.left + rand() % (rect.right - rect.left);
    int y = rect.top + rand() % (rect.bottom - rect.top);

    int steps = 50;
    double stepX = static_cast<double>(x - currentPos.x) / steps;
    double stepY = static_cast<double>(y - currentPos.y) / steps;

    // interp
    for (int i = 0; i < steps; ++i) {
        int newX = currentPos.x + static_cast<int>(stepX * i);
        int newY = currentPos.y + static_cast<int>(stepY * i);
        mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(stepX), static_cast<DWORD>(stepY), 0, 0);
        Sleep(20);
    }

    SetCursorPos(x, y);
}

void SendKeyPress(WORD key, int duration = 200) {
    INPUT ip = { 0 };
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = key;
    
    ip.ki.dwFlags = 0;  // key down
    SendInput(1, &ip, sizeof(INPUT));
    Sleep(duration);
    ip.ki.dwFlags = KEYEVENTF_KEYUP; // key up
    SendInput(1, &ip, sizeof(INPUT));
}

void MovePlayer(std::vector<WORD>& keys) {
    for (int i = 0; i < 2; ++i) {
        SendKeyPress(keys[i], 500);
    }
}

void main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<WORD> keys = { 'W', 'W', 'A', 'A', 'S', VK_LSHIFT, 'D' };

    HWND hwnd = FindWindow(NULL, L"GTA:SA:MP");
    bool escActive = false;

    if (!hwnd) {
        return;
    }

    std::cout << "* Found game window.\n";

    if (GetForegroundWindow() != hwnd) {
        escActive = true;
    }

    std::cout << "* AFK bot is now active.\n================================\n";

    // main loop - afk bot actions
    while (true) {
        if (!hwnd) {
            return;
        }

        Beep(1700, 150);
        Sleep(10);
        Beep(1700, 150);

        Sleep(1000);

        if (GetForegroundWindow() != hwnd) {
            ShowWindow(hwnd, SW_MAXIMIZE);
        }
        std::shuffle(keys.begin(), keys.end(), gen); // randomizing order inside array

        Sleep(500);

        if (escActive) {
            // precaution in case of escActive failing
            MovePlayer(keys);

            // unpause game
            SendKeyPress(VK_ESCAPE, 200);
            escActive = false;
        }

        Sleep(300);

        try {
            MoveCursor(hwnd);
            MovePlayer(keys);
        }
        catch (...) {
            std::cout << "* Error. Let me do my thing.\n";
        }

        // delay between movement and pausing again
        std::uniform_int_distribution<> distr(300, 1500);
        int delayAfter = distr(gen);
        Sleep(delayAfter);

        // pausing the game
        SendKeyPress(VK_ESCAPE, 400);
        escActive = true;

        Beep(1700, 150);
        Sleep(10);
        Beep(1700, 150);

        ShowWindow(hwnd, SW_MINIMIZE);

        // standby
        distr = std::uniform_int_distribution<int>(20 * 60 * 1000, 25 * 60 * 1000); // 20 - 25 min
        int sleepTime = distr(gen);
        std::cout << "* Cycle completed.\n";
        std::cout << "* On standby for " << std::fixed << sleepTime / 1000 / 60 << " minutes. (Next action at " << CalcStandby(sleepTime) << ")\n================================\n";

        Sleep(sleepTime);
    }

    return;
}
