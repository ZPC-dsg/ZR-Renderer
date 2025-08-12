#include <SceneGraph/material.h>

namespace SceneGraph {
	//TextureInfo
	std::unordered_map<aiTextureFlags, TextureInfo::MatExtraTextureFlag> TextureInfo::m_flagmap{
		{aiTextureFlags_Invert, TextureInfo::MatExtraTextureFlag::Invert},{aiTextureFlags_UseAlpha, TextureInfo::MatExtraTextureFlag::UseAlpha},
		{aiTextureFlags_IgnoreAlpha, TextureInfo::MatExtraTextureFlag::IgnoreAlpha}
	};
	
	std::unordered_map<aiTextureMapMode, TextureInfo::MatTextureMapMode> TextureInfo::m_mapmodemap{
		{aiTextureMapMode_Wrap, TextureInfo::MatTextureMapMode::Wrap},{aiTextureMapMode_Clamp, TextureInfo::MatTextureMapMode::Clamp},
		{aiTextureMapMode_Decal, TextureInfo::MatTextureMapMode::Decal},{aiTextureMapMode_Mirror, TextureInfo::MatTextureMapMode::Mirror}
	};

	std::unordered_map<aiTextureMapping, TextureInfo::MatTextureMapping> TextureInfo::m_mappingmap{
		{aiTextureMapping_BOX, TextureInfo::MatTextureMapping::Box},{aiTextureMapping_CYLINDER, TextureInfo::MatTextureMapping::Cylinder},
		{aiTextureMapping_OTHER, TextureInfo::MatTextureMapping::Other},{aiTextureMapping::aiTextureMapping_PLANE, TextureInfo::MatTextureMapping::Plane},
		{aiTextureMapping_SPHERE, TextureInfo::MatTextureMapping::Sphere}
	};

	std::unordered_map<aiTextureOp, TextureInfo::MatTextureOp> TextureInfo::m_opmap{
		{aiTextureOp_Add, TextureInfo::MatTextureOp::Add},{aiTextureOp_Divide, TextureInfo::MatTextureOp::Divide},
		{aiTextureOp_Multiply, TextureInfo::MatTextureOp::Multiply},{aiTextureOp_SignedAdd, TextureInfo::MatTextureOp::SignedAdd},
		{aiTextureOp_SmoothAdd, TextureInfo::MatTextureOp::SmoothAdd},{aiTextureOp_Subtract, TextureInfo::MatTextureOp::Subtract}
	};

	GLenum TextureInfo::GLWrapMode(MatTextureMapMode mode) {
		switch (mode) {
		case MatTextureMapMode::Wrap:return GL_REPEAT;
		case MatTextureMapMode::Clamp:return GL_CLAMP_TO_EDGE;
		case MatTextureMapMode::Mirror:return GL_MIRRORED_REPEAT;
		case MatTextureMapMode::Decal:return GL_CLAMP_TO_EDGE;
		default:return GL_REPEAT;
		}
	}

	//ExtraProperties
	std::unordered_map<aiBlendMode, ExtraProperties::MatBlendMode> ExtraProperties::m_blendmodemap{
		{aiBlendMode_Default, ExtraProperties::MatBlendMode::Default},{aiBlendMode_Additive, ExtraProperties::MatBlendMode::Additive}
	};

	std::unordered_map<aiShadingMode, ExtraProperties::MatShadingMode> ExtraProperties::m_shadingmap{
		{aiShadingMode_Blinn, ExtraProperties::MatShadingMode::Blinn},{aiShadingMode_CookTorrance, ExtraProperties::MatShadingMode::CookTorrance},
		{aiShadingMode_Flat, ExtraProperties::MatShadingMode::Flat},{aiShadingMode_Fresnel, ExtraProperties::MatShadingMode::Fresnel},
		{aiShadingMode_Gouraud, ExtraProperties::MatShadingMode::Gouraud},{aiShadingMode_Minnaert, ExtraProperties::MatShadingMode::Minnaert},
		{aiShadingMode_NoShading, ExtraProperties::MatShadingMode::NoShading},{aiShadingMode_OrenNayar, ExtraProperties::MatShadingMode::OrenNayar},
		{aiShadingMode_PBR_BRDF, ExtraProperties::MatShadingMode::PBR},{aiShadingMode_Phong, ExtraProperties::MatShadingMode::Phong},
		{aiShadingMode_Toon, ExtraProperties::MatShadingMode::Toon},{aiShadingMode_Unlit, ExtraProperties::MatShadingMode::Unlit}
	};

	//Material
	Material::Material(const std::string& name)
		:m_name(name)
	{
	}

	void Material::AddTexture(TextureCategory type, TextureInfo info) {
		m_textures[type].push_back(info);
	}

	const std::vector<TextureInfo> Material::GetTextures(TextureCategory type) const {
		if (HasTextureType(type))
			return m_textures.at(type);
		else
			return {};
	}

	const TextureInfo& Material::GetTexture(TextureCategory type, size_t index) const {
		assert("Material do not have textures of this type!" && HasTextureType(type));
		assert("Index out of range!" && index < GetTextures(type).size());

		return m_textures.at(type)[index];
	}

	std::string Material::TexTypeToString(TextureCategory type) {
		switch (type) {
		case TextureCategory::DIFFUSE:return "Diffuse";
		case TextureCategory::SPECULAR:return "Specular";
		case TextureCategory::AMBIENT:return "Ambient";
		case TextureCategory::EMISSIVE:return "Emissive";
		case TextureCategory::HEIGHT:return "Height";
		case TextureCategory::NORMAL:return "Normal";
		case TextureCategory::SHININESS:return "Shininess";
		case TextureCategory::OPACITY:return "Opacity";
		case TextureCategory::DISPLACEMENT:return "Displacement";
		case TextureCategory::LIGHTMAP:return "LightMap";
		case TextureCategory::REFLECTION:return "Reflection";
		case TextureCategory::AMBIENT_OCCLUSION:return "Ambient_occlusion";
		case TextureCategory::ROUGHNESS:return "Roughness";
		case TextureCategory::METALLIC:return "Metallic";
		default:return "???";
		}
	}

	//TODO:在textures文件夹下添加这些fallback纹理
	std::string Material::DefaultTexture(TextureCategory type) noexcept {
		switch (type) {
		case TextureCategory::DIFFUSE:return "default_textures/diffuse.jpg";
		case TextureCategory::SPECULAR:return "default_textures/specular.jpg";
		case TextureCategory::AMBIENT:return "default_textures/ambient.jpg";
		case TextureCategory::EMISSIVE:return "default_textures/emissive.jpg";
		case TextureCategory::HEIGHT:return "default_textures/height.jpg";
		case TextureCategory::NORMAL:return "default_textures/normal.jpg";
		case TextureCategory::SHININESS:return "default_textures/shininess.jpg";
		case TextureCategory::OPACITY:return "default_textures/opacity.jpg";
		case TextureCategory::DISPLACEMENT:return "default_textures/displacement.jpg";
		case TextureCategory::LIGHTMAP:return "default_textures/lightmap.jpg";
		case TextureCategory::REFLECTION:return "default_textures/reflection.jpg";
		case TextureCategory::AMBIENT_OCCLUSION:return "default_textures/ambient_occlusion.jpg";
		case TextureCategory::ROUGHNESS:return "default_textures/roughness.jpg";
		case TextureCategory::METALLIC:return "default_textures/metallic.jpg";
		default:return "???";
		}
	}
}