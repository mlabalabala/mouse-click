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
std::atomic_bool isRun{true};
std::atomic_bool isLeftButtonPhysicallyDown{false};
std::atomic_bool isRightButtonPhysicallyDown{false};
std::atomic_bool isFinishFSpaceF{true};

HHOOK mouseHook = nullptr;
HHOOK keyboardHook = nullptr;
int T1;
int T2;
int T3;
int T4;

void SimulateFSPACEF();

// --- 钩子回调函数 ---
LRESULT CALLBACK MouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    // std::cout << "\nMouseHookCallback nCode: "<< std::hex << nCode << std::endl;
    if (nCode >= 0) {
        MSLLHOOKSTRUCT* msll = (MSLLHOOKSTRUCT*)lParam;
        bool isInjected = (msll->flags & LLMHF_INJECTED) == LLMHF_INJECTED;
        bool isMoving = wParam == WM_MOUSEMOVE;
        if (isInjected || isMoving) return CallNextHookEx(mouseHook, nCode, wParam, lParam);
        // std::cout << "MouseHookCallback wParam: 0x"<< std::hex << wParam << std::endl;
        switch (wParam) {
            case WM_LBUTTONDOWN:
                // std::cout << "Physical mouse DOWN detected." << std::endl;
                isLeftButtonPhysicallyDown = true;
                break;
            case WM_LBUTTONUP:
                // std::cout << "Physical mouse UP detected." << std::endl;
                isLeftButtonPhysicallyDown = false;
                break;
            case WM_RBUTTONDOWN:
                // std::cout << "Physical mouse DOWN detected." << std::endl;
                isRightButtonPhysicallyDown = true;
                break;
            case WM_RBUTTONUP:
                // std::cout << "Physical mouse UP detected." << std::endl;
                isRightButtonPhysicallyDown = false;
                break;
            case WM_MBUTTONDOWN:
                isRun = !isRun;
                // isRun ? Beep(2000, 200) : Beep(500, 100);
                std::cout << "hook status: " << (isRun ? "on" : "off") << std::endl;
                break;
        }
    }
    return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

LRESULT CALLBACK KeyboardHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;

        bool isInjected = (kb->flags & LLKHF_INJECTED) == LLKHF_INJECTED;
        if (isInjected) return CallNextHookEx(nullptr, nCode, wParam, lParam);

        bool isKeyDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        bool isKeyUp = (wParam == WM_KEYUP || wParam == WM_SYSKEYUP);

        if (isKeyDown) {
            // std::cout
            //         << "Key Down: vk=" << kb->vkCode
            //         << " scan=" << kb->scanCode
            //         << " injected=" << isInjected
            //         << std::endl;
        }

        if (isKeyUp) {
            // std::cout
            //         << "Key Up: vk=" << kb->vkCode
            //         << " scan=" << kb->scanCode
            //         << " injected=" << isInjected
            //         << std::endl;
            if ('F' == kb->vkCode) {
                if (isFinishFSpaceF) {
                    // std::cout << "SimulateFSPACEF..." << std::endl;
                    isFinishFSpaceF = false;
                    std::thread clickThread(SimulateFSPACEF);
                    clickThread.detach();
                }
            }
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}


// --- 随机数函数 ---
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

// --- 模拟按键函数 ---
void SimulateLeftClick() {
    // std::cout << "SimulateLeftClick ... " << std::endl;

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

void SimulateRightClick() {
    // std::cout << "SimulateRightClick ... " << std::endl;

    INPUT inputDownR = {0};
    inputDownR.type = INPUT_MOUSE;
    inputDownR.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    SendInput(1, &inputDownR, sizeof(INPUT));

    Sleep(r(10, 20));

    INPUT inputUpR = {0};
    inputUpR.type = INPUT_MOUSE;
    inputUpR.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    SendInput(1, &inputUpR, sizeof(INPUT));
    Sleep(r(250, 260));

    INPUT inputDownL = {0};
    inputDownL.type = INPUT_MOUSE;
    inputDownL.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &inputDownL, sizeof(INPUT));

    Sleep(r(10, 20));

    INPUT inputUpL = {0};
    inputUpL.type = INPUT_MOUSE;
    inputUpL.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &inputUpL, sizeof(INPUT));

    Sleep(r(10, 20));
}

void SimulateKeyDown(int keyCode) {
    WORD scan = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
    INPUT inputDown = {0};
    inputDown.type = INPUT_KEYBOARD;
    inputDown.ki.wScan = scan;
    inputDown.ki.dwFlags = KEYEVENTF_SCANCODE;
    SendInput(1, &inputDown, sizeof(INPUT));
}

void SimulateKeyUp(int keyCode) {
    WORD scan = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);
    INPUT inputUp = {0};
    inputUp.type = INPUT_KEYBOARD;
    inputUp.ki.wScan = scan;
    inputUp.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    SendInput(1, &inputUp, sizeof(INPUT));
}

void SimulateKeyType(int keyCode) {
    // std::cout << "SimulateKeyType: " << keyCode << std::endl;
    SimulateKeyDown(keyCode);
    Sleep(r(20, 30));
    SimulateKeyUp(keyCode);
}

// 剑客宏
void SimulateFSPACEF() {
    // std::cout << "SimulateFSPACEF ... " << std::endl;
    // SimulateKeyType('F');
    Sleep(r(20, 30));
    SimulateKeyType(VK_SPACE);
    Sleep(r(70, 90));
    SimulateKeyType('F');
    isFinishFSpaceF = true;
}

void AutoClickThread() {
    while (true) {
        if (isRun && isLeftButtonPhysicallyDown) {
            SimulateLeftClick();
        } else {
            Sleep(20);
        }
    }
}
void AutoClickThreadR() {
    while (true) {
        if (isRun && isRightButtonPhysicallyDown) {
            SimulateRightClick();
        } else {
            Sleep(20);
        }
    }
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
    if (config.empty()) {
        system("pause");
        return 1;
    }
    T1 = std::stoi(config["interval.t1"]);
    T2 = std::stoi(config["interval.t2"]);
    T3 = std::stoi(config["interval.t3"]);
    T4 = std::stoi(config["interval.t4"]);
    // 炼狱速刺开关
    int isTurnOnLYSC = std::stoi(config["interval.lysc"]);
    std::cout << "lysc status: " << (isTurnOnLYSC ? "on" : "off") << std::endl;
    // 启动自动点击线程
    std::thread clickThread(AutoClickThread);
    clickThread.detach();
    if (isTurnOnLYSC) {
        std::thread clickThreadR(AutoClickThreadR);
        clickThreadR.detach();
    }

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
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookCallback,hModule,0);

    if (!keyboardHook) {
        std::cout << "Failed to set keyboard hook." << std::endl;
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

    /*int key_code = std::stoi(config["interval.key"]);
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

    mouse_click(t1, t2, t3, t4, key_val);*/
    return 0;
}
