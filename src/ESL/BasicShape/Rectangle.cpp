#include "Rectangle.hpp"

esl::Rectangle::Rectangle(const glm::vec2& size) : m_size(size)
{
	setupGeometry();
}

void esl::Rectangle::setSize(const glm::vec2& size)
{
	m_size = size;
	setupGeometry();
}

// 重写绘制逻辑
void esl::Rectangle::draw(float right, float top)
{
	if (!m_shapeShader) {
		return;
	}
	GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
	GLboolean cullFaceEnabled = glIsEnabled(GL_CULL_FACE);

	// 1. 关闭深度测试，确保滤镜覆盖在最上层
	glDisable(GL_DEPTH_TEST);
	// 2. 关闭面剔除，防止顶点顺序导致的不显示
	glDisable(GL_CULL_FACE);

	// 3. 启用混合并设置反相
	glEnable(GL_BLEND);
	

	m_shapeShader->load();

	// 4. 设置矩阵
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(m_position, 0.0f));
	model = glm::rotate(model, glm::radians(m_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(m_scale, 1.0f));

	glm::mat4 projection = glm::ortho(0.0f, right, 0.0f, top, -1.0f, 1.0f);

	m_shapeShader->setMat4("projection", projection);
	m_shapeShader->setMat4("model", model);
	
	if (m_InversionLayer) {
		m_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ZERO, GL_ZERO, GL_ONE);
	}
	m_shapeShader->setVec4(
		"color",
		m_color
	);

	// 5. 绘制
	glBindVertexArray(VAO);
	if (!VAO) setupGeometry(); // 防御性检查
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// 6. 恢复状态
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
	if (cullFaceEnabled) glEnable(GL_CULL_FACE);

	m_shapeShader->unload();
}

void esl::Rectangle::setupGeometry()
{
	// 定义以 (0,0) 为中心的矩形顶点
	float w = m_size.x / 2.0f;
	float h = m_size.y / 2.0f;

	float vertices[] = {
		w,  h,  // 右上
		w, -h,  // 右下
		-w, -h,  // 左下
		-w,  h   // 左上
	};

	unsigned int indices[] = {
		0, 1, 3, // 第一个三角形
		1, 2, 3  // 第二个三角形
	};

	// 初始化 Shape 类中定义的 protected 成员 VAO/VBO/EBO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// layout(location = 0) in vec2 aPos;
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
