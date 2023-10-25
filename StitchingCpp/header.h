#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <map>

using namespace std;

#define LOGINFO(msg) (std::cout << msg << std::endl)
#define LOGERR(msg) (std::cout << "Error: " << msg << std::endl)