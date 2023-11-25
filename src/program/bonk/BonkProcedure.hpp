#pragma once
#include <bonk/Mod.hpp>
#include <bonk/StageState.hpp>
#include <typeinfo>

namespace bm {
    struct ModifyStep {
    };
    struct Step {
        enum class Type { None, Enable, Modify };
        Step() {}
        Type type = Type::None;
        const std::type_info* typeInfo;
        std::function<void(Mod*)>* modifyFunction;
        const std::type_info& getTypeInfo() const { return *typeInfo; }
    };

    Step& getNextStep();

    template <typename T>
    void addModStep() {
        auto& procedure = getNextStep();
        procedure.type = Step::Type::Enable;
        procedure.typeInfo = &typeid(T);
        addMod<T>();
    }
    template <typename T>
    void modifyStep(std::function<void(T&)>&& func) {
        auto& procedure = getNextStep();
        procedure.type = Step::Type::Modify;
        procedure.typeInfo = &typeid(T);
        procedure.modifyFunction =
            alloc<std::function<void(Mod*)>>([func](void* data) { func(*reinterpret_cast<T*>(data)); });
    }
    bool bonked();
    void procedureStartup();
} // namespace bm
