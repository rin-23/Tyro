#pragma once

struct GLFWwindow;

namespace RAEngine 
{
	class ES2Context
	{
	public:
		ES2Context(GLFWwindow* window);

		void setCurrent();

		void endCurrent();

		void swapBuffers();

		void getFramebufferSize(int* width, int* height);

	private:
		GLFWwindow* mWindow;
	};
}