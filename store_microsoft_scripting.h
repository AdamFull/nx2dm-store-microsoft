#pragma once

namespace nxe::script {
class Host;
}

namespace nxm::store_microsoft {

class MicrosoftRateReview;

/// The Microsoft-only `host.store_microsoft_*` surface (rate-and-review
/// prompt) - deliberately separate from store/store_scripting.cpp, which
/// stays neutral-only. Captured by direct reference rather than looked up
/// through ServiceRegistry: nothing outside store_microsoft itself will
/// ever need to find it, so there's no "which backend provides this"
/// question to resolve.
void expose_store_microsoft_extras(nxe::script::Host &host,
                                    MicrosoftRateReview &rate_review);

}
