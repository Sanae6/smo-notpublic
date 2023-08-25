#include <program/koopa/AmongWriteStream.h>
#include <logger/Logger.hpp>

u32 AmongWriteStreamFormat::write(const void* data, u32 size) {
    Logger::logBuffer(data, size);
    return size;
}

AmongWriteStream::AmongWriteStream() : sead::WriteStream() {
    mSrc = &format;
    setMode(Modes::Text);
}