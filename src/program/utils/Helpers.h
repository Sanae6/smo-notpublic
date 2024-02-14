#pragma once

#include <al/Library/Base/String.h>
#include <al/Library/Nerve/IUseNerve.h>
#include <al/Library/Nerve/Nerve.h>
#include <al/Library/Nerve/NerveKeeper.h>
#include <al/Library/Nerve/NerveUtil.h>
#include <al/Library/Memory/HeapUtil.h>
#include <lib.hpp>
#include <math/seadVector.h>
#include <basis/seadNew.h>
#include <typeinfo>

namespace patch = exl::patch;
namespace inst = exl::armv8::inst;
namespace reg = exl::armv8::reg;
using namespace reg;

template <typename T>
using Trampoline = exl::hook::impl::TrampolineHook<T>;
template <typename T>
using Replace = exl::hook::impl::ReplaceHook<T>;

template <typename Right, typename Left>
bool isSameType([[maybe_unused]] const Left* leftVal) {
    return leftVal != nullptr && al::isEqualString(typeid(*leftVal).name(), typeid(Right).name());
}
template <typename Left>
bool isSameType([[maybe_unused]] const Left* leftVal, const std::type_info& rightValue) {
    return leftVal != nullptr && al::isEqualString(typeid(*leftVal).name(), rightValue.name());
}
template <typename Right>
bool isNerve(const al::IUseNerve* user) {
    return isSameType<Right>(user->getNerveKeeper()->getCurrentNerve());
}

template <typename Cast, typename Host>
Cast unsafeOffset(Host* host, ptrdiff_t offset) {
    return (Cast)(((uintptr_t)host) + offset);
}
template <typename Cast, typename Host>
Cast unsafeOffset(Host host, ptrdiff_t offset) {
    return (Cast)(((uintptr_t)host) + offset);
}
template <typename Cast, typename Host>
Cast& unsafeRef(Host* host, ptrdiff_t offset) {
    return *unsafeOffset<Cast*>(host, offset);
}
template <typename Cast, typename Host>
Cast& unsafeRef(Host host, ptrdiff_t offset) {
    return *unsafeOffset<Cast*>(host, offset);
}
template <typename Return, typename ...Args, typename Func = std::add_pointer<Return(Args...)>::type>
Func getFunc(const char* name) {
    uintptr_t ptr;
    EXL_ASSERT(R_SUCCEEDED(nn::ro::LookupSymbol(&ptr, name)));
    return (Func)ptr;
}

static inline void setNerveOffset(al::IUseNerve* user, ptrdiff_t offsetMain) {
    al::setNerve(user, reinterpret_cast<const al::Nerve*>(exl::util::GetMainModuleInfo().m_Text.m_Start + offsetMain));
}

static inline f32 abs(f32 value) {
    return value < 0 ? -value : value;
}

static inline sead::Vector3f abs(const sead::Vector3f& value) {
    return sead::Vector3f(abs(value.x), abs(value.y), abs(value.z));
}

template <typename T, typename... Args>
static inline T* alloc(Args... args) {
    return new (al::getCurrentHeap(), 8) T(args...);
}

template <typename T, typename... Args>
static inline T* allocWithHeap(sead::Heap* heap, s32 alignment, Args... args) {
    return new (heap ? heap : al::getCurrentHeap(), alignment) T(args...);
}

template <typename T>
static inline T* allocArray(s32 size) {
    return new (al::getCurrentHeap(), 8) T[size];
}

template <typename T>
static inline void free(T* ptr, sead::Heap* heap = nullptr) {
    if (!heap) heap = al::getCurrentHeap();
    EXL_ASSERT(heap != nullptr, "Invalid heap for free!");
    heap->free(ptr);
}
