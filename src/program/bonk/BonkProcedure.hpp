#pragma once
#include <bonk/Mod.hpp>
#include <bonk/StageState.hpp>
#include <typeinfo>

namespace bm {
    struct Step {
        enum class Type { None, Enable, Modify };
        Step() {}
        const char* text;
        Type type = Type::None;
        const std::type_info* typeInfo;
        std::function<void(Mod*)>* modifyFunction;
        const std::type_info& getTypeInfo() const { return *typeInfo; }
    };

    extern s32 bonkStepCount;
    extern Step bonkSteps[30];
    Step& getNextStep();

    template <typename T>
    void addModStep(const char* text) {
        auto& procedure = getNextStep();
        procedure.text = text;
        procedure.type = Step::Type::Enable;
        procedure.typeInfo = &typeid(T);
        addMod<T>();
    }
    template <typename T>
    void modifyStep(const char* text, std::function<void(T&)>&& func) {
        auto& procedure = getNextStep();
        procedure.text = text;
        procedure.type = Step::Type::Modify;
        procedure.typeInfo = &typeid(T);
        procedure.modifyFunction =
            alloc<std::function<void(Mod*)>>([func](void* data) { func(*reinterpret_cast<T*>(data)); });
    }
    bool bonked();
    void procedureStartup();
} // namespace bm
