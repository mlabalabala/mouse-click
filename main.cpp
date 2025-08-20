#include <iostream>
#include <windows.h>
#include <random>
#include <cmath>
#include <thread>
#include <atomic>

#include "ConfigParse.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- 常量定义 ---
// const int WH_MOUSE_LL = 14;
// const int WM_LBUTTONDOWN = 0x0201;
// const int WM_LBUTTONUP = 0x0202;
// const int WM_MBUTTONDOWN = 0x0207;
// const int MOUSEEVENTF_LEFTDOWN = 0x0002;
// const int MOUSEEVENTF_LEFTUP = 0x0004;
// const int LLMHF_INJECTED = 0x00000001;

// --- 全局状态变量 ---
std::atomic<bool> isLeftButtonPhysicallyDown(false);
std::atomic<bool> isRun(true);
HHOOK mouseHook = nullptr;
int T1;
int T2;
int T3;
int T4;

// --- 钩子回调函数 ---
LRESULT CALLBACK MouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        MSLLHOOKSTRUCT* msll = (MSLLHOOKSTRUCT*)lParam;
        bool isInjected = (msll->flags & LLMHF_INJECTED) == LLMHF_INJECTED;

        if (!isInjected) {
            switch (wParam) {
                case WM_LBUTTONDOWN:
                    // std::cout << "Physical mouse DOWN detected." << std::endl;
                    isLeftButtonPhysicallyDown = true;
                    break;
                case WM_LBUTTONUP:
                    // std::cout << "Physical mouse UP detected." << std::endl;
                    isLeftButtonPhysicallyDown = false;
                    break;
                case WM_MBUTTONDOWN:
                    isRun = !isRun;
                    std::cout << "status: " << (isRun ? "running" : "stop") << std::endl;
                    break;
            }
        }
    }

    return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}



// --- 模拟点击函数 ---
void SimulateLeftClick() {
    // std::cout << "click ... " << std::endl;

    INPUT inputDown = {0};
    inputDown.type = INPUT_MOUSE;
    inputDown.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &inputDown, sizeof(INPUT));

    // 简单的睡眠实现（C++11 没有 std::this_thread::sleep_for）
    Sleep(r(T1, T2));

    INPUT inputUp = {0};
    inputUp.type = INPUT_MOUSE;
    inputUp.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &inputUp, sizeof(INPUT));
    Sleep(r(T3, T4));
}

// --- 自动点击线程 ---
void AutoClickThread() {
    while (true) {
        if (isRun && isLeftButtonPhysicallyDown) {
            SimulateLeftClick();
        } else {
            Sleep(20);
        }
    }
}

double generate_normal_random(double mean, double stddev) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    double u1 = dis(gen);
    double u2 = dis(gen);
    double z0 = std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * M_PI * u2);
    return mean + z0 * stddev;
}

long r(int m, int n) {
    double mean = (m + n) / 2.0;
    double sigma = (n - m) / (2 * 1.645);
    double generated_random = generate_normal_random(mean, sigma);
    while (generated_random < m) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(m, n);
        generated_random = dis(gen);
    }

    return (long) generated_random;
}

void mouse_click(int t1, int t2, int t3, int t4, int key) {
    //int t1=130; int t2=150;
    //int t3=20; int t4=30;
    // while (true) {
    //     if (GetAsyncKeyState(key) & 0x8000) {
    //         long interval_t1 = r(t1,t2);
    //         long interval_t2 = r(t3,t4);
    //         //std::cout << interval_t1 << " " << interval_t2 << std::endl;
    //         mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    //         Sleep(interval_t1);
    //         mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    //         Sleep(interval_t2);
    //     }
    // }

    bool flag = true;  // 控制是否执行点击
    bool lastToggleState = false; // 记录上一次XBUTTON1的状态

    while (true) {
        SHORT currentState = GetAsyncKeyState(VK_MBUTTON);

        if ((currentState & 0x8000) && !lastToggleState) {
            flag = !flag;  // 切换开关
            std::cout << "status: " << (flag ? "running ..." : "stopped !") << std::endl;
            Sleep(300);  // 防抖动
        }
        lastToggleState = (currentState & 0x8000);

        if (flag && (GetAsyncKeyState(key) & 0x8000)) {
            long interval_t1 = r(t1, t2);
            long interval_t2 = r(t3, t4);
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            Sleep(interval_t1);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            Sleep(interval_t2);
        }
    }
}

int main() {

    std::map<std::string, std::string> config = GetConfig();
    if (config.empty()) return 0;
    T1 = std::stoi(config["interval.t1"]);
    T2 = std::stoi(config["interval.t2"]);
    T3 = std::stoi(config["interval.t3"]);
    T4 = std::stoi(config["interval.t4"]);
    // 启动自动点击线程
    std::thread clickThread(AutoClickThread);
    clickThread.detach();

    // 设置鼠标钩子
    HINSTANCE hModule = GetModuleHandle(nullptr);
    if (!hModule) {
        std::cout << "Failed to get module handle." << std::endl;
        return 1;
    }

    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookCallback, hModule, 0);
    if (!mouseHook) {
        std::cout << "Failed to set mouse hook." << std::endl;
        return 1;
    }

    // 消息循环
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 清理
    UnhookWindowsHookEx(mouseHook);
    std::cout << "Program terminated." << std::endl;

    /*
    int key_code = std::stoi(config["interval.key"]);
    //std::cout << key_code << std::endl;
    int key_val;
    switch (key_code) {
        case 2: key_val=VK_MBUTTON;
            break;
        case 4: key_val=VK_XBUTTON1;
            break;
        case 5: key_val=VK_XBUTTON2;
            break;
        default: key_val=VK_RBUTTON;
            break;
    }
    //std::cout << key_val << std::endl;

    mouse_click(t1, t2, t3, t4, key_val);
    */
    return 0;
}
