#pragma once

#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <cassert>

#define TYPE_GENERATOR \
	X(GL_VERTEX_SHADER,Vertex) \
	X(GL_FRAGMENT_SHADER,Fragment) \
	X(GL_GEOMETRY_SHADER,Geometry) \
	X(GL_TESS_CONTROL_SHADER,TessControl) \
	X(GL_TESS_EVALUATION_SHADER,TessEvaluation) \
	X(GL_COMPUTE_SHADER,Compute)

#define SWITCH_TYPE \
	X(ShaderObject::ShaderType::Vertex,m_vertexshaders) \
	X(ShaderObject::ShaderType::Fragment,m_fragmentshaders) \
	X(ShaderObject::ShaderType::Geometry,m_geometryshaders) \
	X(ShaderObject::ShaderType::Compute,m_computeshaders) \
	X(ShaderObject::ShaderType::TessControl,m_tesscontrolshaders) \
	X(ShaderObject::ShaderType::TessEvaluation,m_tessevaluationshaders)

namespace Bind {
	class ShaderObject {
		friend class ShaderProgram;

	public:
		enum ShaderType {
#define X(GLType,Type) Type,
			TYPE_GENERATOR
#undef X
		};

		ShaderObject(ShaderType type, const std::string& tag, const std::string& proj, const std::string& file);
		~ShaderObject();

		static GLuint Resolve(ShaderType type, const std::string& tag, const std::string& proj, const std::string& file);
		static GLuint Resolve(ShaderType type, const std::string& tag);

		operator const GLuint& () const noexcept;

		template <ShaderType type>
		static std::shared_ptr<ShaderObject> GetShaderObjectPointer(const std::string& name);
		template <ShaderType type>
		static GLuint GetShaderObject(const std::string& name);

	private:
		inline GLuint GetShader() const noexcept { return m_shader; };

		static void checkCompileErrors(GLuint shader, std::string type);

	private:
		std::string m_tag;
		GLuint m_shader;
		ShaderType m_type;

		static std::unordered_map<std::string, std::shared_ptr<ShaderObject>> m_vertexshaders;
		static std::unordered_map<std::string, std::shared_ptr<ShaderObject>> m_fragmentshaders;
		static std::unordered_map<std::string, std::shared_ptr<ShaderObject>> m_geometryshaders;
		static std::unordered_map<std::string, std::shared_ptr<ShaderObject>> m_computeshaders;
		static std::unordered_map<std::string, std::shared_ptr<ShaderObject>> m_tesscontrolshaders;
		static std::unordered_map<std::string, std::shared_ptr<ShaderObject>> m_tessevaluationshaders;
	};

	template <ShaderObject::ShaderType type>
	std::shared_ptr<ShaderObject> ShaderObject::GetShaderObjectPointer(const std::string& name) {
		switch (type) {
#define X(Type, Member) case Type: return Member.contains(name) ? Member[name] : nullptr;

			SWITCH_TYPE
#undef X
		default:
			return nullptr;
		}
	}

	template <ShaderObject::ShaderType type>
	GLuint ShaderObject::GetShaderObject(const std::string& name) {
		switch (type) {
#define X(Type, Member) case Type: assert(Member.contains(name)); return Member[name]->GetShader();

			SWITCH_TYPE
#undef X
		default:
			return 0;
		}
	}
}

#ifndef SHADER_OBJECT_SOURCE_FILE
#undef TYPE_GENERATOR
#undef SWITCH_TYPE
#endif