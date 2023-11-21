#pragma once
#include <audio.hpp>
#include <atk.hpp>
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
    };

    struct AudioWrap {
//        nn::audio::SubMixType* subMix;
//        nn::audio::VoiceType voice{};
        nn::atk::WaveBuffer waveBuffer{};
        nn::atk::detail::driver::MultiVoice* voice;
        u8* waveData;

        explicit AudioWrap(const char* waveLocation);
        void update();
        ~AudioWrap() {
//            waveAllocHeap->free(waveData);

        }
    };
} // namespace bm
