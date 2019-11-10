#ifndef JSON_ASSERTIONS_H
#define JSON_ASSERTIONS_H
#include <iostream>

namespace Json
{
	void LogError(const char* condition, const char* file, int line, const char* message)
	{
		std::cerr << "Failed " << condition << ", " << message << ", File: " << file << ", Line: " << line;
	}

#define ASSERT_TRUE(condition, message) \
if (!condition) LogError(#condition, __FILE__, __LINE__, message);
}

#endif // !JSON_ASSERTIONS_H
