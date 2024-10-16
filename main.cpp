#include <iostream>
#include <windows.h>
#include <random>
#include <cmath>

#include "ConfigParse.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    while (true) {
        if (GetAsyncKeyState(key) & 0x8000) {
            long interval_t1 = r(t1,t2);
            long interval_t2 = r(t3,t4);
            //std::cout << interval_t1 << " " << interval_t2 << std::endl;
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
    int t1 = std::stoi(config["interval.t1"]);
    int t2 = std::stoi(config["interval.t2"]);
    int t3 = std::stoi(config["interval.t3"]);
    int t4 = std::stoi(config["interval.t4"]);

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
    return 0;
}
