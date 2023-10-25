#pragma once

#include "header.h"

class TimeHelper {
public:
	static long long milliseconds() {
		auto now = std::chrono::system_clock::now();
		// 将时间戳转换为毫秒数
		auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
		return now_ms.time_since_epoch().count();
	}
};