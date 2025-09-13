#include <Bindables/bindable.h>

namespace Bind {
	int Bindable::AllocateID(std::unordered_set<int>& id_set) {
		int sz = id_set.size();

		for (; sz-- >= 0;) {
			if (!id_set.contains(sz)) {
				id_set.insert(sz);
				return sz;
			}
		}

		return -1;
	}

	bool Bindable::NeedBindingPoint() noexcept
	{
		return false;
	}

	// Ĭ��ʲô������
	void Bindable::ChangeBindingPoint(GLuint binding) noexcept
	{
		return;
	}

	// -1Ĭ�ϴ���û�а󶨵�
	GLint Bindable::GetBindingPoint() const noexcept
	{
		return -1;
	}
}