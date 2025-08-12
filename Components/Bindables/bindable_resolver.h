#pragma once

#include <Bindables/bindable.h>

#include <type_traits>
#include <memory>
#include <unordered_map>

namespace Bind {
	class BindableResolver {
	public:
		template <class T,typename... Params>
		static std::shared_ptr<T> Resolve(Params&&... p) {
			static_assert(std::is_base_of<Bindable, T>::value, "Resolver can only resolve types derived from Bindable!");
			//Params参数可以从调用实参中推导出来，可以不在函数模板参数中显式指定
			return Get().resolve_impl<T>(std::forward<Params>(p)...);
		}

		static std::shared_ptr<Bindable> GetBindable(const std::string& name) {
			return Get().get_bindable_impl(name);
		}

	private:
		template <class T, typename... Params>
		std::shared_ptr<T> resolve_impl(Params&&... p) {
			const auto key = T::GenerateUID(std::forward<Params>(p)...);
			const auto i = m_binds.find(key);
			if (i == m_binds.end())
			{
				auto bind = std::make_shared<T>(std::forward<Params>(p)...);
				m_binds[key] = bind;
				return bind;
			}
			else
			{
				return std::static_pointer_cast<T>(i->second);
			}
		}

		//单例
		static BindableResolver& Get() {
			static BindableResolver resolver;
			return resolver;
		}

	private:
		inline std::shared_ptr<Bindable> get_bindable_impl(const std::string& name) {
			return m_binds.contains(name) ? m_binds[name] : nullptr;
		}

	private:
		std::unordered_map<std::string, std::shared_ptr<Bindable>> m_binds;
	};
}
