#include <iostream>
#include <Renderer/RenderingApp.h>
//bool VKfoo();


int main() {
	//std::cout << VKfoo() << "\n";
	VulkanRenderApp app(1920,1080);
	app.initRt();

	return 0;
}