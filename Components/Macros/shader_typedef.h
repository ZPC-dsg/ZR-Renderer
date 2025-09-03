#pragma once

#include <variant>
#include <glm/glm.hpp>
#include <glad/glad.h>

#define TYPE_GENERATOR \
	X(Float,GL_FLOAT,F1,1,1,float,GL_FLOAT) \
	X(Float2,GL_FLOAT_VEC2,F2,2,1,glm::vec2,GL_FLOAT) \
	X(Float3,GL_FLOAT_VEC3,F3,3,1,glm::vec3,GL_FLOAT) \
	X(Float4,GL_FLOAT_VEC4,F4,4,1,glm::vec4,GL_FLOAT) \
	X(Double,GL_DOUBLE,D1,1,1,double,GL_DOUBLE) \
	X(Double2,GL_DOUBLE_VEC2,D2,2,1,glm::dvec2,GL_DOUBLE) \
	X(Double3,GL_DOUBLE_VEC3,D3,3,1,glm::dvec3,GL_DOUBLE) \
	X(Double4,GL_DOUBLE_VEC4,D4,4,1,glm::dvec4,GL_DOUBLE) \
	X(Int,GL_INT,I1,1,1,int,GL_INT) \
	X(Int2,GL_INT_VEC2,I2,2,1,glm::ivec2,GL_INT) \
	X(Int3,GL_INT_VEC3,I3,3,1,glm::ivec3,GL_INT) \
	X(Int4,GL_INT_VEC4,I4,4,1,glm::ivec4,GL_INT) \
	X(Uint,GL_UNSIGNED_INT,U1,1,1,unsigned int,GL_UNSIGNED_INT) \
	X(Uint2,GL_UNSIGNED_INT_VEC2,U2,2,1,glm::uvec2,GL_UNSIGNED_INT) \
	X(Uint3,GL_UNSIGNED_INT_VEC3,U3,3,1,glm::uvec3,GL_UNSIGNED_INT) \
	X(Uint4,GL_UNSIGNED_INT_VEC4,U4,4,1,glm::uvec4,GL_UNSIGNED_INT) \
	X(Bool,GL_BOOL,B1,1,1,bool,GL_BOOL) \
	X(Bool2,GL_BOOL_VEC2,B2,2,1,glm::bvec2,GL_BOOL) \
	X(Bool3,GL_BOOL_VEC3,B3,3,1,glm::bvec3,GL_BOOL) \
	X(Bool4,GL_BOOL_VEC4,B4,4,1,glm::bvec4,GL_BOOL) \
	X(FMat2,GL_FLOAT_MAT2,FM2,2,2,glm::mat2,GL_FLOAT) \
	X(FMat3,GL_FLOAT_MAT3,FM3,3,3,glm::mat3,GL_FLOAT) \
	X(FMat4,GL_FLOAT_MAT4,FM4,4,4,glm::mat4,GL_FLOAT) \
	X(DMat2,GL_DOUBLE_MAT2,DM2,2,2,glm::dmat2,GL_DOUBLE) \
	X(DMat3,GL_DOUBLE_MAT3,DM3,3,3,glm::dmat3,GL_DOUBLE) \
	X(DMat4,GL_DOUBLE_MAT4,DM4,4,4,glm::dmat4,GL_DOUBLE)

enum LeafType {
#define X(Type,GLSLType,Code,Row,Col,CType,EleType) Type,
	TYPE_GENERATOR
#undef X
	Struct,
	Array,
	Empty
};

template <LeafType Type> struct LeafMap { static constexpr bool valid = false; };
#define X(Type,GLSLType,Code,Row,Col,CType,EleType) \
		template <> \
		struct LeafMap<Type> { \
			using SysType = CType; \
			static constexpr size_t SysSize = sizeof(CType); \
			static constexpr const char* SysCode = #Code; \
			static constexpr bool valid = true; \
			\
			static constexpr GLenum GLType = GLSLType; \
			static constexpr GLenum EleFormat = EleType; \
			static constexpr size_t RowCount = Row; \
			static constexpr size_t ColCount = Col; \
		};

TYPE_GENERATOR
#undef X

template <typename T> struct ReverseLeafMap { static constexpr bool valid = false; };
#define X(Type,GLSLType,Code,Row,Col,CType,EleType) \
		template <> \
		struct ReverseLeafMap<typename LeafMap<Type>::SysType> { \
			static constexpr LeafType SysType = Type; \
		    static constexpr bool valid = true; \
		};

TYPE_GENERATOR
#undef X

template <GLenum> struct GLReverseLeafMap { static constexpr bool valid = false; };
#define X(Type,GLSLType,Code,Row,Col,CType,EleType) \
		template <> \
		struct GLReverseLeafMap<GLSLType> { \
			static constexpr LeafType SysType = Type; \
		    static constexpr bool valid = true; \
		};

TYPE_GENERATOR
#undef X

using AvailableType = std::variant<float, glm::vec2, glm::vec3, glm::vec4, double, glm::dvec2, glm::dvec3, glm::dvec4, int, glm::ivec2, glm::ivec3, glm::ivec4,
	unsigned int, glm::uvec2, glm::uvec3, glm::uvec4, bool, glm::bvec2, glm::bvec3, glm::bvec4, glm::mat2, glm::mat3, glm::mat4, glm::dmat2, glm::dmat3, glm::dmat4>;

template<class T, class V>
struct IsVariantMember;

template<class T, class... ALL_V>
struct IsVariantMember<T, std::variant<ALL_V...>> : public std::disjunction<std::is_same<T, ALL_V>...> {};

#ifndef ENABLE_SHADER_TYPE_GENERATOR
#undef TYPE_GENERATOR
#endif