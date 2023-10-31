#pragma once
#include <audio.hpp>

namespace bm {
    struct IffSignature {
        const char name[4];
        u32 size;
    };
    struct WaveHeader {
        IffSignature base;
        IffSignature fmt;
        u16 audioFormat;
        u16 channelCount;
        u32 sampleRate;
        u32 byteRate;
        u16 blockAlign;
        u16 bitsPerSample; // 16
        IffSignature data;
        u8 restOfData[];
    };

    class AudioWrap {
        nn::audio::FinalMixType* finalMix;
        nn::audio::VoiceType voice;
        nn::audio::WaveBuffer waveBuffer = {};
        WaveHeader* waveData;

        AudioWrap(const char* waveLocation);
    };
} // namespace bm
