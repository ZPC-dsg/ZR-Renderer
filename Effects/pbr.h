#pragma once

#include <utils.h>
#include <light.h>

#include <array>
#include <memory>

namespace SceneGraph {
	class DrawableProxy;
}

namespace DrawItems {
	class Box;
}

namespace Bind {
	class ShaderProgram;
}

namespace RTREffects {
	class PBR_IBL :public Utils {
	public:
		PBR_IBL(const std::string& name);
		~PBR_IBL() = default;

		void render() override;
		void prepare() override;
		void prepare_ui(const std::string& name) override;

	private:
		std::string prepare_scene();
		std::vector<std::string> prepare_environment_map();//���Ⱦ���״ͼת��Ϊ�����廷����ͼ������ֵ��һ��Ϊ������ͼ��texture���֣�
		//�ڶ���Ϊ������ͼ֡��������֣�������Ϊ�����ӿ�(512x512)������
		std::string prepare_irradiance_map(std::vector<std::string> ids);//Ԥ����IBL diffuse��Ļ�����ͼ
		std::string prepare_prefiltered_map(std::vector<std::string> ids);//Ԥ����IBL specular��Ļ�����ͼ
		std::string prepare_brdf_LUT(std::vector<std::string> ids);//Ԥ����IBL brdf���ұ�
		void prepare_light(const std::string& pbr_name);

	private:
		std::array<PointLight, 3> m_points;
		DirectionLight m_dir;
		SpotLight m_spot;

		std::shared_ptr<SceneGraph::DrawableProxy> m_proxy;

		std::shared_ptr<DrawItems::Box> m_background;

		std::shared_ptr<Bind::ShaderProgram> m_pbrShader;
		std::shared_ptr<Bind::ShaderProgram> m_backgroundShader;

	private:
		const glm::mat4 m_captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

		const glm::mat4 m_captureViews[6] =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		static constexpr unsigned int m_prefilter_miplevels = 5;
	};
}