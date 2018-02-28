/*
ZJ Wood CPE 471 Lab 3 base code - I. Dunn class re-write
*/

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "WindowManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


template <typename T>
T CubicInterpolate(T p[4], T x)
{
	return p[1] + (T) 0.5 * x*(p[2] - p[0] + x*(2 * p[0] - 5 * p[1] + 4 * p[2] - p[3] + x*(3 * (p[1] - p[2]) + p[3] - p[0])));
}

float Cubic(float in, float out, float t)
{
	float p[4];
	p[0] = p[1] = in;
	p[2] = p[3] = out;

	return CubicInterpolate<float>(p, t);
}

float MoveLinear(float From, float const To, float const Elapsed, float const Speed, float const Clamp)
{
	if (From > To + Clamp)
	{
		From -= Speed * Elapsed;
		if (From < To + Clamp)
		{
			From = To;
		}
	}
	if (From < To - Clamp)
	{
		From += Speed * Elapsed;
		if (From > To - Clamp)
		{
			From = To;
		}
	}

	return From;
}

float MoveQuadratic(float From, float const To, float const Elapsed, float const Speed, float const Clamp)
{
	float const DistanceCanGo = Speed * Elapsed * std::abs(To - From);

	if (From > To + Clamp)
	{
		From -= DistanceCanGo;
		if (From < To + Clamp)
		{
			From = To;
		}
	}
	if (From < To - Clamp)
	{
		From += DistanceCanGo;
		if (From > To - Clamp)
		{
			From = To;
		}
	}

	return From;
}


class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;
	std::string resourceDirectory;

	// Our shader program
	std::shared_ptr<Program> prog;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint IndexBufferID;

	GLuint TextureID;

	float Position = 0.55f;
	float Goal = 0.55f;
	float TexCoordMult = 1.f;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (action == GLFW_RELEASE)
		{
			switch (key)
			{
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, GL_TRUE);
				break;

			case GLFW_KEY_Z:
				Goal = 0.1f;
				break;

			case GLFW_KEY_X:
				Goal = 0.55f;
				break;

			case GLFW_KEY_C:
				Goal = 1.f;
				break;

			case GLFW_KEY_V:
				Goal = 5.f;
				break;

			case GLFW_KEY_B:
				Goal = 10.f;
				break;

			case GLFW_KEY_1:
				TexCoordMult = 0.5f;
				break;

			case GLFW_KEY_2:
				TexCoordMult = 1.f;
				break;

			case GLFW_KEY_3:
				TexCoordMult = 4.f;
				break;

			case GLFW_KEY_4:
				TexCoordMult = 16.f;
				break;

			case GLFW_KEY_5:
				TexCoordMult = 64.f;
				break;

			case GLFW_KEY_6:
				TexCoordMult = 256.f;
				break;

			case GLFW_KEY_T:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				std::cout << "Minification: Nearest." << std::endl;
				break;

			case GLFW_KEY_Y:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				std::cout << "Minification: Linear." << std::endl;
				break;

			case GLFW_KEY_U:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				std::cout << "Minification: Linear + MipMap." << std::endl;
				break;

			case GLFW_KEY_G:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				std::cout << "Magnification: Nearest." << std::endl;
				break;

			case GLFW_KEY_H:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				std::cout << "Magnification: Linear." << std::endl;
				break;

			case GLFW_KEY_Q:
				loadImage("Texture.png");
				break;

			case GLFW_KEY_W:
				loadImage("Image.jpg");
				break;
			}
		}
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;

		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		GLuint VertexBufferID;
		GLuint TexCoordBufferID;

		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &VertexBufferID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		static const GLfloat g_vertex_buffer_data[] =
		{
			-1.0f, -1.0f,  0.0f,
			 1.0f, -1.0f,  0.0f,
			 1.0f,  1.0f,  0.0f,
			-1.0f,  1.0f,  0.0f,
		};
		static const GLfloat g_texcoord_buffer_data[] =
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
			0.0f, 1.0f,
		};
		//actually memcopy the data - only do this once
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_DYNAMIC_DRAW);

		//we need to set up the vertex array
		glEnableVertexAttribArray(0);
		//key function to get up how many elements to pull out at a time (3)
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

		glGenBuffers(1, &TexCoordBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, TexCoordBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_texcoord_buffer_data), g_texcoord_buffer_data, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);



		// Create and bind IBO
		glGenBuffers(1, &IndexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID);

		static const GLuint g_index_buffer_data[] =
		{
			0, 1, 2,
			0, 2, 3
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_index_buffer_data), g_index_buffer_data, GL_DYNAMIC_DRAW);

		glBindVertexArray(0);
	}

	void initTexture()
	{
		// Create texture
		glGenTextures(1, &TextureID);
		glBindTexture(GL_TEXTURE_2D, TextureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		loadImage("Texture.png");
	}

	void loadImage(const std::string & fileName)
	{
		std::string const path = resourceDirectory + "/" + fileName;
		int x, y, n;
		unsigned char * data = stbi_load(path.c_str(), & x, & y, & n, 0);

		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			stbi_image_free(data);
		}
		else
		{
			printf("Failed to load image from file '%s', reason: %s\n", path.c_str(), stbi_failure_reason());
			return;
		}
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		this->resourceDirectory = resourceDirectory;

		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert33.glsl", resourceDirectory + "/simple_frag33.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("MV");
		prog->addUniform("uTexture");
		prog->addUniform("uTexCoordMult");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertTex");
	}

	float lastTime = 0;


	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		float thisTime = (float) glfwGetTime();
		float elapsed = thisTime - lastTime;

		Position = MoveQuadratic(Position, Goal, elapsed, 5.f, 0.001f);

		lastTime = thisTime;

		// Create the matrix stacks - please leave these alone for now
		auto P = std::make_shared<MatrixStack>();
		auto MV = std::make_shared<MatrixStack>();
		// Apply orthographic projection.
		P->pushMatrix();
		P->perspective(40.f, aspect, 0.1f, 100.f);
		MV->pushMatrix();
		MV->translate(glm::vec3(0, 0, -Position));

		// Draw the triangle using GLSL.
		prog->bind();

		//send the matrices to the shaders
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

		glUniform1f(prog->getUniform("uTexCoordMult"), TexCoordMult);

		// bind texture on texture unit
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureID);
		glUniform1i(prog->getUniform("uTexture"), 0);

		glBindVertexArray(VertexArrayID);

		//actually draw from vertex 0, 3 vertices
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		glBindVertexArray(0);

		prog->unbind();

		// Pop matrix stacks.
		MV->popMatrix();
		P->popMatrix();
	}

};

int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();
	application->initTexture();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
