#include <numeric>
#include <iomanip>
#include <imgui/imgui.h>
#include "../Common.h"
#include "../GLM.h"
#include "../OpenGL.h"

class EditorStats
{
public:
    EditorStats();
    void Draw(double dt);

private:
    // FPS graph
    const unsigned int m_SampleSize = 100;
    unsigned int m_FrameCount = 0;
    std::vector<double> m_FrameTimes;
    double m_TimeAccumulator = 0.0;
    double m_AveragedSamplesPerSecond = 10.0;
    const unsigned int m_AveragedSampleSize = 100;
    unsigned int m_CurrentAveragedSampleIndex = 0;
    std::vector<double> m_AveragedSamples;
    void drawFPSGraph(double dt);

    void drawRAMUsage(double dt);

    void drawVRAMStats(double dt);
};
