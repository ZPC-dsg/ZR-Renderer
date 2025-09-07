#define ENABLE_SHADER_TYPE_GENERATOR

#include <Dynamic/dynamic_vertex.h>

namespace Dynamic {
	namespace Dvtx {
		//VertexLayout
		const VertexLayout::Element& VertexLayout::ResolveByIndex(size_t i) const noxnd
		{
			return m_elements[i];
		}

		VertexLayout& VertexLayout::Append(Dynamic::Dsr::VertexAttrib attrib) noxnd
		{
			if (!Has(attrib.type, attrib.name))
			{
				m_elements.emplace_back(attrib, m_input_steppingtype == VertexLayout::InputSteppingType::Interleaved ? InterleavedSize() : ContinuousSize());
			}
			return *this;
		}

		bool VertexLayout::Has(GLenum type, std::string name) const noexcept
		{
			for (auto& e : m_elements)
			{
				if (e.GetType() == type && e.GetName() == name)
				{
					return true;
				}
			}
			return false;
		}

		size_t VertexLayout::InterleavedSize() const noxnd
		{
			return m_elements.empty() ? 0u : m_elements.back().GetOffsetAfter();
		}

		size_t VertexLayout::ContinuousSize() const noxnd {
			return m_elements.empty() ? 0u : m_elements.back().GetOffset() + m_vertex_count * m_elements.back().Size();
		}

		size_t VertexLayout::VertexSize() const noxnd {
			int size = 0;
			for (const auto& ele : m_elements) {
				size += ele.Size();
			}
			return size;
		}

		std::vector<OGL_INPUT_ELEMENT_DESC> VertexLayout::GetOGLLayout() const noxnd {
			std::vector<OGL_INPUT_ELEMENT_DESC> desc;
			desc.reserve(GetElementCount());
			for (const auto& e : m_elements) {
				desc.push_back(e.GetDesc());
			}
			return desc;
		}

		std::string VertexLayout::GetCode() const noxnd
		{
			std::string code;
			for (const auto& e : m_elements)
			{
				code += e.GetCode();
			}
			return code;
		}


		//VertexLayout::Element
		VertexLayout::Element::Element(const Dynamic::Dsr::VertexAttrib& attrib, size_t offset)
			:
			m_attrib(attrib),
			m_offset(offset)
		{
		}

		size_t VertexLayout::Element::GetOffsetAfter() const noxnd
		{
			return m_offset + Size();
		}

		size_t VertexLayout::Element::GetOffset() const
		{
			return m_offset;
		}

		size_t VertexLayout::Element::Size() const noxnd
		{
			return SizeOf(m_attrib.type);
		}

		template <GLenum Type>
		struct SysSizeLookup {
			static constexpr auto Exec(std::string name) noexcept {
				if constexpr (GLReverseLeafMap<Type>::valid) 
				{
					return LeafMap<GLReverseLeafMap<Type>::SysType>::SysSize;
				}
				else 
				{
					return 0; 
				}
			}
		};
		constexpr size_t VertexLayout::Element::SizeOf(GLenum type) noxnd
		{
			return Bridge<SysSizeLookup>(type, std::string());
		}

		template <GLenum Type>
		struct CodeLookup {
			static constexpr auto Exec(std::string name) noexcept {
				if constexpr (GLReverseLeafMap<Type>::valid)
				{
					return LeafMap<GLReverseLeafMap<Type>::SysType>::SysCode;
				}
				else
				{
					return "";
				}
			}
		};
		const char* Dvtx::VertexLayout::Element::GetCode() const noexcept
		{
			return Bridge<CodeLookup>(m_attrib.type, m_attrib.name);
		}

		template <GLenum Type>
		struct DescGenerator {
			static constexpr OGL_INPUT_ELEMENT_DESC Exec(std::string name, Dynamic::Dsr::VertexAttrib attrib, size_t offset) noexcept {
				if constexpr (GLReverseLeafMap<Type>::valid)
				{
					using ValidClass = LeafMap<GLReverseLeafMap<Type>::SysType>;
					return { attrib.location, ValidClass::RowCount, ValidClass::ColCount,
						ValidClass::EleFormat, offset };
				}
				else
				{
					return {};
				}
			}
		};
		OGL_INPUT_ELEMENT_DESC VertexLayout::Element::GetDesc() const noxnd {
			return Bridge<DescGenerator>(m_attrib.type, m_attrib.name, m_attrib, m_offset);
		}

		//CPUVertexBuffer
		CPUVertexBuffer::CPUVertexBuffer(std::vector<Dynamic::Dsr::VertexAttrib> attribs, unsigned int vertices,
			VertexLayout::InputClassification classification, VertexLayout::InputSteppingType type) noxnd {
			m_layout.SetInputClassification(classification).SetInputSteppingType(type).SetVertexCount(vertices);
			for (auto attrib : attribs) {
				m_layout.Append(attrib);
			}

			m_buffer.resize(m_layout.VertexSize() * vertices);
		}

		void CPUVertexBuffer::InitializeVectorData(int offset, std::vector<std::pair<LeafType, std::vector<AvailableType>>>&& data) noxnd
		{
			m_layout.GetInputSteppingType() == VertexLayout::InputSteppingType::Interleaved ? InitializeVectorInterleaved(offset, std::move(data)) : InitializeVectorContinuous(offset, std::move(data));
			return;
		}

		void CPUVertexBuffer::InitializeVectorInterleaved(int offset, std::vector<std::pair<LeafType, std::vector<AvailableType>>>&& data) noxnd
		{
			auto item1 = data[0];
			switch (item1.first)
			{
#define X(Type,GLSLType,Code,Row,Col,CType,EleType) case Type :\
				{ \
					if constexpr (LeafMap<Type>::valid) \
					{ \
						std::vector<typename LeafMap<Type>::SysType> real_data(item1.second.size()); \
						for (size_t i = 0; i < item1.second.size(); i++) \
						{ \
							real_data[i] = std::get<typename LeafMap<Type>::SysType>((item1.second)[i]); \
						} \
						\
						InitializeInterleaved(real_data, offset); \
						offset += LeafMap<Type>::SysSize; \
					} \
					break; \
				}

				TYPE_GENERATOR
#undef X
			}

			data.erase(data.begin());
			if (data.size() == 0)
			{
				return;
			}

			InitializeVectorInterleaved(offset, std::move(data));
		}

		void CPUVertexBuffer::InitializeVectorContinuous(int offset, std::vector<std::pair<LeafType, std::vector<AvailableType>>>&& data) noxnd
		{
			auto item1 = data[0];
			switch (item1.first)
			{
#define X(Type,GLSLType,Code,Row,Col,CType,EleType) case Type :\
				{ \
					if constexpr (LeafMap<Type>::valid) \
					{ \
						std::vector<typename LeafMap<Type>::SysType> real_data(item1.second.size()); \
						for (size_t i = 0; i < item1.second.size(); i++) \
						{ \
							real_data[i] = std::get<typename LeafMap<Type>::SysType>((item1.second)[i]); \
						} \
						\
						InitializeContinuous(real_data, offset); \
						offset += LeafMap<Type>::SysSize * real_data.size(); \
					} \
					break; \
				}

				TYPE_GENERATOR
#undef X
			}

			data.erase(data.begin());
			if (data.size() == 0)
			{
				return;
			}

			InitializeVectorContinuous(offset, std::move(data));
		}
	}
}