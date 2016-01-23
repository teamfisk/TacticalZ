#include "Editor/EditorStats.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#endif
#include <GL/wglew.h>

EditorStats::EditorStats()
{
    m_AveragedSamples.push_back(0.0);
    m_CurrentAveragedSampleIndex = 1;
}

void EditorStats::Draw(double dt)
{
    if (ImGui::Begin("Stats")) {
        drawFPSGraph(dt);
        drawRAMUsage(dt);
        drawVRAMStats(dt);
    }
    ImGui::End();
}

void EditorStats::drawFPSGraph(double dt)
{
    if (m_FrameCount < m_SampleSize) {
        m_FrameTimes.push_back(dt);
    } else {
        m_FrameTimes[m_FrameCount % m_SampleSize] = dt;
    }
    m_FrameCount++;

    double average = 0.0;
    double max = 0.0;
    for (double t : m_FrameTimes) {
        average += t;
        max = std::max(max, t);
    }
    average /= m_FrameTimes.size();

    m_TimeAccumulator += dt;
    if (m_TimeAccumulator >= 1.0/m_AveragedSamplesPerSecond) {
        if (m_CurrentAveragedSampleIndex < m_AveragedSampleSize) {
            m_AveragedSamples.push_back(1.0/average);
        } else {
            m_AveragedSamples[m_CurrentAveragedSampleIndex % m_AveragedSampleSize] = 1.0/average;
        }
        m_CurrentAveragedSampleIndex++;
        m_TimeAccumulator = 0.0;
    }

    float maxFPS = 0.f;
    ImVector<float> values;
    int values_offset = m_CurrentAveragedSampleIndex % m_AveragedSampleSize;
    for (double d : m_AveragedSamples) {
        values.push_back(static_cast<float>(d));
        maxFPS = std::max(maxFPS, static_cast<float>(d));
    }
    std::stringstream header;
    header << std::round(1.0/average) << " FPS (" << std::setprecision(5) << average << " ms)";
    ImGui::PlotLines("##FPSGraph", values.Data, values.Size, values_offset, header.str().c_str(), 0.f, maxFPS + maxFPS/5.f, ImVec2(0, 100));
}

void EditorStats::drawRAMUsage(double dt)
{
#ifdef WIN32
    PROCESS_MEMORY_COUNTERS_EX ppm;
    GetProcessMemoryInfo(GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS)&ppm, sizeof(ppm));
    float megabytes = ppm.WorkingSetSize / (float)std::pow(1024, 2);
    ImGui::Text("Memory: ~%f MiB", megabytes);
#endif
}

void EditorStats::drawVRAMStats(double dt)
{
    //const unsigned int GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX = 0x9049;
    //const unsigned int GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX = 0x9048;
    //glm::ivec4 total;
    //glGetIntegerv(GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, glm::value_ptr(total));
    //if (glGetError() == GL_NO_ERROR) {
    //    glm::ivec4 available;
    //    glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, glm::value_ptr(available));
    //    float megabytes = (total.x - available.x) / 1024.f; // NVidia returns in KiB
    //    ImGui::Text("VRAM: %f", megabytes);
    //}

    //GLuint uNoOfGPUs = wglGetGPUIDsAMD(0, 0);
    //if (!GLERROR("")) {
    //    GLuint* uGPUIDs = new GLuint[uNoOfGPUs];
    //    wglGetGPUIDsAMD(uNoOfGPUs, uGPUIDs);
    //    GLuint uTotalMemoryInMB = 0;
    //    wglGetGPUInfoAMD(uGPUIDs[0],
    //        WGL_GPU_RAM_AMD,
    //        GL_UNSIGNED_INT,
    //        sizeof(GLuint),
    //        &uTotalMemoryInMB);
    //    GLint nCurAvailMemoryInKB[4];
    //    glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI,
    //        &nCurAvailMemoryInKB[0]);
    //    float usedTexture = (nCurAvailMemoryInKB[0] / 1024.f);
    //    glGetIntegerv(GL_VBO_FREE_MEMORY_ATI,
    //        &nCurAvailMemoryInKB[0]);
    //    float usedVBO = (nCurAvailMemoryInKB[0] / 1024.f);
    //    glGetIntegerv(GL_RENDERBUFFER_FREE_MEMORY_ATI,
    //        &nCurAvailMemoryInKB[0]);
    //    float usedFB = (nCurAvailMemoryInKB[0] / 1024.f);
    //    ImGui::Text("VRAM: %f MiB ", (float)uTotalMemoryInMB - usedTexture - usedVBO - usedFB);
    //    ImGui::Text("  Texture: %f MiB", usedTexture);
    //    ImGui::Text("  VBO: %f MiB", usedVBO);
    //    ImGui::Text("  Framebuffer: %f MiB", usedFB);
    //    delete[] uGPUIDs;
    //}
}
