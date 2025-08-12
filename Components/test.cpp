#include <test.h>

#include <stb_image.h>
#include <vector>

#include <shader.h>
#include <buffer.h>
#include <textureimage.h>
#include <init.h>
#include <texturebuilder.h>
#include <computeshader.h>

void test_texture(Utils& app) {
    std::unique_ptr<TextureBase> tex = TextureFactory::build(TEXTURE_IMAGE, 0, 0, "particle_rgb.jpg");

    Shader testShader("", "test.vert", "test.frag");
    testShader.use();
    tex->bind(0);
    testShader.setInt("image", 0);

    Buffer b(GL_UNIFORM_BUFFER, GL_STATIC_DRAW, 10, nullptr);
    GL_CALL(b.bind(5));

    while (!glfwWindowShouldClose(globalSettings::mainWindow)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        app.renderQuad();

        glfwSwapBuffers(globalSettings::mainWindow);
        glfwPollEvents();
    }
}

void test_compute(Utils& app) {
    glEnable(GL_PROGRAM_POINT_SIZE);

    const int NUM_PARTICLES = 2048;
    struct Particle
    {
        glm::vec4 initialPosition;
        glm::vec4 intialColor;
    };

    std::vector<Particle> particles(NUM_PARTICLES);
    
    // ��ʼ������λ��
    for (int i = 0; i < NUM_PARTICLES; ++i) {
        particles[i].initialPosition = glm::vec4((float)(rand() % 100) / 100.0f, (float)(rand() % 100) / 100.0f, 1.0, 1.0);
        particles[i].intialColor = glm::vec4((float)(rand() % 100) / 100.0f, (float)(rand() % 100) / 100.0f, (float)(rand() % 100) / 100.0f, 1.0);
    }

    Buffer particleBuffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_PARTICLES * sizeof(Particle), particles.data());
    particleBuffer.bind(0);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    particleBuffer.rebind(GL_ARRAY_BUFFER);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, initialPosition));
    glEnableVertexAttribArray(1);
    particleBuffer.unbind();
    glBindVertexArray(0);

    ComputeShader computeShader("compute_nbody", "ctest.comp");
    Shader shader("compute_nbody", "vtest.vert", "ftest.frag");

    while (!glfwWindowShouldClose(globalSettings::mainWindow)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        particleBuffer.rebind(GL_SHADER_STORAGE_BUFFER);

        // ִ�� Compute Shader
        computeShader.use();
        glDispatchCompute(NUM_PARTICLES / 64, 1, 1); // ����ÿ����������64������
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // ȷ�����ݸ������

        particleBuffer.rebind(GL_ARRAY_BUFFER);
        // ��������
        shader.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);

        glfwSwapBuffers(globalSettings::mainWindow);
        glfwPollEvents();
    }
}

void test_uniform(Utils& app) {
    struct UBO
    {
        glm::vec4 color;
        glm::vec2 bias;
    } uniform;
    uniform.color = glm::vec4(0.2, 0.4, 0.6, 1.0);
    uniform.bias = glm::vec2(0.0);
    Shader test("compute_nbody", "utest.vert", "utest.frag");
    Buffer ubo(GL_UNIFORM_BUFFER, GL_STATIC_DRAW, sizeof(UBO), NULL);
    ubo.bind(0);
    ubo.update(sizeof(UBO), &uniform);

    while (!glfwWindowShouldClose(globalSettings::mainWindow)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        test.use();
        app.renderQuad();

        glfwSwapBuffers(globalSettings::mainWindow);
        glfwPollEvents();
    }
}