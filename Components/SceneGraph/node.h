#pragma once

#include <SceneGraph/transform.h>
#include <SceneGraph/material.h>
#include <Dynamic/dynamic_constant.h>
#include <Drawables/drawable.h>

#include <vector>
#include <string>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <functional>
#include <tuple>
#include <type_traits>
#include <glad/glad.h>

namespace Bind {
	class Bindable;
}

namespace SceneGraph {
	//控制节点中需要储存一系列的函数来指导不同类型的着色器变量信息的生成和更新
	enum class ConfigurationType {
		Transformation,
		TextureStrength,
		MaterialDiffuse,
		MaterialSpecular,
		MaterialAmbient,
		MaterialEmissive,
		MaterialTransparent,
		MaterialOpacity,
		MaterialShininess,
		MaterialShiniStrength,
	};

#define CONFIG_GENERATOR \
	X(ConfigurationType::MaterialDiffuse, glm::vec3, m_diffuse, UniConstDiffuseFunc) \
	X(ConfigurationType::MaterialSpecular, glm::vec3, m_specular, UniConstSpecularFunc) \
	X(ConfigurationType::MaterialAmbient, glm::vec3, m_ambient, UniConstAmbientFunc) \
	X(ConfigurationType::MaterialEmissive, glm::vec3, m_emissive, UniConstEmissiveFunc) \
	X(ConfigurationType::MaterialTransparent, glm::vec3, m_transparent, UniConstTransparentFunc) \
	X(ConfigurationType::MaterialOpacity, float, m_opacity, UniConstOpacityFunc) \
	X(ConfigurationType::MaterialShininess, float, m_shininess, UniConstShininessFunc) \
	X(ConfigurationType::MaterialShiniStrength, float, m_shininess_strength, UniConstShiniStrengthFunc)

	class ControlNode;

	class Node {
	public:
		Node(const size_t id, const std::string& name);
		virtual ~Node() = default;

		inline const size_t GetID() const noexcept { return m_id; };
		inline const std::string& GetName() const noexcept { return m_name; };
		inline const std::vector<Node*>& GetChildren() const { return m_children; };
		inline const glm::mat4& GetWorldMatrix() const noexcept { return m_world_matrix; };
		inline void SetWorldMatrix(glm::mat4 mat) noexcept { m_world_matrix = mat; };

		virtual void SetParent(Node& parent);
		inline Node* GetParent() { return m_parent; };

		void AddChild(Node& child);
		void ResetChildren(std::vector<Node*> nodes);
		
		void SetBindableUnique(std::shared_ptr<Bind::Bindable> bindable, size_t index = 0);
		void SetBindable(std::shared_ptr<Bind::Bindable> bindabl, size_t index = 0);

		std::shared_ptr<Bind::Bindable> GetBindableUnique(const std::type_index& type_info, size_t index = 0) const;
		std::shared_ptr<Bind::Bindable> GetBindable(const std::type_index& type_info, size_t index, size_t drawable_index = 0) const;
		template <typename T>
		std::shared_ptr<T> GetBindableUnique(size_t index = 0) const {
			return std::dynamic_pointer_cast<T>(GetBindableUnique(typeid(T), index));
		}
		template <typename T>
		std::shared_ptr<T> GetBindable(size_t index, size_t drawable_index = 0) const {
			return std::dynamic_pointer_cast<T>(GetBindable(typeid(T), index, drawable_index));
		}

		bool HasComponent(const std::type_index& type_info) const;
		template <typename T>
		bool HasComponent() {
			return HasComponent(typeid(T));
		}

		Node* FindNodeWithName(const std::string& name);
		Node* FindNodeWithID(unsigned int ID);

		bool HasShader() const noexcept;

		virtual void CookNode(std::pair<std::vector<Dynamic::Dsr::VertexAttrib>, std::vector<Dynamic::Dsr::VertexAttrib>>& attribs, std::vector<DrawItems::VertexType>& instruction,
			std::unordered_map<Material::TextureCategory,std::vector<std::pair<std::string,GLuint>>>& textures, const std::string& rel_path) = 0;

		virtual void Render(ControlNode* node, bool force_update) = 0;
		virtual void Update(ControlNode* node, size_t index = 0) = 0;

		void BindAll();
		void BindIndex(size_t index);
		void UnBindAll();
		void UnBindIndex(size_t index);

	protected:
		unsigned int m_id;
		std::string m_name;

		std::vector<std::unordered_map<std::type_index, std::vector<std::shared_ptr<Bind::Bindable>>>> m_bindables;

		Node* m_parent = nullptr;
		std::vector<Node*> m_children;//不使用智能指针以避免循环引用

		glm::mat4 m_world_matrix = glm::mat4(1.0);
	};

	class EntityNode : public Node {
	public:
		EntityNode(const size_t id, const std::string& name, unsigned int drawable_count = 1);
		~EntityNode() = default;

		const glm::mat4& CalculateWorldMatrix() const noexcept;
		void SetTranslation(const glm::vec3& translate) noexcept;
		void SetRotation(const glm::vec3& axis, float angle) noexcept;
		void SetRotation(const glm::vec3& angles) noexcept;//三个分量分别代表绕x,y,z轴的旋转角度
		void SetRotation(const glm::vec4& quat) noexcept;
		void SetScaling(const glm::vec3& scale) noexcept;

		void SetParent(Node& node) override;

		inline void AddDrawable(DrawItems::Drawable& item) { m_drawables.push_back(&item); };
		inline void AddMaterial(Material& mat) { m_materials.push_back(&mat); };

		inline size_t DrawableCount() const noexcept {
			return m_drawables.size();
		};
		inline const DrawItems::Drawable& GetDrawable(size_t index) const {
			assert(index < m_drawables.size());
			return *m_drawables[index];
		}
		inline const Material& GetMaterial(size_t index) const {
			assert(index < m_materials.size());
			return *m_materials[index];
		}

		void CookNode(std::pair<std::vector<Dynamic::Dsr::VertexAttrib>, std::vector<Dynamic::Dsr::VertexAttrib>>& attribs, std::vector<DrawItems::VertexType>& instruction,
			std::unordered_map<Material::TextureCategory, std::vector<std::pair<std::string, GLuint>>>& textures, const std::string& rel_path) override;

		void Render(ControlNode* node, bool force_update) override;
		void Update(ControlNode* node, size_t index = 0) override;

	private:
		void CookVertex(const std::pair<std::vector<Dynamic::Dsr::VertexAttrib>, std::vector<Dynamic::Dsr::VertexAttrib>>& attribs, const std::vector<DrawItems::VertexType>& instruction);
		void CookTexture(const std::unordered_map<Material::TextureCategory, std::vector<std::pair<std::string, GLuint>>>& textures, const std::string& rel_path);

	private:
		std::vector<DrawItems::Drawable*> m_drawables;
		std::vector<Material*> m_materials;//材质信息用来辅助生成纹理

		Transform m_transform;
	};

	template <typename Derived>
	class UniConstFuncBase {
		friend class EntityNode;

	public:
		UniConstFuncBase(Dynamic::Dcb::ElementRef<Derived>& ref) :m_ref(ref) {}
		virtual ~UniConstFuncBase() = default;

		virtual void operator()(EntityNode& node, size_t index = 0) = 0;
	protected:
		Dynamic::Dcb::ElementRef<Derived>& m_ref;
	};

#define X(ConfigType, Type, Member, ClassName) \
	template <typename Derived, typename Func, typename... Args> \
	class ClassName :public UniConstFuncBase<Derived> { \
	public: \
		ClassName(Func func, Dynamic::Dcb::ElementRef<Derived>& ref, Args&&... args) :UniConstFuncBase<Derived>(ref), m_func(func), m_args(std::make_tuple(std::forward<Args>(args)...)) {} \
		\
		void operator()(EntityNode& node, size_t index = 0) override; \
		\
	private: \
		std::function<typename std::invoke_result<Func, Type, Args...>::type(Type, Args...)> m_func; \
		std::tuple<Args...> m_args; \
	}; \
	\
	template <typename Derived, typename Func, typename... Args> \
	void ClassName<Derived, Func, Args...>::operator()(EntityNode& node, size_t index) { \
		this->m_ref = std::apply([&](auto&&... args){return m_func(node.m_materials[index]->GetExtraInformation().Member, std::forward<Args>(args)...);}, m_args); \
	}

	CONFIG_GENERATOR
#undef X

	template <typename Derived, typename Func, typename... Args>
	class UniConstTransformFunc :public UniConstFuncBase<Derived> {
	public:
		UniConstTransformFunc(Func func, Dynamic::Dcb::ElementRef<Derived>& ref, Args&&... args) :UniConstFuncBase<Derived>(ref), m_func(func), m_args(std::make_tuple(std::forward<Args>(args)...)) {}

		void operator()(EntityNode& node, size_t index = 0) override;

	private:
		std::function<typename std::invoke_result<Func, glm::mat4, Args...>::type(glm::mat4, Args...)> m_func;
		std::tuple<Args...> m_args;
	};

	template <typename Derived, typename Func, typename... Args>
	void UniConstTransformFunc<Derived, Func, Args...>::operator()(EntityNode& node, size_t index) {
		this->m_ref = std::apply([&](auto&&... args) {return m_func(node.GetWorldMatrix(), std::forward<Args>(args)...); }, m_args);
	}

	template <typename Derived, typename Func, typename... Args>
	class UniConstTexStrengthFunc :public UniConstFuncBase<Derived> {
	public:
		UniConstTexStrengthFunc(Func func, Dynamic::Dcb::ElementRef<Derived>& ref, Material::TextureCategory type, unsigned int index, Args&&... args)
			:UniConstFuncBase<Derived>(ref), m_func(func), m_args(std::make_tuple(std::forward<Args>(args)...)), m_type(type), m_index(index) {
		}

		void operator()(EntityNode& node, size_t index = 0) override;

	private:
		std::function<typename std::invoke_result<Func, float, Args...>::type(float, Args...)> m_func;
		std::tuple<Args...> m_args;
		Material::TextureCategory m_type;
		unsigned int m_index;
	};

	template <typename Derived, typename Func, typename... Args>
	void UniConstTexStrengthFunc<Derived, Func, Args...>::operator()(EntityNode& node, size_t index) {
		this->m_ref = std::apply([&](auto&&... args) {return m_func(node.GetMaterial(index).GetTexture(m_type, m_index).m_strength, std::forward<Args>(args)...); }, m_args);
	}

	template <ConfigurationType T> struct FunctionMapping;

#define X(ConfigType, Type, Member, ClassName) \
	template <> \
	struct FunctionMapping<ConfigType> { \
		using SysType = Type; \
	};

	CONFIG_GENERATOR
#undef X
		template <>
	struct FunctionMapping<ConfigurationType::Transformation> {
		using SysType = glm::mat4;
	};
	template <>
	struct FunctionMapping<ConfigurationType::TextureStrength> {
		using SysType = float;
	};

	template <ConfigurationType Type, typename Func, typename... Args>
	concept UniConstFuncConstrains = requires(Func f, FunctionMapping<Type>::SysType && fixmember, Args&&... args) {
		f(std::forward<typename FunctionMapping<Type>::SysType>(fixmember), std::forward<Args>(args)...);
	};

	class ControlNode :public Node {
		friend class EntityNode;

	public:
		ControlNode(const size_t id, const std::string& name);
		~ControlNode() = default;

		template <ConfigurationType Config, typename Func, typename... Args>
			requires UniConstFuncConstrains<Config,Func,Args...>
		void RegisterUniform(Dynamic::Dcb::UniformElementRef& ref, Func f, Args&&... args) {
			if constexpr (Config == ConfigurationType::Transformation) {
				m_uniform_functions[ConfigurationType::Transformation].push_back(std::make_shared
					<UniConstTransformFunc<Dynamic::Dcb::UniformElementRef, Func, Args...>>(f, ref, std::forward<Args>(args)...));
			}
			else if constexpr (Config == ConfigurationType::TextureStrength) {
				static_assert(sizeof...(Args) >= 2, "At least two extra parameters for texture type and texture index are needed for TextureStrength functions!");
				static_assert(std::is_same_v<std::tuple_element_t<0, std::tuple<Args...>>, Material::TextureCategory>, "First extra parameter must be of type Material::TextureCategory!");
				static_assert(std::is_same_v<std::tuple_element_t<1, std::tuple<Args...>>, unsigned int>, "Second extra paremeter must be of type unsigned int!");

				m_uniform_functions[ConfigurationType::TextureStrength].push_back(std::make_shared
					<UniConstTexStrengthFunc<Dynamic::Dcb::UniformElementRef, Func, Args...>>(f, ref, std::forward<Args>(args)...));
			}
#define X(ConfigType, Type, Member, ClassName) \
			else if constexpr (Config == ConfigType) { \
				m_uniform_functions[ConfigType].push_back(std::make_shared<ClassName<Dynamic::Dcb::UniformElementRef, Func, Args...>>(f,ref,std::forward<Args>(args)...)); \
			}

			CONFIG_GENERATOR
#undef X
			else {
				assert("Function type not supported!" && false);
			}
		}
		template <ConfigurationType Config, typename Func, typename... Args>
			requires UniConstFuncConstrains<Config, Func, Args...>
		void RegisterConstant(Dynamic::Dcb::ConstantElementRef& ref, Func f, Args&&... args) {
			if constexpr (Config == ConfigurationType::Transformation) {
				m_constant_functions[ConfigurationType::Transformation].push_back(std::make_shared
					<UniConstTransformFunc<Dynamic::Dcb::ConstantElementRef, Func, Args...>>(f, ref, std::forward<Args>(args)...));
			}
			else if constexpr (Config == ConfigurationType::TextureStrength) {
				static_assert(sizeof...(Args) >= 2, "At least two extra parameters for texture type and texture index are needed for TextureStrength functions!");
				static_assert(std::is_same_v<std::tuple_element_t<0, std::tuple<Args...>>, Material::TextureCategory>, "First extra parameter must be of type Material::TextureCategory!");
				static_assert(std::is_same_v<std::tuple_element_t<1, std::tuple<Args...>>, unsigned int>, "Second extra paremeter must be of type unsigned int!");

				m_constant_functions[ConfigurationType::TextureStrength].push_back(std::make_shared
					<UniConstTexStrengthFunc<Dynamic::Dcb::ConstantElementRef, Func, Args...>>(f, ref, std::forward<Args>(args)...));
			}
#define X(ConfigType, Type, Member, ClassName) \
			else if constexpr(Config == ConfigType) { \
				m_constant_functions[ConfigType].push_back(std::make_shared<ClassName<Dynamic::Dcb::ConstantElementRef, Func, Args...>>(f,ref,std::forward<Args>(args)...)); \
			}

			CONFIG_GENERATOR
#undef X
			else {
				assert("Function type not supported!" && false);
			}
		}

		void AddTextureConfig(const std::string& name, GLuint binding, Material::TextureCategory type);
		void AddVertexConfig(std::vector<DrawItems::VertexType> instruction);

		inline bool HasVertexConfiguration() const noexcept { return m_vertex_instruction.size(); };


		void StartCooking(const std::string& rel_path);
		void CookNode(std::pair<std::vector<Dynamic::Dsr::VertexAttrib>, std::vector<Dynamic::Dsr::VertexAttrib>>& attribs, std::vector<DrawItems::VertexType>& instruction,
			std::unordered_map<Material::TextureCategory, std::vector<std::pair<std::string, GLuint>>>& textures, const std::string& rel_path) override;

		void StartRender();

		void Render(ControlNode* node, bool force_update) override;
		void Update(ControlNode* node, size_t index = 0) override;

	private:
		std::unordered_map<ConfigurationType, std::vector<std::shared_ptr<UniConstFuncBase<Dynamic::Dcb::UniformElementRef>>>> m_uniform_functions;
		std::unordered_map<ConfigurationType, std::vector<std::shared_ptr<UniConstFuncBase<Dynamic::Dcb::ConstantElementRef>>>> m_constant_functions;
		std::unordered_map<Material::TextureCategory, std::vector<std::pair<std::string, GLuint>>> m_texture_vector;
		std::vector<DrawItems::VertexType> m_vertex_instruction;
	};
}