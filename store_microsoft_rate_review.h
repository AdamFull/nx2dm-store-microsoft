#pragma once

#include "store_microsoft/store_microsoft_platform.h"

#include <mutex>

namespace nxm::store_microsoft {

/// Windows.Services.Store's rate-and-review prompt
/// (`StoreContext::RequestRateAndReviewAppAsync`) - a Microsoft-only extra,
/// verified against the real Windows SDK's C++/WinRT projection headers
/// (declared on `IStoreContext4`, one of the interfaces `StoreContext`
/// already aggregates). Shows the native Store rating dialog and reports
/// back whether the user actually changed their rating.
///
/// Same mutex-guarded-cache shape `MicrosoftCore`/`MicrosoftIap` already
/// use (store_microsoft_services.h) - WinRT's `Completed()` callback fires
/// on its own thread-pool thread, not from a per-frame pump this backend
/// owns. `succeeded()`/`canceled_by_user()` translate the SDK's four-way
/// `StoreRateAndReviewStatus` into the two booleans a game actually needs,
/// the same "don't leak the raw enum to Luau" choice `EgsMods`/
/// `StovePCBang` already made.
class MicrosoftRateReview {
public:
  explicit MicrosoftRateReview(MicrosoftPlatform &platform) noexcept
      : m_platform(platform) {}

  /// Fires StoreContext::RequestRateAndReviewAppAsync().
  bool request();
  [[nodiscard]] bool pending() const noexcept;
  [[nodiscard]] bool succeeded() const noexcept;
  [[nodiscard]] bool canceled_by_user() const noexcept;
  [[nodiscard]] bool was_updated() const noexcept;

private:
  MicrosoftPlatform &m_platform;
  mutable std::mutex m_mutex;
  bool m_pending = false;
  winrt::Windows::Services::Store::StoreRateAndReviewStatus m_status =
      winrt::Windows::Services::Store::StoreRateAndReviewStatus::Error;
  bool m_was_updated = false;
};

}
