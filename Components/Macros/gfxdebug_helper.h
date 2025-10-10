#pragma once

#include <glad/glad.h>

// label是一个C风格字符串
#define APP_RANGE_BEGIN(label) \
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 0, label);

// 必须和上面的宏成对使用
#define APP_RANGE_END() \
	glPopDebugGroup();