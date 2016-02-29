#ifndef Sound_h__
#define Sound_h__

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include "Core/ResourceManager.h"

class Sound : public Resource
{
    friend class ResourceManager;
public:
    Sound(std::string path) { m_Buffer = LoadFile(path); m_Path = path; }
    ~Sound() { ClearBuffer(); }
    ALuint Buffer() { return m_Buffer; }
    std::string Path() { return m_Path; }
    float Gain() { return m_Gain; }
    void SetGain(float value) { m_Gain = value; }
    void ClearBuffer() { alDeleteBuffers(1, &m_Buffer); m_BufferCache.clear(); };

private:
    float m_Gain = 1;
    ALuint m_Buffer;
    std::string m_Path;

    // File info
    char m_Type[4];
    unsigned long m_Size, m_ChunkSize;
    short m_FormatType, m_Channels;
    unsigned long m_SampleRate, m_AvgBytesPerSec;
    short m_BytesPerSample, m_BitsPerSample;
    unsigned int m_DataSize;
    std::map<std::string, ALuint> m_BufferCache;

    ALuint LoadFile(std::string path)
    {
        if (m_BufferCache.find(path) != m_BufferCache.end()) {
            return m_BufferCache[path];
        }

        // Open file
        FILE *fp = fopen(path.c_str(), "rb");
        if (!fp) {
            printf("Sound: Failed to open file %s, no such file exists", path.c_str());
            return 0;
        }

        //CHECK FOR VALID WAVE-FILE
        fread(m_Type, sizeof(char), 4, fp);
        if (m_Type[0] != 'R' || m_Type[1] != 'I' || m_Type[2] != 'F' || m_Type[3] != 'F') {
            printf("ERROR: No RIFF in WAVE-file");
            return 0;
        }

        fread(&m_Size, 4 * sizeof(char), 1, fp);
        fread(m_Type, sizeof(char), 4, fp);
        if (m_Type[0] != 'W' || m_Type[1] != 'A' || m_Type[2] != 'V' || m_Type[3] != 'E') {
            printf("ERROR: Not WAVE-file");
            return 0;
        }

        fread(m_Type, sizeof(char), 4, fp);
        if (m_Type[0] != 'f' || m_Type[1] != 'm' || m_Type[2] != 't' || m_Type[3] != ' ') {
            printf("ERROR: No fmt in WAVE-file");
            return 0;
        }

        // READ THE DATA FROM WAVE-FILE
        fread(&m_ChunkSize, 4 * sizeof(char), 1, fp);
        fread(&m_FormatType, 2 * sizeof(char), 1, fp);
        fread(&m_Channels, 2 * sizeof(char), 1, fp);
        fread(&m_SampleRate, 4 * sizeof(char), 1, fp);
        fread(&m_AvgBytesPerSec, 4 * sizeof(char), 1, fp);
        fread(&m_BytesPerSample, 2 * sizeof(char), 1, fp);
        fread(&m_BitsPerSample, 2 * sizeof(char), 1, fp);

        fread(m_Type, sizeof(char), 4, fp);
        if (m_Type[0] != 'd' || m_Type[1] != 'a' || m_Type[2] != 't' || m_Type[3] != 'a') {
            printf("ERROR: WAVE-file Missing data");
            return 0;
        }

        fread(&m_DataSize, 4 * sizeof(char), 1, fp);

        unsigned char* buf = new unsigned char[m_DataSize];
        fread(buf, sizeof(char), m_DataSize, fp);
        fclose(fp);

        // Create buffer
        ALuint format = 0;
        if (m_BitsPerSample == 8) {
            if (m_Channels == 1) {
                format = AL_FORMAT_MONO8;
            } else if (m_Channels == 2) {
                format = AL_FORMAT_STEREO8;
            }
        }
        if (m_BitsPerSample == 16) {
            if (m_Channels == 1) {
                format = AL_FORMAT_MONO16;
            } else if (m_Channels == 2) {
                format = AL_FORMAT_STEREO16;
            }
        }

        ALuint buffer;
        alGenBuffers(1, &buffer);
        alBufferData(buffer, format, buf, m_DataSize, m_SampleRate);
        delete[] buf;

        m_BufferCache[path] = buffer;
        return buffer;
    }
};

#endif
