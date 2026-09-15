#include "framework/nxtest.h"

#include "app/engine.h"
#include "script/script_host.h"
#include "store_microsoft/store_microsoft_platform.h"
#include "store_microsoft/store_microsoft_rate_review.h"
#include "store_microsoft/store_microsoft_scripting.h"

namespace {

using namespace nxm::store_microsoft;
namespace script = nxe::script;

struct Exposed {
  MicrosoftPlatform platform;
  MicrosoftRateReview rate_review{platform};
  script::Host host;
  nx::vector<script::Host::ServiceInfo> services;

  Exposed() {
    expose_store_microsoft_extras(host, rate_review);
    services = host.services();
  }

  [[nodiscard]] const script::Host::ServiceInfo *
  find(const nx::string_view name) const {
    for (const script::Host::ServiceInfo &one : services)
      if (one.name == name)
        return &one;
    return nullptr;
  }
};

} // namespace

// This is the one place a mismatch between what store_microsoft_scripting.cpp
// actually registers and what modules/store_microsoft/script-services.json
// declares to Luau would show up - see test_modio_scripting.cpp's identical
// role for modio. No backend is registered in this harness (no real
// Windows.Services.Store call runs), so every callable here just exercises
// its own "no platform" refusal path.
TEST_CASE("store_microsoft scripting: every service is exposed with the "
          "shape a script is told about") {
  const Exposed exposed;

  static constexpr struct {
    nx::string_view name;
    nx::string_view signature;
  } WANT[] = {
      {"store_microsoft_rate_review_request", "()->(boolean)"},
      {"store_microsoft_rate_review_pending", "()->(boolean)"},
      {"store_microsoft_rate_review_succeeded", "()->(boolean)"},
      {"store_microsoft_rate_review_canceled_by_user", "()->(boolean)"},
      {"store_microsoft_rate_review_was_updated", "()->(boolean)"},
  };

  CHECK(exposed.services.size() == nx::array_size(WANT));
  for (const auto &want : WANT) {
    const script::Host::ServiceInfo *const found = exposed.find(want.name);
    REQUIRE(found != nullptr);
    CHECK(found->signature == want.signature);
  }
}

TEST_CASE("store_microsoft scripting: the module hands them over on its "
          "own") {
  std::unique_ptr<nxe::Module> found;
  for (const nxe::ModuleFactory factory : nxe::enabled_module_factories()) {
    std::unique_ptr<nxe::Module> module = factory();
    if (module != nullptr && module->name() == "store_microsoft")
      found = std::move(module);
  }
  REQUIRE(found != nullptr);

  nxe::Engine engine{nxe::Game{}};
  nxe::ModuleContext ctx{engine};
  script::Host host;
  found->on_expose_scripts(host, ctx);

  script::Host direct;
  MicrosoftPlatform platform;
  MicrosoftRateReview rate_review{platform};
  expose_store_microsoft_extras(direct, rate_review);
  CHECK(host.exposed_count() == direct.exposed_count());
  CHECK(host.exposed_count() > 0u);
}
