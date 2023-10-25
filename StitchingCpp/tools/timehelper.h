#pragma once

#include "header.h"

class TimeHelper {
public:
	static long long milliseconds() {
		auto now = std::chrono::system_clock::now();
		// ��ʱ���ת��Ϊ������
		auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
		return now_ms.time_since_epoch().count();
	}
};