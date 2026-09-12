#include "store_microsoft/store_microsoft_platform.h"
#include "store_microsoft/store_microsoft_services.h"

#include "store/store_service.h"

#include "core/app/engine.h"
#include "core/app/module.h"
#include "core/app/module_context.h"

#include "core/foundation/diagnostics/log.h"

namespace nxm::store_microsoft {
namespace {

const nx::log::Category log_store_microsoft = nx::log::category("store_microsoft");

// store.achievements/store.cloud_saves/store.presence are deliberately
// absent - Windows.Services.Store has none of those subsystems (Xbox
// achievements/stats/social live in the separate, much larger Xbox
// Live/GDK SDK, out of scope here), the same "simply doesn't provide it"
// shape used elsewhere in this module family - see
// store_microsoft_services.h.
constexpr nxe::ModuleService PROVIDED_SERVICES[] = {
    {.id = store::kCoreService, .version = {1, 0, 0}},
    {.id = store::kIapService, .version = {1, 0, 0}},
};

class StoreMicrosoftModule final : public nxe::Module {
public:
  StoreMicrosoftModule() : m_core(m_platform), m_iap(m_platform) {}

  [[nodiscard]] nxe::ModuleDescriptor descriptor() const noexcept override {
    nxe::ModuleDescriptor out{};
    out.id = "store_microsoft";
    out.version = {1, 0, 0};
    out.provided_services = PROVIDED_SERVICES;
    out.platforms = nxe::ModulePlatform::Windows;
    return out;
  }

  bool on_register(nxe::ModuleContext &ctx) override {
    nxe::ServiceRegistrar registrar = ctx.service_registrar();
    store::StoreCore &core = m_core;
    store::StoreIap &iap = m_iap;
    return registrar.provide(store::kCoreService, PROVIDED_SERVICES[0].version, core) &&
           registrar.provide(store::kIapService, PROVIDED_SERVICES[1].version, iap);
  }

  bool on_attach(nxe::ModuleContext &) override {
    // No pump system registered here, unlike every other store backend -
    // Windows.Services.Store has no per-frame processing step at all; every
    // async call resolves on its own WinRT thread-pool thread (see
    // store_microsoft_platform.h). There's also no per-project config file:
    // unlike Steam/EOS/GOG/Stove, this backend takes no developer
    // credentials at runtime at all - StoreContext::GetDefault() resolves
    // everything from the process's own package identity, so there's
    // nothing here for a store_microsoft.ini to hold.
    if (m_platform.initialize())
      nx::logi(log_store_microsoft, "attached");
    else
      nx::logi(log_store_microsoft, "no package identity; staying idle");
    return true;
  }

  void on_detach(nxe::ModuleContext &) override { m_platform.shutdown(); }

private:
  MicrosoftPlatform m_platform;
  MicrosoftCore m_core;
  MicrosoftIap m_iap;
};

} // namespace
} // namespace nxm::store_microsoft

NX_DECLARE_MODULE(store_microsoft, nxm::store_microsoft::StoreMicrosoftModule)
