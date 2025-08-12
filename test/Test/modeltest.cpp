#include "modeltest.h"

#include <assimploader.h>
#include <drawableloader.h>
#include <Bindables/shaderprogram.h>

ModelTest::ModelTest(const std::string& scene_name)
	:Utils(scene_name)
{
	globalSettings::mainCamera.set_perspective(60.0f, globalSettings::screen_width, globalSettings::screen_height, 0.1f, 512.0f);
	globalSettings::mainCamera.set_position(glm::vec3(0.0f, 0.0f, 4.0f));
}

void ModelTest::prepare() {
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_CULL_FACE);

	//m_proxy = AssimpLoader::LoadModel("backpack", "backpack.obj", m_main_scene);
	m_proxy = AssimpLoader::LoadModel("Sponza", "sponza.obj", m_main_scene);
	GLuint vertex = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Vertex, "vertex", "test", "model_test.vert");
	GLuint fragment = Bind::ShaderObject::Resolve(Bind::ShaderObject::ShaderType::Fragment, "fragment", "test", "model_test.frag");
	m_shader = Bind::ShaderProgram::Resolve("shader", std::vector<GLuint>{vertex, fragment});
	m_proxy->AddRootBindable(m_shader);
	std::vector<DrawItems::VertexType> instruction{ DrawItems::VertexType::Position,DrawItems::VertexType::Normal,DrawItems::VertexType::Texcoord };
	m_proxy->AddRootVertexRule(instruction);
	m_proxy->AddRootTextureRule("texture_diffuse1", 0, SceneGraph::Material::TextureCategory::DIFFUSE);
	m_proxy->AddRootTextureRule("texture_specular1", 1, SceneGraph::Material::TextureCategory::SPECULAR);
	m_proxy->AddRootUniformRule<SceneGraph::ConfigurationType::Transformation>(m_shader->EditUniform("model").GetLeafUniform(),
		[](glm::mat4 model)->glm::mat4{return model; });
	m_proxy->Cook();
	m_proxy->ScaleModel(glm::vec3(0.02f));

	GeometryParameter param;
	param.m_name = "test_cube";
	param.m_translate = glm::vec3(2.0f, 2.0f, 0.0f);
	param.m_texture_types = std::vector<SceneGraph::Material::TextureCategory>{ SceneGraph::Material::TextureCategory::DIFFUSE,
	SceneGraph::Material::TextureCategory::SPECULAR };
	param.m_texture_paths = std::vector<std::string>{ "girl.jpg","default_textures/specular.jpg" };
	m_drawables.push_back(DrawableLoader::LoadGeometry<GeometryType::Box>(m_main_scene, param));
	m_drawables[0]->AddRootBindable(m_shader);
	m_drawables[0]->AddRootVertexRule(instruction);
	m_drawables[0]->AddRootTextureRule("texture_diffuse1", 0, SceneGraph::Material::TextureCategory::DIFFUSE);
	m_drawables[0]->AddRootTextureRule("texture_specular1", 1, SceneGraph::Material::TextureCategory::SPECULAR);
	m_drawables[0]->AddRootUniformRule<SceneGraph::ConfigurationType::Transformation>(m_shader->EditUniform("model").GetLeafUniform(),
		[](glm::mat4 model)->glm::mat4 {return model; });
	
	param.m_name = "test_sphere";
	param.m_translate = glm::vec3(1.0f, 1.0f, 1.0f);
	DrawableLoader::LoadGeometryAsChild<GeometryType::Sphere>(m_main_scene, "Box_test_cube_root", param);
	m_drawables[0]->Cook();
}

void ModelTest::prepare_ui(const std::string& name) {

}

void ModelTest::render() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_shader->EditUniform("view") = globalSettings::mainCamera.get_view();
	m_shader->EditUniform("projection") = globalSettings::mainCamera.get_perspective();
	m_shader->EditUniform("camera_pos") = globalSettings::mainCamera.get_position();

	m_proxy->Render();
	m_drawables[0]->Render();
}
