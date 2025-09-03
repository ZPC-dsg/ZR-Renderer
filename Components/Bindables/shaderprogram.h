#pragma once

#include <Bindables/includer.h>
#include <Dynamic/dynamic_constant.h>
#include <Bindables/shaderobject.h>

namespace Bind {
	class ShaderProgram :public Bindable {
	public:
		ShaderProgram(const std::string& tag, std::vector<GLuint> shaders);
		ShaderProgram(GLuint ID);
		~ShaderProgram();

		void Bind() noxnd override;
		void UnBind() noxnd override;

		static std::shared_ptr<ShaderProgram> Resolve(const std::string& tag, std::vector<GLuint> shaders);
		static std::string GenerateUID(const std::string& tag, std::vector<GLuint> shaders);
		std::string GetUID() const noexcept override;

		inline GLuint get_program() const noexcept { return m_program; }

		Dynamic::Dcb::CPUUniformBlock& operator[](const std::string& key) noxnd;
		Dynamic::Dcb::CPUUniformBlock& EditUniform(const std::string& key) noxnd;

		std::type_index GetTypeInfo() const noexcept override;

		void BindWithoutUpdate() noxnd;
		void UpdateOnly() noxnd;//已知之前已经将着色器绑好了以免重复绑定

		void SetTexture(const std::string& shader_name, unsigned int binding) noxnd;

	private:
		static void CheckCompileErrors(GLuint program);

		static void UpdateUniform(LeafType type, GLint location, const AvailableType& data) noxnd;

	private:
		std::string m_tag;
		GLuint m_program;
		std::unordered_map<std::string, Dynamic::Dcb::CPUUniformBlock> m_uniforms;
	};
}