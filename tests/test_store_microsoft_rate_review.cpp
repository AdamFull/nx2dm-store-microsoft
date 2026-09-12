#include "framework/nxtest.h"

#include "store_microsoft/store_microsoft_platform.h"
#include "store_microsoft/store_microsoft_rate_review.h"

// Same guard-path bar as test_store_microsoft_services.cpp: a
// never-initialized platform (ready() always false here) proves every
// operation refuses safely without ever making a real
// Windows.Services.Store call.

using namespace nxm::store_microsoft;

TEST_CASE("store_microsoft extras: RateReview refuses safely with no "
          "platform") {
  MicrosoftPlatform platform;
  MicrosoftRateReview rate_review(platform);
  CHECK_FALSE(rate_review.request());
  CHECK_FALSE(rate_review.pending());
  CHECK_FALSE(rate_review.succeeded());
  CHECK_FALSE(rate_review.canceled_by_user());
  CHECK_FALSE(rate_review.was_updated());
}
