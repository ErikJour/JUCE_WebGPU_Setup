//
// Created by Erik Jourgensen on 10/28/26.
//

#include "Scene.h"
#include <iostream>
#include <ostream>
//#include "SphereGeometry.h"
//===================================================================================
//Setup
//====================================================================================
void Scene::init(WGPUDevice device, WGPUQueue queue)                          { mDevice = device; mQueue = queue;        }
void Scene::setSurface(const WGPUSurface surface)                             { mSurface                 = surface;      }
void Scene::setSurfaceFormat(const WGPUTextureFormat format)                  { mSurfaceFormat           = format;       }
void Scene::setSurfaceSize(const uint32_t width, const uint32_t height)       { mWidth = width; mHeight  = height;       }
void Scene::setShaderModule(const WGPUShaderModule shaderModule)              { mShaderModule            = shaderModule; }
void Scene::setPipelineDesc(const WGPURenderPipelineDescriptor &pipelineDesc) { mPipelineDesc            = pipelineDesc; }
void Scene::terminate()
{
    if (mPlaneIndexBuffer)   { wgpuBufferRelease(mPlaneIndexBuffer);                           mPlaneIndexBuffer   = nullptr; }
    if (mPlaneVertexBuffer)  { wgpuBufferRelease(mPlaneVertexBuffer);                          mPlaneVertexBuffer  = nullptr; }
    if (mSphereIndexBuffer)  { wgpuBufferRelease(mSphereIndexBuffer);                          mSphereIndexBuffer  = nullptr; }
    if (mSphereVertexBuffer) { wgpuBufferRelease(mSphereVertexBuffer);                         mSphereVertexBuffer = nullptr; }
    if (mBindGroup)          { wgpuBindGroupRelease(mBindGroup);                               mBindGroup          = nullptr; }
    if (mUniformBuffer)      { wgpuBufferRelease(mUniformBuffer);                              mUniformBuffer      = nullptr; }
    if (mPipeline)           { wgpuRenderPipelineRelease(mPipeline);                           mPipeline           = nullptr; }
    if (mSurface)            { wgpuSurfaceUnconfigure(mSurface); wgpuSurfaceRelease(mSurface); mSurface            = nullptr; }
}
//===================================================================================
//Shaders
//===================================================================================
bool Scene::createShader() {
#ifdef DEBUG
    mShaderPaths         = getShaderPaths();
    mLastShaderWriteTime = latestWriteTime(mShaderPaths);
    mShaderModule        = ResourceManager::loadShaderModules(mShaderPaths, mDevice);
#endif
    if (!mShaderModule) {
        std::cerr << "Failed to create shader module." << std::endl;
        return false;
    }
    return true;
}

void Scene::reloadShader()
{
    const WGPUShaderModule newShaderModule = ResourceManager::loadShaderModules(mShaderPaths, mDevice);
    if (!newShaderModule) {
        std::cerr << "Shader compile failed" << std::endl;
        return;
    }
    wgpuShaderModuleRelease(mShaderModule);
    mShaderModule = newShaderModule;
    mPipelineDesc.vertex.module = mShaderModule;
    createPipeline();
    std::cout << "Shader reloaded" << std::endl;
}
//====================================================================================
//Rendering
//====================================================================================
void Scene::configureVertexLayout() {
    //==========================================================
    //Position
    //=========================================================
    mVertexAttributes[0].shaderLocation = 0;
    mVertexAttributes[0].format         = WGPUVertexFormat_Float32x3;
    mVertexAttributes[0].offset         = 0;
    //==========================================================
    //Normal
    //=========================================================
    mVertexAttributes[1].shaderLocation = 1;
    mVertexAttributes[1].format         = WGPUVertexFormat_Float32x3;
    mVertexAttributes[1].offset         = 3 * sizeof(float);;
    //==========================================================
    //Color
    //=========================================================
    mVertexAttributes[2].shaderLocation = 2;
    mVertexAttributes[2].format         = WGPUVertexFormat_Float32x3;
    mVertexAttributes[2].offset         = 6 * sizeof(float);

    mVertexBufferLayout.resize(1);
    mVertexBufferLayout[0].attributeCount = 3;
    mVertexBufferLayout[0].attributes     = mVertexAttributes.data();
    mVertexBufferLayout[0].arrayStride    = 9 * sizeof(float);
    mVertexBufferLayout[0].stepMode       = WGPUVertexStepMode_Vertex;

    mPipelineDesc.vertex.bufferCount      = 1;
    mPipelineDesc.vertex.buffers          = mVertexBufferLayout.data();
}
bool Scene::createPipeline()
{
    //====================================================================================
    //Configure communication from the CPU to the GPU
    //====================================================================================
    //Release bind group, uniform buffer, and pipeline so we can reset these
    //====================================================================================
    if (mBindGroup)     { wgpuBindGroupRelease(mBindGroup);     mBindGroup     = nullptr; }
    if (mUniformBuffer) { wgpuBufferRelease(mUniformBuffer);    mUniformBuffer = nullptr; }
    if (mPipeline)      { wgpuRenderPipelineRelease(mPipeline); mPipeline      = nullptr; }
    //====================================================================================
    //Rules for sending CPU data to the GPU
    //====================================================================================
    static constexpr uint32_t kAlignment      = 256;
    mUniformStride                            = (sizeof(MyUniforms) + kAlignment - 1) & ~(kAlignment - 1);
    WGPUBindGroupLayoutEntry bglEntry         = {};
    bglEntry.binding                          = 0;
    bglEntry.visibility                       = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    bglEntry.buffer.type                      = WGPUBufferBindingType_Uniform;
    bglEntry.buffer.hasDynamicOffset          = true;
    bglEntry.buffer.minBindingSize            = sizeof(MyUniforms);
    WGPUBindGroupLayoutDescriptor bglDesc     = {};
    bglDesc.entryCount                        = 1;
    bglDesc.entries                           = &bglEntry;
    const WGPUBindGroupLayout bglLayout       = wgpuDeviceCreateBindGroupLayout(mDevice, &bglDesc);
    WGPUPipelineLayoutDescriptor layoutDesc   = {};
    layoutDesc.label                          = WGPU_STR("Layout Descriptor");
    layoutDesc.bindGroupLayoutCount           = 1;
    layoutDesc.bindGroupLayouts               = &bglLayout;

    if (mPipelineDesc.layout) {
        wgpuPipelineLayoutRelease(mPipelineDesc.layout);
        mPipelineDesc.layout = nullptr;
    }
    mPipelineDesc.layout                      = wgpuDeviceCreatePipelineLayout(mDevice, &layoutDesc);
    //====================================================================================
    //Configure Shaders
    //====================================================================================
    mColorTarget.format         = mSurfaceFormat;          //This is our pixel format
    mColorTarget.blend          = &mBlendState;            //This is our blend format
    mColorTarget.writeMask      = WGPUColorWriteMask_All; //All color channels can be written to
    mPipelineDesc.vertex.module = mShaderModule;
    mPipelineDesc.label         = WGPU_STR("Pipeline");
    mFragmentState.module       = mShaderModule;
    mFragmentState.entryPoint   = WGPU_STR("fs_main");
    mFragmentState.targetCount  = 1;
    mFragmentState.targets      = &mColorTarget;
    mFragmentState.constants    = nullptr;
    mPipelineDesc.fragment      = &mFragmentState;

    if (!mDevice) {
        std::cout << "Failed to create device" << std::endl;
        return false;
    }

    mPipeline                   = wgpuDeviceCreateRenderPipeline(mDevice, &mPipelineDesc);

    if (!mPipeline) {
        std::cerr << "Failed to create render pipeline." << std::endl;
        return false;
    }
    //====================================================================================
    //Allocate GPU Memory
    //====================================================================================
    WGPUBufferDescriptor bufferDesc = {};
    bufferDesc.label               = { "Uniform buffer", strlen("Uniform buffer") };
    bufferDesc.size                 = materialCount *  mUniformStride;
    bufferDesc.usage                = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
    mUniformBuffer                  = wgpuDeviceCreateBuffer(mDevice, &bufferDesc);
    WGPUBindGroupEntry bgEntry      = {};
    bgEntry.binding                 = 0;
    bgEntry.buffer                  = mUniformBuffer;
    bgEntry.offset                  = 0;
    bgEntry.size                    = sizeof(MyUniforms);
    WGPUBindGroupDescriptor bgDesc  = {};
    bgDesc.layout                   = bglLayout;
    bgDesc.entryCount               = 1;
    bgDesc.entries                  = &bgEntry;
    mBindGroup                      = wgpuDeviceCreateBindGroup(mDevice, &bgDesc);
    updateTexture(mWidth, mHeight);
    wgpuBindGroupLayoutRelease(bglLayout);

    return true;
}

void Scene::updateTexture(const uint32_t width, const uint32_t height)
{
    //====================================================================================
    //Assign width and height
    //====================================================================================
    mWidth                             = width;
    mHeight                            = height;
    //====================================================================================
    //Release texture view and texture
    //====================================================================================
    if (mTextureView) { wgpuTextureViewRelease(mTextureView); mTextureView = nullptr; }
    if (mTexture)     { wgpuTextureRelease(mTexture);         mTexture     = nullptr; }
    //====================================================================================
    //Setup texture and texture view descriptors
    //====================================================================================
    WGPUTextureDescriptor desc         = {};
    desc.usage                         = WGPUTextureUsage_RenderAttachment;
    desc.dimension                     = WGPUTextureDimension_2D;
    desc.size                       = { mWidth, mHeight, 1 };
    desc.format                        = WGPUTextureFormat_Depth24Plus;
    desc.mipLevelCount                 = 1;
    desc.sampleCount                   = 1;
    WGPUTextureViewDescriptor descView = {};
    descView.format                    = WGPUTextureFormat_Depth24Plus;
    descView.dimension                 = WGPUTextureViewDimension_2D;
    descView.mipLevelCount             = 1;
    descView.arrayLayerCount           = 1;
    descView.aspect                    = WGPUTextureAspect_DepthOnly;
    //====================================================================================
    //Assign texture and texture view
    //====================================================================================
    mTexture                           = wgpuDeviceCreateTexture(mDevice, &desc);
    mTextureView                       = wgpuTextureCreateView(mTexture, &descView);
}

std::pair<WGPUSurfaceTexture, WGPUTextureView> Scene::getNextSurfaceViewData() const {
    //====================================================================================
    //Setup our surface to draw into - turn this into a single function that returns mSurfaceTexture and mTargetView
    //====================================================================================
    WGPUSurfaceTexture surfaceTexture = {};
    wgpuSurfaceGetCurrentTexture(mSurface, &surfaceTexture);
    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
        return { surfaceTexture, nullptr };
    }
    //====================================================================================
    //Let's format our blank texture
    //====================================================================================
    WGPUTextureViewDescriptor viewDescriptor = {};
    viewDescriptor.nextInChain               = nullptr;
    viewDescriptor.label                     = WGPU_STR("Surface texture view");
    viewDescriptor.format                    = mSurfaceFormat;
    viewDescriptor.dimension                 = WGPUTextureViewDimension_2D;
    viewDescriptor.mipLevelCount             = 1;
    viewDescriptor.arrayLayerCount           = 1;
    viewDescriptor.aspect                    = WGPUTextureAspect_All;
    const WGPUTextureView targetView         = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

    return { surfaceTexture, targetView };
}

void Scene::setUniforms(const float time)
{
    mUniforms.time = time;

    mUniforms.pressed = mMouseClicked;

    mUniforms.buttonX = mButtonOffsetX;

    for (uint32_t m = 0; m < materialCount; m++) {
        mUniforms.materialId = m;
        wgpuQueueWriteBuffer(mQueue, mUniformBuffer,
                             m * mUniformStride, &mUniforms, sizeof(MyUniforms));
    }
}

void Scene::renderMeshes(const WGPURenderPassEncoder renderPass) const {

    setMeshBuffers(mPlaneVertexBuffer, mPlaneIndexBuffer, mPlaneIndexCount, MAT_PLANE, renderPass);
    setMeshBuffers(mSphereVertexBuffer, mSphereIndexBuffer, mSphereIndexCount, MAT_SPHERE, renderPass);
}

void Scene::renderFrame(const float time) {
    //====================================================================================
    //Add reloadable shader setup here
    //====================================================================================
#ifdef DEBUG
    auto writeTime = latestWriteTime(mShaderPaths);
    if (writeTime != mLastShaderWriteTime) {
        mLastShaderWriteTime = writeTime;
        reloadShader();
    }
#endif
    if (!mPipeline) return;
    if (!mSurface)  return;
    setUniforms(time);
    //====================================================================================
    //Get the surface texture and the target view
    //====================================================================================
    auto [ surfaceTexture, targetView ] = getNextSurfaceViewData();
    wgpuTextureRelease(surfaceTexture.texture);
    //====================================================================================
    //Create an object on the CPU side to record our drawing commands
    //====================================================================================
    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.label = WGPU_STR("Frame encoder");
    const WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(mDevice, &encoderDesc);
    //====================================================================================
    //Clear the canvas and set color before draw
    //====================================================================================
    WGPURenderPassColorAttachment colorAttachment = {};
    colorAttachment.view          = targetView;
    colorAttachment.loadOp        = WGPULoadOp_Clear;
    colorAttachment.storeOp       = WGPUStoreOp_Store;
    colorAttachment.clearValue = { 0.0, 0.0, 0.0, 1.0 };
    colorAttachment.depthSlice    = WGPU_DEPTH_SLICE_UNDEFINED;
    //====================================================================================
    //Setup master redner pass
    //====================================================================================
    WGPURenderPassDescriptor renderPassDesc = {};
    renderPassDesc.label                    = WGPU_STR("Render pass");
    renderPassDesc.colorAttachmentCount     = 1;
    renderPassDesc.colorAttachments         = &colorAttachment;
    //====================================================================================
    //Configure the depth buffer
    //====================================================================================
    WGPURenderPassDepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.view            = mTextureView;
    depthStencilAttachment.depthClearValue = 1.0f;
    depthStencilAttachment.depthLoadOp     = WGPULoadOp_Clear;
    depthStencilAttachment.depthStoreOp    = WGPUStoreOp_Store;
    depthStencilAttachment.depthReadOnly   = false;
    depthStencilAttachment.stencilLoadOp   = WGPULoadOp_Undefined;
    depthStencilAttachment.stencilStoreOp  = WGPUStoreOp_Undefined;
    depthStencilAttachment.stencilReadOnly = true;
    renderPassDesc.depthStencilAttachment  = &depthStencilAttachment;
    //====================================================================================
    //Begin render pass
    //====================================================================================
    const WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
    //====================================================================================
    //Load shaders
    //====================================================================================
    wgpuRenderPassEncoderSetPipeline(renderPass, mPipeline);
    renderMeshes(renderPass);
    //====================================================================================
    //Finish the render
    //====================================================================================
    wgpuRenderPassEncoderEnd(renderPass);
    wgpuRenderPassEncoderRelease(renderPass);
    //====================================================================================
    //Free the CPU
    //====================================================================================
    //====================================================================================
    //Pass our data to the GPU
    //====================================================================================
    WGPUCommandBufferDescriptor cmdDesc = {};
    cmdDesc.label                       = WGPU_STR("DFrame command buffer");
    const WGPUCommandBuffer command     = wgpuCommandEncoderFinish(encoder, &cmdDesc);
    wgpuCommandEncoderRelease(encoder);

    wgpuQueueSubmit(mQueue, 1, &command);
    //====================================================================================
    //Release Resources
    //====================================================================================
    wgpuCommandBufferRelease(command);
    wgpuTextureViewRelease(targetView);
    //====================================================================================
    //Present frame
    //====================================================================================
    wgpuSurfacePresent(mSurface);
    wgpuDeviceTick(mDevice);
}

void Scene::initializeScene() {

}

//===================================================================================
//Meshes
//====================================================================================
void Scene::setMeshBuffers(WGPUBuffer vertexBuffer, WGPUBuffer indexBuffer, uint32_t indexCount, uint32_t material, WGPURenderPassEncoder renderPass) const
{
    if (vertexBuffer && indexBuffer && indexCount > 0) {
        const uint32_t offset = material * mUniformStride;
        wgpuRenderPassEncoderSetBindGroup(renderPass, 0, mBindGroup, 1, &offset);
        wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, vertexBuffer, 0, wgpuBufferGetSize(vertexBuffer));
        wgpuRenderPassEncoderSetIndexBuffer(renderPass, indexBuffer, WGPUIndexFormat_Uint16, 0, wgpuBufferGetSize(indexBuffer));
        wgpuRenderPassEncoderDrawIndexed(renderPass, indexCount, 1, 0, 0, 0);
    }
}


