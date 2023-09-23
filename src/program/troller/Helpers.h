#pragma once

#include <al/Library/Base/String.h>
#include <al/Library/Nerve/IUseNerve.h>
#include <al/Library/Nerve/Nerve.h>
#include <al/Library/Nerve/NerveUtil.h>
#include <lib.hpp>
#include <typeinfo>

template <typename Right, typename Left>
bool isSameType([[maybe_unused]] const Left* leftVal) {
    return al::isEqualString(typeid(*leftVal).name(), typeid(Right).name());
}
template <typename Right>
bool isNerve(const al::IUseNerve* user) {
    return isSameType<Right, al::Nerve>(user->getNerveKeeper()->getCurrentNerve());
}
template <typename Cast, typename Host>
Cast& unsafeRef(Host* host, ptrdiff_t offset) {
    return *(Cast*)(((uintptr_t)host) + offset);
}
static inline void setNerveOffset(al::IUseNerve* user, ptrdiff_t offsetMain) {
    al::setNerve(user, reinterpret_cast<const al::Nerve*>(exl::util::GetMainModuleInfo().m_Text.m_Start + offsetMain));
}
