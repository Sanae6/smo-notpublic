#include <al/Library/Memory/HeapUtil.h>
#include <fs.h>
#include <lib.hpp>
#include <logger/Logger.hpp>
#include <utils/AudioWrap.hpp>
#include <utils/Helpers.h>

namespace au {
    AudioWrap::AudioWrap(const char* waveLocation) {
        waveAllocHeap = al::getCurrentHeap();

        auto hwMan = exl::util::GetMainModuleInfo().m_Text.m_Start + 0x2474ed0;
        finalMix = unsafeOffset<nn::audio::FinalMixType*>(hwMan, 0x630);
        auto config = unsafeOffset<nn::audio::AudioRendererConfig*>(hwMan, 0x18);

        Logger::log("Got hardware manager shit\n");

        nn::fs::FileHandle handle{};
        EXL_ASSERT(R_SUCCEEDED(nn::fs::OpenFile(&handle, waveLocation, nn::fs::OpenMode_Read)));
        Logger::log("Opened file\n");
        long fileSize = 0;
        EXL_ASSERT(R_SUCCEEDED(nn::fs::GetFileSize(&fileSize, handle)));
        Logger::log("File size: %llu\n", fileSize);
        Logger::log("Heap: %llu\n", waveAllocHeap);
        Logger::log("Heap: %llu\n", waveAllocHeap->getFreeSize());
        waveData = (WaveHeader*)waveAllocHeap->tryAlloc(fileSize, 64);
        Logger::log("File data: %p\n", waveData);
        EXL_ASSERT(R_SUCCEEDED(nn::fs::ReadFile(handle, 0, waveData, fileSize)));
        nn::fs::CloseFile(handle);

        Logger::log("Loaded wave file, %d, %d\n", waveData->sampleRate, waveData->channelCount);

        EXL_ASSERT(waveData->bitsPerSample == 16, "Bps is not 16");
        EXL_ASSERT(nn::audio::AcquireVoiceSlot(config, &voice, waveData->sampleRate, waveData->channelCount,
                                                           nn::audio::SampleFormat_PcmInt16,
                                                           nn::audio::VoiceType::PriorityHighest, nullptr, 0),
                   "Failed to acquire voice slot");
        waveBuffer.buffer = waveData->restOfData;
        waveBuffer.bufferSize = waveData->data.size;
        waveBuffer.startSampleOffset = 0;
        waveBuffer.endSampleOffset = (s32)(waveData->data.size / sizeof(s16)) / waveData->channelCount;
        waveBuffer.shouldLoop = true;
        waveBuffer.isEndOfStream = true;

        Logger::log("Created wave buffer %d-%d\n", waveBuffer.startSampleOffset, waveBuffer.endSampleOffset);

        nn::audio::SetVoicePlayState(&voice, nn::audio::VoiceType::PlayState_Start);
        nn::audio::AppendWaveBuffer(&voice, &waveBuffer);
        nn::audio::SetVoiceDestination(config, &voice, finalMix);
        nn::audio::SetVoiceMixVolume(&voice, finalMix, 0.5f, 0, 0);
        nn::audio::SetVoiceMixVolume(&voice, finalMix, 0.5f, 1, 1);

        Logger::log("Setup voice state\n");

        Logger::log("we ballin\n");
        Logger::log("we ballin\n");
        Logger::log("we ballin\n");
        Logger::log("we ballin\n");
        Logger::log("we ballin\n");
        Logger::log("we ballin\n");
    }
} // namespace au
