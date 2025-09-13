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

	// 默认什么都不做
	void Bindable::ChangeBindingPoint(GLuint binding) noexcept
	{
		return;
	}

	// -1默认代表没有绑定点
	GLint Bindable::GetBindingPoint() const noexcept
	{
		return -1;
	}
}