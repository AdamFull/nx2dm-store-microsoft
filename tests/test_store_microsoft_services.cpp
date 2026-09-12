#include "framework/nxtest.h"

#include "store_microsoft/store_microsoft_platform.h"
#include "store_microsoft/store_microsoft_services.h"

// Every service method here checks MicrosoftPlatform::ready() before
// touching StoreContext at all, so a never-initialized platform (always
// false - see test_store_microsoft_platform.cpp for why that's also true
// for real in this environment) proves the same guard-path bar every other
// backend's own tests already established, without ever making a real
// Windows.Services.Store call.

using namespace nxm::store_microsoft;

TEST_CASE("store_microsoft services: MicrosoftCore refuses safely with no "
          "platform") {
  MicrosoftPlatform platform;
  MicrosoftCore core(platform);
  CHECK_FALSE(core.is_owned());
  CHECK_FALSE(core.is_owned("some_dlc"));
  CHECK(core.owned_dlc_ids().empty());
  CHECK(core.store_name() == "microsoft");
  core.refresh_license();
  CHECK(core.owned_dlc_ids().empty());
}

TEST_CASE("store_microsoft services: MicrosoftIap refuses safely with no "
          "platform") {
  MicrosoftPlatform platform;
  MicrosoftIap iap(platform);
  CHECK(iap.products().empty());
  CHECK_FALSE(iap.purchase("some_store_id"));
  CHECK_FALSE(iap.purchase_pending());
  CHECK(iap.purchase_error().empty());
  iap.refresh_products();
  CHECK(iap.products().empty());
}
