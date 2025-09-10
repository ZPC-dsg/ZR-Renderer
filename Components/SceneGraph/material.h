#pragma once

#include <assimp/scene.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace SceneGraph {
	struct TextureInfo {
		enum class MatExtraTextureFlag {
			None,
			Invert,//������ɫΪ1-������ɫ
			UseAlpha,
			IgnoreAlpha//��UseAlpha����
		};

		enum class MatTextureMapMode {
			Wrap,
			Clamp,
			Decal,//If the texture coordinates for a pixel are outside [0...1] the texture is not applied to that pixel.
			Mirror
		};

		enum class MatTextureMapping {//���������û���������꣬assimp�����Զ������������꣬��ö��ָ���������ɵ�ӳ�䷽ʽ
			None,//������������
			Sphere,
			Cylinder,
			Box,
			Plane,
			Other
		};

		enum class MatTextureOp {//������ǰ������ô��֮ǰ�����ɵ���ɫ��ϣ����Ӳο�https://documentation.help/assimp/material_8h.html#a204a0a39264125e160bd8fb2117f06e9
			Multiply,
			Add,
			Subtract,
			Divide,
			SmoothAdd,
			SignedAdd
		};

		std::string m_path;

		float m_strength = 1.0;//������ɫǿ�ȣ��ڽ����κ���һ���Ĵ���֮ǰ��Ҫ����ȡ��������ɫ���ȳ������ǿ��
		int m_uv_channel = -1;//-1����û���Լ���uv channel��Ҫô����ģ��ֻ��һ��uv����ϵͳ��Ҫô������û��ָ��ʹ���ĸ�����ϵͳ����ʱֱ�Ӱ����������������

		float m_uvtransform_rotate = 0.0;
		glm::vec2 m_uvtransform_scale = glm::vec2(1.0);
		glm::vec2 m_uvtransform_translate = glm::vec2(0.0);

		MatExtraTextureFlag m_textureflag = MatExtraTextureFlag::None;
		std::array<MatTextureMapMode, 2> m_wrapmode = { MatTextureMapMode::Clamp,MatTextureMapMode::Clamp };//xy���Ե�map mode
		MatTextureOp m_textureop = MatTextureOp::Add;
		MatTextureMapping m_mapping = MatTextureMapping::None;

		static std::unordered_map<aiTextureFlags, MatExtraTextureFlag> m_flagmap;
		static std::unordered_map<aiTextureMapMode, MatTextureMapMode> m_mapmodemap;
		static std::unordered_map<aiTextureMapping, MatTextureMapping> m_mappingmap;
		static std::unordered_map<aiTextureOp, MatTextureOp> m_opmap;

		static GLenum GLWrapMode(MatTextureMapMode mode);
	};

	struct ExtraProperties {
		enum class MatBlendMode {
			None,
			Default,//srccolor * alpha + dstcolor * (1 - alpha)
			Additive//srccolor * 1 + dstcolor * 1
		};

		enum class MatShadingMode {//���庬��ο�https://documentation.help/assimp/material_8h.html#a204a0a39264125e160bd8fb2117f06e9
			Flat,
			Gouraud,
			Phong,
			Blinn,
			Toon,
			OrenNayar,
			Minnaert,
			CookTorrance,
			NoShading,
			Fresnel,
			PBR,
			Unlit
		};

		MatBlendMode m_blendmode = MatBlendMode::None;
		MatShadingMode m_shadingmode = MatShadingMode::Blinn;

		glm::vec3 m_diffuse = glm::vec3(0.0);
		glm::vec3 m_specular = glm::vec3(0.0);
		glm::vec3 m_ambient = glm::vec3(0.0);
		glm::vec3 m_emissive = glm::vec3(0.0);
		glm::vec3 m_transparent = glm::vec3(0.0);//����͸��ɫ������������а�͸���⣬��Ҫ�������ɫ���Թ����ɫ

		bool m_is_twosided = true;//��Ⱦ����ʱ�Ƿ���Ҫ���ñ����޳�

		float m_opacity = 1.0;
		float m_shininess = 64.0;//phong����ģ���о��淴������ݴ�ϵ��
		float m_shininess_strength = 0.08;//�������淴�����ɫǿ��

		static std::unordered_map<aiBlendMode, MatBlendMode> m_blendmodemap;
		static std::unordered_map<aiShadingMode, MatShadingMode> m_shadingmap;
	};

	class Material {
	public:
		enum TextureCategory {
			DIFFUSE,
			SPECULAR,
			AMBIENT,
			EMISSIVE,
			HEIGHT,
			NORMAL,
			SHININESS,
			OPACITY,
			DISPLACEMENT,
			LIGHTMAP,
			REFLECTION,
			AMBIENT_OCCLUSION,
			ROUGHNESS,
			METALLIC,
		};

		Material(const std::string& name);
		~Material() = default;

		void AddTexture(TextureCategory type, TextureInfo info);

		inline Material& SetBlendMode(ExtraProperties::MatBlendMode mode) { m_extras.m_blendmode = mode; return *this; };
		inline Material& SetShadingMode(ExtraProperties::MatShadingMode mode) { m_extras.m_shadingmode = mode; return *this; };
		inline Material& SetDiffuse(glm::vec3 diff) { m_extras.m_diffuse = diff; return *this; };
		inline Material& SetDiffuse(float x, float y, float z) { return SetDiffuse(glm::vec3(x, y, z)); };
		inline Material& SetSpecular(glm::vec3 spec) { m_extras.m_specular = spec; return *this; };
		inline Material& SetSpecular(float x, float y, float z) { return SetSpecular(glm::vec3(x, y, z)); };
		inline Material& SetAmbient(glm::vec3 amb) { m_extras.m_ambient = amb; return *this; };
		inline Material& SetAmbient(float x, float y, float z) { return SetAmbient(glm::vec3(x, y, z)); };
		inline Material& SetEmissive(glm::vec3 emi) { m_extras.m_emissive = emi; return *this; };
		inline Material& SetEmissive(float x, float y, float z) { return SetEmissive(glm::vec3(x, y, z)); };
		inline Material& SetTransparent(glm::vec3 trans) { m_extras.m_transparent = trans; return *this; };
		inline Material& SetTransparent(float x, float y, float z) { return SetTransparent(glm::vec3(x, y, z)); };
		inline Material& SetTwoSided(bool two) { m_extras.m_is_twosided = two; return *this; };
		inline Material& SetOpacity(float opa) { m_extras.m_opacity = opa; return *this; };
		inline Material& SetShininess(float shin) { m_extras.m_shininess = shin; return *this; };
		inline Material& SetShiniStength(float stren) { m_extras.m_shininess_strength = stren; return *this; };

		inline bool HasTextureType(TextureCategory type) const noexcept { return m_textures.contains(type); };

		const std::vector<TextureInfo> GetTextures(TextureCategory type) const;
		const TextureInfo& GetTexture(TextureCategory type, size_t index) const;

		inline const std::string& GetName() const noexcept { return m_name; };
		inline const ExtraProperties& GetExtraInformation() const noexcept { return m_extras; };

	public:
		static std::string DefaultTexture(TextureCategory type) noexcept;
		static std::string TexTypeToString(TextureCategory type);

	private:
		std::string m_name;

		std::unordered_map<TextureCategory, std::vector<TextureInfo>> m_textures;
		ExtraProperties m_extras;
	};

	template <typename Enum>
	typename std::enable_if<std::is_enum<Enum>::value, Enum>::type operator|(Enum a, Enum b) {
		using underlying = typename std::underlying_type<Enum>::type;
		return static_cast<Enum>(static_cast<underlying>(a) | static_cast<underlying>(b));
	}

	template <typename Enum>
	typename std::enable_if<std::is_enum<Enum>::value, Enum>::type& operator|=(Enum& a, Enum b) {
		using underlying = typename std::underlying_type<Enum>::type;
		a = static_cast<Enum>(static_cast<underlying>(a) | static_cast<underlying>(b));
		return a;
	}
}