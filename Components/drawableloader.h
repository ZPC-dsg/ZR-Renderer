#pragma once

#include <SceneGraph/material.h>
#include <SceneGraph/drawableproxy.h>
#include <SceneGraph/scene.h>
#include <Drawables/includer.h>

#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <vector>

enum class GeometryType {
	Box,
	Plane,
	Sphere,
};

struct GeometryParameter {
	std::string m_name;
	glm::vec3 m_scale = glm::vec3(1.0);
	glm::vec3 m_rotate = glm::vec3(0.0);//ÑØx,y,zÖáµÄÐý×ª½Ç¶È
	glm::vec3 m_translate = glm::vec3(0.0);
	std::vector<SceneGraph::Material::TextureCategory> m_texture_types;
	std::vector<std::string> m_texture_paths;
};

class DrawableLoader {
public:
	template <GeometryType type, typename... Args>
	static std::shared_ptr<SceneGraph::DrawableProxy> LoadGeometry(SceneGraph::Scene& scene, const GeometryParameter& param, Args&&... args);
	template <GeometryType type, typename... Args>
	static void LoadGeometryAsChild(SceneGraph::Scene& scene, const std::string& name, const GeometryParameter& param, Args&&... args);
};

#define GEOMETRY_GENERATOR \
	X(Box) \
	X(Plane) \
	X(Sphere)

template <GeometryType type, typename... Args>
std::shared_ptr<SceneGraph::DrawableProxy> DrawableLoader::LoadGeometry(SceneGraph::Scene& scene, const GeometryParameter& param, Args&&... args) {
	assert(param.m_texture_types.size() == param.m_texture_paths.size());

	std::string control_name;
	switch (type) {
#define X(Type) case GeometryType::Type: control_name = #Type + std::string("_") + param.m_name + "_root"; break;
		GEOMETRY_GENERATOR
#undef X
	default:
		assert("Unknown geometry type!" && false);
	}

	std::unique_ptr<SceneGraph::Node> root(new SceneGraph::ControlNode(0, control_name));
	scene.AddChild(*root);
	root->SetParent(scene.GetRootNode());

	std::unique_ptr<SceneGraph::EntityNode> geo(new SceneGraph::EntityNode(1, param.m_name));
	geo->SetScaling(param.m_scale);
	geo->SetRotation(param.m_rotate);
	geo->SetTranslation(param.m_translate);

	geo->SetParent(*root);
	root->AddChild(*geo);

	std::unique_ptr<SceneGraph::Material> mat;
	switch (type) {
#define X(Type) \
	case GeometryType::Type: { \
		std::unique_ptr<DrawItems::Drawable> item(new DrawItems::Type(param.m_name, std::forward<Args>(args)...)); \
		geo->AddDrawable(*item); \
		scene.AddDrawable(typeid(DrawItems::Type), std::move(item)); \
		mat = std::unique_ptr<SceneGraph::Material>(new SceneGraph::Material(#Type + std::string("_") + param.m_name + "_material")); \
		break; \
	}
		GEOMETRY_GENERATOR
#undef X
	default:
		assert("Unknown geometry type!" && false);
	}
	for (int i = 0; i < param.m_texture_types.size(); i++) {
		SceneGraph::TextureInfo info;
		info.m_path = param.m_texture_paths[i];
		mat->AddTexture(param.m_texture_types[i], info);
	}
	geo->AddMaterial(*mat);
	scene.AddMaterial(std::move(mat));

	std::shared_ptr<SceneGraph::DrawableProxy> proxy(new SceneGraph::DrawableProxy(&scene, static_cast<SceneGraph::ControlNode&>(*root), param.m_name));

	scene.AddNode(std::move(geo));
	scene.AddNode(std::move(root));

	return proxy;
}

template <GeometryType type, typename... Args>
static void DrawableLoader::LoadGeometryAsChild(SceneGraph::Scene& scene, const std::string& name, const GeometryParameter& param, Args&&... args) {
	assert(param.m_texture_types.size() == param.m_texture_paths.size());

	SceneGraph::Node* node = scene.FindNode(name);
	if (!node) {
		assert("Name does not correspond to an existing node!" && false);
		return;
	}

	std::unique_ptr<SceneGraph::EntityNode> geo(new SceneGraph::EntityNode(1, param.m_name));
	geo->SetScaling(param.m_scale);
	geo->SetRotation(param.m_rotate);
	geo->SetTranslation(param.m_translate);

	geo->SetParent(*node);
	node->AddChild(*geo);

	std::unique_ptr<SceneGraph::Material> mat;
	switch (type) {
#define X(Type) \
	case GeometryType::Type: { \
		std::unique_ptr<DrawItems::Drawable> item(new DrawItems::Type(param.m_name, std::forward<Args>(args)...)); \
		geo->AddDrawable(*item); \
		scene.AddDrawable(typeid(DrawItems::Type), std::move(item)); \
		mat = std::unique_ptr<SceneGraph::Material>(new SceneGraph::Material(#Type + std::string("_") + param.m_name + "_material")); \
		break;\
	}
		GEOMETRY_GENERATOR
#undef X
	default:
		assert("Unknown geometry type!" && false);
	}
	for (int i = 0; i < param.m_texture_types.size(); i++) {
		SceneGraph::TextureInfo info;
		info.m_path = param.m_texture_paths[i];
		mat->AddTexture(param.m_texture_types[i], info);
	}
	geo->AddMaterial(*mat);
	scene.AddMaterial(std::move(mat));

	scene.AddNode(std::move(geo));
	return;
}