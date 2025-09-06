#include <Bindables/Sampler.h>
#include <Bindables/bindable_resolver.h>

namespace Bind
{
	Sampler::Sampler(const std::string& name, GLuint unit, const OGL_TEXTURE_PARAMETER& param)
		:m_name(name), m_unit(unit), m_param(param)
	{
		glCreateSamplers(1, &m_sampler);
		SetParameters(param);
	}

	Sampler::~Sampler()
	{
		glDeleteSamplers(1, &m_sampler);
	}

	void Sampler::Bind() noxnd
	{
		glBindSampler(m_unit, m_sampler);
	}

	void Sampler::UnBind() noxnd
	{
		glBindSampler(m_unit, 0);
	}

	std::shared_ptr<Sampler> Sampler::Resolve(const std::string& name, GLuint unit, const OGL_TEXTURE_PARAMETER& param)
	{
		return BindableResolver::Resolve<Sampler>(name, unit, param);
	}

	std::string Sampler::GenerateUID(const std::string& name, GLuint unit, const OGL_TEXTURE_PARAMETER& param)
	{
		using namespace std::string_literals;
		return typeid(Sampler).name() + "#"s + name + "#"s + std::to_string(unit);
	}

	std::string Sampler::GetUID() const noexcept
	{
		return GenerateUID(m_name, m_unit, m_param);
	}

	std::type_index Sampler::GetTypeInfo() const noexcept
	{
		return typeid(Sampler);
	}

	void Sampler::SetParameters(const OGL_TEXTURE_PARAMETER& param)
	{
		glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, param.min_filter);
		glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, param.mag_filter);
		glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_R, param.wrap_z);
		glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_S, param.wrap_x);
		glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_T, param.wrap_y);

		if (param.wrap_x == GL_CLAMP_TO_BORDER || param.wrap_y == GL_CLAMP_TO_BORDER) {
			if (param.border_color.has_value()) {
				float color[4] = { param.border_color.value().x,param.border_color.value().y,param.border_color.value().z,param.border_color.value().w };
				glSamplerParameterfv(m_sampler, GL_TEXTURE_BORDER_COLOR, color);
			}
			else {
				float color[4] = { 0,0,0,0 };
				glSamplerParameterfv(m_sampler, GL_TEXTURE_BORDER_COLOR, color);
			}
		}
	}
}