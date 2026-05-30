#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/stat.h>
#include "qgpu.h"

#define PI 3.14159265359f
#define SHADERS "bin/"

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
    uint32_t currentVOffset;
    uint32_t currentIOffset;
    int lastKeyState[GLFW_KEY_LAST];
    int lastMouseState[GLFW_MOUSE_BUTTON_LAST];
    void* mappedVertexBuffer;
    void* mappedIndexBuffer;
} InternalContext;

static InternalContext g_ctx;
RawTexture txts[TEXTURES];

// ==========================================
float q_abs(float x) { return (x < 0) ? -x : x; }
float q_sqrt(float x) {
    if (x <= 0) return 0;
    float xhalf = 0.5f * x;
    union { float f; int i; } conv;
    conv.f = x; conv.i = 0x5f3759df - (conv.i >> 1);
    x = conv.f; x = x * (1.5f - xhalf * x * x);
    return 1.0f / x;
}
float q_sin(float x) {
    while (x > PI) x -= 2.0f * PI;
    while (x < -PI) x += 2.0f * PI;
    float abs_x = q_abs(x), pi2 = PI * PI, sin_x = (16.0f * x * (PI - abs_x)) / (5.0f * pi2 - 4.0f * x * (PI - abs_x));
    return sin_x;
}
float q_cos(float x) { return q_sin(x + (PI / 2.0f)); }
int AABB(float x, float y, float posX, float posY, float width, float height) { return (posX - width / 2 <= x && x <= posX + width / 2) && (posY - height / 2 <= y && y <= posY + height / 2); }
// ==========================================
void print(const char* format, ...) {
    printf("[QGPU]: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}
// ==========================================
static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(g_ctx.physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) { if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) { return i; } }
    return 0;
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
static VkShaderModule createShaderModule(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return VK_NULL_HANDLE;
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* code = malloc(length);
    if (!code) { fclose(file); return VK_NULL_HANDLE; }
    size_t readElements = fread(code, 1, length, file);
    (void)readElements;
    fclose(file);
    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = length,
        .pCode = (uint32_t*)code
    };
    VkShaderModule shaderModule;
    vkCreateShaderModule(g_ctx.device, &createInfo, NULL, &shaderModule);
    free(code);
    return shaderModule;
}
// ==========================================
void cleanup_textures() { for (int i = 0; i < TEXTURES; i++) { if (txts[i].pixels != NULL) { free(txts[i].pixels); txts[i].pixels = NULL; } } }
void qgpuCreate(int width, int height, const char* title, void (*updateFunc)()) {
    if (!glfwInit()) return;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    g_ctx.window = glfwCreateWindow(width, height, title, NULL, NULL);
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    VkInstanceCreateInfo instanceInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions
    };
    vkCreateInstance(&instanceInfo, NULL, &g_ctx.instance);
    glfwCreateWindowSurface(g_ctx.instance, g_ctx.window, NULL, &g_ctx.surface);
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(g_ctx.instance, &deviceCount, NULL);
    VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(g_ctx.instance, &deviceCount, devices);
    g_ctx.physicalDevice = devices[0];
    free(devices);
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = 0,
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
    VkAttachmentReference colorAttachmentRef = { .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkSubpassDescription subpass = { .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS, .colorAttachmentCount = 1, .pColorAttachments = &colorAttachmentRef };
    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };
    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };
    vkCreateRenderPass(g_ctx.device, &renderPassInfo, NULL, &g_ctx.renderPass);
    VkPushConstantRange pushConstantRange = { .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .offset = 0, .size = sizeof(float) * 4 };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };
    vkCreatePipelineLayout(g_ctx.device, &pipelineLayoutInfo, NULL, &g_ctx.pipelineLayout);
    VkShaderModule vertModule = createShaderModule(SHADERS "vert.spv");
    VkShaderModule fragModule = createShaderModule(SHADERS "frag.spv");
    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = vertModule, .pName = "main"},
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = fragModule, .pName = "main"}
    };
    VkVertexInputBindingDescription bindingDesc = { .binding = 0, .stride = sizeof(QGPU_Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX };
    VkVertexInputAttributeDescription attribDescs[2] = {
        {.binding = 0, .location = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(QGPU_Vertex, pos)},
        {.binding = 0, .location = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(QGPU_Vertex, color)}
    };
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDesc,
        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = attribDescs
    };
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
    VkPipelineViewportStateCreateInfo viewportState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1 };
    VkPipelineRasterizationStateCreateInfo rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, .lineWidth = 1.0f, .cullMode = VK_CULL_MODE_NONE, .frontFace = VK_FRONT_FACE_CLOCKWISE };
    VkPipelineMultisampleStateCreateInfo multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT };
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
    VkPipelineColorBlendStateCreateInfo colorBlending = { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, .attachmentCount = 1, .pAttachments = &colorBlendAttachment };
    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, .dynamicStateCount = 2, .pDynamicStates = dynamicStates };
    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2, .pStages = shaderStages, .pVertexInputState = &vertexInputInfo, .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState, .pRasterizationState = &rasterizer, .pMultisampleState = &multisampling,
        .pColorBlendState = &colorBlending, .pDynamicState = &dynamicState, .layout = g_ctx.pipelineLayout, .renderPass = g_ctx.renderPass, .subpass = 0
    };
    vkCreateGraphicsPipelines(g_ctx.device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &g_ctx.graphicsPipeline);
    vkDestroyShaderModule(g_ctx.device, fragModule, NULL);
    vkDestroyShaderModule(g_ctx.device, vertModule, NULL);
    g_ctx.swapchainFramebuffers = malloc(sizeof(VkFramebuffer) * g_ctx.imageCount);
    for (uint32_t i = 0; i < g_ctx.imageCount; i++) {
        VkFramebufferCreateInfo fbInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, .renderPass = g_ctx.renderPass,
            .attachmentCount = 1, .pAttachments = &g_ctx.swapchainImageViews[i],
            .width = (uint32_t)fbW, .height = (uint32_t)fbH, .layers = 1
        };
        vkCreateFramebuffer(g_ctx.device, &fbInfo, NULL, &g_ctx.swapchainFramebuffers[i]);
    }
    VkCommandPoolCreateInfo poolInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .queueFamilyIndex = 0, .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
    vkCreateCommandPool(g_ctx.device, &poolInfo, NULL, &g_ctx.commandPool);
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = g_ctx.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    vkAllocateCommandBuffers(g_ctx.device, &allocInfo, &g_ctx.currentCmd);
    createBuffer(sizeof(QGPU_Vertex) * 65536, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g_ctx.vertexBuffer, &g_ctx.vertexBufferMemory);
    createBuffer(sizeof(uint32_t) * 65536, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g_ctx.indexBuffer, &g_ctx.indexBufferMemory);
    vkMapMemory(g_ctx.device, g_ctx.vertexBufferMemory, 0, sizeof(QGPU_Vertex) * 65536, 0, &g_ctx.mappedVertexBuffer);
    vkMapMemory(g_ctx.device, g_ctx.indexBufferMemory, 0, sizeof(uint32_t) * 65536, 0, &g_ctx.mappedIndexBuffer);
    VkSemaphoreCreateInfo semaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    vkCreateSemaphore(g_ctx.device, &semaphoreInfo, NULL, &g_ctx.imageAvailableSemaphore);
    vkCreateSemaphore(g_ctx.device, &semaphoreInfo, NULL, &g_ctx.renderFinishedSemaphore);
    memset(g_ctx.lastKeyState, 0, sizeof(g_ctx.lastKeyState));
    memset(g_ctx.lastMouseState, 0, sizeof(g_ctx.lastMouseState));
    while (!glfwWindowShouldClose(g_ctx.window)) {
        for (int i = 0; i < GLFW_KEY_LAST; i++) g_ctx.lastKeyState[i] = glfwGetKey(g_ctx.window, i);
        for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++) g_ctx.lastMouseState[i] = glfwGetMouseButton(g_ctx.window, i);
        glfwPollEvents();
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(g_ctx.device, g_ctx.swapchain, UINT64_MAX, g_ctx.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) { continue; } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { continue; }
        g_ctx.currentVOffset = 0; g_ctx.currentIOffset = 0;
        vkResetCommandBuffer(g_ctx.currentCmd, 0);
        VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT };
        vkBeginCommandBuffer(g_ctx.currentCmd, &beginInfo);
        VkClearValue clearColor = {{{0.1f, 0.1f, 0.1f, 1.0f}}};
        VkRenderPassBeginInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = g_ctx.renderPass,
            .framebuffer = g_ctx.swapchainFramebuffers[imageIndex],
            .renderArea = {{0, 0}, {(uint32_t)width, (uint32_t)height}},
            .clearValueCount = 1,
            .pClearValues = &clearColor
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
        updateFunc();
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
        if (vkQueueSubmit(g_ctx.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) { printf("Quene submit error!\n"); }
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
    cleanup_textures();
    vkUnmapMemory(g_ctx.device, g_ctx.vertexBufferMemory);
    vkUnmapMemory(g_ctx.device, g_ctx.indexBufferMemory);
    vkDestroySemaphore(g_ctx.device, g_ctx.renderFinishedSemaphore, NULL);
    vkDestroySemaphore(g_ctx.device, g_ctx.imageAvailableSemaphore, NULL);
    vkDestroyBuffer(g_ctx.device, g_ctx.indexBuffer, NULL);
    vkFreeMemory(g_ctx.device, g_ctx.indexBufferMemory, NULL);
    vkDestroyBuffer(g_ctx.device, g_ctx.vertexBuffer, NULL);
    vkFreeMemory(g_ctx.device, g_ctx.vertexBufferMemory, NULL);
    vkDestroyCommandPool(g_ctx.device, g_ctx.commandPool, NULL);
    for (uint32_t i = 0; i < g_ctx.imageCount; i++) { vkDestroyFramebuffer(g_ctx.device, g_ctx.swapchainFramebuffers[i], NULL); vkDestroyImageView(g_ctx.device, g_ctx.swapchainImageViews[i], NULL); }
    free(g_ctx.swapchainFramebuffers);
    free(g_ctx.swapchainImageViews);
    free(g_ctx.swapchainImages);
    vkDestroyPipeline(g_ctx.device, g_ctx.graphicsPipeline, NULL);
    vkDestroyPipelineLayout(g_ctx.device, g_ctx.pipelineLayout, NULL);
    vkDestroyRenderPass(g_ctx.device, g_ctx.renderPass, NULL);
    vkDestroySwapchainKHR(g_ctx.device, g_ctx.swapchain, NULL);
    vkDestroyDevice(g_ctx.device, NULL);
    vkDestroySurfaceKHR(g_ctx.instance, g_ctx.surface, NULL);
    vkDestroyInstance(g_ctx.instance, NULL);
    glfwDestroyWindow(g_ctx.window);
    glfwTerminate();
}

void drawGeometry(float posX, float posY, QGPU_Vertex* vertices, uint32_t vCount, uint32_t* indices, uint32_t iCount) {
    if (vCount == 0 || iCount == 0) return;
    if (g_ctx.currentVOffset + vCount >= 65536 || g_ctx.currentIOffset + iCount >= 65536) return;
    int w, h;
    glfwGetFramebufferSize(g_ctx.window, &w, &h);
    float pushConstants[4] = { posX, -posY, (float)w, (float)h };
    vkCmdPushConstants(g_ctx.currentCmd, g_ctx.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 4, pushConstants);
    QGPU_Vertex* vDst = (QGPU_Vertex*)g_ctx.mappedVertexBuffer + g_ctx.currentVOffset;
    memcpy(vDst, vertices, vCount * sizeof(QGPU_Vertex));
    uint32_t* iDst = (uint32_t*)g_ctx.mappedIndexBuffer + g_ctx.currentIOffset;
    memcpy(iDst, indices, iCount * sizeof(uint32_t));
    vkCmdDrawIndexed(g_ctx.currentCmd, iCount, 1, g_ctx.currentIOffset, g_ctx.currentVOffset, 0);
    g_ctx.currentVOffset += vCount;
    g_ctx.currentIOffset += iCount;
}

// ==========================================
void drawRect(float posX, float posY, float sizeX, float sizeY, QColor clr) {
    float r = clr.r, g = clr.g, b = clr.b, a = clr.a;
    float x = sizeX / 2.0f, y = sizeY / 2.0f;
    QGPU_Vertex v[] = {
        {{ -x ,  y}, {r, g, b, a}},
        {{  x ,  y}, {r, g, b, a}},
        {{  x , -y}, {r, g, b, a}},
        {{ -x , -y}, {r, g, b, a}}
    };
    uint32_t i[] = {0, 1, 2,  0, 2, 3};
    drawGeometry(posX, posY, v, 4, i, 6);
}
void drawTriangle(float posX, float posY, float p1X, float p1Y, float p2X, float p2Y, float p3X, float p3Y, QColor clr) {
    float r = clr.r, g = clr.g, b = clr.b, a = clr.a;
    QGPU_Vertex v[] = {
        {{ p1X, p1Y }, {r, g, b, a}},
        {{ p2X, p2Y }, {r, g, b, a}},
        {{ p3X, p3Y }, {r, g, b, a}}
    };
    uint32_t i[] = {0, 1, 2};
    drawGeometry(posX, posY, v, 3, i, 3);
}
void drawCircle(float posX, float posY, float radius, int segments, QColor clr) {
    float r = clr.r, g = clr.g, b = clr.b, a = clr.a;
    int vCount = segments + 1;
    int iCount = segments * 3;
    QGPU_Vertex* v = malloc(vCount * sizeof(QGPU_Vertex));
    uint32_t* indices = malloc(iCount * sizeof(uint32_t));
    v[0] = (QGPU_Vertex){{0.0f, 0.0f}, {r, g, b, a}};
    for (int i = 0; i < segments; i++) {
        float angle = ((float)i / segments) * 2.0f * PI;
        v[i + 1] = (QGPU_Vertex){{q_cos(angle) * radius, q_sin(angle) * radius}, {r, g, b, a}};

        int io = i * 3;
        indices[io + 0] = 0;
        indices[io + 1] = i + 1;
        indices[io + 2] = (i == segments - 1) ? 1 : i + 2;
    }
    drawGeometry(posX, posY, v, vCount, indices, iCount);
    free(v);
    free(indices);
}
void drawLine(float x1, float y1, float x2, float y2, float thickness, QColor clr) {
    float r = clr.r, g = clr.g, b = clr.b, a = clr.a;
    float dx = x2 - x1, dy = y2 - y1;
    float len = q_sqrt(dx * dx + dy * dy);
    if (len == 0) return;
    float nx = -dy / len * (thickness / 2.0f);
    float ny =  dx / len * (thickness / 2.0f);
    QGPU_Vertex v[4] = {
        {{x1 + nx, y1 + ny}, {r, g, b, a}},
        {{x2 + nx, y2 + ny}, {r, g, b, a}},
        {{x2 - nx, y2 - ny}, {r, g, b, a}},
        {{x1 - nx, y1 - ny}, {r, g, b, a}}
    };
    uint32_t indices[6] = { 0, 1, 2, 0, 2, 3 };
    drawGeometry(0, 0, v, 4, indices, 6);
}
void drawWireRect(float posX, float posY, float sizeX, float sizeY, float thickness, QColor clr) {
    float hX = sizeX / 2.0f, hY = sizeY / 2.0f;
    drawLine(posX - hX, posY - hY, posX + hX, posY - hY, thickness, clr);
    drawLine(posX + hX, posY - hY, posX + hX, posY + hY, thickness, clr);
    drawLine(posX + hX, posY + hY, posX - hX, posY + hY, thickness, clr);
    drawLine(posX - hX, posY + hY, posX - hX, posY - hY, thickness, clr);
}
void drawWireTriangle(float posX, float posY, float p1X, float p1Y, float p2X, float p2Y, float p3X, float p3Y, float thickness, QColor clr) {
    drawLine(posX + p1X, posY + p1Y, posX + p2X, posY + p2Y, thickness, clr);
    drawLine(posX + p2X, posY + p2Y, posX + p3X, posY + p3Y, thickness, clr);
    drawLine(posX + p3X, posY + p3Y, posX + p1X, posY + p1Y, thickness, clr);
}
void drawWireCircle(float posX, float posY, float radius, int segments, float thickness, QColor clr) {
    float r = clr.r, g = clr.g, b = clr.b, a = clr.a;
    int vCount = segments * 4;
    int iCount = segments * 6;
    QGPU_Vertex* v = malloc(vCount * sizeof(QGPU_Vertex));
    uint32_t* indices = malloc(iCount * sizeof(uint32_t));
    for (int i = 0; i < segments; i++) {
        float a1 = ((float)i / segments) * 2.0f * PI
        , a2 = ((float)(i + 1) / segments) * 2.0f * PI
        , x1 = q_cos(a1) * radius, y1 = q_sin(a1) * radius
        , x2 = q_cos(a2) * radius, y2 = q_sin(a2) * radius
        , dx = x2 - x1, dy = y2 - y1
        , len = q_sqrt(dx * dx + dy * dy);
        if (len == 0) len = 1.0f;
        float nx = -dy / len * (thickness / 2.0f), ny =  dx / len * (thickness / 2.0f);
        int vo = i * 4, io = i * 6;
        v[vo + 0] = (QGPU_Vertex){{x1 + nx, y1 + ny}, {r, g, b, a}};
        v[vo + 1] = (QGPU_Vertex){{x2 + nx, y2 + ny}, {r, g, b, a}};
        v[vo + 2] = (QGPU_Vertex){{x2 - nx, y2 - ny}, {r, g, b, a}};
        v[vo + 3] = (QGPU_Vertex){{x1 - nx, y1 - ny}, {r, g, b, a}};
        indices[io + 0] = vo + 0; indices[io + 1] = vo + 1; indices[io + 2] = vo + 2;
        indices[io + 3] = vo + 0; indices[io + 4] = vo + 2; indices[io + 5] = vo + 3;
    }
    drawGeometry(posX, posY, v, vCount, indices, iCount);
    free(v);
    free(indices);
}
// ==========================================
int count_files_with_ext(const char *path, const char *ext) {
    int count = 0;
    struct dirent *entry;
    struct stat statbuf;
    DIR *dir = opendir(path);
    if (!dir) return 0;
    while ((entry = readdir(dir)) != NULL) {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (stat(full_path, &statbuf) == -1) continue;
        if (S_ISDIR(statbuf.st_mode)) { if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue; count += count_files_with_ext(full_path, ext); }
        else { char *dot = strrchr(entry->d_name, '.'); if (dot && strcmp(dot, ext) == 0) { count++; } }
    }
    closedir(dir);
    return count;
}
void loadTexture(const char* filename, int slot) {
    if (slot < 0 || slot >= TEXTURES) return;
    if (txts[slot].pixels != NULL) { free(txts[slot].pixels); txts[slot].pixels = NULL; }
    FILE* file = fopen(filename, "r");
    if (!file) return;
    char line[16];
    int width = 0, height = 0;
    if (fgets(line, sizeof(line), file)) { sscanf(line, "%d %d", &width, &height); }
    int pixelCount = width * height;
    unsigned char* pixelData = (unsigned char*)malloc(pixelCount * 4);
    if (!pixelData) { fclose(file); return; }
    int currentByte = 0;
    while (fgets(line, sizeof(line), file) && currentByte < pixelCount * 4) {
        int r, g, b, a;
        if (sscanf(line, "%d %d %d %d", &r, &g, &b, &a) == 4) {
            pixelData[currentByte++] = (unsigned char)r;
            pixelData[currentByte++] = (unsigned char)g;
            pixelData[currentByte++] = (unsigned char)b;
            pixelData[currentByte++] = (unsigned char)a;
        }
    }
    txts[slot].pixels = pixelData;
    txts[slot].pixelCount = pixelCount;
    txts[slot].width = width;
    txts[slot].height = height;
    fclose(file);
    printf("Loaded texture '%s' to slot '%d' (%dx%d)\n", filename, slot, width, height);
}
void drawTextureScaling(int slot, float scale, float posX, float posY) {
    if (slot < 0 || slot >= TEXTURES || txts[slot].pixels == NULL) return;
    RawTexture* tex = &txts[slot];
    int vc = tex->pixelCount * 4, ic = tex->pixelCount * 6;
    float halfW = (tex->width * scale) / 2.0f, halfH = (tex->height * scale) / 2.0f;
    QGPU_Vertex* v = malloc(vc * sizeof(QGPU_Vertex));
    uint32_t* i_ptr = malloc(ic * sizeof(uint32_t));
    if (!v || !i_ptr) { free(v); free(i_ptr); return; }
    int vIdx = 0, iIdx = 0;
    for (int y = 0; y < tex->height; y++) {
        for (int x = 0; x < tex->width; x++) {
            int p = (y * tex->width + x) * 4;
            float r = tex->pixels[p] / 255.0f,
            g = tex->pixels[p + 1] / 255.0f,
            b = tex->pixels[p + 2] / 255.0f,
            a = tex->pixels[p + 3] / 255.0f,
            x0 = (x * scale) - halfW,
            x1 = ((x + 1) * scale) - halfW,
            y0 = halfH - ((y + 1) * scale),
            y1 = halfH - (y * scale);
            v[vIdx + 0] = (QGPU_Vertex){{x0, y1}, {r, g, b, a}};
            v[vIdx + 1] = (QGPU_Vertex){{x1, y1}, {r, g, b, a}};
            v[vIdx + 2] = (QGPU_Vertex){{x1, y0}, {r, g, b, a}};
            v[vIdx + 3] = (QGPU_Vertex){{x0, y0}, {r, g, b, a}};
            uint32_t offset = vIdx;
            i_ptr[iIdx + 0] = offset + 0;
            i_ptr[iIdx + 1] = offset + 1;
            i_ptr[iIdx + 2] = offset + 2;
            i_ptr[iIdx + 3] = offset + 0;
            i_ptr[iIdx + 4] = offset + 2;
            i_ptr[iIdx + 5] = offset + 3;
            vIdx += 4;
            iIdx += 6;
        }
    }
    drawGeometry(posX, posY, v, vc, i_ptr, ic);
    free(v);
    free(i_ptr);
}
void drawTextureScale(float posX, float posY, int textureID, float scale) { drawTextureScaling(textureID, scale, posX, posY); }
// ==========================================
int getKey(int key) { if (!g_ctx.window || key < 0 || key >= GLFW_KEY_LAST) return 0; return glfwGetKey(g_ctx.window, key) == GLFW_PRESS; }
int onKey(int key) {
    if (!g_ctx.window || key < 0 || key >= GLFW_KEY_LAST) return 0;
    int current = glfwGetKey(g_ctx.window, key);
    int last = g_ctx.lastKeyState[key];
    return (current == GLFW_PRESS && last == GLFW_RELEASE);
}
int getMouse(int button) { if (!g_ctx.window || button < 0 || button >= GLFW_MOUSE_BUTTON_LAST) return 0; return glfwGetMouseButton(g_ctx.window, button) == GLFW_PRESS; }
int onMouse(int button) {
    if (!g_ctx.window || button < 0 || button >= GLFW_MOUSE_BUTTON_LAST) return 0;
    int current = glfwGetMouseButton(g_ctx.window, button);
    int last = g_ctx.lastMouseState[button];
    return (current == GLFW_PRESS && last == GLFW_RELEASE);
}
void getMousePos(double* x, double* y) {
    if (!g_ctx.window || !x || !y) return;
    double lx = 0, ly = 0; glfwGetCursorPos(g_ctx.window, &lx, &ly);
    *x = lx - (double)getWidth() / 2;
    *y = ly - (double)getHeight() / 2;
}
int getWidth() { int w, h; if (!g_ctx.window) return 0; glfwGetWindowSize(g_ctx.window, &w, &h); return w; }
int getHeight() { int w, h; if (!g_ctx.window) return 0; glfwGetWindowSize(g_ctx.window, &w, &h); return h; }
// ==========================================
int drawButton(float posX, float posY, float width, float height, QColor clr, QColor hoverClr, QColor pressClr) {
    double mx, my; getMousePos(&mx, &my);
    int hovered = AABB((float)mx, (float)my, posX, posY, width, height), o = 0;
    if (hovered) { if (onMouse(LMB)) { o = 1; } else if (getMouse(LMB)) { o = 2; } }
    drawRect(posX, posY, width, height, hovered ? (o == 0 ? hoverClr : pressClr) : clr);
    return o;
}
int drawSlider(float* value, float min, float max, float posX, float posY, float width, float height, float handleW, float handleH, QColor backgroundClr, QColor fillClr, QColor handleClr) {
    double mx = 0, my = 0; getMousePos(&mx, &my);
    int hovered = AABB((float)mx, (float)my, posX, posY, width, height);
    int changed = 0;
    if (getMouse(LMB)) {
        if (hovered) {
            float t = ((float)mx - (posX - width / 2.0f)) / width;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            *value = min + t * (max - min);
            changed = 1;
        }
    }
    float t = (*value - min) / (max - min);
    drawRect(posX, posY, width, height, backgroundClr);
    drawRect(posX - width/2.0f + width * t / 2.0f, posY, width * t, height, fillClr);
    drawRect(posX - width / 2.0f + t * width, posY, handleW, handleH, handleClr);
    return changed;
}
int drawToggle(int* value, float posX, float posY, float width, float height, QColor offClr, QColor onClr) {
    double mx, my; getMousePos(&mx, &my);
    int hovered = AABB((float)mx, (float)my, posX, posY, width, height);
    int m = hovered && onMouse(LMB);
    if (m) { *value = !*value; }
    drawRect(posX, posY, width, height, *value == 0 ? offClr : onClr);
    return m;
}
