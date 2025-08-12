#define DRAWABLE_SOURCE_FILE

#include <Drawables/drawable.h>
#include <logging.h>

#include <memory>
#include <unordered_set>
#include <algorithm>

namespace DrawItems {
	Drawable::Drawable(const std::string& name):m_name(name){}

	std::vector<std::function<void(std::shared_ptr<Bind::InputLayout>)>> Drawable::m_renderfunctions = {
		DrawDefault,DrawIndexed,DrawInstanced,DrawIndexedInstanced
	};

	void Drawable::Draw() {
		m_VAO->Bind();
		m_renderfunctions[m_render_index](m_VAO);
	}

	//attribs已经按照location序排列好，并且要求attribs的location从0开始连续递增，instruction中元素和attribs一一对应
	void Drawable::GenerateVAO(const std::vector<Dynamic::Dsr::VertexAttrib>& attribs, std::vector<VertexType> instruction) {
		if (!CheckAttribExists(instruction)) {
			std::string error_code = "Vertex Attributes do not match for drawable: " + m_name;
			assert(error_code.c_str() && false);
			return;
		}

		LOGI("Generating VAO for drawable {}...", m_name.c_str())

		Dynamic::Dvtx::CPUVertexBuffer vert(attribs, m_positions.size());
		int offset = 0;
		int tex_offset = 0, color_offset = 0;
		for (auto type : instruction) {
			switch (type) {
#define X(Enum, Member, Type) \
			case VertexType::Enum: \
				vert.InitializeContinuous(Member, offset); \
				offset += Member.size() * sizeof(Type); \
				break;

			VERTEX_GENERATOR
#undef X
			case VertexType::Texcoord:
				vert.InitializeContinuous(m_texcoords[tex_offset++], offset);
				offset += m_texcoords[0].size() * sizeof(glm::vec2);
				break;
			case VertexType::VertexColor:
				vert.InitializeContinuous(m_vertexcolors[color_offset++], offset);
				offset += m_vertexcolors[0].size() * sizeof(glm::vec4);
				break;
			default:
				assert("Unknown type!" && false);
			}
		}

		std::shared_ptr<Bind::VertexBuffer> vert_buffer = std::make_shared<Bind::VertexBuffer>(m_name + "_vertex", vert);
		std::shared_ptr<Bind::IndexBuffer> ind_buffer = nullptr;
		if(m_indices.size())
			ind_buffer = std::make_shared<Bind::IndexBuffer>(m_name + "_index", m_indices);
		m_VAO = Bind::InputLayout::Resolve(m_name + "_VAO", std::vector<std::shared_ptr<Bind::VertexBuffer>>{vert_buffer},
			std::vector<std::shared_ptr<Bind::VertexBuffer>>{}, ind_buffer);

		m_render_index = m_VAO->HasIndexBuffer() + ((m_VAO->IsInstanceDraw()) << 1);
		LOGI("Generating done!")
	}

	std::string vertex_type_to_string(VertexType type) {
		switch (type) {
#define X(Enum, Member, Type) case VertexType::Enum: return #Enum;
			VERTEX_GENERATOR
#undef X
		case VertexType::Texcoord:
			return "Texcoord";
		case VertexType::VertexColor:
			return "VertexColor";
		default:
			return "???";
		}
	}

	bool Drawable::CheckAttribExists(const std::vector<VertexType>& attribs) {
		std::unordered_set<VertexType> st(attribs.begin(), attribs.end());
		bool res = true;
		for(auto s:st){
			switch(s){
#define X(Enum, Member, Type) \
			case VertexType::Enum: \
				if(Member.size() == 0) { \
					LOG_WARNING("Vertex Attribute: {} can not be specified because it does not exist for this drawable!", vertex_type_to_string(s)) \
					res = false; \
				}
				break;

				VERTEX_GENERATOR
#undef X
			case VertexType::Texcoord: {
				int num = std::count(attribs.begin(), attribs.end(), VertexType::Texcoord);
				if (num > m_texcoords.size()) {
					LOG_WARNING("Texcoord specified out of range!")
					res = false;
				}
				break;
			}
			case VertexType::VertexColor: {
				int num = std::count(attribs.begin(), attribs.end(), VertexType::VertexColor);
				if (num > m_vertexcolors.size()) {
					LOG_WARNING("Vertex color specified out of range!")
					res = false;
				}
				break;
			}
			default:
				break;
			}
		}
		return res;
	}
}