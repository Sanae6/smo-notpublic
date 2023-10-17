#pragma once

#include <al/Library/Base/String.h>
#include <al/Library/Nerve/IUseNerve.h>
#include <al/Library/Nerve/Nerve.h>
#include <al/Library/Nerve/Nerve.h>
#include <al/Library/Nerve/NerveUtil.h>
#include <lib.hpp>
#include <typeinfo>

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
Cast& unsafeRef(Host* host, ptrdiff_t offset) {
    return *(Cast*)(((uintptr_t)host) + offset);
}
static inline void setNerveOffset(al::IUseNerve* user, ptrdiff_t offsetMain) {
    al::setNerve(user, reinterpret_cast<const al::Nerve*>(exl::util::GetMainModuleInfo().m_Text.m_Start + offsetMain));
}
