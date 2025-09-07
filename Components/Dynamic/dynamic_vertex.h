#pragma once

#define ENABLE_SHADER_TYPE_GENERATOR

#include <Dynamic/shader_reflection.h>
#include <ogl_structures.h>

namespace DrawItems {
	class Drawable;
}

namespace Dynamic {
	namespace Dvtx {
		class VertexLayout {
		public:
			template<template<GLenum> class F, typename... Args>
			static constexpr auto Bridge(GLenum type, std::string name, Args&&... args) noxnd {
				switch (type) {
#define X(Type,GLSLType,Code,Row,Col,CType,EleType) case GLSLType : return F<GLSLType>::Exec(name, std::forward<Args>(args)... );
					TYPE_GENERATOR
#undef X
				}
				assert("Invalid element type!" && false);//一定触发断言
				return F<GL_FLOAT>::Exec(name, std::forward<Args>(args)...);
			}

			enum InputClassification {
				PerVertex,
				PerInstance
			};

			enum InputSteppingType {
				Interleaved,
				Continuous
			};

			class Element
			{
			public:
				Element(const Dynamic::Dsr::VertexAttrib& attrib, size_t offset);
				size_t GetOffsetAfter() const noxnd;
				size_t GetOffset() const;
				size_t Size() const noxnd;
				static constexpr size_t SizeOf(GLenum type) noxnd;

				inline GLenum GetType() const noexcept { return m_attrib.type; }
				inline std::string GetName() const noexcept { return m_attrib.name; }
				inline GLuint GetLocation() const noexcept { return m_attrib.location; }

				OGL_INPUT_ELEMENT_DESC GetDesc() const noxnd;
				const char* GetCode() const noexcept;
			private:
				Dynamic::Dsr::VertexAttrib m_attrib;
				size_t m_offset;
			};

			template<GLenum Type>
			const Element& Resolve(std::string name) const noxnd
			{
				for (auto& e : m_elements)
				{
					if (e.GetType() == Type && e.GetName() == name)
					{
						return e;
					}
				}
				assert("Could not resolve element type" && false);
				return Element({}, 0);
			}
			const Element& ResolveByIndex(size_t i) const noxnd;
			VertexLayout& Append(Dynamic::Dsr::VertexAttrib attrib) noxnd;
			inline VertexLayout& SetInputClassification(InputClassification input) noexcept { m_input_classification = input; return *this; }
			inline VertexLayout& SetInputSteppingType(InputSteppingType type) noexcept { m_input_steppingtype = type; return *this; }
			inline VertexLayout& SetVertexCount(unsigned int vertices) noexcept { m_vertex_count = vertices; return *this; }
			size_t InterleavedSize() const noxnd;
			size_t ContinuousSize() const noxnd;
			size_t VertexSize() const noxnd;
			inline size_t GetElementCount() const noexcept { return m_elements.size(); }
			std::vector<OGL_INPUT_ELEMENT_DESC> GetOGLLayout() const noxnd;
			std::string GetCode() const noxnd;
			bool Has(GLenum type, std::string name) const noexcept;

			inline InputClassification GetInputClassification() const noexcept { return m_input_classification; }
			inline InputSteppingType GetInputSteppingType() const noexcept { return m_input_steppingtype; }
			inline size_t GetCount() const noexcept { return m_vertex_count; }

		private:
			std::vector<Element> m_elements;
			unsigned int m_vertex_count;
			InputClassification m_input_classification;
			InputSteppingType m_input_steppingtype;
		};

		class CPUVertexBuffer {
			friend class DrawItems::Drawable;

		public:
			//构造实例缓冲的时候vertices代表实例数
			CPUVertexBuffer(std::vector<Dynamic::Dsr::VertexAttrib> attribs, unsigned int vertices, VertexLayout::InputClassification classification = VertexLayout::InputClassification::PerVertex,
				VertexLayout::InputSteppingType type = VertexLayout::InputSteppingType::Continuous) noxnd;

			inline const VertexLayout& get_layout() const noexcept { return m_layout; }
			inline void* get_data() const noexcept { return (void*)m_buffer.data(); }

			//void Resize(size_t newSize) noxnd;
			//暂且不支持顶点缓冲的大小变化

			inline size_t Count() const noxnd { return m_layout.GetCount(); }
			inline size_t BufferSize() const noxnd { return m_buffer.size(); }

			// TODO : 统一名字，需要先统一vector参数传入方式，不然调用可能产生歧义
			template <typename First, typename ...Rest>
			void InitializeData(int offset, const std::vector<First>& first, const std::vector<Rest>&... rest) noxnd;
			void InitializeVectorData(int offset, std::vector<std::pair<LeafType, std::vector<AvailableType>>>&& data) noxnd;

		private:
			template <typename First, typename ...Rest>
			void InitializeInterleaved(int offset, const std::vector<First>& first, const std::vector<Rest>&... rest) noxnd;
			template <typename First, typename ...Rest>
			void InitializeContinuous(int offset, const std::vector<First>& first, const std::vector<Rest>&... rest) noxnd;
			void InitializeVectorInterleaved(int offset, std::vector<std::pair<LeafType, std::vector<AvailableType>>>&& data) noxnd;
			template <typename T>
			void InitializeInterleaved(const std::vector<T>& data, int offset) noxnd;
			template <typename T>
			void InitializeContinuous(const std::vector<T>& data, int offset) noxnd;
			void InitializeVectorContinuous(int offset, std::vector<std::pair<LeafType, std::vector<AvailableType>>>&& data) noxnd;
			void InitializeInterleaved(int offset) noexcept { return; }
			void InitializeContinuous(int offset) noexcept { return; }

		private:
			std::vector<char> m_buffer;
			VertexLayout m_layout;
		};

		template <typename First, typename ...Rest>
		void CPUVertexBuffer::InitializeData(int offset, const std::vector<First>& first, const std::vector<Rest>&... rest) noxnd {
			m_layout.GetInputSteppingType() == VertexLayout::InputSteppingType::Interleaved ? InitializeInterleaved(offset, first, rest...) : InitializeContinuous(offset, first, rest...);
			return;
		}

		template <typename First, typename ...Rest>
		void CPUVertexBuffer::InitializeInterleaved(int offset, const std::vector<First>& first, const std::vector<Rest>&... rest) noxnd {
			InitializeInterleaved(first, offset);
			offset += sizeof(First);
			InitializeInterleaved(rest..., offset);
		}

		template <typename T>
		void CPUVertexBuffer::InitializeInterleaved(const std::vector<T>& data, int offset) noxnd {
			assert(data.size() == m_layout.GetCount());

			int l_offset = offset;
			for (int i = 0; i < data.size(); i++) {
				*reinterpret_cast<T*>(m_buffer.data() + l_offset) = data[i];
				l_offset += m_layout.InterleavedSize();
			}

			return;
		}

		template <typename First, typename ...Rest>
		void CPUVertexBuffer::InitializeContinuous(int offset, const std::vector<First>& first, const std::vector<Rest>&... rest) noxnd {
			InitializeContinuous(first, offset);
			offset += first.size() * sizeof(First);
			InitializeContinuous(rest..., offset);
		}

		template <typename T>
		void CPUVertexBuffer::InitializeContinuous(const std::vector<T>& data, int offset) noxnd {
			assert(data.size() == m_layout.GetCount());

			int l_offset = offset;
			for (int i = 0; i < data.size(); i++) {
				*reinterpret_cast<T*>(m_buffer.data() + l_offset) = data[i];
				l_offset += sizeof(T);
			}

			return;
		}
	}
}

#ifndef DVTX_SOURCE_FILE
#undef ENABLE_SHADER_TYPE_GENERATOR
#endif