#pragma once

#include <al/Library/Nerve/Nerve.h>
#include <al/Library/Nerve/NerveKeeper.h>

#define MAKE_NERVE_FUNC(Class, Action, Func, Namespace)                                                                \
    class Class##Nrv##Action : public al::Nerve {                                                                      \
        void execute(al::NerveKeeper* keeper) const override { keeper->getParent<Namespace Class>()->exe##Func(); }    \
                                                                                                                       \
    public:                                                                                                            \
        static Class##Nrv##Action sInstance;                                                                           \
    };                                                                                                                 \
    Class##Nrv##Action Class##Nrv##Action::sInstance
#define MAKE_NERVE_BASE_FUNC(Class, Action, Func, Namespace)                                                           \
    class Class##Nrv##Action : public al::Nerve {                                                                      \
        void execute(al::NerveKeeper* keeper) const override;                                                          \
                                                                                                                       \
    public:                                                                                                            \
        static Class##Nrv##Action sInstance;                                                                           \
    }
#define MAKE_NERVE_IMPL_FUNC(Class, Action, Func, Namespace)                                                           \
    void Class##Nrv##Action::execute(al::NerveKeeper* keeper) const {                                                  \
        keeper->getParent<Namespace Class>()->exe##Func();                                                             \
    }                                                                                                                  \
    Class##Nrv##Action Class##Nrv##Action::sInstance;
#define MAKE_NERVE_FAKE(Class, Action)                                                                                 \
    class Class##Nrv##Action : public al::Nerve {};

#define MAKE_NERVE(Class, Action) MAKE_NERVE_FUNC(Class, Action, Action, )
#define MAKE_NERVE_BASE(Class, Action) MAKE_NERVE_BASE_FUNC(Class, Action, Action, )
#define MAKE_NERVE_IMPL(Class, Action) MAKE_NERVE_IMPL_FUNC(Class, Action, Action, )
#define MAKE_NERVE_NAMESPACE(Class, Action, Namespace) MAKE_NERVE_FUNC(Class, Action, Action, Namespace::)
