
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>



#include <iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.3f, -5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool firstPerson = false;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

////////////////////////////////////////////////////////////////

enum PlayerState {
	IDLE,
	WALKING,
	DANCE
};

PlayerState state = IDLE;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// capture mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("anim_model.vs", "anim_model.fs");


	// load models
	// -----------
	Model ourModel(FileSystem::getPath("resources/objects/woman2/Ch41_nonPBR.dae"));
	Animation walkAnimation_F(FileSystem::getPath("resources/objects/woman2/walking_1.dae"), &ourModel);
	Animation walkAnimation_B(FileSystem::getPath("resources/objects/woman2/Walking_Backwards.dae"), &ourModel);
	Animation danceAnimation(FileSystem::getPath("resources/objects/woman2/Hip_Hop_Dancing.dae"), &ourModel);
	Animator animator(&walkAnimation_F);


	// mode toggle
	int mode = 0;
	bool cPressedLastFrame = false;

	// camera
	camera.Front = glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f) - camera.Position);

	// init player state
	glm::vec3 playerPos = glm::vec3(0.0f);
	float playerSpeed = 2.5f;
	float playerYaw = 0.0f;

	float baseYaw = 0.0f; // facing camera forward
	bool turning = false;

	float modelYaw = 0.0f; // rotate model

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);
		animator.UpdateAnimation(deltaTime);

		// STATE - walk
		glm::vec3 direction(0.0f);

		bool moving = false;
		bool forwardPressed = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
		bool backwardPressed = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
		bool rightPressed = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
		bool leftPressed = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;

		// TOGGLE
		if (state != DANCE)
		{
			// forward
			glm::vec3 forward = glm::vec3(
				sin(playerYaw),
				0.0f,
				cos(playerYaw)
			);

			glm::vec3 direction(0.0f);
			float moveAngle = 0.0f;
			bool hasInput = false;

			// direction
			if (forwardPressed && leftPressed)
			{
				moveAngle = glm::radians(45.0f);
				modelYaw = glm::radians(45.0f);
				hasInput = true;
			}
			else if (forwardPressed && rightPressed)
			{
				moveAngle = glm::radians(315.0f);
				modelYaw = glm::radians(315.0f);
				hasInput = true;
			}
			else if (backwardPressed && rightPressed)
			{
				moveAngle = glm::radians(225.0f);
				modelYaw = glm::radians(225.0f);
				hasInput = true;
			}
			else if (backwardPressed && leftPressed)
			{
				moveAngle = glm::radians(135.0f);
				modelYaw = glm::radians(135.0f);
				hasInput = true;
			}
			else if (forwardPressed)
			{
				moveAngle = 0.0f;
				modelYaw = 0.0f;
				hasInput = true;
			}
			else if (backwardPressed)
			{
				moveAngle = glm::radians(180.0f);
				modelYaw = glm::radians(180.0f);
				hasInput = true;
			}
			else if (leftPressed)
			{
				moveAngle = glm::radians(90.0f);
				modelYaw = glm::radians(90.0f);
				hasInput = true;
			}
			else if (rightPressed)
			{
				moveAngle = glm::radians(270.0f);
				modelYaw = glm::radians(270.0f);
				hasInput = true;
			}

			if (hasInput)
			{
				float finalYaw = playerYaw + moveAngle;

				direction = glm::vec3(
					sin(finalYaw),
					0.0f,
					cos(finalYaw)
				);

				playerPos += direction * playerSpeed * deltaTime;
				moving = true;
			}
		}

		float maxAngle = glm::radians(90.0f);

		if (playerYaw > maxAngle)
			playerYaw = maxAngle;

		if (playerYaw < -maxAngle)
			playerYaw = -maxAngle;

		//TOGGLE
		if (state != DANCE)
		{
			if (moving)
			{
				if (state != WALKING)
				{
					state = WALKING;
					animator.PlayAnimation(&walkAnimation_F);
				}
			}
			else
			{
				if (state != IDLE)
				{
					state = IDLE;
					// stop animation 
				}
				else {
					animator.PlayAnimation(&walkAnimation_F);
					animator.UpdateAnimation(0.0f);
				}
			}
		}

		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		ourShader.use();

		// model transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		bool cPressedNow = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;

		if (cPressedNow && !cPressedLastFrame)
		{
			if (state == DANCE)
			{
				state = IDLE;
			}
			else
			{
				state = DANCE;
				animator.PlayAnimation(&danceAnimation); // dance
			}
		}

		cPressedLastFrame = cPressedNow;

		auto transforms = animator.GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i)
			ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);


		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, playerPos + glm::vec3(0.0f, -0.4f, 0.0f));
		//////////////////////////////////////////////////
		model = glm::rotate(model, modelYaw, glm::vec3(0, 1, 0));
		///////////////////////////////////////////////
		model = glm::scale(model, glm::vec3(0.5f));
		ourShader.setMat4("model", model);
		ourModel.Draw(ourShader);


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}