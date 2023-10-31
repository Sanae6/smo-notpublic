#pragma once
#include <audio.hpp>
#include <heap/seadHeap.h>

namespace au {
    struct IffSignature {
        const char name[4];
        u32 size;
    };
    struct WaveHeader {
        IffSignature base;
        u32 format;
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

    struct AudioWrap {
        sead::Heap* waveAllocHeap;
        nn::audio::FinalMixType* finalMix;
        nn::audio::VoiceType voice{};
        nn::audio::WaveBuffer waveBuffer{};
        WaveHeader* waveData;

        explicit AudioWrap(const char* waveLocation);
        ~AudioWrap() {
            waveAllocHeap->free(waveData);
        }
    };
} // namespace bm
