#pragma once

#include <glad/glad.h>

// label��һ��C����ַ���
#define APP_RANGE_BEGIN(label) \
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 0, label);

// ���������ĺ�ɶ�ʹ��
#define APP_RANGE_END() \
	glPopDebugGroup();