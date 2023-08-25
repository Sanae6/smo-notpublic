#pragma once

#include <stream/seadStream.h>
#include <stream/seadStreamSrc.h>

class AmongWriteStreamFormat : public sead::StreamSrc {
    u32 write(const void* data, u32 size) override;

    u32 read(void* data, u32 size) override { return 0; }
    u32 skip(s32 offset) override { return 0; }
    void rewind() override {}
    bool isEOF() override { return 0; }
    bool flush() override { return true; }
};

class AmongWriteStream : public sead::WriteStream {
public:
    AmongWriteStream();

private:
    AmongWriteStreamFormat format;
};