#pragma once

#include <Bindables/rendertarget.h>
#include <Bindables/shaderprogram.h>
#include <Drawables/plane.h>
#include <logging.h>

#define TEXTURE_RED_BIT (1<<3)
#define TEXTURE_GREEN_BIT (1<<2)
#define TEXTURE_BLUE_BIT (1<<1)
#define TEXTURE_ALPHA_BIT 1

namespace Common
{
	class RenderHelper
	{
	public:
		static void Initialize()
		{
			InitializeFullScrPlane();
		}

		// 着色器简单地接受一张2D纹理，将这张纹理渲染到整个指定的render target上，另外，这个texture的绑定点一定需要设置为0
		// channels代表希望渲染的通道组合，每通道用一位代表，位从高到低为RGBA，默认渲染RGB三通道
		// 默认会清除render target的Color Buffer，这是合理的假设
		static void RenderTextureToScreen(std::shared_ptr<Bind::AbstractTexture> image, unsigned int channels = 0xE, std::shared_ptr<Bind::RenderTarget> frame = nullptr, bool clear_depth = true, bool clear_stencil = false)
		{
			if (channels == 0)
			{
				LOGW("No channel specified for output!");
				std::exit(EXIT_FAILURE);
			}
			auto vert = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "rendertexture_vertex", "Common", "render_texture.vert");
			auto frag = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "rendertexture_fragment", "Common", "render_texture.frag");
			auto shader = Bind::ShaderProgram::Resolve("rendertexture_shader", std::vector<GLuint>{vert, frag});

			shader->BindWithoutUpdate();
			shader->EditUniform("channels") = channels;
			shader->UpdateOnly();
			image->Bind();
			if (frame)
			{
				frame->Bind();
				if (clear_depth)
				{
					glClear(GL_DEPTH_BUFFER_BIT);
				}
				if (clear_stencil)
				{
					glClear(GL_STENCIL_BUFFER_BIT);
				}
			}
			else
			{
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glClear(GL_DEPTH_BUFFER_BIT);
			}
			glClear(GL_COLOR_BUFFER_BIT);

			Get().full_screen_plane.Draw();

			shader->UnBind();
			image->UnBind();
			if (frame)
			{
				frame->UnBind();
			}
		}

		// 将如法线这样的pack在两通道内的单位向量unpack并渲染到屏幕上
		// channels指定pack位置，输入要求正好置位两位，默认为RG通道
		// 另外，要求texture在pack的时候一定是按顺序pack前两个分量
		static void RenderUnitVectorToScreen(std::shared_ptr<Bind::ImageTexture2D> image, uint8_t channels = 0xC, std::shared_ptr<Bind::RenderTarget> frame = nullptr, bool clear_depth = true, bool clear_stencil = false)
		{
			auto vert = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "renderunitvector_vertex", "Common", "render_texture.vert");
			auto frag = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "renderunitvector_fragment", "Common", "render_unit.frag");
			auto shader = Bind::ShaderProgram::Resolve("renderunitvector_shader", std::vector<GLuint>{vert, frag});

			shader->BindWithoutUpdate();
			shader->EditUniform("channels") = static_cast<unsigned int>(channels);
			shader->UpdateOnly();
			image->Bind();
			if (frame)
			{
				frame->Bind();
				if (clear_depth)
				{
					glClear(GL_DEPTH_BUFFER_BIT);
				}
				if (clear_stencil)
				{
					glClear(GL_STENCIL_BUFFER_BIT);
				}
			}
			else
			{
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glClear(GL_DEPTH_BUFFER_BIT);
			}
			glClear(GL_COLOR_BUFFER_BIT);

			Get().full_screen_plane.Draw();

			shader->UnBind();
			image->UnBind();
			if (frame)
			{
				frame->UnBind();
			}
		}

		// 默认depth在第一个分量
		static void RenderLinearDepthToScreen(std::shared_ptr<Bind::ImageTexture2D> depth_image, float near_plane, float far_plane, std::shared_ptr<Bind::RenderTarget> frame = nullptr, bool clear_depth = true, bool clear_stencil = false)
		{
			auto vert = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "renderlineardepth_vertex", "Common", "render_texture.vert");
			auto frag = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "renderlineardepth_fragment", "Common", "render_linear_depth.frag");
			auto shader = Bind::ShaderProgram::Resolve("renderlineardepth_shader", std::vector<GLuint>{vert, frag});

			shader->BindWithoutUpdate();
			shader->EditUniform("near_plane") = near_plane;
			shader->EditUniform("far_plane") = far_plane;
			shader->UpdateOnly();
			depth_image->Bind();
			if (frame)
			{
				frame->Bind();
				if (clear_depth)
				{
					glClear(GL_DEPTH_BUFFER_BIT);
				}
				if (clear_stencil)
				{
					glClear(GL_STENCIL_BUFFER_BIT);
				}
			}
			else
			{
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glClear(GL_DEPTH_BUFFER_BIT);
			}
			glClear(GL_COLOR_BUFFER_BIT);

			Get().full_screen_plane.Draw();

			shader->UnBind();
			depth_image->UnBind();
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