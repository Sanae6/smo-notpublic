#include <al/Library/Memory/HeapUtil.h>
#include <fs.h>
#include <init.h>
#include <lib.hpp>
#include <logger/Logger.hpp>
#include <utils/AudioWrap.hpp>
#include <utils/Helpers.h>

namespace au {
    const int audioAlignment = 64;
    char waveBufferMemory[14 * 1024 * 1024] __attribute__((aligned(audioAlignment)));
    nn::mem::StandardAllocator waveAllocator(waveBufferMemory, sizeof(waveBufferMemory));
    AudioWrap::AudioWrap(const char* waveLocation) {
        Logger::log("Got hardware manager shit\n");

        nn::fs::FileHandle handle{};
        EXL_ASSERT(R_SUCCEEDED(nn::fs::OpenFile(&handle, waveLocation, nn::fs::OpenMode_Read)));
        Logger::log("Opened file %s\n", waveLocation);
        long fileSize = 0;
        EXL_ASSERT(R_SUCCEEDED(nn::fs::GetFileSize(&fileSize, handle)));
        Logger::log("File size: %llu\n", fileSize);
        WaveHeader waveHeader = {};
        EXL_ASSERT(R_SUCCEEDED(nn::fs::ReadFile(handle, 0, &waveHeader, sizeof(WaveHeader))));
        Logger::log("File header: %llu\n", waveHeader.bitsPerSample);

        waveData = (u8*)waveAllocator.Allocate(waveHeader.data.size, audioAlignment);
        EXL_ASSERT(R_SUCCEEDED(nn::fs::ReadFile(handle, sizeof(WaveHeader), waveData, waveHeader.data.size)));
        nn::fs::CloseFile(handle);
        Logger::log("File data: %p %d\n", waveData, waveHeader.data.size);

        Logger::log("Loaded wave file, %d, %d\n", waveHeader.sampleRate, waveHeader.channelCount);

        EXL_ASSERT(waveHeader.bitsPerSample == 16, "Bps is not 16");
        //        EXL_ASSERT(nn::audio::AcquireVoiceSlot(config, &voice, waveHeader.sampleRate, waveHeader.channelCount,
        //                                               nn::audio::SampleFormat_PcmInt16,
        //                                               nn::audio::VoiceType::PriorityHighest, nullptr, 0),
        //                   "Failed to acquire voice slot");

        voice = nn::atk::detail::driver::MultiVoiceManager::GetInstance().AllocVoice(1, 255, nullptr, nullptr);
        EXL_ASSERT(voice != nullptr, "Failed to allocate voice");

        waveBuffer.buffer = waveData;
        waveBuffer.bufferSize = waveHeader.data.size;
        waveBuffer.sampleOffset = 0;
        waveBuffer.sampleCount = (s32)(waveHeader.data.size / sizeof(s16)) / waveHeader.channelCount;
        waveBuffer.shouldLoop = false;

        voice->SetSampleFormat(nn::atk::SampleFormat_PcmS16);
        voice->SetSampleRate((s32)waveHeader.sampleRate);
        voice->SetSubMixIndex(0);
        voice->AppendWaveBuffer(0, &waveBuffer, true);
        voice->Start();

        Logger::log("Created wave buffer %d\n", waveBuffer.sampleCount);

        //        nn::audio::SetVoicePlayState(&voice, nn::audio::VoiceType::PlayState_Start);
        //        EXL_ASSERT(nn::audio::AppendWaveBuffer(&voice, &waveBuffer), "Failed to append to wave buffer");
        //        nn::audio::SetVoiceDestination(config, &voice, subMix);
        //        nn::audio::SetVoiceMixVolume(&voice, subMix, 1.0f, 0, 0);
        //        nn::audio::SetVoiceMixVolume(&voice, subMix, 1.0f, 1, 1);

        Logger::log("Setup voice state\n");



        //        auto requestUpdate =
        //            getFunc<nn::Result,
        //            uintptr_t>("_ZN2nn3atk6detail6driver15HardwareManager26RequestUpdateAudioRendererEv");
        //        EXL_ASSERT(requestUpdate(hwMan).isSuccess());

        Logger::log("we ballin\n");
        Logger::log("we ballin\n");
        Logger::log("we ballin\n");
        Logger::log("we ballin\n");
        Logger::log("we ballin\n");
        Logger::log("we ballin\n");
    }
    void AudioWrap::update() {
//        Logger::log("Ass %lld %d\n", voice->GetCurrentPlayingSample(), voice->IsRun());
//        Logger::log("Saa %s\n", BTOC(unsafeRef<bool>(&nn::atk::detail::driver::SoundThread::GetInstance(), 0x3dd)));
        voice->Update();
    }
} // namespace au
