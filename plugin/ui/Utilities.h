//
// Created by Erik Jourgensen on 10/28/26.
//
#pragma once

#include <cassert>
#include <iostream>
#include <webgpu/webgpu.h>
#include <filesystem>

static constexpr uint32_t kMSAASamples = 4;

inline WGPUAdapter requestAdapterSync(WGPUInstance instance, WGPURequestAdapterOptions const* options) {
    struct UserData {
        WGPUAdapter adapter  = nullptr;
        bool requestEnded    = false;
    };
    UserData userData;

    wgpuInstanceRequestAdapter(instance, options,
                               [](WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* pUserData) {
                                   UserData& userDataReference = *reinterpret_cast<UserData*>(pUserData);
                                   if (status == WGPURequestAdapterStatus_Success)
                                       userDataReference.adapter = adapter;
                                   else
                                       std::cout << "Could not get WebGPU Adapter: " << message.data << std::endl;
                                   userDataReference.requestEnded = true;
                               },
                               &userData
    );

    while (!userData.requestEnded) wgpuInstanceProcessEvents(instance);
    assert(userData.requestEnded);
    return userData.adapter;
}

inline WGPUDevice requestDeviceSync(WGPUInstance instance, WGPUAdapter adapter, WGPUDeviceDescriptor const* descriptor) {
    struct UserData {
        WGPUDevice device  = nullptr;
        bool requestEnded    = false;
    };
    UserData userData;

    wgpuAdapterRequestDevice(adapter, descriptor,
                             [](WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void* pUserData) {
                                 UserData& userDataReference = *reinterpret_cast<UserData*>(pUserData);
                                 if (status == WGPURequestDeviceStatus_Success)
                                     userDataReference.device = device;
                                 else
                                     std::cout << "Could not get WebGPU Device: " << message.data << std::endl;
                                 userDataReference.requestEnded = true;
                             },
                             &userData
    );

    while (!userData.requestEnded) wgpuInstanceProcessEvents(instance);
    assert(userData.requestEnded);
    return userData.device;
}

#ifdef DEBUG
inline std::filesystem::file_time_type latestWriteTime(const std::vector<std::filesystem::path>& paths) {
    std::filesystem::file_time_type latest{};
    for (const auto& p : paths)
        latest = std::max(latest, std::filesystem::last_write_time(p));
    return latest;
}
#endif

