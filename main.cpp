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

enum class WorkMode {
    NullMode = 0,
    LianyuMode,
    XukongMode,
    ModeCount
};

WorkMode g_mode = WorkMode::XukongMode;

// --- 全局状态变量 ---
std::atomic_bool isLysdOn{false};
std::atomic_bool isLyscOn{false};
std::atomic_bool isRightBtnFFOn{false};
std::atomic_bool isFinishFSpaceF{true};

std::atomic_bool isNullMode{true};
std::atomic_bool isLianyuMode{false};
std::atomic_bool isXukongMode{false};

HHOOK mouseHook = nullptr;
HHOOK keyboardHook = nullptr;
HWND g_osdWnd = nullptr;

constexpr UINT WM_SHOW_MODE = WM_APP + 1;

int T1;
int T2;
int T3;
int T4;

void FSPACEF();
void RightBtnFF();
void SwitchMode();


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

// --- 钩子回调函数 ---
LRESULT CALLBACK MouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    // std::cout << "\nMouseHookCallback nCode: "<< std::hex << nCode << std::endl;
    if (nCode >= 0) {
        MSLLHOOKSTRUCT* msll = (MSLLHOOKSTRUCT*)lParam;
        bool isInjected = (msll->flags & LLMHF_INJECTED) == LLMHF_INJECTED;
        bool isMoving = wParam == WM_MOUSEMOVE;
        if (isInjected || isMoving) return CallNextHookEx(mouseHook, nCode, wParam, lParam);
        WORD XButton = HIWORD(msll->mouseData);
        // std::cout << "MouseHookCallback wParam: 0x"<< std::hex << wParam << std::endl;
        // std::cout << "MouseHookCallback XButton: 0x"<< std::hex << XButton << std::endl;
        switch (wParam) {
            case WM_LBUTTONDOWN:
                isLysdOn = true;
                return 0;
            case WM_LBUTTONUP:
                isLysdOn = false;
                return 0;
            case WM_RBUTTONDOWN:
                isLyscOn = true;
                isRightBtnFFOn = true;
                return 0;
            case WM_RBUTTONUP:
                isLyscOn = false;
                isRightBtnFFOn = false;
                return 0;
            case WM_MBUTTONUP:
                return 0;
            case WM_XBUTTONUP: {
                if (XButton == XBUTTON1) {
                    SwitchMode();
                } else if (XButton == XBUTTON2) {
                    if (isFinishFSpaceF) {
                        // std::cout << "SimulateFSPACEF..." << std::endl;
                        isFinishFSpaceF = false;
                        std::thread clickThread(FSPACEF);
                        clickThread.detach();
                    }
                }
                return 0;
            }
            default:
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
            /*std::cout
                    << "Key Down: vk=" << kb->vkCode
                    << " scan=" << kb->scanCode
                    << " injected=" << isInjected
                    << std::endl;*/
        }

        if (isKeyUp) {
            /*std::cout
                    << "Key Up: vk=" << kb->vkCode
                    << " scan=" << kb->scanCode
                    << " injected=" << isInjected
                    << std::endl;*/
            /*if ('F' == kb->vkCode) {
                if (isFinishFSpaceF) {
                    // std::cout << "SimulateFSPACEF..." << std::endl;
                    isFinishFSpaceF = false;
                    std::thread clickThread(FSPACEF);
                    clickThread.detach();
                }
            }*/
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK OsdWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static std::wstring text;

    switch (msg) {
        case WM_SHOW_MODE:
            switch ((WorkMode)wParam) {
                case WorkMode::NullMode: text = L"空"; break;
                case WorkMode::LianyuMode: text = L"炼狱"; break;
                case WorkMode::XukongMode: text = L"虚空"; break;
            }

            ShowWindow(hWnd, SW_SHOW);
            InvalidateRect(hWnd, nullptr, TRUE);
            SetTimer(hWnd, 1, 1000, nullptr); // 800ms 后消失
            return 0;

        case WM_TIMER:
            KillTimer(hWnd, 1);
            ShowWindow(hWnd, SW_HIDE);
            return 0;

        case WM_ERASEBKGND:
            return 1;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT rc;
            GetClientRect(hWnd, &rc);

            SetBkMode(hdc, TRANSPARENT);
            HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdc, &rc, hBrush);
            DeleteObject(hBrush);
            SetTextColor(hdc, RGB(255, 0, 0));
            DrawTextW(hdc, text.c_str(), -1, &rc,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            EndPaint(hWnd, &ps);
            return 0;
        }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

HWND CreateOsdWindow(HINSTANCE hInst) {
    WNDCLASSW wc{};
    wc.lpfnWndProc   = OsdWndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = L"ModeOSD";

    RegisterClassW(&wc);

    int width  = 50;
    int height = 30;

    // int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    // int y = GetSystemMetrics(SM_CYSCREEN) / 5;
    int x = 0;
    int y = 0;

    HWND hWnd = CreateWindowExW(
            WS_EX_TOPMOST |
            WS_EX_LAYERED |
            WS_EX_TRANSPARENT |
            WS_EX_TOOLWINDOW,

            wc.lpszClassName,
            L"",
            WS_POPUP,

            x, y, width, height,
            nullptr, nullptr, hInst, nullptr
    );

    SetLayeredWindowAttributes(hWnd, 0, 200, LWA_ALPHA); // 半透明
    ShowWindow(hWnd, SW_HIDE);

    return hWnd;
}

void disableAllMode() {
    isLianyuMode = false;
    isXukongMode = false;
}

void SwitchMode() {
    int mode = static_cast<int>(g_mode);
    mode = (mode+1) % static_cast<int>(WorkMode::ModeCount);
    g_mode = static_cast<WorkMode>(mode);

    switch (g_mode) {
        case WorkMode::NullMode:
            std::cout << "NullMode Mode ..." << std::endl;
            disableAllMode();
            break;
        case WorkMode::LianyuMode:
            std::cout << "Lianyu Mode ..." << std::endl;
            disableAllMode();
            isLianyuMode = true;
            break;
        case WorkMode::XukongMode:
            std::cout << "Xukong Mode ..." << std::endl;
            disableAllMode();
            isXukongMode = true;
            break;
    }
    PostMessage(g_osdWnd, WM_SHOW_MODE, (WPARAM)g_mode, 0);
}

// --- 模拟按键函数 ---
void SimulateLeftClick(int t1, int t2) {
    // std::cout << "SimulateLeftClick ... " << t1 << " " << t2 << std::endl;

    INPUT inputDown = {0};
    inputDown.type = INPUT_MOUSE;
    inputDown.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &inputDown, sizeof(INPUT));

    Sleep(r(t1, t2));

    INPUT inputUp = {0};
    inputUp.type = INPUT_MOUSE;
    inputUp.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &inputUp, sizeof(INPUT));
}

void SimulateRightClick(int t1, int t2) {
    // std::cout << "SimulateRightClick ... " << std::endl;

    INPUT inputDownR = {0};
    inputDownR.type = INPUT_MOUSE;
    inputDownR.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    SendInput(1, &inputDownR, sizeof(INPUT));

    Sleep(r(t1, t2));

    INPUT inputUpR = {0};
    inputUpR.type = INPUT_MOUSE;
    inputUpR.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    SendInput(1, &inputUpR, sizeof(INPUT));
}

void LYSD() {
    SimulateLeftClick(T1, T2);
    Sleep(r(T3, T4));
}

void LYSC() {
    // std::cout << "SimulateRightClick ... " << std::endl;
    SimulateRightClick(10, 20);
    Sleep(r(250, 260));
    SimulateLeftClick(10, 20);
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
void FSPACEF() {
    // std::cout << "SimulateFSPACEF ... " << std::endl;
    SimulateKeyType('F');
    Sleep(r(25, 35));
    SimulateKeyType(VK_SPACE);
    Sleep(r(30, 40));
    SimulateKeyType('F');
    isFinishFSpaceF = true;
}

// 虚空卡重刀
void RightBtnFF() {
    // std::cout << "RightBtnFF ... " << std::endl;
    Sleep(r(80, 85));
    SimulateRightClick(15, 25);
    Sleep(r(570, 580));
    SimulateKeyType('F');
    Sleep(r(10, 20));
    SimulateRightClick(15, 25);
}

void LYSDThr() {
    while (true) {
        if (isLianyuMode && isLysdOn) {
            LYSD();
        } else {
            Sleep(r(10,25));
        }
    }
}
void LYSCThr() {
    while (true) {
        if (isLianyuMode && isLyscOn) {
            LYSC();
        } else {
            Sleep(r(10,25));
        }
    }
}
void RightBtnFFThr() {
    while (true) {
        if (isXukongMode && isRightBtnFFOn) {
            RightBtnFF();
        } else {
            Sleep(r(10,25));
        }
    }
}

/*
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
*/

// int main() {
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    std::map<std::string, std::string> config = GetConfig();
    if (config.empty()) {
        system("pause");
        return 1;
    }
    SwitchMode();
    T1 = std::stoi(config["interval.t1"]);
    T2 = std::stoi(config["interval.t2"]);
    T3 = std::stoi(config["interval.t3"]);
    T4 = std::stoi(config["interval.t4"]);
    // 炼狱速刺开关
    int isTurnOnLYSC = std::stoi(config["interval.lysc"]);
    std::cout << "lysc status: " << (isTurnOnLYSC ? "on" : "off") << std::endl;
    // 启动自动点击线程
    std::thread clickThread(LYSDThr);
    clickThread.detach();
    if (isTurnOnLYSC) {
        std::thread clickThreadLianyuR(LYSCThr);
        clickThreadLianyuR.detach();
    }
    std::thread clickThreadXukongR(RightBtnFFThr);
    clickThreadXukongR.detach();

    // 设置鼠标钩子
    HINSTANCE hModule = GetModuleHandle(nullptr);
    if (!hModule) {
        std::cout << "Failed to get module handle." << std::endl;
        return 1;
    }

    g_osdWnd = CreateOsdWindow(hInst);
    mouseHook = SetWindowsHookExW(WH_MOUSE_LL, MouseHookCallback, hModule, 0);
    if (!mouseHook) {
        std::cout << "Failed to set mouse hook." << std::endl;
        return 1;
    }
    keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardHookCallback,hModule,0);

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
    UnhookWindowsHookEx(keyboardHook);
    std::cout << "Program terminated." << std::endl;

    // int key_code = std::stoi(config["interval.key"]);
    // std::cout << key_code << std::endl;
    // int key_val;
    // switch (key_code) {
    //     case 2: key_val=VK_MBUTTON;
    //         break;
    //     case 4: key_val=VK_XBUTTON1;
    //         break;
    //     case 5: key_val=VK_XBUTTON2;
    //         break;
    //     default: key_val=VK_RBUTTON;
    //         break;
    // }
    // //std::cout << key_val << std::endl;
    //
    // mouse_click(t1, t2, t3, t4, key_val);
    return 0;
}

