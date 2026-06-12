#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/stat.h>
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "qgpu.h"

#define PI 3.14159265359f
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
typedef struct { unsigned char* pixels; int pixelCount; int width; int height; VkBuffer buffer; VkDeviceMemory memory; } RawTexture;
static InternalContext g_ctx;
RawTexture txts[MAX_TEXTURES];
uint32_t g_currentRenderType = 0;
VkDescriptorSetLayout g_descriptorSetLayout;
VkDescriptorPool      g_descriptorPool;
VkDescriptorSet       g_descriptorSets[MAX_TEXTURES];
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
void print(const char* format, ...) { printf("[QGPU]: "); va_list args; va_start(args, format); vprintf(format, args); va_end(args); printf("\n"); }
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
void cleanup_textures() { for (int i = 0; i < MAX_TEXTURES; i++) { if (txts[i].pixels != NULL) {
    if (g_ctx.device != VK_NULL_HANDLE) { vkDestroyBuffer(g_ctx.device, txts[i].buffer, NULL); vkFreeMemory(g_ctx.device, txts[i].memory, NULL); } txts[i].pixels = NULL; } } }
void qgpuCreate(int width, int height, const char* title, void (*initFunc)(), void (*updateFunc)()) {
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
    VkDescriptorSetLayoutBinding layoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = NULL
    };
    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &layoutBinding
    };
    vkCreateDescriptorSetLayout(g_ctx.device, &layoutInfo, NULL, &g_descriptorSetLayout);
    VkDescriptorPoolSize poolSize = {
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = MAX_TEXTURES
    };
    VkDescriptorPoolCreateInfo descPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
        .maxSets = MAX_TEXTURES
    };
    vkCreateDescriptorPool(g_ctx.device, &descPoolInfo, NULL, &g_descriptorPool);
    VkDescriptorSetLayout layouts[MAX_TEXTURES];
    for (int i = 0; i < MAX_TEXTURES; i++) { layouts[i] = g_descriptorSetLayout; }
    VkDescriptorSetAllocateInfo allocInfoDesc = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = g_descriptorPool,
        .descriptorSetCount = MAX_TEXTURES,
        .pSetLayouts = layouts
    };
    if (vkAllocateDescriptorSets(g_ctx.device, &allocInfoDesc, g_descriptorSets) != VK_SUCCESS) { print("Failed to allocate descriptor sets!"); }
    VkPushConstantRange pushConstantRange = { .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .offset = 0, .size = 20 };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &g_descriptorSetLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };
    vkCreatePipelineLayout(g_ctx.device, &pipelineLayoutInfo, NULL, &g_ctx.pipelineLayout);
    VkShaderModule vertModule = createShaderModule(FILES "vert.spv");
    VkShaderModule fragModule = createShaderModule(FILES "frag.spv");
    VkPipelineShaderStageCreateInfo shaderStages[2] = {
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = vertModule, .pName = "main"},
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = fragModule, .pName = "main"}
    };
    VkVertexInputBindingDescription bindingDesc = { .binding = 0, .stride = sizeof(QGPU_Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX };
    VkVertexInputAttributeDescription attribDescs[3] = {
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
    createBuffer(sizeof(QGPU_Vertex) * MAX_VERTICES, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g_ctx.vertexBuffer, &g_ctx.vertexBufferMemory);
    createBuffer(sizeof(uint32_t) * MAX_VERTICES * 1.5f, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g_ctx.indexBuffer, &g_ctx.indexBufferMemory);
    vkMapMemory(g_ctx.device, g_ctx.vertexBufferMemory, 0, sizeof(QGPU_Vertex) * MAX_VERTICES, 0, &g_ctx.mappedVertexBuffer);
    vkMapMemory(g_ctx.device, g_ctx.indexBufferMemory, 0, sizeof(uint32_t) * (uint32_t)(MAX_VERTICES * 1.5f), 0, &g_ctx.mappedIndexBuffer);
    VkSemaphoreCreateInfo semaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    vkCreateSemaphore(g_ctx.device, &semaphoreInfo, NULL, &g_ctx.imageAvailableSemaphore);
    vkCreateSemaphore(g_ctx.device, &semaphoreInfo, NULL, &g_ctx.renderFinishedSemaphore);
    memset(g_ctx.lastKeyState, 0, sizeof(g_ctx.lastKeyState));
    memset(g_ctx.lastMouseState, 0, sizeof(g_ctx.lastMouseState));
    if (initFunc) initFunc();
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
        if (updateFunc) updateFunc();
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
    vkDestroyDescriptorPool(g_ctx.device, g_descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(g_ctx.device, g_descriptorSetLayout, NULL);
    vkDestroyDevice(g_ctx.device, NULL);
    vkDestroySurfaceKHR(g_ctx.instance, g_ctx.surface, NULL);
    vkDestroyInstance(g_ctx.instance, NULL);
    glfwDestroyWindow(g_ctx.window);
    glfwTerminate();
}
// ========================================================================================================================================================================
// ========================================================================================================================================================================
// ========================================================================================================================================================================
void drawGeometry(float posX, float posY, QGPU_Vertex* vertices, uint32_t vCount, uint32_t* indices, uint32_t iCount) {
    if (vCount == 0 || iCount == 0) return;
    if (g_ctx.currentVOffset + vCount >= MAX_VERTICES || g_ctx.currentIOffset + iCount >= MAX_VERTICES * 1.5f) return;
    int w, h; glfwGetFramebufferSize(g_ctx.window, &w, &h);
    uint32_t pushData[5];
    float fPosX = posX, fPosY = posY, fW = (float)w, fH = (float)h;
    memcpy(&pushData[0], &fPosX, 4);
    memcpy(&pushData[1], &fPosY, 4);
    memcpy(&pushData[2], &fW, 4);
    memcpy(&pushData[3], &fH, 4);
    pushData[4] = g_currentRenderType;
    vkCmdPushConstants(g_ctx.currentCmd, g_ctx.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, 20, pushData);
    QGPU_Vertex* vDst = (QGPU_Vertex*)g_ctx.mappedVertexBuffer + g_ctx.currentVOffset;
    memcpy(vDst, vertices, vCount * sizeof(QGPU_Vertex));
    uint32_t* iDst = (uint32_t*)g_ctx.mappedIndexBuffer + g_ctx.currentIOffset;
    memcpy(iDst, indices, iCount * sizeof(uint32_t));
    vkCmdDrawIndexed(g_ctx.currentCmd, iCount, 1, g_ctx.currentIOffset, g_ctx.currentVOffset, 0);
    g_ctx.currentVOffset += vCount;
    g_ctx.currentIOffset += iCount;
}
// ========================================================================================================================================================================
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
// ========================================================================================================================================================================
void loadTexture(const char* filename, int slot) {
    if (slot < 0 || slot >= MAX_TEXTURES) return;
    if (txts[slot].pixels != NULL) {
        vkDestroyBuffer(g_ctx.device, txts[slot].buffer, NULL);
        vkFreeMemory(g_ctx.device, txts[slot].memory, NULL);
        free(txts[slot].pixels);
        txts[slot].pixels = NULL;
    }
    FILE* file = fopen(filename, "r");
    if (!file) { print("Cannot find texture '%s'!", filename); return; }
    char line[16];
    int width = 0, height = 0, currentPixel = 0;
    if (fgets(line, sizeof(line), file)) { sscanf(line, "%d %d", &width, &height); }
    int pixelCount = width * height;
    size_t ssboSize = sizeof(uint32_t) * 2 + (pixelCount * sizeof(uint32_t));
    uint32_t* ssboData = (uint32_t*)malloc(ssboSize);
    if (!ssboData) { fclose(file); return; }
    ssboData[0] = (uint32_t)width;
    ssboData[1] = (uint32_t)height;
    while (fgets(line, sizeof(line), file) && currentPixel < pixelCount) {
        int r, g, b, a; if (sscanf(line, "%d %d %d %d", &r, &g, &b, &a) == 4) {
            uint32_t packedColor = ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | (uint32_t)r;
            ssboData[2 + currentPixel] = packedColor;
            currentPixel++;
        }
    }
    fclose(file);
    createBuffer(ssboSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &txts[slot].buffer, &txts[slot].memory);
    void* mappedData;
    vkMapMemory(g_ctx.device, txts[slot].memory, 0, ssboSize, 0, &mappedData);
    memcpy(mappedData, ssboData, ssboSize);
    vkUnmapMemory(g_ctx.device, txts[slot].memory);
    free(ssboData);
    VkDescriptorBufferInfo bufferInfo = { .buffer = txts[slot].buffer, .offset = 0, .range = ssboSize };
    VkWriteDescriptorSet descriptorWrite = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = g_descriptorSets[slot],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .pBufferInfo = &bufferInfo
    };
    vkUpdateDescriptorSets(g_ctx.device, 1, &descriptorWrite, 0, NULL);
    txts[slot].pixels = (void*)1;
    txts[slot].width = width;
    txts[slot].height = height;
    txts[slot].pixelCount = pixelCount;
    printf("Loaded texture '%s' to SSBO slot '%d' (%dx%d)\n", filename, slot, width, height);
}
void drawTextureScale(float posX, float posY, int slot, float scale) {
    if (slot < 0 || slot >= MAX_TEXTURES || txts[slot].pixels == NULL) return;
    float w = (txts[slot].width * scale) / 2.0f, h = (txts[slot].height * scale) / 2.0f;
    QGPU_Vertex v[] = {
        {{ -w,  h }, {0.0f, 0.0f, 1.0f, 1.0f}},
        {{  w,  h }, {1.0f, 0.0f, 1.0f, 1.0f}},
        {{  w, -h }, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ -w, -h }, {0.0f, 1.0f, 1.0f, 1.0f}}
    };
    uint32_t i[] = {0, 1, 2,  0, 2, 3};
    vkCmdBindDescriptorSets(g_ctx.currentCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g_ctx.pipelineLayout, 0, 1, &g_descriptorSets[slot], 0, NULL);
    g_currentRenderType = 1;
    drawGeometry(posX, posY, v, 4, i, 6);
    g_currentRenderType = 0;
}
// ========================================================================================================================================================================
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
    *y = -(ly - (double)getHeight() / 2);
}
int getWidth() { int w, h; if (!g_ctx.window) return 0; glfwGetWindowSize(g_ctx.window, &w, &h); return w; }
int getHeight() { int w, h; if (!g_ctx.window) return 0; glfwGetWindowSize(g_ctx.window, &w, &h); return h; }
// ========================================================================================================================================================================
int drawButton(float posX, float posY, float width, float height, QColor clr, QColor hoverClr, QColor pressClr) {
    double mx, my; getMousePos(&mx, &my);
    int hovered = AABB((float)mx, (float)my, posX, posY, width, height), o = 0;
    if (hovered) { if (onMouse(LMB)) { o = 1; } else if (getMouse(LMB)) { o = 2; } }
    drawRect(posX, posY, width, height, hovered ? (o == 0 ? hoverClr : pressClr) : clr);
    return o;
}
int drawSlider(float posX, float posY, float width, float height, float handleW, float handleH, float* value, float min, float max, QColor backgroundClr, QColor fillClr, QColor handleClr) {
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
int drawToggle(float posX, float posY, float width, float height, int* value, QColor offClr, QColor onClr) {
    double mx, my; getMousePos(&mx, &my);
    int hovered = AABB((float)mx, (float)my, posX, posY, width, height);
    int m = hovered && onMouse(LMB);
    if (m) { *value = !*value; }
    drawRect(posX, posY, width, height, *value == 0 ? offClr : onClr);
    return m;
}
// ========================================================================================================================================================================
static const unsigned char font_basic[138][8] = {
    [128]  = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Full block
    [129]  = {0x18, 0x3C, 0x5A, 0x99, 0x18, 0x18, 0x18, 0x00}, // Up arrow
    [130]  = {0x18, 0x18, 0x18, 0x99, 0x5A, 0x3C, 0x18, 0x00}, // Down arrow
    [131]  = {0x10, 0x30, 0x60, 0xFF, 0x60, 0x30, 0x10, 0x00}, // Left arrow
    [132]  = {0x08, 0x0C, 0x06, 0xFF, 0x06, 0x0C, 0x08, 0x00}, // Right arrow
    [133]  = {0x18, 0x24, 0x24, 0x7e, 0x7e, 0x7e, 0x7e, 0x00}, // Locker
    [134]  = {0x00, 0x18, 0x3c, 0x7e, 0x3c, 0x3c, 0x3c, 0x00}, // Home
    [135]  = {0x00, 0x24, 0x7e, 0x7e, 0x7e, 0x3c, 0x18, 0x00}, // Heart
    [136]  = {0x00, 0x60, 0x5c, 0x4e, 0x42, 0x42, 0x7e, 0x00}, // Folder
    [137]  = {0x00, 0x7c, 0x4a, 0x46, 0x42, 0x42, 0x7e, 0x00}, // File
    [' ']  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['!']  = {0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x00},
    ['"']  = {0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['#']  = {0x24, 0x24, 0x7E, 0x24, 0x7E, 0x24, 0x24, 0x00},
    ['$']  = {0x18, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x18, 0x00},
    ['%']  = {0x62, 0x66, 0x0C, 0x18, 0x30, 0x66, 0x46, 0x00},
    ['&']  = {0x38, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0x76, 0x00},
    ['\''] = {0x18, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['(']  = {0x0C, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00},
    [')']  = {0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00},
    ['*']  = {0x00, 0x14, 0x08, 0x3E, 0x08, 0x14, 0x00, 0x00},
    ['+']  = {0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00},
    [',']  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30},
    ['-']  = {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00},
    ['.']  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00},
    ['/']  = {0x02, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00},

    [':']  = {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00},
    [';']  = {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30, 0x00},
    ['<']  = {0x0C, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0C, 0x00},
    ['=']  = {0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00},
    ['>']  = {0x30, 0x18, 0x0C, 0x03, 0x0C, 0x18, 0x30, 0x00},
    ['?']  = {0x3C, 0x66, 0x06, 0x0C, 0x18, 0x00, 0x18, 0x00},
    ['@']  = {0x3C, 0x42, 0x9D, 0xA1, 0xA1, 0x9D, 0x40, 0x3E},

    ['[']  = {0x3C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3C, 0x00},
    ['\\'] = {0x40, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00},
    [']']  = {0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3C, 0x00},
    ['^']  = {0x18, 0x24, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['_']  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00},
    ['`']  = {0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},

    ['{']  = {0x0E, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0E, 0x00},
    ['|']  = {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
    ['}']  = {0x70, 0x18, 0x18, 0x0C, 0x18, 0x18, 0x70, 0x00},
    ['~']  = {0x3A, 0x5C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

    ['0']  = {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00},
    ['1']  = {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
    ['2']  = {0x3C, 0x66, 0x06, 0x0C, 0x30, 0x60, 0x7E, 0x00},
    ['3']  = {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00},
    ['4']  = {0x0C, 0x1C, 0x2C, 0x4C, 0x7E, 0x0C, 0x0C, 0x00},
    ['5']  = {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00},
    ['6']  = {0x1C, 0x30, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00},
    ['7']  = {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00},
    ['8']  = {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00},
    ['9']  = {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00},

    ['A']  = {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00},
    ['B']  = {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
    ['C']  = {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00},
    ['D']  = {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00},
    ['E']  = {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00},
    ['F']  = {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00},
    ['G']  = {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3E, 0x00},
    ['H']  = {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    ['I']  = {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
    ['J']  = {0x1E, 0x06, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00},
    ['K']  = {0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00},
    ['L']  = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00},
    ['M']  = {0x66, 0xFF, 0xDB, 0xDB, 0xC3, 0xC3, 0xC3, 0x00},
    ['N']  = {0x66, 0x76, 0x7E, 0x6E, 0x66, 0x66, 0x66, 0x00},
    ['O']  = {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    ['P']  = {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00},
    ['Q']  = {0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x1E, 0x03},
    ['R']  = {0x7C, 0x66, 0x66, 0x7C, 0x78, 0x6C, 0x66, 0x00},
    ['S']  = {0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00},
    ['T']  = {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
    ['U']  = {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    ['V']  = {0x66, 0x66, 0x66, 0x66, 0x24, 0x24, 0x18, 0x00},
    ['W']  = {0xC6, 0xC6, 0xC6, 0xD6, 0xD6, 0xEE, 0x6C, 0x00},

    ['X']  = {0x66, 0x66, 0x24, 0x18, 0x24, 0x66, 0x66, 0x00},
    ['Y']  = {0x66, 0x66, 0x24, 0x18, 0x18, 0x18, 0x18, 0x00},
    ['Z']  = {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00},

    ['a']  = {0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00},
    ['b']  = {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x00},
    ['c']  = {0x00, 0x00, 0x3C, 0x60, 0x60, 0x66, 0x3C, 0x00},
    ['d']  = {0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x00},
    ['e']  = {0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00},
    ['f']  = {0x1C, 0x22, 0x20, 0x7C, 0x20, 0x20, 0x20, 0x00},
    ['g']  = {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x7C},
    ['h']  = {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00},
    ['i']  = {0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00},
    ['j']  = {0x06, 0x00, 0x0E, 0x06, 0x06, 0x06, 0x06, 0x3C},
    ['k']  = {0x60, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x00},
    ['l']  = {0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
    ['m']  = {0x00, 0x00, 0x6C, 0xFE, 0xD6, 0xC6, 0xC6, 0x00},
    ['n']  = {0x00, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00},
    ['o']  = {0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00},
    ['p']  = {0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60},
    ['q']  = {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x06},
    ['r']  = {0x00, 0x00, 0x7C, 0x66, 0x60, 0x60, 0x60, 0x00},
    ['s']  = {0x00, 0x00, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x00},
    ['t']  = {0x20, 0x20, 0x7C, 0x20, 0x20, 0x22, 0x1C, 0x00},
    ['u']  = {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00},
    ['v']  = {0x00, 0x00, 0x66, 0x66, 0x24, 0x24, 0x18, 0x00},
    ['w']  = {0x00, 0x00, 0xC6, 0xD6, 0xD6, 0x7C, 0x28, 0x00},
    ['x']  = {0x00, 0x00, 0x66, 0x24, 0x18, 0x24, 0x66, 0x00},
    ['y']  = {0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x7C},
    ['z']  = {0x00, 0x00, 0x7E, 0x0C, 0x18, 0x30, 0x7E, 0x00}
};
void drawChar(float posX, float posY, unsigned char symbol, float scale, QColor color) {
    if (symbol >= 138) return;
    for (int row = 0; row < 8; row++) {
        unsigned char row_byte = font_basic[symbol][row]; for (int col = 0; col < 8; col++)
        { if ((row_byte >> (7 - col)) & 1) { float pixelX = posX + (col * scale), pixelY = posY - (row * scale); drawRect(pixelX, pixelY, scale, scale, color); } }
    }
}
void drawText(float posX, float posY, char* text, float scale, QColor color) {
    float sx = CHAR_SIZE * scale * 1.25, sy = CHAR_SIZE * scale * 1.25, cx = posX, cy = posY;
    while (*text) {
        if (*text == '\n') { cx = posX; cy -= sy; text++; continue; }
        if (*text == ' ') { cx += sx; text++; continue; }
        drawChar(cx, cy, *text, scale, color);
        cx += sx; text++;
    }
}
