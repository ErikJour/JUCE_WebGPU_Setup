juce_add_plugin(JuceWebGpuSetup
        PLUGIN_NAME "Juce WebGPU Setup"
        COMPANY_NAME AnimatedInstruments
        IS_SYNTH true
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        PLUGIN_MANUFACTURER_CODE JOUR
        PLUGIN_CODE JWGP
        FORMATS AU VST3 Standalone
        COPY_PLUGIN_AFTER_BUILD TRUE
)

set_target_properties(JuceWebGpuSetup PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        COMPILE_WARNINGS_AS_ERROR ON
)

if (MSVC)
    target_compile_options(JuceWebGpuSetup PRIVATE /W4)
else()
    target_compile_options(JuceWebGpuSetup PRIVATE -Wall -Wextra -pedantic)
endif()

#WebGPU
target_copy_webgpu_binaries(JuceWebGpuSetup)
if (APPLE)
    set_target_properties(JuceWebGpuSetup PROPERTIES
            XCODE_GENERATE_SCHEME ON
            XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal"
            XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME "Yes"
    )
endif()

if(APPLE)
    target_sources(JuceWebGpuSetup PRIVATE
            ui/GpuSurface.mm)
    target_link_libraries(JuceWebGpuSetup PRIVATE
            "-framework QuartzCore"
            "-framework Metal")
endif()

target_sources(JuceWebGpuSetup
        PRIVATE
        source/JuceWebGpuEditor.cpp
        source/JuceWebGpuProcessor.cpp
        ui/webGpuWindow.cpp
        ui/Scene.cpp
)

target_include_directories(JuceWebGpuSetup
        PRIVATE
        include
        source
        ui
        shaders
)

target_link_libraries(JuceWebGpuSetup
        PRIVATE
        webgpu
        juce::juce_audio_utils
        juce::juce_gui_extra
        juce::juce_dsp
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(JuceWebGpuSetup
        PUBLIC
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

#==============================
#Shader files
#==============================
set(SHADER_FILES
        "shaders/vertex_main.wgsl"
        "shaders/fragment_main.wgsl"
#        "shaders/common.wgsl"
#        "shaders/mat_plane.wgsl"
#        "shaders/mat_sphere.wgsl"
)

set(SHADER_SOURCE "")
foreach(SHADER_FILE ${SHADER_FILES})
    file(READ "${SHADER_FILE}" SHADER_CHUNK)
    string(APPEND SHADER_SOURCE "${SHADER_CHUNK}\n")
endforeach()

target_compile_definitions(JuceWebGpuSetup
        PRIVATE
        DEBUG_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)

target_include_directories(JuceWebGpuSetup PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")



