#include <Common/computeProxy.h>

#include <Bindables/shaderprogram.h>
#include <Bindables/constantbuffer.h>
#include <logging.h>

namespace Common
{
	ComputeProxy& ComputeProxy::AddBindable(std::shared_ptr<Bind::Bindable> bindable, GLint binding)
	{
		auto shader = std::dynamic_pointer_cast<Bind::ShaderProgram>(bindable);
		auto constant = std::dynamic_pointer_cast<Bind::ConstantBuffer>(bindable);

		if (shader)
		{
			if (m_compute_shader)
			{
				LOGW("Compute shader already exists!Only one shader allowed for a compute proxy!");
				std::exit(EXIT_FAILURE);
			}
			m_compute_shader = shader;
		}
		else if (constant)
		{
			if (std::string buffer_name = constant->BufferName(); m_constants.contains(buffer_name))
			{
				LOGW("Constant buffer name: {} already exists!", buffer_name.c_str());
				std::exit(EXIT_FAILURE);
			}
			else
			{
				m_constants[buffer_name] = constant;
				if (binding >= 0)
				{
					m_constants_binding_change_list[buffer_name] = GLuint(binding);
				}
			}
		}
		else
		{
			if (bindable->NeedBindingPoint())
			{
				m_binding_change_list[m_bindables.size()] = GLuint(binding);
			}
			m_bindables.push_back(bindable);
		}

		return *this;
	}

	void ComputeProxy::Finalize()
	{
		if (!m_compute_shader)
		{
			LOGW("No compute shader specified!One shader needed for any compute job!");
			std::exit(EXIT_FAILURE);
		}

		// 更新着色器和常量缓冲区
		m_compute_shader->Bind();
		m_compute_shader->UnBind();
		for (auto& [_, buffer] : m_constants)
		{
			buffer->Bind();
			buffer->Update();
			buffer->UnBind();
		}
	}

	void ComputeProxy::Dispatch()
	{
		BindAll();
		glDispatchCompute(m_dimensions[0], m_dimensions[1], m_dimensions[2]);
		UnBindAll();
	}

	void ComputeProxy::BindAll()
	{
		for (auto& [index, binding] : m_binding_change_list)
		{
			GLuint binding_new = binding;
			binding = GLuint(m_bindables[index]->GetBindingPoint());
			m_bindables[index]->ChangeBindingPoint(binding_new);
		}
		for (auto& [name, binding] : m_constants_binding_change_list)
		{
			GLuint binding_new = binding;
			binding = GLuint(m_constants[name]->GetBindingPoint());
			m_constants[name]->ChangeBindingPoint(binding_new);
		}

		for (auto& bindable : m_bindables)
		{
			bindable->Bind();
		}

		m_compute_shader->Bind();

		for (auto& [_, constant] : m_constants)
		{
			constant->Update();
			constant->Bind();
		}
	}

	void ComputeProxy::UnBindAll()
	{
		for (auto& bindable : m_bindables)
		{
			bindable->UnBind();
		}

		m_compute_shader->UnBind();

		for (auto& [_, constant] : m_constants)
		{
			constant->UnBind();
		}

		for (auto& [index, binding] : m_binding_change_list)
		{
			GLuint binding_old = binding;
			binding = GLuint(m_bindables[index]->GetBindingPoint());
			m_bindables[index]->ChangeBindingPoint(binding_old);
		}
		for (auto& [name, binding] : m_constants_binding_change_list)
		{
			GLuint binding_old = binding;
			binding = GLuint(m_constants[name]->GetBindingPoint());
			m_constants[name]->ChangeBindingPoint(binding_old);
		}
	}
}