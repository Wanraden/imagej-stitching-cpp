/*
 * #%L
 * Fiji distribution of ImageJ for the life sciences.
 * %%
 * Copyright (C) 2007 - 2022 Fiji developers.
 * %%
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 * #L%
 */
#include "header.h"

/**
 * Utility class to deal with logging.
 *
 * @author Curtis Rueden
 */
class Log {

	/** Issues a message to the ImageJ log window, but only in debug mode. */
public:

	/** Issues an informational message to the ImageJ log window. */
	static void info(string message) {
		cout << message << endl;
	}

	/** Issues an informational message with timestamp to the ImageJ log window. */
	void timestamp(string message) {
		auto now = std::chrono::system_clock::now();
		// 将时间戳转换为毫秒数
		auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
		auto value = now_ms.time_since_epoch().count();
		info("(" + to_string(value) + "):" + message);
	}

	/**
	 * Issues an error message to the ImageJ log window.
	 *
	 * @see IJ#log(string)
	 */
	static void error(string message) {
		cout << "Error: " << message << endl;
	}

	/**
	 * Issues an exception stack trace to an ImageJ exception window.
	 *
	 * @see IJ#handleException(Throwable)
	 */
	static void error(exception t) {
		error("", t);
	}

	/**
	 * Issues an error message to the ImageJ log window, plus an exception stack
	 * trace to an ImageJ exception window.
	 *
	 * @see IJ#log(string)
	 * @see IJ#handleException(Throwable)
	 */
	static void error(string message, exception t) {
		
	}

	static void initLog(int type)
	{
	}


private:
	static string message(exception t) {
		return t.what();
	}

private:
	static int type;
	static int value;
};
