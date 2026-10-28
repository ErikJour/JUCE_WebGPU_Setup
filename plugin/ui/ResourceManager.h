//
// Created by Erik Jourgensen on 10/28/26.
//

#pragma once
#include <vector>
#include <filesystem>
#include <fstream>
#include "webgpu/webgpu.h"
#include <iostream>

class ResourceManager {
    public:
    static WGPUShaderModule loadShaderModules( const std::vector<std::filesystem::path>& paths, WGPUDevice device) {
        std::string combined;
        for (const auto& path : paths) {
            std::ifstream file(path);
            if (!file.is_open()) {
                std::cerr << "Could not open shader file: " << path << std::endl;
                return nullptr;
            }
            file.seekg(0, std::ios::end);
            const auto size = static_cast<size_t>(file.tellg());
            std::string source(size, ' ');
            file.seekg(0);
            file.read(source.data(), static_cast<std::streamsize>(size));
            combined += source;
            combined += '\n';
        }
        WGPUShaderModuleWGSLDescriptor shaderCodeDescriptor{};
        shaderCodeDescriptor.chain.next     = nullptr;
        shaderCodeDescriptor.chain.sType    = WGPUSType_ShaderSourceWGSL;
        shaderCodeDescriptor.code        = { combined.c_str(), combined.size() };
        WGPUShaderModuleDescriptor shaderModuleDescriptor{};
        shaderModuleDescriptor.nextInChain  = &shaderCodeDescriptor.chain;
        return wgpuDeviceCreateShaderModule(device, &shaderModuleDescriptor);
    }
};
