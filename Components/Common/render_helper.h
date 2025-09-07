#pragma once

#include <Bindables/rendertarget.h>
#include <Bindables/shaderprogram.h>
#include <Drawables/plane.h>

namespace Common
{
	class RenderHelper
	{
	public:
		static void Initialize()
		{
			InitializeFullScrPlane();
		}

		// ��ɫ���򵥵ؽ���һ��2D����������������Ⱦ������ָ����render target�ϣ����⣬���texture�İ󶨵�һ����Ҫ����Ϊ0
		static void RenderTextureToScreen(std::shared_ptr<Bind::ImageTexture2D> image, std::shared_ptr<Bind::RenderTarget> frame = nullptr)
		{
			auto vert = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "rendertexture_vertex", "default", "render_texture.vert");
			auto frag = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "rendertexture_fragment", "default", "render_texture.frag");
			auto shader = Bind::ShaderProgram::Resolve("rendertexture_shader", std::vector<GLuint>{vert, frag});

			shader->Bind();
			image->Bind();
			if (frame)
			{
				frame->Bind();
			}
			else
			{
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

			Get().full_screen_plane.Draw();

			shader->UnBind();
			image->UnBind();
			if (frame)
			{
				frame->UnBind();
			}
		}

	private:
		static RenderHelper& Get()
		{
			static RenderHelper helper;
			return helper;
		}

		static void InitializeFullScrPlane()
		{
			auto& helper = RenderHelper::Get();

			helper.full_screen_plane = DrawItems::Plane("fullscr_rectangle");
			std::vector<Dynamic::Dsr::VertexAttrib> attrib(2);
			attrib[0].location = 0;
			attrib[0].name = "aPos";
			attrib[0].size = 1;
			attrib[0].type = GL_FLOAT_VEC3;
			attrib[1].location = 1;
			attrib[1].name = "aTexCoord";
			attrib[1].size = 1;
			attrib[1].type = GL_FLOAT_VEC2;
			helper.full_screen_plane.GenerateVAO(attrib, { DrawItems::VertexType::Position,DrawItems::VertexType::Texcoord });
		}

	private:
		DrawItems::Plane full_screen_plane;
	};
}