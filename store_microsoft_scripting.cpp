#include "store_microsoft/store_microsoft_scripting.h"

#include "store_microsoft/store_microsoft_rate_review.h"

#include "core/script/script_host.h"

namespace nxm::store_microsoft {

void expose_store_microsoft_extras(nxe::script::Host &host,
                                    MicrosoftRateReview &rate_review) {
  host.expose_as("store_microsoft_rate_review_request",
                 [&rate_review]() { return rate_review.request(); });
  host.expose_as("store_microsoft_rate_review_pending",
                 [&rate_review]() { return rate_review.pending(); });
  host.expose_as("store_microsoft_rate_review_succeeded",
                 [&rate_review]() { return rate_review.succeeded(); });
  host.expose_as("store_microsoft_rate_review_canceled_by_user",
                 [&rate_review]() { return rate_review.canceled_by_user(); });
  host.expose_as("store_microsoft_rate_review_was_updated",
                 [&rate_review]() { return rate_review.was_updated(); });
}

}
