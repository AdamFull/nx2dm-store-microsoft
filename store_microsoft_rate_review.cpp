#include "store_microsoft/store_microsoft_rate_review.h"

namespace nxm::store_microsoft {

bool MicrosoftRateReview::pending() const noexcept {
  const std::lock_guard lock(m_mutex);
  return m_pending;
}

bool MicrosoftRateReview::succeeded() const noexcept {
  const std::lock_guard lock(m_mutex);
  return m_status ==
         winrt::Windows::Services::Store::StoreRateAndReviewStatus::Succeeded;
}

bool MicrosoftRateReview::canceled_by_user() const noexcept {
  const std::lock_guard lock(m_mutex);
  return m_status == winrt::Windows::Services::Store::StoreRateAndReviewStatus::
                          CanceledByUser;
}

bool MicrosoftRateReview::was_updated() const noexcept {
  const std::lock_guard lock(m_mutex);
  return m_was_updated;
}

bool MicrosoftRateReview::request() {
  if (!m_platform.ready())
    return false;
  {
    const std::lock_guard lock(m_mutex);
    m_pending = true;
  }
  m_platform.context().RequestRateAndReviewAppAsync().Completed(
      [this](const winrt::Windows::Foundation::IAsyncOperation<
                 winrt::Windows::Services::Store::StoreRateAndReviewResult> &op,
             const winrt::Windows::Foundation::AsyncStatus status) {
        const std::lock_guard lock(m_mutex);
        m_pending = false;
        if (status != winrt::Windows::Foundation::AsyncStatus::Completed) {
          m_status =
              winrt::Windows::Services::Store::StoreRateAndReviewStatus::Error;
          return;
        }
        try {
          const winrt::Windows::Services::Store::StoreRateAndReviewResult
              result = op.GetResults();
          m_status = result.Status();
          m_was_updated = result.WasUpdated();
        } catch (const winrt::hresult_error &) {
          m_status =
              winrt::Windows::Services::Store::StoreRateAndReviewStatus::Error;
        }
      });
  return true;
}

}
