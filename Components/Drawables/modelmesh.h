#pragma once

#include <Drawables/drawable.h>
#include <assimp/scene.h>

//TODO mesh��AABB��aimesh��Ҳ�а�����֮��ʵ��frustum culling��ʱ������������Ϣ

namespace DrawItems {
	class ModelMesh :public Drawable {
	public:
		ModelMesh(const aiMesh& mesh);
		~ModelMesh() = default;

	private:
		void parse_mesh(const aiMesh& mesh);
	};
}