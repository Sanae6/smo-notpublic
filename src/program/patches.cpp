#include "patches.hpp"

namespace patch = exl::patch;
namespace inst = exl::armv8::inst;
namespace reg = exl::armv8::reg;


namespace nn {

    namespace gfx {
        struct DescriptorSlot;
        struct ResTextureFile;

        template <class T>
        struct TTextureView {
        };

        template <class T, class F>
        struct ApiVariation {
        };

        template <int Ver>
        struct ApiType {
        };

    } // namespace gfx

    namespace vfx {

        struct Heap;
        struct EmitterResource;
        struct EmitterSetResource;

        namespace detail {
            struct BinaryData;
            struct BufferSizeCalculator;
            struct ResNameTableRecord;
        } // namespace detail

        class Resource {
        public:
            // Resource(nn::gfx::TDevice<nn::gfx::ApiVariation<nn::gfx::ApiType<4>,nn::gfx::ApiVersion<8>>>
            // *,nn::vfx::Heap *,void
            // *,nn::gfx::TMemoryPool<nn::gfx::ApiVariation<nn::gfx::ApiType<4>,nn::gfx::ApiVersion<8>>>
            // *,ulong,ulong,int,nn::vfx::System *,bool,nn::vfx::Resource*);	T
            virtual ~Resource();
            void Trace(nn::vfx::detail::BinaryData*);
            void Finalize(nn::vfx::Heap*);
            void TraceGfxResTextureFile(nn::vfx::detail::BinaryData*);
            void TracePrimitiveArray(nn::vfx::detail::BinaryData*);
            void TraceG3dPrimitiveArray(nn::vfx::detail::BinaryData*);
            void TraceShaderBinaryArray(nn::vfx::detail::BinaryData*);
            void TraceEmitterSetArray(nn::vfx::detail::BinaryData*, nn::vfx::detail::BufferSizeCalculator*);
            void InitializeEmitterGraphicsResource(nn::vfx::EmitterResource*);
            void FinalizeEmitterResource(nn::vfx::EmitterResource*);
            void Unrelocate(void*);
            // void RegisterTextureViewToDescriptorPool(bool (*)(nn::gfx::DescriptorSlot
            // *,nn::gfx::TTextureView<nn::gfx::ApiVariation<nn::gfx::ApiType<4>,nn::gfx::ApiVersion<8>>> const&,void
            // *),void *);
            void GetTextureDescriptorSlot(nn::gfx::DescriptorSlot*, ulong) const;
            void SearchRecordFromResNameTable(nn::vfx::detail::ResNameTableRecord*, ulong);
            void BindExternalResTextureFile(nn::gfx::ResTextureFile*);
            // void UnregisterTextureViewFromDescriptorPool(void (*)(nn::gfx::DescriptorSlot
            // *,nn::gfx::TTextureView<nn::gfx::ApiVariation<nn::gfx::ApiType<4>,nn::gfx::ApiVersion<8>>> const&,void
            // *),void *); void BindExternalG3dResFile(nn::g3d::ResFile *); void
            // BindExternalResShaderFile(nn::gfx::ResShaderFile *,nn::gfx::ResShaderFile *);
            void InitializeEmitterSetResource(nn::vfx::EmitterSetResource*, nn::vfx::detail::BufferSizeCalculator*);
            void InitializeEmitterResource(nn::vfx::EmitterResource*, nn::vfx::detail::BinaryData*, int,
                                           nn::vfx::detail::BufferSizeCalculator*);
            void GetPrimitive(ulong) const;
            void GetG3dPrimitive(ulong) const;
            void SearchEmitterSetId(char const*) const;
            void SearchEmitterSetIdWithFilePath(char const*) const;
            void BindResource(int, nn::vfx::EmitterSetResource*);
            void UnbindResource(int);
            bool IsExistChildEmitter(int) const;
            bool IsNeedFade(int) const;
            void OutputResourceInfo(void) const;
        };
    } // namespace vfx
} // namespace nn

HOOK_DEFINE_TRAMPOLINE(ResourceCtor) {
    static void Callback(nn::vfx::Resource* res, void* a, void* b, void* c, void* d, void* e, void* f, int g, void* h, bool i, void* j) {
        Orig(res, a, b, c,d,e,f,g,h,i,j);
        res->OutputResourceInfo();
    }
};

void costumeRoomPatches() {
    patch::CodePatcher p(0x262850);
    p.WriteInst(inst::Movz(reg::W0, 0));
    p.Seek(0x2609B4);
    p.WriteInst(inst::Movz(reg::W0, 0));

    p.Seek(0x25FF74);
    p.WriteInst(inst::Movz(reg::W0, 1));
    p.Seek(0x25FF74);
    p.WriteInst(inst::Movz(reg::W0, 0));
}

void stubSocketInit() {
    patch::CodePatcher p(0x95C498);
    p.WriteInst(inst::Nop());
}

void runCodePatches() {
    costumeRoomPatches();
    stubSocketInit();
//    ResourceCtor::InstallAtSymbol("_ZN2nn3vfx8ResourceC1EPNS_3gfx7TDeviceINS2_12ApiVariationINS2_7ApiTypeILi4EEENS2_10ApiVersionILi8EEEEEEEPNS0_4HeapEPvPNS2_11TMemoryPoolIS9_EEmmiPNS0_6SystemEbPS1_");
}
