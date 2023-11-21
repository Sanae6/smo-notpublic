#pragma once

#include <types.h>

namespace nn::atk {
    enum SampleFormat { SampleFormat_PcmS8, SampleFormat_PcmS16, SampleFormat_DspAdpcm, SampleFormat_PcmS32 };
    struct WaveBuffer {
        enum Status {
            Status_Free,
            Status_Wait,
            Status_Play,
            Status_Done,
        };
        void* buffer;
        u64 bufferSize;
        u64 sampleCount;
        u64 sampleOffset;
        void* adpcmContext;
        void* userData;
        bool shouldLoop;
        Status status;
        WaveBuffer* next;
    };
    namespace detail {
        enum VoiceState { VoiceState_Play, VoiceState_Stop, VoiceState_Pause };

        class LowLevelVoice;
        class Voice {
        private:
            u32 priority;
            VoiceState voiceState;
            char voiceParam[0x78];
            SampleFormat format;
            u32 sampleRate;
            s16 adpcmParam[16];
            s32 subMixIndex;
            u64 playPosition;
            u32 voiceEnableFlag;
            u32 commandTag;
            WaveBuffer* begin;
            WaveBuffer* end;
            LowLevelVoice* lowLevelVoice;

        public:
            Voice();
            ~Voice();
            bool AllocVoice(u32);
            void AppendWaveBuffer(WaveBuffer* buffer);
            void SetState(VoiceState state);
            void Free();
        };

        namespace driver {
            class MultiVoice {
            public:
                enum VoiceCallbackStatus {
                    VoiceCallbackStatus_FinishWave,
                    VoiceCallbackStatus_Cancel,
                    VoiceCallbackStatus_DropVoice,
                    VoiceCallbackStatus_DropDsp
                };
                using VoiceCallback = void (*)(MultiVoice*, VoiceCallbackStatus status, void*);

                void SetSampleFormat(SampleFormat format);
                void SetSampleRate(s32 sampleRate);
                void SetSubMixIndex(s32 index);

                void Start();
                void Update();

                bool IsRun() const;

                void AppendWaveBuffer(s32 channel, WaveBuffer* buffer, bool last);

                ptrdiff_t GetCurrentPlayingSample() const;
            };
            class MultiVoiceManager {
            private:
            public:
                static MultiVoiceManager& GetInstance();

                MultiVoice* AllocVoice(int channelCount, int priority, MultiVoice::VoiceCallback callback, void* userData);
            };

            class SoundThread {
            public:
                static SoundThread& GetInstance();
            };
        } // namespace driver
    } // namespace detail
} // namespace nn::atk