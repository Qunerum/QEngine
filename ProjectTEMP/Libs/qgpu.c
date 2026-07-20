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
#define QGPU_VERSION_MINOR 0
#define QGPU_VERSION_PATCH 1

// ==========================================
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
} InternalContext;
static InternalContext g_ctx;
static float backgroundR, backgroundG, backgroundB;
// ==========================================
static int _showBanner = 1, _madeWith = 1, _showInfo = 1, _showColors = 1, _showLogs = 1, qgpuClr = MAGENTA, creator = LIGHT_RED, title = YELLOW, frame = GRAY;
// ═ ║ ╬
// ╔ ╗ ╚ ╝
// ╠ ╣ ╦ ╩
// = = = QPrint
static int qclamp(int v, int min, int max) { return v < min ? min : v > max ? max : v; }
static int oldClr = 255, actClr = 255, actStyle = 0; // White , Regular text
static void vprintc(int color, const char* format, va_list args) {
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
    vprintc(color, format, args);
    va_end(args);
}
void qgPrint(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintc(actClr, format, args);
    va_end(args);
}
void qgLog(const char* format, ...) {
    if (_showLogs) return;
    va_list args;
    va_start(args, format);
    vprintc(DARK_GRAY, format, args);
    va_end(args);
}
void qgWarn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintc(YELLOW, format, args);
    va_end(args);
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
    qgPrintc(frame, "║ Name: "); qgPrintc(qgpuClr, "QGPU"); qgPrintc(frame, "             ╚╝\n");
    qgPrintc(frame, "║ Version: "); qgPrintc(qgpuClr, "%i.%i.%i\n", QGPU_VERSION_MAJOR, QGPU_VERSION_MINOR, QGPU_VERSION_PATCH);
    qgPrintc(frame, "║ Creator: "); qgPrintc(creator, "Qunerum"); qgPrintc(frame, "       ╔╗\n");
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
// ==========================================
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
// ==========================================
void qgpuCreate(int width, int height, const char* title, void (*initFunc)(), void (*updateFunc)()) {
    if (!glfwInit()) return;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    g_ctx.window = glfwCreateWindow(width, height, title, NULL, NULL);
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
        0x07230203,0x00010000,0x000d000b,0x00000039,
        0x00000000,0x00020011,0x00000001,0x0006000b,
        0x00000001,0x4c534c47,0x6474732e,0x3035342e,
        0x00000000,0x0003000e,0x00000000,0x00000001,
        0x0009000f,0x00000000,0x00000004,0x6e69616d,
        0x00000000,0x0000000c,0x00000027,0x00000035,
        0x00000037,0x00030003,0x00000002,0x000001c2,
        0x000a0004,0x475f4c47,0x4c474f4f,0x70635f45,
        0x74735f70,0x5f656c79,0x656e696c,0x7269645f,
        0x69746365,0x00006576,0x00080004,0x475f4c47,
        0x4c474f4f,0x6e695f45,0x64756c63,0x69645f65,
        0x74636572,0x00657669,0x00040005,0x00000004,
        0x6e69616d,0x00000000,0x00050005,0x00000009,
        0x616e6966,0x736f506c,0x00000000,0x00040005,
        0x0000000c,0x6f506e69,0x00000073,0x00040005,
        0x0000000f,0x68737550,0x00000000,0x00050006,
        0x0000000f,0x00000000,0x7366666f,0x00007465,
        0x00060006,0x0000000f,0x00000001,0x65726373,
        0x65526e65,0x00000073,0x00040005,0x00000011,
        0x68737570,0x00000000,0x00060005,0x00000025,
        0x505f6c67,0x65567265,0x78657472,0x00000000,
        0x00060006,0x00000025,0x00000000,0x505f6c67,
        0x7469736f,0x006e6f69,0x00070006,0x00000025,
        0x00000001,0x505f6c67,0x746e696f,0x657a6953,
        0x00000000,0x00070006,0x00000025,0x00000002,
        0x435f6c67,0x4470696c,0x61747369,0x0065636e,
        0x00070006,0x00000025,0x00000003,0x435f6c67,
        0x446c6c75,0x61747369,0x0065636e,0x00030005,
        0x00000027,0x00000000,0x00050005,0x00000035,
        0x67617266,0x6f6c6f43,0x00000072,0x00040005,
        0x00000037,0x6f436e69,0x00726f6c,0x00040047,
        0x0000000c,0x0000001e,0x00000000,0x00030047,
        0x0000000f,0x00000002,0x00050048,0x0000000f,
        0x00000000,0x00000023,0x00000000,0x00050048,
        0x0000000f,0x00000001,0x00000023,0x00000008,
        0x00030047,0x00000025,0x00000002,0x00050048,
        0x00000025,0x00000000,0x0000000b,0x00000000,
        0x00050048,0x00000025,0x00000001,0x0000000b,
        0x00000001,0x00050048,0x00000025,0x00000002,
        0x0000000b,0x00000003,0x00050048,0x00000025,
        0x00000003,0x0000000b,0x00000004,0x00040047,
        0x00000035,0x0000001e,0x00000000,0x00040047,
        0x00000037,0x0000001e,0x00000001,0x00020013,
        0x00000002,0x00030021,0x00000003,0x00000002,
        0x00030016,0x00000006,0x00000020,0x00040017,
        0x00000007,0x00000006,0x00000002,0x00040020,
        0x00000008,0x00000007,0x00000007,0x00040017,
        0x0000000a,0x00000006,0x00000003,0x00040020,
        0x0000000b,0x00000001,0x0000000a,0x0004003b,
        0x0000000b,0x0000000c,0x00000001,0x0004001e,
        0x0000000f,0x00000007,0x00000007,0x00040020,
        0x00000010,0x00000009,0x0000000f,0x0004003b,
        0x00000010,0x00000011,0x00000009,0x00040015,
        0x00000012,0x00000020,0x00000001,0x0004002b,
        0x00000012,0x00000013,0x00000000,0x00040020,
        0x00000014,0x00000009,0x00000007,0x0004002b,
        0x00000012,0x00000018,0x00000001,0x0004002b,
        0x00000006,0x0000001b,0x3f000000,0x0004002b,
        0x00000006,0x0000001e,0x3f800000,0x00040017,
        0x00000021,0x00000006,0x00000004,0x00040015,
        0x00000022,0x00000020,0x00000000,0x0004002b,
        0x00000022,0x00000023,0x00000001,0x0004001c,
        0x00000024,0x00000006,0x00000023,0x0006001e,
        0x00000025,0x00000021,0x00000006,0x00000024,
        0x00000024,0x00040020,0x00000026,0x00000003,
        0x00000025,0x0004003b,0x00000026,0x00000027,
        0x00000003,0x0004002b,0x00000022,0x00000028,
        0x00000000,0x00040020,0x00000029,0x00000007,
        0x00000006,0x0004002b,0x00000022,0x0000002e,
        0x00000002,0x00040020,0x0000002f,0x00000001,
        0x00000006,0x00040020,0x00000033,0x00000003,
        0x00000021,0x0004003b,0x00000033,0x00000035,
        0x00000003,0x00040020,0x00000036,0x00000001,
        0x00000021,0x0004003b,0x00000036,0x00000037,
        0x00000001,0x00050036,0x00000002,0x00000004,
        0x00000000,0x00000003,0x000200f8,0x00000005,
        0x0004003b,0x00000008,0x00000009,0x00000007,
        0x0004003d,0x0000000a,0x0000000d,0x0000000c,
        0x0007004f,0x00000007,0x0000000e,0x0000000d,
        0x0000000d,0x00000000,0x00000001,0x00050041,
        0x00000014,0x00000015,0x00000011,0x00000013,
        0x0004003d,0x00000007,0x00000016,0x00000015,
        0x00050081,0x00000007,0x00000017,0x0000000e,
        0x00000016,0x00050041,0x00000014,0x00000019,
        0x00000011,0x00000018,0x0004003d,0x00000007,
        0x0000001a,0x00000019,0x0005008e,0x00000007,
        0x0000001c,0x0000001a,0x0000001b,0x00050088,
        0x00000007,0x0000001d,0x00000017,0x0000001c,
        0x00050050,0x00000007,0x0000001f,0x0000001e,
        0x0000001e,0x00050083,0x00000007,0x00000020,
        0x0000001d,0x0000001f,0x0003003e,0x00000009,
        0x00000020,0x00050041,0x00000029,0x0000002a,
        0x00000009,0x00000028,0x0004003d,0x00000006,
        0x0000002b,0x0000002a,0x00050041,0x00000029,
        0x0000002c,0x00000009,0x00000023,0x0004003d,
        0x00000006,0x0000002d,0x0000002c,0x00050041,
        0x0000002f,0x00000030,0x0000000c,0x0000002e,
        0x0004003d,0x00000006,0x00000031,0x00000030,
        0x00070050,0x00000021,0x00000032,0x0000002b,
        0x0000002d,0x00000031,0x0000001e,0x00050041,
        0x00000033,0x00000034,0x00000027,0x00000013,
        0x0003003e,0x00000034,0x00000032,0x0004003d,
        0x00000021,0x00000038,0x00000037,0x0003003e,
        0x00000035,0x00000038,0x000100fd,0x00010038
    };
    VkShaderModuleCreateInfo vertInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = sizeof(vert_code),
        .pCode = vert_code
    };
    VkShaderModule vertModule = VK_NULL_HANDLE;
    vkCreateShaderModule(g_ctx.device, &vertInfo, NULL, &vertModule);
    const uint32_t frag_code[] = {
        0x07230203,0x00010000,0x000d000b,0x0000000d,
        0x00000000,0x00020011,0x00000001,0x0006000b,
        0x00000001,0x4c534c47,0x6474732e,0x3035342e,
        0x00000000,0x0003000e,0x00000000,0x00000001,
        0x0007000f,0x00000004,0x00000004,0x6e69616d,
        0x00000000,0x00000009,0x0000000b,0x00030010,
        0x00000004,0x00000007,0x00030003,0x00000002,
        0x000001c2,0x000a0004,0x475f4c47,0x4c474f4f,
        0x70635f45,0x74735f70,0x5f656c79,0x656e696c,
        0x7269645f,0x69746365,0x00006576,0x00080004,
        0x475f4c47,0x4c474f4f,0x6e695f45,0x64756c63,
        0x69645f65,0x74636572,0x00657669,0x00040005,
        0x00000004,0x6e69616d,0x00000000,0x00050005,
        0x00000009,0x4374756f,0x726f6c6f,0x00000000,
        0x00050005,0x0000000b,0x67617266,0x6f6c6f43,
        0x00000072,0x00040047,0x00000009,0x0000001e,
        0x00000000,0x00040047,0x0000000b,0x0000001e,
        0x00000000,0x00020013,0x00000002,0x00030021,
        0x00000003,0x00000002,0x00030016,0x00000006,
        0x00000020,0x00040017,0x00000007,0x00000006,
        0x00000004,0x00040020,0x00000008,0x00000003,
        0x00000007,0x0004003b,0x00000008,0x00000009,
        0x00000003,0x00040020,0x0000000a,0x00000001,
        0x00000007,0x0004003b,0x0000000a,0x0000000b,
        0x00000001,0x00050036,0x00000002,0x00000004,
        0x00000000,0x00000003,0x000200f8,0x00000005,
        0x0004003d,0x00000007,0x0000000c,0x0000000b,
        0x0003003e,0x00000009,0x0000000c,0x000100fd,
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
    createBuffer(sizeof(uint32_t) * MAX_VERTICES * 1.5f, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g_ctx.indexBuffer, &g_ctx.indexBufferMemory);
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
    if (initFunc) initFunc();
    while (!glfwWindowShouldClose(g_ctx.window)) {
        for (int i = 0; i < GLFW_KEY_LAST; i++) g_ctx.lastKeyState[i] = glfwGetKey(g_ctx.window, i);
        for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++) g_ctx.lastMouseState[i] = glfwGetMouseButton(g_ctx.window, i);
        glfwPollEvents();
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(g_ctx.device, g_ctx.swapchain, UINT64_MAX, g_ctx.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) { continue; } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { continue; }
        g_ctx.currentVOffset = 0;
        g_ctx.currentIOffset = 0;
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
    }
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
// ========================================================================================================================================================================
// ========================================================================================================================================================================
// ========================================================================================================================================================================
void qgSetBackground(float r, float g, float b) {
    backgroundR = r;
    backgroundG = g;
    backgroundB = b;
}
uint32_t qgAddVertex(float x, float y, float layer, float r, float g, float b, float a) {
    if (g_ctx.currentVOffset >= MAX_VERTICES) return -1;
    QGPU_Vertex* vDst = (QGPU_Vertex*)g_ctx.mappedVertexBuffer + g_ctx.currentVOffset;
    vDst->pos[0] = x;
    vDst->pos[1] = -y;
    vDst->pos[2] = layer;
    vDst->color[0] = r;
    vDst->color[1] = g;
    vDst->color[2] = b;
    vDst->color[3] = a;
    g_ctx.currentVOffset++;
    return g_ctx.currentVOffset-1;
}
void qgAddIndex(uint32_t index) {
    if (g_ctx.currentIOffset >= (uint32_t)(MAX_VERTICES * 3)) return;
    uint32_t* iDst = (uint32_t*)g_ctx.mappedIndexBuffer + g_ctx.currentIOffset;
    *iDst = index;
    g_ctx.currentIOffset++;
}
void qgAddGeometry(QGPU_Vertex* verts, uint32_t vCount, uint32_t* indices, uint32_t iCount) {
    if (vCount == 0 || iCount == 0) return;
    uint32_t baseVertexOffset = g_ctx.currentVOffset;
    for (uint32_t i = 0; i < vCount; i++) qgAddVertex(verts[i].pos[0], verts[i].pos[1], verts[i].pos[2], verts[i].color[0], verts[i].color[1], verts[i].color[2], verts[i].color[3]);
    for (uint32_t i = 0; i < iCount; i++) qgAddIndex(indices[i] + baseVertexOffset);
}
// ========================================================================================================================================================================
// ========================================================================================================================================================================
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
void qgGetMousePos(double* x, double* y) {
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
