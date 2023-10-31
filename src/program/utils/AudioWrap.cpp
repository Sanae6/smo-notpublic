#include <al/Library/Memory/HeapUtil.h>
#include <fs.h>
#include <lib.hpp>
#include <program/utils/AudioWrap.hpp>
#include <utils/Helpers.h>

namespace bm {
    AudioWrap::AudioWrap(const char* waveLocation) {
        uintptr_t* hwMan = *(uintptr_t**)exl::util::modules::GetTargetOffset(0x2474ed0);
        finalMix = unsafeRef<nn::audio::FinalMixType*>(hwMan, 0x630);
        auto config = unsafeRef<nn::audio::AudioRendererConfig*>(hwMan, 0x18);

        nn::fs::FileHandle handle{};
        EXL_ASSERT(R_SUCCEEDED(nn::fs::OpenFile(&handle, waveLocation, nn::fs::OpenMode_Read)));
        long fileSize = 0;
        EXL_ASSERT(R_SUCCEEDED(nn::fs::GetFileSize(&fileSize, handle)));
        waveData = (WaveHeader*)al::getCurrentHeap()->alloc(fileSize, 64);
        EXL_ASSERT(R_SUCCEEDED(nn::fs::ReadFile(handle, 0, waveData, fileSize)));
        nn::fs::CloseFile(handle);

        EXL_ASSERT(waveData->bitsPerSample == 16, "Bps is not 16");
        EXL_ASSERT(R_SUCCEEDED(nn::audio::AcquireVoiceSlot(config, &voice, waveData->sampleRate, waveData->channelCount, nn::audio::SampleFormat_PcmInt16,
                                                           nn::audio::VoiceType::PriorityHighest, nullptr, 0)),
                   "Failed to acquire voice slot");
        waveBuffer.bufferSize = waveData->data.size;
        waveBuffer.buffer = waveData->restOfData;
        waveBuffer.startSampleOffset = 0;
        waveBuffer.endSampleOffset = (s32)(waveData->data.size / sizeof(s16)) / waveData->channelCount;
        waveBuffer.shouldLoop = true;
        waveBuffer.isEndOfStream = false;
        nn::audio::AppendWaveBuffer(&voice, &waveBuffer);
        nn::audio::SetVoiceDestination(config, &voice, finalMix);
        nn::audio::SetVoiceMixVolume(&voice, finalMix, 0.5f, 0, 0);
        nn::audio::SetVoiceMixVolume(&voice, finalMix, 0.5f, 1, 1);
        nn::audio::SetVoicePlayState(&voice, nn::audio::VoiceType::PlayState_Start);
    }
} // namespace bm
