#pragma once

#define ENABLE_SHADER_TYPE_GENERATOR

#include <Dynamic/shader_reflection.h>
#include <Macros/shader_typedef.h>

#include <variant>
#include <functional>

namespace Dynamic {
	namespace Dcb {
#ifdef _DEBUG
		std::string type_debug_output(LeafType type);
		std::string type_debug_output(GLenum type);
#endif
		
		class LayoutElement {
		private:
			struct ExtraDataBase {
				ExtraDataBase() = default;
				virtual ~ExtraDataBase() = default;
			};

			friend class Layout;
			friend class ConstantLayout;
			friend class UniformLayout;
			friend struct ConstantExtraData;
			friend struct UniformExtraData;
		
		public:
			std::string GetSignature() const noxnd;
			inline LeafType GetType() const noexcept { return m_type; };
			inline size_t GetOffset() const noxnd { assert(m_offset.has_value());  return m_offset.value(); };
			bool Exists() const noexcept;

			//仅对结构体类型元素适用
			virtual LayoutElement& operator[](const std::string& key) noxnd = 0;
			const LayoutElement& operator[](const std::string& key) const noxnd;
		    //仅对数组类型元素适用
			virtual LayoutElement& operator[](size_t index) noxnd = 0;
			const LayoutElement& operator[](size_t index) const noxnd;

			static LeafType GLSLTypeToLeafType(GLenum type) noexcept;

		protected:
			LayoutElement() = default;
			LayoutElement(LeafType type, size_t offset);

			virtual std::string GetSignatureForStruct() const noxnd = 0;
			virtual std::string GetSignatureForArray() const noxnd = 0;

		protected:
			LeafType m_type = LeafType::Empty;
			std::unique_ptr<ExtraDataBase> m_extradata;
			std::optional<size_t> m_offset;//Empty元素没有offset
		};

		class ConstantLayoutElement :public LayoutElement {
			friend class Layout;
			friend class ConstantLayout;
			friend struct ConstantExtraData;

		public:
			size_t GetSizeInBytes() const noxnd;
			size_t GetOffsetEnd() const noxnd;

			LayoutElement& operator[](const std::string& key) noxnd override;
			LayoutElement& operator[](size_t index) noxnd override;

			template<typename T>
			size_t Resolve() const noxnd
			{
				switch (m_type)
				{
#define X(Type,GLSLType,Code,Row,Col,CType,EleType) case Type: assert(typeid(LeafMap<Type>::SysType) == typeid(T)); return m_offset.value();
					TYPE_GENERATOR
#undef X
				default:
					assert("Tried to resolve non-leaf element!" && false);
					return 0u;
				}
			}

		private:
			ConstantLayoutElement() = default;
			ConstantLayoutElement(LeafType type, size_t offset);

			std::string GetSignatureForStruct() const noxnd override;
			std::string GetSignatureForArray() const noxnd override;

			static ConstantLayoutElement& GetEmptyElement() noexcept
			{
				static ConstantLayoutElement empty{};
				return empty;
			}

			void AppendChild(const Dynamic::Dsr::ConstantTreeNode& root, const Dynamic::Dsr::ConstantEntryPoint& entry);
		};

		class UniformLayoutElement :public LayoutElement {
			friend class Layout;
			friend class UniformLayout;
			friend struct UniformExtraData;

		public:
			inline GLint GetLocation() const noxnd { assert(m_location.has_value()); return m_location.value(); };

			LayoutElement& operator[](const std::string& key) noxnd override;
			LayoutElement& operator[](size_t index) noxnd override;

			template<typename T>
			GLuint Resolve() const noxnd
			{
				switch (m_type)
				{
#define X(Type,GLSLType,Code,Row,Col,CType,EleType) case Type: assert(typeid(LeafMap<Type>::SysType) == typeid(T)); return m_location.value();
					TYPE_GENERATOR
#undef X
				default:
					assert("Tried to resolve non-leaf element!" && false);
					return 0u;
				}
			}

			inline bool IsDirty() const noexcept { return m_is_dirty; };
			inline void SetDirtyFlag(bool is_dirty) noexcept { m_is_dirty = is_dirty; };

			inline bool IsLeaf() const noexcept { return m_location.has_value(); };

		private:
			UniformLayoutElement() = default;
			UniformLayoutElement(LeafType type, size_t offset, std::optional<GLint> location = {});

			std::string GetSignatureForStruct() const noxnd override;
			std::string GetSignatureForArray() const noxnd override;

			static UniformLayoutElement& GetEmptyElement() noexcept
			{
				static UniformLayoutElement empty{};
				return empty;
			}

			using LeafVector = std::vector<std::reference_wrapper<UniformLayoutElement>>;
			void AppendChild(const Dynamic::Dsr::ConstantTreeNode& root, const Dynamic::Dsr::UniformEntryPoint& entry);

		private:
			std::optional<GLint> m_location;//只有叶子节点有Location信息
			bool m_is_dirty = false;//只有叶子节点这个值可能为true
		};

		struct ConstantExtraData {
			struct Struct :public LayoutElement::ExtraDataBase {
				std::vector<std::pair<std::string, ConstantLayoutElement>> m_elements;
			};

			struct Array :public LayoutElement::ExtraDataBase {
				std::vector<ConstantLayoutElement> m_elements;
			};
		};

		struct UniformExtraData {
			struct Struct :public LayoutElement::ExtraDataBase {
				std::vector<std::pair<std::string, UniformLayoutElement>> m_elements;
			};

			struct Array :public LayoutElement::ExtraDataBase {
				std::vector<UniformLayoutElement> m_elements;
			};
		};

		class Layout {
			friend class CPUConstantBuffer;
			friend class CPUUniformBlock;

		public:
			std::string GetSignature() const noxnd;
			inline std::shared_ptr<LayoutElement> ShareRoot() const noexcept { return m_root; };

			virtual LayoutElement& operator[](const std::string& key) noxnd;
			const LayoutElement& operator[](const std::string& key) const noxnd;

		protected:
			Layout() = default;

		protected:
			std::shared_ptr<LayoutElement> m_root;//根节点一定是一个Struct类型的元素
		};

		class ConstantLayout :public Layout {
			friend class CPUConstantBuffer;

		public:
			size_t GetSizeInBytes() const noexcept;

		private:
			ConstantLayout(const Dynamic::Dsr::ConstantEntryPoint& entry);
		};

		class UniformLayout :public Layout {
			friend class CPUUniformBlock;

		public:
			LayoutElement& operator[](const std::string& key) noxnd override;

			LayoutElement& operator[](size_t index) noxnd;
			const LayoutElement& operator[](size_t index) const noxnd;

			UniformLayoutElement& RootElement() noxnd;

		private:
			UniformLayout(const Dynamic::Dsr::UniformEntryPoint& entry);

			void ClearAllFlags() noxnd;

			LeafType Type(size_t index) const noxnd;
			bool LeafIsDirty(size_t index) const noxnd;
			GLint LeafLocation(size_t index) const noxnd;

			void ConstructLeafVector(const UniformLayoutElement& node);

		private:
			UniformLayoutElement::LeafVector m_leafs;//保存所有叶子节点在一个向量中，方便在改变数据后统一更新（直接遍历向量即可）
		};

		template <typename Derived>
		class ConstElementRef {
			friend class CPUConstantBuffer;
			friend class CPUUniformBlock;

		public:
			class Ptr {
				friend class ConstConstantElementRef;
				friend class ConstUniformElementRef;

			public:
				template <typename T>
				operator const T* () const noxnd {
					static_assert(ReverseLeafMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion!");
					return &static_cast<const T&>(static_cast<const Derived&>(*m_ref));
				}

			private:
				Ptr(const ConstElementRef<Derived>* ref) noexcept;

			private:
				const ConstElementRef<Derived>* m_ref;
			};

		public:
			bool Exists() const noexcept;

			Ptr operator&() const noxnd;

			template <typename T>
			operator const T& () const noxnd {
				return static_cast<const Derived&>(*this);
			}

		protected:
			ConstElementRef(const LayoutElement* layout);

		protected:
			const LayoutElement* m_layout;
		};

		class ConstConstantElementRef :public ConstElementRef<ConstConstantElementRef> {
			friend class CPUConstantBuffer;
			friend class ConstantElementRef;

		public:
			ConstConstantElementRef operator[](const std::string& key) const noxnd;
			ConstConstantElementRef operator[](size_t index) const noxnd;

			template <typename T>
			operator const T& () const noxnd {
				static_assert(ReverseLeafMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in conversion!");
				return *reinterpret_cast<const T*>(m_data + static_cast<const ConstantLayoutElement*>(this->m_layout)->Resolve<T>());
			}

		private:
			ConstConstantElementRef(const LayoutElement* layout, const char* data);

		private:
			const char* m_data;
		};

		class ConstUniformElementRef :public ConstElementRef<ConstUniformElementRef> {
			friend class CPUUniformBlock;
			friend class UniformElementRef;

		public:
			ConstUniformElementRef operator[](const std::string& key) const noxnd;
			ConstUniformElementRef operator[](size_t index) const noxnd;

			template <typename T>
			operator const T& () const noxnd {
				static_assert(ReverseLeafMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in conversion!");
				assert("Unable to fetch value from non-leaf element!" && (!static_cast<const UniformLayoutElement*>(this->m_layout)->IsLeaf()));
				return std::get<T>(m_data[this->m_layout->GetOffset()]);
			}


		private:
			ConstUniformElementRef(const LayoutElement* layout, const AvailableType* data);

		private:
			const AvailableType* m_data;//整个uniform所包含的全部叶子节点数据都存储在UniformMap中
		};

		template <typename Derived>
		class ElementRef {
			friend class CPUConstantBuffer;
			friend class CPUUniformBlock;

		public:
			class Ptr {
				friend class ElementRef;

			public:
				template <typename T>
				operator T* () const noxnd {
					static_assert(ReverseLeafMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion!");
					return &static_cast<T&>(static_cast<Derived&>(*m_ref));
				}
			private:
				Ptr(ElementRef<Derived>* ref) noexcept;

			private:
				ElementRef<Derived>* m_ref;
			};

		public:
			bool Exists() const noexcept;

			Ptr operator&() noxnd;

			template <typename T>
			T& operator=(const T& rhs) noxnd {
				return static_cast<Derived&>(*this) = rhs;
			}

			inline LayoutElement* GetLayout() const { return this->m_layout; };

		protected:
			ElementRef(LayoutElement* layout); 

		protected:
			LayoutElement* m_layout;
		};

		class ConstantElementRef :public ElementRef<ConstantElementRef> {
			friend class CPUConstantBuffer;

		public:
			template <typename T>
			operator T& () const noxnd {
				static_assert(ReverseLeafMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in conversion!");
				return *reinterpret_cast<T*>(m_data + static_cast<ConstantLayoutElement*>(this->m_layout)->Resolve<T>());
			}

			operator ConstConstantElementRef() const noxnd;
			ConstantElementRef operator[](const std::string& key) noxnd;
			ConstantElementRef operator[](size_t index) noxnd;

			template <typename T>
			T& operator=(const T& rhs) noxnd {
				static_assert(ReverseLeafMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in assignment!");
				return static_cast<T&>(*this) = rhs;
			}

			bool operator==(const ConstantElementRef& rhs) const;

		private:
			ConstantElementRef(LayoutElement* layout, char* data);

		private:
			char* m_data;
		};

		class UniformElementRef :public ElementRef<UniformElementRef> {
			friend class CPUUniformBlock;

		public:
			template <typename T>
			operator T& () const noxnd {
				static_assert(ReverseLeafMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in conversion!");
				assert("Unable to fetch value from non-leaf element!" && static_cast<const UniformLayoutElement*>(this->m_layout)->IsLeaf());
				return std::get<T>(m_data[this->m_layout->GetOffset()]);
			}

			operator ConstUniformElementRef() const noxnd;
			UniformElementRef operator[](const std::string& key) noxnd;
			UniformElementRef operator[](size_t index) noxnd;

			template <typename T>
			T& operator=(const T& rhs) noxnd {
				static_assert(ReverseLeafMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in assignment!");
				static_cast<UniformLayoutElement*>(this->m_layout)->SetDirtyFlag(true);
				return static_cast<T&>(*this) = rhs;
			}

			bool operator==(const UniformElementRef& rhs) const;

		private:
			UniformElementRef(LayoutElement* layout, AvailableType* data);

		private:
			AvailableType* m_data;
		};

		class CPUConstantBuffer {
		public:
			CPUConstantBuffer(const Dynamic::Dsr::ConstantEntryPoint& entry);

			ConstantElementRef operator[](const std::string& key) noxnd;
			ConstConstantElementRef operator[](const std::string& key) const noxnd;
			
			inline const char* GetData() const noexcept { return m_data.data(); };
			inline size_t GetSizeInBytes() const noexcept { return m_data.size(); };
			inline ConstantLayoutElement& GetRootElement() const noexcept { return *m_root; };
			inline std::shared_ptr<ConstantLayoutElement> ShareRootElement() const noexcept { return m_root; };

			void CopyFrom(const CPUConstantBuffer& rhs) noxnd;

		private:
			std::vector<char> m_data;
			std::shared_ptr<ConstantLayoutElement> m_root;
		};

		class CPUUniformBlock {
		public:
			CPUUniformBlock(const Dynamic::Dsr::UniformEntryPoint& entry);
			CPUUniformBlock() = default;

			UniformElementRef operator[](const std::string& key) noxnd;
			ConstUniformElementRef operator[](const std::string& key) const noxnd;
			UniformElementRef operator[](size_t index) noxnd;
			ConstUniformElementRef operator[](size_t index) const noxnd;

			template <typename T>
			operator T& () noxnd {
				return UniformElementRef(&(*(m_layout->ShareRoot())), *m_data);
			}
			template <typename T>
			operator const T& () const noxnd {
				return ConstUniformElementRef(&(*(m_layout->ShareRoot())), *m_data);
			}

			template <typename T>
			T& operator=(const T& rhs) noxnd {
				static_assert(IsVariantMember<T, AvailableType>::value, "Value type not supported!");
				assert("Only Leaf Elements are allowed to use this operator!" && std::static_pointer_cast<UniformLayoutElement>(m_layout->ShareRoot())->IsLeaf());
				UniformElementRef ref(m_layout->ShareRoot().get(), &(m_data[0]));
				return ref = rhs;
			}

			UniformElementRef GetLeafUniform();
			ConstUniformElementRef GetConstLeafUniform() const;

			inline size_t UniformAmount() const noexcept { return m_data.size(); };
			inline std::vector<AvailableType> GetData()const noexcept { return m_data; };
			inline UniformLayoutElement& GetRootElement() const noexcept { return static_cast<UniformLayoutElement&>(*(m_layout->m_root)); };
			inline std::shared_ptr<UniformLayoutElement> ShareRootElement() const noexcept { return std::static_pointer_cast<UniformLayoutElement>(m_layout->m_root); };

			void CopyFrom(const CPUUniformBlock& rhs) noxnd;

			void ClearAllFlags() noxnd;

			LeafType UniformType(size_t index) const noxnd;
			bool UniformIsDirty(size_t index) const noxnd;
			const AvailableType& UniformInformation(size_t index, GLint& location) const noxnd;

		private:
			void InitializeVariant(AvailableType& element, GLenum type);

		private:
			std::vector<AvailableType> m_data;
			std::shared_ptr<UniformLayout> m_layout;
		};
	}
}

#ifndef DYNAMIC_CONSTANT_SOURCE_FILE
#undef ENABLE_SHADER_TYPE_GENERATOR
#endif