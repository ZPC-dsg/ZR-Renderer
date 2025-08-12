#include <pbr.h>
#include <drawableloader.h>
#include <SceneGraph/material.h>
#include <Bindables/shaderprogram.h>
#include <Bindables/imagetexture2D.h>
#include <Bindables/cubemap.h>
#include <Bindables/rendertarget.h>
#include <Bindables/viewscissor.h>

namespace RTREffects {
	PBR_IBL::PBR_IBL(const std::string& name) :Utils(name) {
		globalSettings::mainCamera.set_perspective(60.0f, globalSettings::screen_width, globalSettings::screen_height, 0.1f, 512.0f);
		globalSettings::mainCamera.set_position(glm::vec3(0.0f, 0.0f, 4.0f));
	}

	void PBR_IBL::render() {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

		m_pbrShader->EditUniform("view") = globalSettings::mainCamera.get_view();
		m_pbrShader->EditUniform("projection") = globalSettings::mainCamera.get_perspective();
		m_pbrShader->EditUniform("cameraPos") = globalSettings::mainCamera.get_position();
		m_pbrShader->Bind();
		m_proxy->Render();

		m_backgroundShader->EditUniform("view") = globalSettings::mainCamera.get_view();
		m_backgroundShader->EditUniform("projection") = globalSettings::mainCamera.get_perspective();
		m_backgroundShader->Bind();
		m_background->Draw();
	}

	void PBR_IBL::prepare() {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		std::string pbr = prepare_scene();
		prepare_light(pbr);
		auto id = prepare_environment_map();
		std::string tex = prepare_irradiance_map(id);
		std::string pre = prepare_prefiltered_map(id);

		m_proxy->Bind();

		auto cubemap = Bind::BindableResolver::GetBindable(id[0]);
		cubemap->Bind();

		auto vert = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "background_vertex_shader", "pbr", "backgroundShader.vert");
		auto frag = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "background_fragment_shader", "pbr", "backgroundShader.frag");
		m_backgroundShader = Bind::ShaderProgram::Resolve("background_shader", { vert,frag });
		m_backgroundShader->BindWithoutUpdate();
		m_backgroundShader->SetTexture("environmentMap", 10);
		m_backgroundShader->UnBind();
	}

	std::string PBR_IBL::prepare_scene() {
		using Mat = SceneGraph::Material::TextureCategory;
		using Vert = DrawItems::VertexType;
		using Conf = SceneGraph::ConfigurationType;
		using Shad = Bind::ShaderObject::ShaderType;

		GeometryParameter param;
		param.m_name = "sphere_1";
		param.m_texture_paths = { "pbr_materials/gold/albedo.png","pbr_materials/gold/ao.png","pbr_materials/gold/metallic.png",
		"pbr_materials/gold/normal.png","pbr_materials/gold/roughness.png" };
		param.m_texture_types = { Mat::DIFFUSE,Mat::AMBIENT_OCCLUSION,Mat::METALLIC,Mat::NORMAL,Mat::ROUGHNESS };
		m_proxy = DrawableLoader::LoadGeometry<GeometryType::Sphere>(m_main_scene, param);

		std::string r_name = m_proxy->GetRootName();

		param.m_name = "sphere_2";
		param.m_translate = glm::vec3(2.0, 0.0, 0.0);
		param.m_texture_paths = { "pbr_materials/grass/albedo.png","pbr_materials/grass/ao.png","pbr_materials/grass/metallic.png",
		"pbr_materials/grass/normal.png","pbr_materials/grass/roughness.png" };
		DrawableLoader::LoadGeometryAsChild<GeometryType::Sphere>(m_main_scene, r_name, param);

		param.m_name = "sphere_3";
		param.m_translate = glm::vec3(4.0, 0.0, 0.0);
		param.m_texture_paths = { "pbr_materials/plastic/albedo.png","pbr_materials/plastic/ao.png","pbr_materials/plastic/metallic.png",
		"pbr_materials/plastic/normal.png","pbr_materials/plastic/roughness.png" };
		DrawableLoader::LoadGeometryAsChild<GeometryType::Sphere>(m_main_scene, r_name, param);

		param.m_name = "sphere_4";
		param.m_translate = glm::vec3(-2.0, 0.0, 0.0);
		param.m_texture_paths = { "pbr_materials/rusted_iron/albedo.png","pbr_materials/rusted_iron/ao.png","pbr_materials/rusted_iron/metallic.png",
		"pbr_materials/rusted_iron/normal.png","pbr_materials/rusted_iron/roughness.png" };
		DrawableLoader::LoadGeometryAsChild<GeometryType::Sphere>(m_main_scene, r_name, param);

		param.m_name = "sphere_5";
		param.m_translate = glm::vec3(-4.0, 0.0, 0.0);
		param.m_texture_paths = { "pbr_materials/wall/albedo.png","pbr_materials/wall/ao.png","pbr_materials/wall/metallic.png",
		"pbr_materials/wall/normal.png","pbr_materials/wall/roughness.png" };
		DrawableLoader::LoadGeometryAsChild<GeometryType::Sphere>(m_main_scene, r_name, param);

		m_proxy->AddRootVertexRule({ Vert::Position,Vert::Normal,Vert::Texcoord });

		m_proxy->AddRootTextureRule("albedoMap", 0, Mat::DIFFUSE);
		m_proxy->AddRootTextureRule("normalMap", 1, Mat::NORMAL);
		m_proxy->AddRootTextureRule("metallicMap", 2, Mat::METALLIC);
		m_proxy->AddRootTextureRule("roughnessMap", 3, Mat::ROUGHNESS);
		m_proxy->AddRootTextureRule("aoMap", 4, Mat::AMBIENT_OCCLUSION);

		GLuint vert = Bind::ShaderObject::Resolve(Shad::Vertex, "pbr_vertex_shader","pbr","pbrShader.vert");
		GLuint frag = Bind::ShaderObject::Resolve(Shad::Fragment, "pbr_fragment_shader", "pbr", "pbrShader.frag");
		auto pbrShader = Bind::ShaderProgram::Resolve("pbr_shader", { vert,frag });
		m_pbrShader = pbrShader;

		pbrShader->BindWithoutUpdate();
		pbrShader->SetTexture("albedoMap", 0);
		pbrShader->SetTexture("normalMap", 1);
		pbrShader->SetTexture("metallicMap", 2);
		pbrShader->SetTexture("roughnessMap", 3);
		pbrShader->SetTexture("aoMap", 4);

		pbrShader->SetTexture("irradianceMap", 5);
		pbrShader->SetTexture("prefilterMap", 6);
		pbrShader->SetTexture("brdfLUT", 7);
		pbrShader->UnBind();

		m_proxy->AddRootUniformRule<Conf::Transformation>(pbrShader->EditUniform("model").GetLeafUniform(),
			[](glm::mat4 model)->glm::mat4 {return model; });
		m_proxy->AddRootUniformRule<Conf::Transformation>(pbrShader->EditUniform("normalMat").GetLeafUniform(),
			[](glm::mat4 model)->glm::mat3 {return glm::transpose(glm::inverse(glm::mat3(model))); });
		m_proxy->AddRootBindable(pbrShader);

		m_proxy->Cook();

		return pbrShader->GetUID();
	}

	std::vector<std::string> PBR_IBL::prepare_environment_map() {
		OGL_TEXTURE_PARAMETER param;
		param.wrap_x = GL_CLAMP_TO_EDGE;
		param.wrap_y = GL_CLAMP_TO_EDGE;
		auto hdr = Bind::ImageTexture2D::Resolve("hdr_image", "hdr/newport_loft.hdr", param, 0);

		unsigned int fbo_width = 512, fbo_height = 512;
		auto render_target = Bind::RenderTarget::Resolve("environment_framebuffer", fbo_width, fbo_height, 1, GL_RGB16F);
		param.wrap_z = GL_CLAMP_TO_EDGE;
		param.min_filter = GL_LINEAR_MIPMAP_LINEAR;
		render_target->AppendTexture<GL_TEXTURE_CUBE_MAP>("environment_texture", param).AppendDepthComponent<GL_RENDERBUFFER>("environment_depth")
			.CheckCompleteness();

		using Shad = Bind::ShaderObject::ShaderType;
		auto equi_vert = Bind::ShaderObject::Resolve(Shad::Vertex, "cubemap_vert_shader", "pbr", "cubemapShader.vert");
		auto equi_frag = Bind::ShaderObject::Resolve(Shad::Fragment, "equi_to_cubemap_fragment_shader", "pbr", "equirectangularToCubemapShader.frag");
		auto equiShader = Bind::ShaderProgram::Resolve("equi_to_cubemap_shader", { equi_vert,equi_frag });

		equiShader->EditUniform("projection") = m_captureProjection;
		equiShader->Bind();
		equiShader->SetTexture("equirectangularMap", 0);
		
		OGL_VIEWPORT_SCISSOR_STATE state;
		state.viewport.width = 512.0;
		state.viewport.height = 512.0;
		auto viewport = Bind::ViewScissor::Resolve(state);

		m_background = std::make_shared<DrawItems::Box>();
		m_background->GenerateVAO({ Dynamic::Dsr::ShaderReflection::GetVertexAttribs(equiShader->get_program()) }, { DrawItems::VertexType::Position });

		viewport->Bind();
		render_target->Bind();
		hdr->Bind();
		for (int i = 0; i < 6; i++) {
			equiShader->EditUniform("view") = m_captureViews[i];
			equiShader->UpdateOnly();
			render_target->change_texture_slice(0, i);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			m_background->Draw();
		}

		viewport->UnBind();
		render_target->UnBind();
		equiShader->UnBind();
		hdr->UnBind();
		
		auto environment_map = render_target->get_texture_image<Bind::CubeMap>(0, param, 10, true);
		return { environment_map->GetUID(),render_target->GetUID(),viewport->GetUID() };
	}

	std::string PBR_IBL::prepare_irradiance_map(std::vector<std::string> ids) {
		auto cubemap = std::static_pointer_cast<Bind::CubeMap>(Bind::BindableResolver::GetBindable(ids[0]));

		auto vert = Bind::ShaderObject::GetShaderObject<Bind::ShaderObject::ShaderType::Vertex>("cubemap_vert_shader");
		auto frag = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "irradiance_convolution_fragment_shader", "pbr",
			"irradianceconvolutionShader.frag");
		auto irradianceShader = Bind::ShaderProgram::Resolve("irradiance_convolution_shader", { vert,frag });

		auto render_target = std::static_pointer_cast<Bind::RenderTarget>(Bind::BindableResolver::GetBindable(ids[1]));
		OGL_TEXTURE_PARAMETER param;
		param.wrap_x = GL_CLAMP_TO_EDGE;
		param.wrap_y = GL_CLAMP_TO_EDGE;
		param.wrap_z = GL_CLAMP_TO_EDGE;
		render_target->ClearTextures(32, 32, GL_RGB16F);
		render_target->AppendTexture<GL_TEXTURE_CUBE_MAP>("irradiance_map", param);

		OGL_VIEWPORT_SCISSOR_STATE state;
		state.viewport.width = 32;
		state.viewport.height = 32;
		auto viewport = Bind::ViewScissor::Resolve(state);

		irradianceShader->EditUniform("projection") = m_captureProjection;
		irradianceShader->Bind();
		irradianceShader->SetTexture("environmentMap", 0);

		viewport->Bind();
		render_target->Bind();
		cubemap->Bind();
		for (int i = 0; i < 6; i++) {
			irradianceShader->EditUniform("view") = m_captureViews[i];
			irradianceShader->UpdateOnly();
			render_target->change_texture_slice(0, i);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			m_background->Draw();
		}

		viewport->UnBind();
		render_target->UnBind();
		cubemap->UnBind();
		irradianceShader->UnBind();

		auto irradiance_texture = render_target->get_texture_image<Bind::CubeMap>(0, param, 5);
		m_proxy->AddRootBindable(irradiance_texture);
		return irradiance_texture->GetUID();
	}

	std::string PBR_IBL::prepare_prefiltered_map(std::vector<std::string> ids) {
		auto cubemap = std::static_pointer_cast<Bind::CubeMap>(Bind::BindableResolver::GetBindable(ids[0]));

		auto vert = Bind::ShaderObject::GetShaderObject<Bind::ShaderObject::ShaderType::Vertex>("cubemap_vert_shader");
		auto frag = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "prefiltered_map_fragment_shader", "pbr", "prefilterShader.frag");
		auto prefilterShader = Bind::ShaderProgram::Resolve("prefilter_shader", { vert,frag });

		auto render_target = std::static_pointer_cast<Bind::RenderTarget>(Bind::BindableResolver::GetBindable(ids[1]));
		OGL_TEXTURE_PARAMETER param;
		param.wrap_x = GL_CLAMP_TO_EDGE;
		param.wrap_y = GL_CLAMP_TO_EDGE;
		param.wrap_z = GL_CLAMP_TO_EDGE;
		param.min_filter = GL_LINEAR_MIPMAP_LINEAR;
		render_target->ClearTextures(128.0, 128.0, GL_RGB16F);
		render_target->AppendTexture<GL_TEXTURE_CUBE_MAP>("prefiltered_map", param, m_prefilter_miplevels);

		prefilterShader->EditUniform("projection") = m_captureProjection;
		prefilterShader->Bind();
		prefilterShader->SetTexture("environmentMap", 0);

		render_target->Bind();
		cubemap->Bind();
		for (unsigned int mip = 0; mip < m_prefilter_miplevels; mip++) {
			unsigned int mip_width = static_cast<unsigned int>(128.0 * pow(0.5, mip));
			unsigned int mip_height = static_cast<unsigned int>(128.0 * pow(0.5, mip));

			render_target->change_depthstencil_storage(mip_width, mip_height);
			glViewport(0, 0, mip_width, mip_height);

			float roughness = (float)mip / (float)(m_prefilter_miplevels - 1);
			prefilterShader->EditUniform("roughness") = roughness;
			for (int i = 0; i < 6; i++) {
				prefilterShader->EditUniform("view") = m_captureViews[i];
				prefilterShader->UpdateOnly();
				render_target->change_texture_slice(0, i, mip);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				m_background->Draw();
			}
		}

		glViewport(0, 0, globalSettings::screen_width, globalSettings::screen_height);
		cubemap->UnBind();
		prefilterShader->UnBind();
		render_target->UnBind();

		auto prefilter_map = render_target->get_texture_image<Bind::CubeMap>(0, param, 6);
		m_proxy->AddRootBindable(prefilter_map);
		return prefilter_map->GetUID();
	}

	std::string PBR_IBL::prepare_brdf_LUT(std::vector<std::string> ids) {
		auto vert = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "brdf_LUT_vertex_shader", "pbr", "brdfShader.vert");
		auto frag = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "brdf_LUT_fragment_shader", "pbr", "brdfShader.frag");
		auto brdfShader = Bind::ShaderProgram::Resolve("brdfShader", { vert,frag });

		auto render_target = std::static_pointer_cast<Bind::RenderTarget>(Bind::BindableResolver::GetBindable(ids[1]));
		render_target->ClearTextures(512.0, 512.0, GL_RG16F);
		OGL_TEXTURE_PARAMETER param;
		param.wrap_x = GL_CLAMP_TO_EDGE;
		param.wrap_y = GL_CLAMP_TO_EDGE;
		render_target->AppendTexture<GL_TEXTURE_2D>("brdf_LUT", param);

		auto quad = std::make_shared<DrawItems::Plane>();
		quad->GenerateVAO(Dynamic::Dsr::ShaderReflection::GetVertexAttribs(brdfShader->get_program()), { DrawItems::VertexType::Position,
			DrawItems::VertexType::Texcoord });

		auto viewport = Bind::BindableResolver::GetBindable(ids[2]);

		viewport->Bind();
		brdfShader->Bind();
		render_target->Bind();
		quad->Draw();
		viewport->UnBind();
		brdfShader->UnBind();
		render_target->UnBind();

		auto brdf_texture = render_target->get_texture_image<Bind::ImageTexture2D>(0, param, 7);
		m_proxy->AddRootBindable(brdf_texture);
		return brdf_texture->GetUID();
	}

	void PBR_IBL::prepare_light(const std::string& pbr_name) {
		m_dir.direction = glm::normalize(glm::vec3(1.0f, -1.0f, -1.0f));
		m_dir.color = 10.0f * glm::vec3(0.45f, 0.21f, 0.66f);

		m_spot.direction = glm::normalize(glm::vec3(-1.0f, -0.5f, 0.7f));
		m_spot.color = 10.0f * glm::vec3(0.14f, 0.85f, 0.47f);
		m_spot.position = 3.0f * glm::vec3(4.0f, 3.0f, -2.0f);
		m_spot.inner_angle = 0.8f;
		m_spot.outer_angle = 0.5f;

		for (int i = 0; i < 3; i++) {
			m_points[i].position = 10.0f * glm::vec3(2.0f * float((i - 1)));
			m_points[i].color = 5.0f * glm::vec3(float(i) / 3.0f, float(3.0f - i) / 3.0f, 0.5f);
		}

		using namespace std::string_literals;

		m_pbrShader->EditUniform("dir")["direction"s] = m_dir.direction;
		m_pbrShader->EditUniform("dir")["color"s] = m_dir.color;
		m_pbrShader->EditUniform("spot")["position"s] = m_spot.position;
		m_pbrShader->EditUniform("spot")["color"s] = m_spot.color;
		m_pbrShader->EditUniform("spot")["direction"s] = m_spot.direction;
		m_pbrShader->EditUniform("spot")["inner_angle"s] = m_spot.inner_angle;
		m_pbrShader->EditUniform("spot")["outer_angle"s] = m_spot.outer_angle;
		for (int i = 0; i < 3; i++) {
			m_pbrShader->EditUniform("points")[i]["position"s] = m_points[i].position;
			m_pbrShader->EditUniform("points")[i]["color"s] = m_points[i].color;
		}

		m_pbrShader->Bind();
		m_pbrShader->UnBind();
	}

	void PBR_IBL::prepare_ui(const std::string& name) {

	}
}