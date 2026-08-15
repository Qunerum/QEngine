#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define QGPU_COLORS
#include "qgpu.h"

#define QGPU_VERSION_MAJOR 1
#define QGPU_VERSION_MINOR 2
#define QGPU_VERSION_PATCH 5

// ========================================================================================================================================================================
// ===== QGPU =============================================================================================================================================================
// ========================================================================================================================================================================
typedef struct {
	GLFWwindow* window;
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
	VkSwapchainKHR swapchain;
	uint32_t imageCount;
	VkImage* swapchainImages;
	VkImageView* swapchainImageViews;
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkCommandPool commandPool;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	VkFramebuffer* swapchainFramebuffers;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkCommandBuffer currentCmd;
	uint32_t currentVOffset, currentIOffset;
	int lastKeyState[GLFW_KEY_LAST], lastMouseState[GLFW_MOUSE_BUTTON_LAST];
	void *mappedVertexBuffer, *mappedIndexBuffer;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	float pivotX, pivotY, pivotZ, rotX, rotY, rotZ;
	int hasRotation;
} InternalContext;
static InternalContext g_ctx;
static double lastTime = 0;
static float backgroundR, backgroundG, backgroundB, lights[MAX_LIGHTS * 5], currentFPS;
static int lightCount, frameCount;
uint32_t* sphereVertexIndices;
#define qFontX 8
#define qFontY 11
#define qFontMax 6
typedef struct { int8_t data[qFontY][qFontMax]; } qgChar;
qgChar newChars[65536] = {0};
// ========================================================================================================================================================================
// ===== TOOLS ============================================================================================================================================================
// ========================================================================================================================================================================
static int len(const char* t) { int x = 0; while (t[x] != '\0') x++; return x; }
static float PI = 3.14159265358979323846f;
static int qclamp(int v, int min, int max) { return v < min ? min : v > max ? max : v; }
static float qclampf(float v, float min, float max) { return v < min ? min : v > max ? max : v; }
static float qpow(float v, float exp) {
	if (exp == 0) return 1;
	float r = 1;
	for (int i = 0; i < exp; i++) r *= v;
	return r;
}
static float qsqrt(float number) {
	if (number <= 0.0f) return 0.0f;
	float x = number * 0.5f;
	for (int i = 0; i < 4; i++) x = 0.5f * (x + number / x);
	return x;
}
static unsigned long long factorial(int n) {
	unsigned long long result = 1;
	for (int i = 1; i <= n; i++) result *= i;
	return result;
}
float toRad(float degrees) { return degrees * (PI / 180.0f); }
static float qSin(float rad) {
	float sum = 0.0f;
	for (int i = 0; i < 10; i++) {
		int sign = (i % 2 == 0) ? 1 : -1, power_exp = 2 * i + 1;
		sum += sign * (qpow(rad, power_exp) / (float)factorial(power_exp));
	}
	return sum;
}
static float qCos(float rad) {
	float sum = 0.0f;
	for (int i = 0; i < 10; i++) {
		int sign = (i % 2 == 0) ? 1 : -1, power_exp = 2 * i;
		sum += sign * (qpow(rad, power_exp) / (float)factorial(power_exp));
	}
	return sum;
}
static void transformPoint(float* x, float* y, float* z) {
	if (!g_ctx.hasRotation) return;
	float px = *x - g_ctx.pivotX, py = *y - g_ctx.pivotY, pz = *z - g_ctx.pivotZ, radX = g_ctx.rotX * (PI / 180.0f), radY = g_ctx.rotY * (PI / 180.0f), radZ = g_ctx.rotZ * (PI / 180.0f),
	cx = qCos(radX), sx = qSin(radX), cy = qCos(radY), sy = qSin(radY), cz = qCos(radZ), sz = qSin(radZ),
	y1 = py * cx - pz * sx, z1 = py * sx + pz * cx, x1 = px, x2 = x1 * cy + z1 * sy, z2 = -x1 * sy + z1 * cy, y2 = y1, x3 = x2 * cz - y2 * sz, y3 = x2 * sz + y2 * cz, z3 = z2;
	*x = x3 + g_ctx.pivotX;
	*y = y3 + g_ctx.pivotY;
	*z = z3 + g_ctx.pivotZ;
}
static float getLight(float x, float y, float z) {
	float totalLight = 0.1f;
	for (int i = 0; i < lightCount; i++) {
		float lx = lights[i*5], ly = lights[i*5+1], lz = lights[i*5+2], rng = lights[i*5+3], pow = lights[i*5+4];
		if (rng == 0 || pow == 0) continue;
		float dis = qsqrt(qpow(lx - x, 2) + qpow(ly - y, 2) + qpow(lz - z, 2));
		if (dis > rng) continue;
		float attenuation = 1.0f - (dis / rng);
		totalLight += pow * attenuation;
	}
	return totalLight;
}
// ========================================================================================================================================================================
// ===== VISUAL ===========================================================================================================================================================
// ========================================================================================================================================================================
// ===== Configuration
static int _showBanner = 1, _madeWith = 1, _showInfo = 1, _showColors = 1, _showLogs = 1, qgpuClr = MAGENTA, creator = LIGHT_RED, title = YELLOW, frame = GRAY, frmTxt = LIGHT_GRAY;
// ===== QPrint
static int oldClr = 255, actClr = 255, actStyle = 0; // White , Regular text
void qgVprintc(int color, const char* format, va_list args) {
	printf("\033[%i;38;5;%im", actStyle, color);
	vprintf(format, args);
	printf("\033[0m");
}
void qgSetColor(int color) {
	oldClr = actClr;
	actClr = qclamp(color, 0, 255);
}
void qgRestoreColor() {
	int x = actClr;
	actClr = oldClr;
	oldClr = x;
}
void qgSetStyle(int style) { actStyle = qclamp(style, 0, 1); }
void qgPrintc(int color, const char* format, ...) {
	va_list args;
	va_start(args, format);
	qgVprintc(color, format, args);
	va_end(args);
}
void qgPrint(const char* format, ...) {
	va_list args;
	va_start(args, format);
	qgVprintc(actClr, format, args);
	va_end(args);
}
void qgLog(const char* format, ...) {
	if (!_showLogs) return;
	va_list args;
	va_start(args, format);
	qgVprintc(DARK_GRAY, format, args);
	va_end(args);
}
void qgLogVertices() {
	if (!_showLogs) return;
	float p = ((float)g_ctx.currentVOffset / MAX_VERTICES) * 100;
	if (p > 95) qgPrintc(LIGHT_RED, "%i/%i (%.0f%%)\n", g_ctx.currentVOffset, MAX_VERTICES, p);
	else if (p > 75) qgPrintc(RED, "%i/%i (%.0f%%)\n", g_ctx.currentVOffset, MAX_VERTICES, p);
	else if (p > 50) qgPrintc(ORANGE, "%i/%i (%.0f%%)\n", g_ctx.currentVOffset, MAX_VERTICES, p);
	else if (p > 25) qgPrintc(YELLOW, "%i/%i (%.0f%%)\n", g_ctx.currentVOffset, MAX_VERTICES, p);
	else if (p > 10) qgPrintc(DARK_YELLOW, "%i/%i (%.0f%%)\n", g_ctx.currentVOffset, MAX_VERTICES, p);
	else qgPrintc(GREEN, "%i/%i (%.0f%%)\n", g_ctx.currentVOffset, MAX_VERTICES, p);
}
void qgWarn(const char* format, ...) {
	va_list args;
	va_start(args, format);
	qgVprintc(DARK_YELLOW, format, args);
	va_end(args);
}
void qgError(const char* format, ...) {
	va_list args;
	va_start(args, format);
	qgVprintc(DARK_RED, format, args);
	va_end(args);
	exit(1);
}
void qgSetShow(int shower, int state) {
	switch (shower) {
		case QGPU_SHOW_BANNER: _showBanner = state; break;
		case QGPU_SHOW_MADE_WITH_QGPU: _madeWith = state; break;
		case QGPU_SHOW_INFO: _showInfo = state; break;
		case QGPU_SHOW_COLORS: _showColors = state; break;
		case QGPU_SHOW_LOGS: _showLogs = state; break;
	}
}
static void printBanner() {
	qgPrintc(165, "╔═════╗ ╔═════╗ ╔═════╗ ╔═╗ ╔═╗\n");
	qgPrintc(164, "║ ╔═╗ ║ ║ ╔═══╝ ║ ╔═╗ ║ ║ ║ ║ ║\n");
	qgPrintc(163, "║ ║ ║ ║ ║ ║ ╔═╗ ║ ╚═╝ ║ ║ ║ ║ ║\n");
	qgPrintc(162, "║ ╚═╝ ║ ║ ╚═╝ ║ ║ ╔═══╝ ║ ╚═╝ ║\n");
	qgPrintc(161, "╚═══╗ ║ ╚═════╝ ╚═╝     ╚═════╝\n");
	qgPrintc(160, "    ╚═╝\n");
}
static void printMadeWith() { qgPrintc(ORANGE, "The application was made with the "); qgPrintc(qgpuClr, "QGPU"); qgPrintc(ORANGE, " library.\n"); }
static void printInfo() {
	qgPrintc(frame, "╔═╣   "); qgPrintc(title, "Info"); qgPrintc(frame, "   ╠═══════════╦╗\n");
	qgPrintc(frame, "║ "); qgPrintc(frmTxt, "Name: "); qgPrintc(qgpuClr, "QGPU"); qgPrintc(frame, "             ╚╝\n");
	qgPrintc(frame, "║ "); qgPrintc(frmTxt, "Version: "); qgPrintc(qgpuClr, "%i.%i.%i\n", QGPU_VERSION_MAJOR, QGPU_VERSION_MINOR, QGPU_VERSION_PATCH);
	qgPrintc(frame, "║ "); qgPrintc(frmTxt, "Creator: "); qgPrintc(creator, "Qunerum"); qgPrintc(frame, "       ╔╗\n");
	qgPrintc(frame, "╚════════════════════════╩╝\n");
}
static void c(int v) { printf("\033[0;38;5;%im██ ", v); }
static void printColors() {
	qgPrintc(frame,"╔═╣  "); qgPrintc(title, "Colors"); qgPrintc(frame, "  ╠═╗  ╔═══════╗\n");
	qgPrintc(frame,"╚═╦══════════╦═╝  ║ "); c(WHITE); c(BLACK); qgPrintc(frame,"║\n");
	qgPrintc(frame,"╔═╩══════════╩════╩═══════╣\n");
	qgPrintc(frame,"║ "); c(LIGHT_GRAY); c(LIGHT_RED); c(LIGHT_GREEN); c(LIGHT_YELLOW); c(LIGHT_ORANGE); c(LIGHT_BLUE); c(LIGHT_MAGENTA); c(LIGHT_CYAN); qgPrintc(frame,"║\n");
	qgPrintc(frame,"║ "); c(GRAY);       c(RED);       c(GREEN);       c(YELLOW);       c(ORANGE);       c(BLUE);       c(MAGENTA);       c(CYAN);       qgPrintc(frame,"║\n");
	qgPrintc(frame,"║ "); c(DARK_GRAY);  c(DARK_RED);  c(DARK_GREEN);  c(DARK_YELLOW);  c(DARK_ORANGE);  c(DARK_BLUE);  c(DARK_MAGENTA);  c(DARK_CYAN);  qgPrintc(frame,"║\n");
	qgPrintc(frame,"╚═════════════════════════╝\n");
}
// ========================================================================================================================================================================
// ===== UTILITY ==========================================================================================================================================================
// ========================================================================================================================================================================
static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(g_ctx.physicalDevice, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) return i;
	return 0;
}
void createDepthResources(int width, int height) {
	VkImageCreateInfo imageInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.extent.width = width,
		.extent.height = height,
		.extent.depth = 1,
		.mipLevels = 1,
		.arrayLayers = 1,
		.format = VK_FORMAT_D32_SFLOAT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	if (vkCreateImage(g_ctx.device, &imageInfo, NULL, &g_ctx.depthImage) != VK_SUCCESS) { }
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(g_ctx.device, g_ctx.depthImage, &memRequirements);
	VkMemoryAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	if (vkAllocateMemory(g_ctx.device, &allocInfo, NULL, &g_ctx.depthImageMemory) != VK_SUCCESS) { }
	vkBindImageMemory(g_ctx.device, g_ctx.depthImage, g_ctx.depthImageMemory, 0);
	VkImageViewCreateInfo viewInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = g_ctx.depthImage,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = VK_FORMAT_D32_SFLOAT,
		.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
		.subresourceRange.baseMipLevel = 0,
		.subresourceRange.levelCount = 1,
		.subresourceRange.baseArrayLayer = 0,
		.subresourceRange.layerCount = 1
	};
	if (vkCreateImageView(g_ctx.device, &viewInfo, NULL, &g_ctx.depthImageView) != VK_SUCCESS) { }
}
static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
	VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	vkCreateBuffer(g_ctx.device, &bufferInfo, NULL, buffer);
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(g_ctx.device, *buffer, &memReqs);
	VkMemoryAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memReqs.size,
		.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, properties)
	};
	vkAllocateMemory(g_ctx.device, &allocInfo, NULL, bufferMemory);
	vkBindBufferMemory(g_ctx.device, *buffer, *bufferMemory, 0);
}
void render() {
	if (g_ctx.currentIOffset > 0) {
		int w, h;
		glfwGetFramebufferSize(g_ctx.window, &w, &h);
		float pushData[4] = { (float)w * 0.5f, (float)h * 0.5f, (float)w, (float)h };
		vkCmdPushConstants(g_ctx.currentCmd, g_ctx.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, 16, pushData);
		vkCmdDrawIndexed(g_ctx.currentCmd, g_ctx.currentIOffset, 1, 0, 0, 0);
	}
}
// ========================================================================================================================================================================
// ===== INIT =============================================================================================================================================================
// ========================================================================================================================================================================
static int inInit = 0;
void qgpuCreate(int width, int height, const char* title, void (*initFunc)(), void (*updateFunc)()) {
	if (!glfwInit()) return;
	sphereVertexIndices = malloc(sizeof(uint32_t) * (MAX_SPHERE_RINGS + 1) * (MAX_SPHERE_SECTORS + 1));
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	g_ctx.window = glfwCreateWindow(width, height, title, NULL, NULL);
	glfwSwapInterval(0);
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	VkInstanceCreateInfo instanceInfo = { .
		sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.enabledExtensionCount = glfwExtensionCount,
		.ppEnabledExtensionNames = glfwExtensions
	};
	vkCreateInstance(&instanceInfo, NULL, &g_ctx.instance);
	glfwCreateWindowSurface(g_ctx.instance, g_ctx.window, NULL, &g_ctx.surface);
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(g_ctx.instance, &deviceCount, NULL);
	VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
	vkEnumeratePhysicalDevices(g_ctx.instance, &deviceCount, devices); g_ctx.physicalDevice = devices[0];
	free(devices);
	float queuePriority = 1.0f;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(g_ctx.physicalDevice, &queueFamilyCount, NULL);
	VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(g_ctx.physicalDevice, &queueFamilyCount, queueFamilies);
	uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsQueueFamilyIndex = i;
			break;
		}
	}
	free(queueFamilies);
	VkDeviceQueueCreateInfo queueCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = graphicsQueueFamilyIndex,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
	};
	const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkDeviceCreateInfo deviceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCreateInfo,
		.enabledExtensionCount = 1,
		.ppEnabledExtensionNames = deviceExtensions
	};
	vkCreateDevice(g_ctx.physicalDevice, &deviceCreateInfo, NULL, &g_ctx.device);
	vkGetDeviceQueue(g_ctx.device, 0, 0, &g_ctx.graphicsQueue);
	int fbW, fbH;
	glfwGetFramebufferSize(g_ctx.window, &fbW, &fbH);
	VkSwapchainCreateInfoKHR swapchainInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = g_ctx.surface,
		.minImageCount = 2,
		.imageFormat = VK_FORMAT_B8G8R8A8_UNORM,
		.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		.imageExtent = {(uint32_t)fbW, (uint32_t)fbH},
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR
	};
	vkCreateSwapchainKHR(g_ctx.device, &swapchainInfo, NULL, &g_ctx.swapchain);
	vkGetSwapchainImagesKHR(g_ctx.device, g_ctx.swapchain, &g_ctx.imageCount, NULL);
	g_ctx.swapchainImages = malloc(sizeof(VkImage) * g_ctx.imageCount);
	vkGetSwapchainImagesKHR(g_ctx.device, g_ctx.swapchain, &g_ctx.imageCount, g_ctx.swapchainImages);
	g_ctx.swapchainImageViews = malloc(sizeof(VkImageView) * g_ctx.imageCount);
	for (uint32_t i = 0; i < g_ctx.imageCount; i++) {
		VkImageViewCreateInfo viewInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = g_ctx.swapchainImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_B8G8R8A8_UNORM,
			.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
		};
		vkCreateImageView(g_ctx.device, &viewInfo, NULL, &g_ctx.swapchainImageViews[i]);
	}
	createDepthResources(fbW, fbH);
	VkAttachmentDescription colorAttachment = {
		.format = VK_FORMAT_B8G8R8A8_UNORM,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};
	VkAttachmentReference colorAttachmentRef = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};
	VkAttachmentDescription depthAttachment = {
		.format = VK_FORMAT_D32_SFLOAT,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	VkAttachmentReference depthAttachmentRef = {
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
	VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
		.pDepthStencilAttachment = &depthAttachmentRef
	};
	VkSubpassDependency dependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.srcAccessMask = 0,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
	};
	VkAttachmentDescription attachments[2] = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 2,
		.pAttachments = attachments,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dependency
	};
	vkCreateRenderPass(g_ctx.device, &renderPassInfo, NULL, &g_ctx.renderPass);
	VkPushConstantRange pushConstantRange = {
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = 16
	};
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 0,
		.pSetLayouts = NULL,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pushConstantRange
	};
	vkCreatePipelineLayout(g_ctx.device, &pipelineLayoutInfo, NULL, &g_ctx.pipelineLayout);
	const uint32_t vert_code[] = {
		0x07230203,0x00010000,0x000d000b,0x00000040,0x00000000,0x00020011,0x00000001,0x0006000b,0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
		0x0009000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000c,0x00000032,0x0000003c,0x0000003e,0x00030003,0x00000002,0x000001c2,0x000a0004,0x475f4c47,0x4c474f4f,0x70635f45,
		0x74735f70,0x5f656c79,0x656e696c,0x7269645f,0x69746365,0x00006576,0x00080004,0x475f4c47,0x4c474f4f,0x6e695f45,0x64756c63,0x69645f65,0x74636572,0x00657669,0x00040005,0x00000004,
		0x6e69616d,0x00000000,0x00050005,0x00000009,0x616e6966,0x736f506c,0x00000000,0x00040005,0x0000000c,0x6f506e69,0x00000073,0x00040005,0x0000000f,0x68737550,0x00000000,0x00050006,
		0x0000000f,0x00000000,0x7366666f,0x00007465,0x00060006,0x0000000f,0x00000001,0x65726373,0x65526e65,0x00000073,0x00040005,0x00000011,0x68737570,0x00000000,0x00060005,0x00000022,
		0x6d726f6e,0x7a696c61,0x65446465,0x00687470,0x00060005,0x00000030,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000030,0x00000000,0x505f6c67,0x7469736f,0x006e6f69,
		0x00070006,0x00000030,0x00000001,0x505f6c67,0x746e696f,0x657a6953,0x00000000,0x00070006,0x00000030,0x00000002,0x435f6c67,0x4470696c,0x61747369,0x0065636e,0x00070006,0x00000030,
		0x00000003,0x435f6c67,0x446c6c75,0x61747369,0x0065636e,0x00030005,0x00000032,0x00000000,0x00050005,0x0000003c,0x67617266,0x6f6c6f43,0x00000072,0x00040005,0x0000003e,0x6f436e69,
		0x00726f6c,0x00040047,0x0000000c,0x0000001e,0x00000000,0x00030047,0x0000000f,0x00000002,0x00050048,0x0000000f,0x00000000,0x00000023,0x00000000,0x00050048,0x0000000f,0x00000001,
		0x00000023,0x00000008,0x00030047,0x00000030,0x00000002,0x00050048,0x00000030,0x00000000,0x0000000b,0x00000000,0x00050048,0x00000030,0x00000001,0x0000000b,0x00000001,0x00050048,
		0x00000030,0x00000002,0x0000000b,0x00000003,0x00050048,0x00000030,0x00000003,0x0000000b,0x00000004,0x00040047,0x0000003c,0x0000001e,0x00000000,0x00040047,0x0000003e,0x0000001e,
		0x00000001,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000002,0x00040020,0x00000008,0x00000007,
		0x00000007,0x00040017,0x0000000a,0x00000006,0x00000003,0x00040020,0x0000000b,0x00000001,0x0000000a,0x0004003b,0x0000000b,0x0000000c,0x00000001,0x0004001e,0x0000000f,0x00000007,
		0x00000007,0x00040020,0x00000010,0x00000009,0x0000000f,0x0004003b,0x00000010,0x00000011,0x00000009,0x00040015,0x00000012,0x00000020,0x00000001,0x0004002b,0x00000012,0x00000013,
		0x00000000,0x00040020,0x00000014,0x00000009,0x00000007,0x0004002b,0x00000012,0x00000018,0x00000001,0x0004002b,0x00000006,0x0000001b,0x3f000000,0x0004002b,0x00000006,0x0000001e,
		0x3f800000,0x00040020,0x00000021,0x00000007,0x00000006,0x00040015,0x00000023,0x00000020,0x00000000,0x0004002b,0x00000023,0x00000024,0x00000002,0x00040020,0x00000025,0x00000001,
		0x00000006,0x0004002b,0x00000006,0x00000028,0x447a0000,0x0004002b,0x00000006,0x0000002b,0x00000000,0x00040017,0x0000002d,0x00000006,0x00000004,0x0004002b,0x00000023,0x0000002e,
		0x00000001,0x0004001c,0x0000002f,0x00000006,0x0000002e,0x0006001e,0x00000030,0x0000002d,0x00000006,0x0000002f,0x0000002f,0x00040020,0x00000031,0x00000003,0x00000030,0x0004003b,
		0x00000031,0x00000032,0x00000003,0x0004002b,0x00000023,0x00000033,0x00000000,0x00040020,0x0000003a,0x00000003,0x0000002d,0x0004003b,0x0000003a,0x0000003c,0x00000003,0x00040020,
		0x0000003d,0x00000001,0x0000002d,0x0004003b,0x0000003d,0x0000003e,0x00000001,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,0x00000005,0x0004003b,0x00000008,
		0x00000009,0x00000007,0x0004003b,0x00000021,0x00000022,0x00000007,0x0004003d,0x0000000a,0x0000000d,0x0000000c,0x0007004f,0x00000007,0x0000000e,0x0000000d,0x0000000d,0x00000000,
		0x00000001,0x00050041,0x00000014,0x00000015,0x00000011,0x00000013,0x0004003d,0x00000007,0x00000016,0x00000015,0x00050081,0x00000007,0x00000017,0x0000000e,0x00000016,0x00050041,
		0x00000014,0x00000019,0x00000011,0x00000018,0x0004003d,0x00000007,0x0000001a,0x00000019,0x0005008e,0x00000007,0x0000001c,0x0000001a,0x0000001b,0x00050088,0x00000007,0x0000001d,
		0x00000017,0x0000001c,0x00050050,0x00000007,0x0000001f,0x0000001e,0x0000001e,0x00050083,0x00000007,0x00000020,0x0000001d,0x0000001f,0x0003003e,0x00000009,0x00000020,0x00050041,
		0x00000025,0x00000026,0x0000000c,0x00000024,0x0004003d,0x00000006,0x00000027,0x00000026,0x00050088,0x00000006,0x00000029,0x00000027,0x00000028,0x0003003e,0x00000022,0x00000029,
		0x0004003d,0x00000006,0x0000002a,0x00000022,0x0008000c,0x00000006,0x0000002c,0x00000001,0x0000002b,0x0000002a,0x0000002b,0x0000001e,0x0003003e,0x00000022,0x0000002c,0x00050041,
		0x00000021,0x00000034,0x00000009,0x00000033,0x0004003d,0x00000006,0x00000035,0x00000034,0x00050041,0x00000021,0x00000036,0x00000009,0x0000002e,0x0004003d,0x00000006,0x00000037,
		0x00000036,0x0004003d,0x00000006,0x00000038,0x00000022,0x00070050,0x0000002d,0x00000039,0x00000035,0x00000037,0x00000038,0x0000001e,0x00050041,0x0000003a,0x0000003b,0x00000032,
		0x00000013,0x0003003e,0x0000003b,0x00000039,0x0004003d,0x0000002d,0x0000003f,0x0000003e,0x0003003e,0x0000003c,0x0000003f,0x000100fd,0x00010038
	};
	VkShaderModuleCreateInfo vertInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = sizeof(vert_code),
		.pCode = vert_code
	};
	VkShaderModule vertModule = VK_NULL_HANDLE;
	vkCreateShaderModule(g_ctx.device, &vertInfo, NULL, &vertModule);
	const uint32_t frag_code[] = {
		0x07230203,0x00010000,0x000d000b,0x0000000d,0x00000000,0x00020011,0x00000001,0x0006000b,0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
		0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000b,0x00030010,0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x000a0004,0x475f4c47,0x4c474f4f,
		0x70635f45,0x74735f70,0x5f656c79,0x656e696c,0x7269645f,0x69746365,0x00006576,0x00080004,0x475f4c47,0x4c474f4f,0x6e695f45,0x64756c63,0x69645f65,0x74636572,0x00657669,0x00040005,
		0x00000004,0x6e69616d,0x00000000,0x00050005,0x00000009,0x4374756f,0x726f6c6f,0x00000000,0x00050005,0x0000000b,0x67617266,0x6f6c6f43,0x00000072,0x00040047,0x00000009,0x0000001e,
		0x00000000,0x00040047,0x0000000b,0x0000001e,0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,
		0x00000004,0x00040020,0x00000008,0x00000003,0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040020,0x0000000a,0x00000001,0x00000007,0x0004003b,0x0000000a,0x0000000b,
		0x00000001,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,0x00000005,0x0004003d,0x00000007,0x0000000c,0x0000000b,0x0003003e,0x00000009,0x0000000c,0x000100fd,
		0x00010038
	};
	VkShaderModuleCreateInfo fragInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = sizeof(frag_code),
		.pCode = frag_code
	};
	VkShaderModule fragModule = VK_NULL_HANDLE;
	vkCreateShaderModule(g_ctx.device, &fragInfo, NULL, &fragModule);
	VkPipelineShaderStageCreateInfo shaderStages[2] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertModule,
			.pName = "main"
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = fragModule,
			.pName = "main"
		}
	};
	VkVertexInputBindingDescription bindingDesc = {
		.binding = 0,
		.stride = sizeof(QGPU_Vertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
	};
	VkVertexInputAttributeDescription attribDescs[2] = {
		{
			.binding = 0,
			.location = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(QGPU_Vertex, pos)
		},
		{
			.binding = 0,
			.location = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(QGPU_Vertex, color)
		}
	};
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &bindingDesc,
		.vertexAttributeDescriptionCount = 2,
		.pVertexAttributeDescriptions = attribDescs
	};
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
	};
	VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1
	};
	VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.lineWidth = 1.0f,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE
	};
	VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
	};
	VkPipelineDepthStencilStateCreateInfo depthStencil = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE
	};
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD
	};
	VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment
	};
	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamicStates
	};
	VkGraphicsPipelineCreateInfo pipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = &depthStencil,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicState,
		.layout = g_ctx.pipelineLayout,
		.renderPass = g_ctx.renderPass,
		.subpass = 0
	};
	vkCreateGraphicsPipelines(g_ctx.device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &g_ctx.graphicsPipeline);
	vkDestroyShaderModule(g_ctx.device, fragModule, NULL);
	vkDestroyShaderModule(g_ctx.device, vertModule, NULL);
	g_ctx.swapchainFramebuffers = malloc(sizeof(VkFramebuffer) * g_ctx.imageCount);
	for (uint32_t i = 0; i < g_ctx.imageCount; i++) {
		VkImageView attachments[2] = { g_ctx.swapchainImageViews[i], g_ctx.depthImageView };
		VkFramebufferCreateInfo fbInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = g_ctx.renderPass,
			.attachmentCount = 2,
			.pAttachments = attachments,
			.width = (uint32_t)fbW,
			.height = (uint32_t)fbH,
			.layers = 1
		};
		vkCreateFramebuffer(g_ctx.device, &fbInfo, NULL, &g_ctx.swapchainFramebuffers[i]);
	}
	VkCommandPoolCreateInfo poolInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.queueFamilyIndex = graphicsQueueFamilyIndex,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
	};
	vkCreateCommandPool(g_ctx.device, &poolInfo, NULL, &g_ctx.commandPool);
	VkCommandBufferAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = g_ctx.commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};
	vkAllocateCommandBuffers(g_ctx.device, &allocInfo, &g_ctx.currentCmd);
	createBuffer(sizeof(QGPU_Vertex) * MAX_VERTICES, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g_ctx.vertexBuffer, &g_ctx.vertexBufferMemory);
	createBuffer(sizeof(uint32_t) * MAX_VERTICES * 3, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g_ctx.indexBuffer, &g_ctx.indexBufferMemory);
	vkMapMemory(g_ctx.device, g_ctx.vertexBufferMemory, 0, sizeof(QGPU_Vertex) * MAX_VERTICES, 0, &g_ctx.mappedVertexBuffer);
	vkMapMemory(g_ctx.device, g_ctx.indexBufferMemory, 0, sizeof(uint32_t) * (uint32_t)(MAX_VERTICES * 1.5f), 0, &g_ctx.mappedIndexBuffer);
	VkSemaphoreCreateInfo semaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	vkCreateSemaphore(g_ctx.device, &semaphoreInfo, NULL, &g_ctx.imageAvailableSemaphore);
	vkCreateSemaphore(g_ctx.device, &semaphoreInfo, NULL, &g_ctx.renderFinishedSemaphore);
	memset(g_ctx.lastKeyState, 0, sizeof(g_ctx.lastKeyState));
	memset(g_ctx.lastMouseState, 0, sizeof(g_ctx.lastMouseState));
	qgSetStyle(BOLD);
	if (_showBanner) printBanner();
	if (_madeWith) printMadeWith();
	if (_showInfo) printInfo();
	if (_showColors) printColors();
	qgSetStyle(REGULAR);
	printf("\n");
	inInit = 1;
	if (initFunc) initFunc();
	inInit = 0;
	while (!glfwWindowShouldClose(g_ctx.window)) {
		for (int i = 0; i < GLFW_KEY_LAST; i++) g_ctx.lastKeyState[i] = glfwGetKey(g_ctx.window, i);
		for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++) g_ctx.lastMouseState[i] = glfwGetMouseButton(g_ctx.window, i);
		glfwPollEvents();
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(g_ctx.device, g_ctx.swapchain, UINT64_MAX, g_ctx.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) { continue; } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { continue; }
		g_ctx.currentVOffset = 0;
		g_ctx.currentIOffset = 0;
		lightCount = 0;
		qgResetRotation();
		vkResetCommandBuffer(g_ctx.currentCmd, 0);
		VkCommandBufferBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};
		vkBeginCommandBuffer(g_ctx.currentCmd, &beginInfo);
		VkClearValue clearValues[2] = {
			{{{backgroundR, backgroundG, backgroundB, 1.0f}}},
			{.depthStencil = {0, 0}}
		};
		VkRenderPassBeginInfo renderPassInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = g_ctx.renderPass,
			.framebuffer = g_ctx.swapchainFramebuffers[imageIndex],
			.renderArea = {{0, 0}, {(uint32_t)width, (uint32_t)height}},
			.clearValueCount = 2,
			.pClearValues = clearValues
		};
		vkCmdBeginRenderPass(g_ctx.currentCmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(g_ctx.currentCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g_ctx.graphicsPipeline);
		VkViewport viewport = {0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f};
		VkRect2D scissor = {{0, 0}, {(uint32_t)width, (uint32_t)height}};
		vkCmdSetViewport(g_ctx.currentCmd, 0, 1, &viewport);
		vkCmdSetScissor(g_ctx.currentCmd, 0, 1, &scissor);
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(g_ctx.currentCmd, 0, 1, &g_ctx.vertexBuffer, offsets);
		vkCmdBindIndexBuffer(g_ctx.currentCmd, g_ctx.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		if (updateFunc) updateFunc();
		render();
		vkCmdEndRenderPass(g_ctx.currentCmd);
		vkEndCommandBuffer(g_ctx.currentCmd);
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		VkSubmitInfo submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &g_ctx.imageAvailableSemaphore,
			.pWaitDstStageMask = waitStages,
			.commandBufferCount = 1,
			.pCommandBuffers = &g_ctx.currentCmd,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &g_ctx.renderFinishedSemaphore
		};
		if (vkQueueSubmit(g_ctx.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) printf("Quene submit error!\n");
		VkPresentInfoKHR presentInfo = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &g_ctx.renderFinishedSemaphore,
			.swapchainCount = 1,
			.pSwapchains = &g_ctx.swapchain,
			.pImageIndices = &imageIndex
		};
		vkQueuePresentKHR(g_ctx.graphicsQueue, &presentInfo);
		vkDeviceWaitIdle(g_ctx.device);
		double currentTime = glfwGetTime();
		frameCount++;
		if (currentTime - lastTime >= 0.5) {
			currentFPS = (float)frameCount / (float)(currentTime - lastTime);
			frameCount = 0;
			lastTime = currentTime;
		}
	}
	free(sphereVertexIndices);
	vkDeviceWaitIdle(g_ctx.device);
	vkUnmapMemory(g_ctx.device, g_ctx.vertexBufferMemory);
	vkUnmapMemory(g_ctx.device, g_ctx.indexBufferMemory);
	vkDestroySemaphore(g_ctx.device, g_ctx.renderFinishedSemaphore, NULL);
	vkDestroySemaphore(g_ctx.device, g_ctx.imageAvailableSemaphore, NULL);
	vkDestroyBuffer(g_ctx.device, g_ctx.indexBuffer, NULL);
	vkFreeMemory(g_ctx.device, g_ctx.indexBufferMemory, NULL);
	vkDestroyBuffer(g_ctx.device, g_ctx.vertexBuffer, NULL);
	vkFreeMemory(g_ctx.device, g_ctx.vertexBufferMemory, NULL);
	vkDestroyCommandPool(g_ctx.device, g_ctx.commandPool, NULL);
	for (uint32_t i = 0; i < g_ctx.imageCount; i++) {
		vkDestroyFramebuffer(g_ctx.device, g_ctx.swapchainFramebuffers[i], NULL);
		vkDestroyImageView(g_ctx.device, g_ctx.swapchainImageViews[i], NULL);
	}
	free(g_ctx.swapchainFramebuffers);
	free(g_ctx.swapchainImageViews);
	free(g_ctx.swapchainImages);
	vkDestroyPipeline(g_ctx.device, g_ctx.graphicsPipeline, NULL);
	vkDestroyPipelineLayout(g_ctx.device, g_ctx.pipelineLayout, NULL);
	vkDestroyRenderPass(g_ctx.device, g_ctx.renderPass, NULL);
	vkDestroySwapchainKHR(g_ctx.device, g_ctx.swapchain, NULL);
	vkDestroyImageView(g_ctx.device, g_ctx.depthImageView, NULL);
	vkDestroyImage(g_ctx.device, g_ctx.depthImage, NULL);
	vkFreeMemory(g_ctx.device, g_ctx.depthImageMemory, NULL);
	vkDestroyDevice(g_ctx.device, NULL);
	vkDestroySurfaceKHR(g_ctx.instance, g_ctx.surface, NULL);
	vkDestroyInstance(g_ctx.instance, NULL);
	glfwDestroyWindow(g_ctx.window);
	glfwTerminate();
}
float qgGetFPS() { return currentFPS; }
// ========================================================================================================================================================================
// ===== DRAWING ==========================================================================================================================================================
// ========================================================================================================================================================================
void qgSetBackground(float r, float g, float b) {
	backgroundR = r;
	backgroundG = g;
	backgroundB = b;
}
// ===== ROTATION
void qgSetRotationPivot(float x, float y, float z) {
	g_ctx.pivotX = x;
	g_ctx.pivotY = y;
	g_ctx.pivotZ = z;
}
static float rndToNrm(float v) { return v - ((int)(v / 360.0f) * 360.0f); }
void qgSetRotation(float rx, float ry, float rz) {
	g_ctx.rotX = rndToNrm(rx);
	g_ctx.rotY = rndToNrm(ry);
	g_ctx.rotZ = rndToNrm(rz);
	g_ctx.hasRotation = 1;
}
void qgResetRotation() {
	g_ctx.pivotX = 0.0f;
	g_ctx.pivotY = 0.0f;
	g_ctx.pivotZ = 0.0f;
	g_ctx.rotX = 0.0f;
	g_ctx.rotY = 0.0f;
	g_ctx.rotZ = 0.0f;
	g_ctx.hasRotation = 0;
}
// ===== VERTICES & INDICES ===============================================================================================================================================
static int qLightOn = 1;
void qgSetRenderType(int type) {
	switch (type) {
		case QGPU_RENDER_TYPE_NO_LIGHT: qLightOn = 0; break;
		case QGPU_RENDER_TYPE_LIGHT: qLightOn = 1; break;
	}
}
uint32_t qgAddVertex(float x, float y, float z, float r, float g, float b, float a) {
	if (g_ctx.currentVOffset >= MAX_VERTICES) { qgWarn("Cannot add new vertex\n"); return -1; }
	transformPoint(&x, &y, &z);
	QGPU_Vertex* vDst = (QGPU_Vertex*)g_ctx.mappedVertexBuffer + g_ctx.currentVOffset;
	float m = qLightOn ? getLight(x, y, z) : 1;
	vDst->pos[0] = x;
	vDst->pos[1] = -y;
	vDst->pos[2] = z;
	vDst->color[0] = qclampf(r*m, 0, 1);
	vDst->color[1] = qclampf(g*m, 0, 1);
	vDst->color[2] = qclampf(b*m, 0, 1);
	vDst->color[3] = qclampf(a, 0, 1);
	g_ctx.currentVOffset++;
	return g_ctx.currentVOffset-1;
}
void qgAddIndex(uint32_t index) {
	if (g_ctx.currentIOffset >= (uint32_t)(MAX_VERTICES * 3)) { qgWarn("Cannot add new index\n"); return; }
	uint32_t* iDst = (uint32_t*)g_ctx.mappedIndexBuffer + g_ctx.currentIOffset;
	*iDst = index;
	g_ctx.currentIOffset++;
}
void qgAddGeometry(QGPU_Vertex* verts, uint32_t vCount, uint32_t* indices, uint32_t iCount) {
	if (vCount <= 0 || iCount <= 0) { qgWarn("The number of vertices or indices is less than or equal to 0\n"); return; }
	uint32_t baseVertexOffset = g_ctx.currentVOffset;
	for (uint32_t i = 0; i < vCount; i++) qgAddVertex(verts[i].pos[0], verts[i].pos[1], verts[i].pos[2], verts[i].color[0], verts[i].color[1], verts[i].color[2], verts[i].color[3]);
	for (uint32_t i = 0; i < iCount; i++) qgAddIndex(indices[i] + baseVertexOffset);
}
// ===== LIGHTS ===========================================================================================================================================================
void qgAddLight(float x, float y, float z, float range, float intense) {
	if (lightCount >= MAX_LIGHTS) { qgWarn("Cannot add new light\n"); return; }
	int l = lightCount * 5;
	lights[ l ] = x;
	lights[l+1] = y;
	lights[l+2] = z;
	lights[l+3] = range;
	lights[l+4] = intense;
	lightCount++;
}
// ===== READY 2D ==========================================================================================================================================================
void qgAddTriangle(float p1x, float p1y, float p1z, float p2x, float p2y, float p2z, float p3x, float p3y, float p3z, float r, float g, float b, float a) {
	qgAddIndex(qgAddVertex(p1x, p1y, p1z, r, g, b, a));
	qgAddIndex(qgAddVertex(p2x, p2y, p2z, r, g, b, a));
	qgAddIndex(qgAddVertex(p3x, p3y, p3z, r, g, b, a));
}
void qgAddRect(float px, float py, float pz, float sx, float sy, float r, float g, float b, float a) {
	float x = sx / 2, y = sy / 2, v[4] = {
		px - x, px + x,
		py - y, py + y
	};
	qgAddTriangle(v[0], v[3], pz, v[1], v[3], pz, v[1], v[2], pz, r, g, b, a);
	qgAddTriangle(v[0], v[3], pz, v[1], v[2], pz, v[0], v[2], pz, r, g, b, a);
}
void qgAddCircle(float px, float py, float pz, int segments, float radius, float r, float g, float b, float a) {
	if (segments < 3) return;
	float angleStep = (2.0f * PI) / segments;
	uint32_t center = qgAddVertex(px, py, pz, r, g, b, a),
	first = qgAddVertex(px + radius, py, pz, r, g, b, a),
	last = first;
	for (int i = 1; i < segments; i++) {
		float currentAngle = angleStep * i, cx = qCos(currentAngle) * radius, cy = qSin(currentAngle) * radius;
		qgAddIndex(center);
		uint32_t v = qgAddVertex(px + cx, py + cy, pz, r, g, b, a);
		qgAddIndex(v);
		qgAddIndex(last);
		last = v;
	}
	qgAddIndex(center);
	qgAddIndex(first);
	qgAddIndex(last);
}
// ===== READY 3D ==========================================================================================================================================================
void qgAddBox(float px, float py, float pz, float sx, float sy, float sz, float r, float g, float b, float a) {
	float x = sx / 2, y = sy / 2, z = sz / 2, v[24] = {
		px-x, py+y, pz+z,
		px+x, py+y, pz+z,
		px+x, py-y, pz+z,
		px-x, py-y, pz+z,
		px-x, py+y, pz-z,
		px+x, py+y, pz-z,
		px+x, py-y, pz-z,
		px-x, py-y, pz-z
	};
	qgAddTriangle(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], r, g, b, a);
	qgAddTriangle(v[0], v[1], v[2], v[6], v[7], v[8], v[9], v[10], v[11], r, g, b, a);
	qgAddTriangle(v[15], v[16], v[17], v[12], v[13], v[14], v[21], v[22], v[23], r, g, b, a);
	qgAddTriangle(v[15], v[16], v[17], v[21], v[22], v[23], v[18], v[19], v[20], r, g, b, a);
	qgAddTriangle(v[3], v[4], v[5], v[15], v[16], v[17], v[18], v[19], v[20], r, g, b, a);
	qgAddTriangle(v[3], v[4], v[5], v[18], v[19], v[20], v[6], v[7], v[8], r, g, b, a);
	qgAddTriangle(v[12], v[13], v[14], v[0], v[1], v[2], v[9], v[10], v[11], r, g, b, a);
	qgAddTriangle(v[12], v[13], v[14], v[9], v[10], v[11], v[21], v[22], v[23], r, g, b, a);
	qgAddTriangle(v[12], v[13], v[14], v[15], v[16], v[17], v[3], v[4], v[5], r, g, b, a);
	qgAddTriangle(v[12], v[13], v[14], v[3], v[4], v[5], v[0], v[1], v[2], r, g, b, a);
	qgAddTriangle(v[9], v[10], v[11], v[6], v[7], v[8], v[18], v[19], v[20], r, g, b, a);
	qgAddTriangle(v[9], v[10], v[11], v[18], v[19], v[20], v[21], v[22], v[23], r, g, b, a);
}
void qgAddSphere(float px, float py, float pz, float radius, int rings, int sectors, float r, float g, float b, float a) {
	if (rings < 2 || sectors < 3) return;
	if (!sphereVertexIndices) return;
	for (int i = 0; i <= rings; ++i) {
		float v = (float)i / (float)rings;
		float phi = v * PI;
		float yCost = qCos(phi);
		float ySint = qSin(phi);
		for (int j = 0; j <= sectors; ++j) {
			float u = (float)j / (float)sectors;
			float theta = u * (2.0f * PI);
			float x = px + radius * ySint * qCos(theta);
			float y = py + radius * yCost;
			float z = pz + radius * ySint * qSin(theta);
			sphereVertexIndices[i * (sectors + 1) + j] = qgAddVertex(x, y, z, r, g, b, a);
		}
	}
	for (int i = 0; i < rings; ++i) {
		for (int j = 0; j < sectors; ++j) {
			uint32_t first = i * (sectors + 1) + j;
			uint32_t second = first + sectors + 1;
			qgAddIndex(sphereVertexIndices[first]);
			qgAddIndex(sphereVertexIndices[second]);
			qgAddIndex(sphereVertexIndices[first + 1]);
			qgAddIndex(sphereVertexIndices[first + 1]);
			qgAddIndex(sphereVertexIndices[second]);
			qgAddIndex(sphereVertexIndices[second + 1]);
		}
	}
}
// ========================================================================================================================================================================
// ===== TEXT =============================================================================================================================================================
// ========================================================================================================================================================================
static float qFontSize = 16, qFontR = 1, qFontG = 1, qFontB = 1, qFontA = 1;
static int qFontStyle = QGPU_FONT_STYLE_REGULAR;
static uint8_t cti(char c) {
	switch (c) {
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case 'A': return 11;
		case 'B': return 12;
		case 'C': return 13;
		case 'D': return 14;
		case 'E': return 15;
		case 'F': return 16;
		case 'G': return 17;
		case 'H': return 18;
	}
	return 0;
}
void qgConvertFont(const char* pathQFR, const char* pathQF) {
	if (!inInit) { qgWarn("Couldn't convert font in Update function! Please convert it in Init function.\n"); return; }
	int l = len(pathQFR) - 1;
	if (l < 4) return;
	if (pathQFR[l - 3] != '.' || pathQFR[l - 2] != 'q' || pathQFR[l - 1] != 'f' || pathQFR[l] != 'r') qgError("The file does not have the .qfr extension!\n");
	l = len(pathQF) - 1;
	if (l < 3) return;
	if (pathQF[l - 2] != '.' || pathQF[l - 1] != 'q' || pathQF[l] != 'f') qgError("The file does not have the .qf extension!\n");
	FILE *qfr = fopen(pathQFR, "r");
	if (!qfr) qgError("Failed to open qfr file! (%s)\n", pathQFR);
	FILE *qf = fopen(pathQF, "wb");
	if (!qf) qgError("Failed to create qf file! (%s)\n", pathQF);
	char line[84];
	uint16_t len = 0;
	fwrite(&len, sizeof(uint16_t), 1, qf);
	while (fgets(line, sizeof(line), qfr)) {
		if (line[81] == '\n') line[81] = '\0';
		if (line[0] == '/') continue;
		const char code[5] = {line[0], line[1], line[2], line[3], '\0'};
		uint16_t c = (uint16_t)strtol(code, NULL, 16);
		fwrite(&c, sizeof(uint16_t), 1, qf);
		int8_t l[qFontY][qFontMax];
		for (int i = 0; i < qFontY; i++) {
			for (int j = 0; j < qFontMax; j++) {
				uint8_t x = cti(line[5+(7*i)+j]);
				if (x == 0 && j != 0) l[i][j - 1] = -l[i][j - 1];
				l[i][j] = x;
			}
		}
		fwrite(l, sizeof(int8_t), qFontY * qFontMax, qf);
		len++;
	}
	fseek(qf, 0, SEEK_SET);
	fwrite(&len, sizeof(uint16_t), 1, qf);
	fclose(qfr);
	fclose(qf);
}
void qgLoadFont(const char* path) {
	if (!inInit) { qgWarn("Couldn't load font in Update function! Please load it in Init function.\n"); return; }
	int l = len(path) - 1;
	if (l < 3) return;
	if (path[l - 2] != '.' || path[l - 1] != 'q' || path[l] != 'f') qgError("The file does not have the .qf extension!\n");
	FILE *qf = fopen(path, "rb");
	if (!qf) qgError("Failed to open qf file! (%s)\n", path);
	uint16_t len = 0;
	fread(&len, sizeof(uint16_t), 1, qf);
	if (len == 0) return;
	for (uint16_t i = 0; i < len; i++) {
		uint16_t c = 0;
		fread(&c, sizeof(uint16_t), 1, qf);
		fread(newChars[c].data, sizeof(int8_t), qFontY * qFontMax, qf);
	}
	qgLog("Added a %i new chars!\n", len);
	fclose(qf);
}
void qgSetFontData(float fontSize, int style, float r, float g, float b, float a) {
	qFontSize = fontSize;
	qFontStyle = style;
	qFontR = r;
	qFontG = g;
	qFontB = b;
	qFontA = a;
}
static qgChar qFont[128] = {
// ===== ! " # $ % & ' ( ) * + , - . / ====================================================================================================================================
	['!'] = (qgChar){.data = {{0}, {0}, {0}, {3, -12}, {3, -12}, {3, -12}, {3, -12}, {3, -12}, {0}, {3, -12}, {0}}},
	['"'] = (qgChar){.data = {{0}, {0}, {0}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {0}, {0}, {0}, {0}, {0}}},
	['#'] = (qgChar){.data = {{0}, {0}, {0}, {2, 11, 2, -11}, {2, 11, 2, -11}, {1, -16}, {2, 11, 2, -11}, {1, -16}, {2, 11, 2, -11}, {2, 11, 2, -11}, {0}}},
	['$'] = (qgChar){.data = {{0}, {0}, {0}, {3, -12}, {2, -15}, {1, -12}, {2, -14}, {5, -12}, {1, -15}, {3, -12}, {0}}},
	['%'] = (qgChar){.data = {{0}, {0}, {0}, {1, 12, 3, -11}, {1, 12, 2, -12}, {4, -12}, {3, -12}, {2, -12}, {1, 12, 2, -12}, {1, 11, 3, -12}, {0}}},
	['&'] = (qgChar){.data = {{0}, {0}, {0}, {2, -13}, {1, 12, 1, -12}, {2, -13}, {1, 13, 1, -12}, {12, 1, -13}, {12, 2, -12}, {1, 13, 1, -12}, {0}}},
	['\''] = (qgChar){.data = {{0}, {0}, {0}, {3, -12}, {3, -12}, {2, -12}, {0}, {0}, {0}, {0}, {0}}},
	['('] = (qgChar){.data = {{0}, {0}, {0}, {4, -12}, {3, -12}, {2, -12}, {2, -12}, {2, -12}, {3, -12}, {4, -12}, {0}}},
	[')'] = (qgChar){.data = {{0}, {0}, {0}, {2, -12}, {3, -12}, {4, -12}, {4, -12}, {4, -12}, {3, -12}, {2, -12}, {0}}},
	['*'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {3, 11, 1, -11}, {4, -11}, {2, -15}, {4, -11}, {3, 11, 1, -11}, {0}, {0}}},
	['+'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {3, -12}, {3, -12}, {1, -16}, {3, -12}, {3, -12}, {0}, {0}}},
	[','] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {3, -12}, {3, -12}, {2, -12}}},
	['-'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {0}, {1, -16}, {0}, {0}, {0}, {0}}},
	['.'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {3, -12}, {3, -12}, {0}}},
	['/'] = (qgChar){.data = {{0}, {0}, {0}, {6, -11}, {5, -12}, {4, -12}, {3, -12}, {2, -12}, {1, -12}, {1, -11}, {0}}},
// ===== : ; < = > ? @ ====================================================================================================================================================
	[':'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {3, -12}, {3, -12}, {0}, {3, -12}, {3, -12}, {0}, {0}}},
	[';'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {3, -12}, {3, -12}, {0}, {3, -12}, {3, -12}, {2, -12}, {0}}},
	['<'] = (qgChar){.data = {{0}, {0}, {0}, {4, -12}, {3, -12}, {2, -12}, {1, -12}, {2, -12}, {3, -12}, {4, -12}, {0}}},
	['='] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {1, -16}, {0}, {1, -16}, {0}, {0}, {0}}},
	['>'] = (qgChar){.data = {{0}, {0}, {0}, {2, -12}, {3, -12}, {4, -12}, {5, -12}, {4, -12}, {3, -12}, {2, -12}, {0}}},
	['?'] = (qgChar){.data = {{0}, {0}, {0}, {2, -14}, {1, 12, 2, -12}, {5, -12}, {4, -12}, {3, -12}, {0}, {3, -12}, {0}}},
	['@'] = (qgChar){.data = {{0}, {0}, {0}, {2, -14}, {1, 11, 4, -11}, {11, 2, 13, 1, -11}, {11, 1, 11, 4, -11}, {11, 1, 11, 4, -11}, {11, 2, 13, 1, -11}, {1, -11}, {2, -15}}},
// ===== [ \ ] ^ _ ` ======================================================================================================================================================
	['['] = (qgChar){.data = {{0}, {0}, {0}, {2, -14}, {2, -12}, {2, -12}, {2, -12}, {2, -12}, {2, -12}, {2, -14}, {0}}},
	['\\'] = (qgChar){.data = {{0}, {0}, {0}, {1, -11}, {2, -12}, {3, -12}, {4, -12}, {5, -12}, {6, -12}, {7, -11}, {0}}},
	[']'] = (qgChar){.data = {{0}, {0}, {0}, {2, -14}, {4, -12}, {4, -12}, {4, -12}, {4, -12}, {4, -12}, {2, -14}, {0}}},
	['^'] = (qgChar){.data = {{0}, {0}, {0}, {3, -12}, {2, 11, 2, -11}, {1, 11, 4, -11}, {0}, {0}, {0}, {0}, {0}}},
	['_'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {1, -16}, {0}}},
	['`'] = (qgChar){.data = {{0}, {0}, {0}, {2, -12}, {2, -12}, {3, -12}, {0}, {0}, {0}, {0}, {0}}},
// ===== { | } ~ ==========================================================================================================================================================
	['{'] = (qgChar){.data = {{0}, {0}, {0}, {4, -13}, {3, -12}, {3, -12}, {2, -12}, {3, -12}, {3, -12}, {4, -13}, {0}}},
	['|'] = (qgChar){.data = {{0}, {0}, {3, -12}, {3, -12}, {3, -12}, {3, -12}, {3, -12}, {3, -12}, {3, -12}, {3, -12}, {3, -12}}},
	['}'] = (qgChar){.data = {{0}, {0}, {0}, {1, -13}, {3, -12}, {3, -12}, {4, -12}, {3, -12}, {3, -12}, {1, -13}, {0}}},
	['~'] = (qgChar){.data = {{0}, {0}, {0}, {2, 13, 1, -11}, {1, 11, 1, -13}, {0}, {0}, {0}, {0}, {0}, {0}}},
// ===== 0 1 2 3 4 5 6 7 8 9 ==============================================================================================================================================
	['0'] = (qgChar){.data = {{0}, {0}, {0}, {2, -14}, {1, 12, 2, -12}, {1, 12, 1, -13}, {1, 13, 1, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, -14}, {0}}},
	['1'] = (qgChar){.data = {{0}, {0}, {0}, {3, -12}, {2, -13}, {3, -12}, {3, -12}, {3, -12}, {3, -12}, {2, -14}, {0}}},
	['2'] = (qgChar){.data = {{0}, {0}, {0}, {2, -14}, {1, 12, 2, -12}, {5, -12}, {4, -12}, {2, -12}, {1, -12}, {1, -16}, {0}}},
	['3'] = (qgChar){.data = {{0}, {0}, {0}, {2, -14}, {1, 12, 2, -12}, {5, -12}, {3, -13}, {5, -12}, {1, 12, 2, -12}, {2, -14}, {0}}},
	['4'] = (qgChar){.data = {{0}, {0}, {0}, {4, -12}, {3, -13}, {2, 11, 1, -12}, {1, 11, 2, -12}, {1, -16}, {4, -12}, {4, -12}, {0}}},
	['5'] = (qgChar){.data = {{0}, {0}, {0}, {1, -16}, {1, -12}, {1, -15}, {5, -12}, {5, -12}, {1, 12, 2, -12}, {2, -14}, {0}}},
	['6'] = (qgChar){.data = {{0}, {0}, {0}, {3, -13}, {2, -12}, {1, -12}, {1, -15}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, -14}, {0}}},
	['7'] = (qgChar){.data = {{0}, {0}, {0}, {1, -16}, {5, -12}, {4, -12}, {3, -12}, {2, -12}, {2, -12}, {2, -12}, {0}}},
	['8'] = (qgChar){.data = {{0}, {0}, {0}, {2, -14}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, -14}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, -14}, {0}}},
	['9'] = (qgChar){.data = {{0}, {0}, {0}, {2, -14}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, -15}, {5, -12}, {4, -12}, {2, -13}, {0}}},
// ===== A - Z ============================================================================================================================================================
	['A'] = (qgChar){.data = {{0}, {0}, {0}, {3, -12}, {2, -14}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, -16}, {1, 12, 2, -12}, {1, 12, 2, -12}, {0}}},
	['B'] = (qgChar){.data = {{0}, {0}, {0}, {1, -15}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, -15}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, -15}, {0}}},
	['C'] = (qgChar){.data = {{0}, {0}, {0}, {2, -14}, {1, 12, 2, -12}, {1, -12}, {1, -12}, {1, -12}, {1, 12, 2, -12}, {2, -14}, {0}}},
	['D'] = (qgChar){.data = {{0}, {0}, {0}, {1, -14}, {1, 12, 1, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 1, -12}, {1, -14}, {0}}},
	['E'] = (qgChar){.data = {{0}, {0}, {0}, {1, -16}, {1, -12}, {1, -12}, {1, -15}, {1, -12}, {1, -12}, {1, -16}, {0}}},
	['F'] = (qgChar){.data = {{0}, {0}, {0}, {1, -16}, {1, -12}, {1, -12}, {1, -15}, {1, -12}, {1, -12}, {1, -12}, {0}}},
	['G'] = (qgChar){.data = {{0}, {0}, {0}, {2, -14}, {1, 12, 2, -12}, {1, -12}, {1, 12, 1, -13}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, -15}, {0}}},
	['H'] = (qgChar){.data = {{0}, {0}, {0}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, -16}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {0}}},
	['I'] = (qgChar){.data = {{0}, {0}, {0}, {2, -14}, {3, -12}, {3, -12}, {3, -12}, {3, -12}, {3, -12}, {2, -14}, {0}}},
	['J'] = (qgChar){.data = {{0}, {0}, {0}, {3, -14}, {5, -12}, {5, -12}, {5, -12}, {5, -12}, {1, 12, 2, -12}, {2, -14}, {0}}},
	['K'] = (qgChar){.data = {{0}, {0}, {0}, {1, 12, 2, -12}, {1, 12, 1, -12}, {1, -14}, {1, -13}, {1, -14}, {1, 12, 1, -12}, {1, 12, 2, -12}, {0}}},
	['L'] = (qgChar){.data = {{0}, {0}, {0}, {2, -12}, {2, -12}, {2, -12}, {2, -12}, {2, -12}, {2, -12}, {2, -15}, {0}}},
	['M'] = (qgChar){.data = {{0}, {0}, {0}, {1, 12, 2, -12}, {13, 1, -13}, {12, 1, 11, 1, -12}, {12, 1, 11, 1, -12}, {12, 3, -12}, {12, 3, -12}, {12, 3, -12}, {0}}},
	['N'] = (qgChar){.data = {{0}, {0}, {0}, {1, 12, 2, -12}, {1, 13, 1, -12}, {1, -16}, {1, 12, 1, -13}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {0}}},
	['O'] = (qgChar){.data = {{0}, {0}, {0}, {2, -14}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, -14}, {0}}},
	['P'] = (qgChar){.data = {{0}, {0}, {0}, {1, -15}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, -15}, {1, -12}, {1, -12}, {1, -12}, {0}}},
	['Q'] = (qgChar){.data = {{0}, {0}, {0}, {2, -14}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, -15}, {6, -12}}},
	['R'] = (qgChar){.data = {{0}, {0}, {0}, {1, -15}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, -15}, {1, -14}, {1, 12, 1, -12}, {1, 12, 2, -12}, {0}}},
	['S'] = (qgChar){.data = {{0}, {0}, {0}, {2, -14}, {1, 12, 2, -12}, {1, -12}, {2, -14}, {5, -12}, {1, 12, 2, -12}, {2, -14}, {0}}},
	['T'] = (qgChar){.data = {{0}, {0}, {0}, {1, -16}, {3, -12}, {3, -12}, {3, -12}, {3, -12}, {3, -12}, {3, -12}, {0}}},
	['U'] = (qgChar){.data = {{0}, {0}, {0}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, -14}, {0}}},
	['V'] = (qgChar){.data = {{0}, {0}, {0}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, 11, 2, -11}, {2, 11, 2, -11}, {3, -12}, {0}}},
	['W'] = (qgChar){.data = {{0}, {0}, {0}, {12, 3, -12}, {12, 3, -12}, {12, 3, -12}, {12, 1, 11, 1, -12}, {12, 1, 11, 1, -12}, {13, 1, -14}, {1, 12, 2, -12}, {0}}},
	['X'] = (qgChar){.data = {{0}, {0}, {0}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, 11, 2, -11}, {3, -12}, {2, 11, 2, -11}, {1, 12, 2, -12}, {1, 12, 2, -12}, {0}}},
	['Y'] = (qgChar){.data = {{0}, {0}, {0}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, 11, 2, -11}, {3, -12}, {3, -12}, {3, -12}, {3, -12}, {0}}},
	['Z'] = (qgChar){.data = {{0}, {0}, {0}, {1, -16}, {5, -12}, {4, -12}, {3, -12}, {2, -12}, {1, -12}, {1, -16}, {0}}},
// ===== a - z ============================================================================================================================================================
	['a'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {2, -14}, {5, -12}, {2, -15}, {1, 12, 2, -12}, {2, -15}, {0}}},
	['b'] = (qgChar){.data = {{0}, {0}, {0}, {1, -12}, {1, -12}, {1, -15}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, -15}, {0}}},
	['c'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {2, -14}, {1, -12}, {1, -12}, {1, -12}, {2, -14}, {0}}},
	['d'] = (qgChar){.data = {{0}, {0}, {0}, {5, -12}, {5, -12}, {2, -15}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, -15}, {0}}},
	['e'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {2, -14}, {1, 12, 2, -12}, {1, -16}, {1, -12}, {2, -14}, {0}}},
	['f'] = (qgChar){.data = {{0}, {0}, {0}, {3, -13}, {2, 12, 1, -12}, {2, -12}, {1, -15}, {2, -12}, {2, -12}, {2, -12}, {0}}},
	['g'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {2, -15}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, -15}, {5, -12}, {1, -15}}},
	['h'] = (qgChar){.data = {{0}, {0}, {0}, {1, -12}, {1, -12}, {1, -15}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {0}}},
	['i'] = (qgChar){.data = {{0}, {0}, {0}, {3, -12}, {0}, {2, -13}, {3, -12}, {3, -12}, {3, -12}, {2, -14}, {0}}},
	['j'] = (qgChar){.data = {{0}, {0}, {0}, {5, -12}, {0}, {4, -13}, {5, -12}, {5, -12}, {5, -12}, {5, -12}, {2, -14}}},
	['k'] = (qgChar){.data = {{0}, {0}, {0}, {1, -12}, {1, -12}, {1, 12, 2, -12}, {1, 12, 1, -12}, {1, -14}, {1, 12, 1, -12}, {1, 12, 2, -12}, {0}}},
	['l'] = (qgChar){.data = {{0}, {0}, {0}, {2, -13}, {3, -12}, {3, -12}, {3, -12}, {3, -12}, {3, -12}, {2, -14}, {0}}},
	['m'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {1, 12, 1, -12}, {-17}, {12, 1, 11, 1, -12}, {12, 3, -12}, {12, 3, -12}, {0}}},
	['n'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {1, -15}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {0}}},
	['o'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {2, -14}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, -14}, {0}}},
	['p'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {1, -15}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, -15}, {1, -12}, {1, -12}}},
	['q'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {2, -15}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, -15}, {5, -12}, {5, -12}}},
	['r'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {1, -15}, {1, 12, 2, -12}, {1, -12}, {1, -12}, {1, -12}, {0}}},
	['s'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {2, -15}, {1, -12}, {2, -14}, {5, -12}, {1, -15}, {0}}},
	['t'] = (qgChar){.data = {{0}, {0}, {0}, {2, -12}, {2, -12}, {1, -15}, {2, -12}, {2, -12}, {2, -12}, {3, -13}, {0}}},
	['u'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, -15}, {0}}},
	['v'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, 11, 2, -11}, {2, -14}, {3, -12}, {0}}},
	['w'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {12, 3, -12}, {12, 1, 11, 1, -12}, {12, 1, 11, 1, -12},{1, -15}, {1, 12, 1, -12}, {0}}},
	['x'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {1, 12, 2, -12}, {2, 11, 2, -11}, {3, -12}, {2, 11, 2, -11}, {1, 12, 2, -12}, {0}}},
	['y'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {1, 12, 2, -12}, {1, 12, 2, -12}, {1, 12, 2, -12}, {2, -15}, {5, -12}, {1, -15}}},
	['z'] = (qgChar){.data = {{0}, {0}, {0}, {0}, {0}, {1, -16}, {4, -12}, {3, -12}, {2, -12}, {1, -16}, {0}}}
};
void qgAddChar(float px, float py, float pz, uint16_t c) {
	qgChar qc = newChars[c];
	if (c < 128) qc = qFont[c];

	for (int ly = 0; ly < qFontY; ly++) {
		int x = 0;
		for (int lx = 0; lx < qFontMax; lx++) {
			int8_t v = qc.data[ly][lx];
			if (v == 0) break;
			int isEnd = v < 0;
			if (isEnd) v = -v;
			if (v > 10) {
				int width_units = v - 10;
				float center_offset = x + (width_units * 0.5f),
				nx = px + (center_offset * qFontSize),
				ny = py - (ly * qFontSize);
				qgAddRect(nx, ny, pz, width_units * qFontSize, qFontSize, qFontR, qFontG, qFontB, qFontA);
				x += width_units;
			} else x += v;
			if (isEnd) break;
		}
	}
}
int ih(char c) { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'); }
void qgAddText(float px, float py, float pz, const char* text) {
	int x = 0, y = 0, i = 0, l = len(text);
	while (text[i] != '\0') {
		if (text[i] == '\n') {
			x = 0;
			y++;
			i++;
			continue;
		}
		if (text[i] == ' ') {
			x++;
			i++;
			continue;
		}
		if (text[i] == '\t') {
			x += 4;
			i++;
			continue;
		}
		uint16_t c = (unsigned char)text[i];
		if (l - i > 6 && text[i] == 'q' && text[i+1] == ';' && ih(text[i+2])  && ih(text[i+3])  && ih(text[i+4])  && ih(text[i+5]) && text[i+6] == ';') {
			char code[5] = {text[i+2], text[i+3], text[i+4], text[i+5], '\0'};
			c = (uint16_t)strtol(code, NULL, 16);
			i += 6;
		}
		qgAddChar(px + (x * qFontSize * qFontX), py - (y * qFontSize * qFontY), pz, c);
		x++;
		i++;
	}
}
// ========================================================================================================================================================================
// ===== INPUT ============================================================================================================================================================
// ========================================================================================================================================================================
int qgGetKey(int key) {
	if (!g_ctx.window || key < 0 || key >= GLFW_KEY_LAST) return 0;
	return glfwGetKey(g_ctx.window, key) == GLFW_PRESS;
}
int qgOnKey(int key) {
	if (!g_ctx.window || key < 0 || key >= GLFW_KEY_LAST) return 0;
	int current = glfwGetKey(g_ctx.window, key), last = g_ctx.lastKeyState[key];
	return (current == GLFW_PRESS && last == GLFW_RELEASE);
}
int qgGetMouse(int button) {
	if (!g_ctx.window || button < 0 || button >= GLFW_MOUSE_BUTTON_LAST) return 0;
	return glfwGetMouseButton(g_ctx.window, button) == GLFW_PRESS;
}
int qgOnMouse(int button) {
	if (!g_ctx.window || button < 0 || button >= GLFW_MOUSE_BUTTON_LAST) return 0;
	int current = glfwGetMouseButton(g_ctx.window, button), last = g_ctx.lastMouseState[button];
	return (current == GLFW_PRESS && last == GLFW_RELEASE);
}
void qgGetMousePos(float* x, float* y) {
	if (!g_ctx.window || !x || !y) return;
	double lx = 0, ly = 0;
	glfwGetCursorPos(g_ctx.window, &lx, &ly);
	*x = lx - (double)qgGetWidth() / 2;
	*y = -(ly - (double)qgGetHeight() / 2);
}
int qgGetWidth() {
	if (!g_ctx.window) return 0;
	int w, h;
	glfwGetWindowSize(g_ctx.window, &w, &h);
	return w;
}
int qgGetHeight() {
	if (!g_ctx.window) return 0;
	int w, h;
	glfwGetWindowSize(g_ctx.window, &w, &h);
	return h;
}
