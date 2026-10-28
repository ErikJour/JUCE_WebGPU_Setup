//
// Created by Erik Jourgensen on 10/28/26.
//

#pragma once
#include <filesystem>
#include <webgpu/webgpu.h>
#include <utility>
#include <vector>
#include "Utilities.h"
#include "ResourceManager.h"
#include "MyUniforms.h"
#include "shaderPaths.h"
//#include "PlaneGeometry.h"

#define WGPU_STR(s) WGPUStringView { s, sizeof(s) -1 };
static constexpr uint32_t materialCount = 2;

class Scene {
public:
    Scene()  = default;
    ~Scene() = default;
    //===================================================================================
    void init(WGPUDevice device, WGPUQueue queue);
    void setSurface(WGPUSurface surface);
    void setSurfaceFormat(WGPUTextureFormat format);
    void setSurfaceSize(uint32_t width, uint32_t height);
    void setShaderModule(WGPUShaderModule shaderModule);
    void terminate();
    //===================================================================================
    bool createShader();
    void reloadShader();
    //===================================================================================
    void configureVertexLayout();
    void setPipelineDesc(const WGPURenderPipelineDescriptor &pipelineDesc);
    bool createPipeline();
    void updateTexture(uint32_t width, uint32_t height);
    void setUniforms();
    void renderMeshes(WGPURenderPassEncoder renderPass) const;
    void renderFrame();
    void initializeScene();
    //===================================================================================
    //Mesh Work
    //===================================================================================
    void setMeshBuffers(WGPUBuffer vertexBuffer, WGPUBuffer indexBuffer, uint32_t indexCount, uint32_t material, WGPURenderPassEncoder renderPass) const;
    //===================================================================================
    //Mouse and Key Input
    //===================================================================================
    //===================================================================================
    //Public Getter Functions
    //===================================================================================
    std::pair<WGPUSurfaceTexture, WGPUTextureView> getNextSurfaceViewData() const;
    WGPUFragmentState    getFragmentState() const { return mFragmentState; }
    WGPUColorTargetState getColorTarget()   const { return mColorTarget; }
    WGPUBlendState       getBlendState()    const { return mBlendState; }


private:
    WGPUFragmentState            mFragmentState             = {};
    WGPUColorTargetState         mColorTarget               = {};
    WGPUBlendState               mBlendState                = {};
    WGPURenderPipeline           mPipeline                  = {};
    WGPUSurface                  mSurface                   = nullptr;
    WGPUDevice                   mDevice                    = nullptr;
    WGPUQueue                    mQueue                     = nullptr;
    WGPUTextureFormat            mSurfaceFormat             = WGPUTextureFormat_Undefined;
    uint32_t                     mWidth                     = 0;
    uint32_t                     mHeight                    = 0;
    WGPUBuffer                   mUniformBuffer             = nullptr;
    WGPUBindGroup                mBindGroup                 = nullptr;
    std::array<WGPUVertexAttribute, 3> mVertexAttributes    = {};
    std::vector<WGPUVertexBufferLayout> mVertexBufferLayout = {};
    uint32_t                     mUniformStride             = 0;
    MyUniforms                   mUniforms                  = {};
    WGPURenderPipelineDescriptor mPipelineDesc              = {};
    WGPUShaderModule             mShaderModule              = {};
    std::vector<std::filesystem::path> mShaderPaths;
    std::filesystem::file_time_type mLastShaderWriteTime;
    WGPUTextureView              mTextureView               = nullptr;
    WGPUTexture                  mTexture                   = nullptr;

    //Mesh Buffers
    WGPUBuffer                   mPlaneVertexBuffer         = nullptr;
    WGPUBuffer                   mPlaneIndexBuffer          = nullptr;
    uint32_t                     mPlaneIndexCount           = 0;
    WGPUBuffer                   mSphereVertexBuffer        = nullptr;
    WGPUBuffer                   mSphereIndexBuffer         = nullptr;
    uint32_t                     mSphereIndexCount          = 0;

    //Input
    float mMouseX         = 0.0f;
    float mMouseY         = 0.0f;
    bool  mMouseClicked   = false;
    float mButtonOffsetX  = 0.0f;
};

