#pragma once

#include <Bindables/bindable.h>
#include <ogl_structures.h>

#include <memory>

namespace Bind
{
	class Sampler :public Bindable
	{
	public:
		Sampler(const std::string& name, GLuint unit, const OGL_TEXTURE_PARAMETER& param);
		~Sampler();

		void Bind() noxnd override;
		void UnBind() noxnd override;

		static std::shared_ptr<Sampler> Resolve(const std::string& name, GLuint unit, const OGL_TEXTURE_PARAMETER& param);
		static std::string GenerateUID(const std::string& name, GLuint unit, const OGL_TEXTURE_PARAMETER& param);

		std::string GetUID() const noexcept override;
		std::type_index GetTypeInfo() const noexcept override;

	private:
		void SetParameters(const OGL_TEXTURE_PARAMETER& param);

	private:
		std::string m_name;
		GLuint m_unit;
		OGL_TEXTURE_PARAMETER m_param;

		GLuint m_sampler;
	};
}