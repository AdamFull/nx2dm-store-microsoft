#include "store_microsoft/store_microsoft_services.h"

#include "core/foundation/strings/format.h"

#include <winrt/Windows.Foundation.Collections.h>

#include <utility>

namespace nxm::store_microsoft {

// -- MicrosoftCore --------------------------------------------------------

bool MicrosoftCore::is_owned(const nx::string_view dlc_id) const {
  if (!m_platform.ready())
    return false;
  const std::lock_guard lock(m_mutex);
  if (dlc_id.empty())
    return m_base_active;
  for (const nx::string &id : m_owned_dlc_ids)
    if (id.view() == dlc_id)
      return true;
  return false;
}

nx::vector<nx::string> MicrosoftCore::owned_dlc_ids() const {
  const std::lock_guard lock(m_mutex);
  return m_owned_dlc_ids;
}

void MicrosoftCore::refresh_license() {
  if (!m_platform.ready())
    return;
  // Completed()'s handler captures `this` and fires on a WinRT thread-pool
  // thread at some later point - this assumes the module (and hence this
  // service) outlives any in-flight refresh, the same implicit assumption
  // every other backend's own async-callback pattern already makes (none
  // of them handle a callback landing mid-teardown either).
  m_platform.context().GetAppLicenseAsync().Completed(
      [this](const winrt::Windows::Foundation::IAsyncOperation<
                 winrt::Windows::Services::Store::StoreAppLicense> &op,
             const winrt::Windows::Foundation::AsyncStatus status) {
        if (status != winrt::Windows::Foundation::AsyncStatus::Completed)
          return;
        try {
          const winrt::Windows::Services::Store::StoreAppLicense license =
              op.GetResults();
          nx::vector<nx::string> owned_dlc_ids;
          for (const auto &entry : license.AddOnLicenses()) {
            if (entry.Value().IsActive())
              owned_dlc_ids.push_back(utf8_from_hstring(entry.Key()));
          }
          const std::lock_guard lock(m_mutex);
          m_base_active = license.IsActive();
          m_owned_dlc_ids = std::move(owned_dlc_ids);
        } catch (const winrt::hresult_error &) {
        }
      });
}

// -- MicrosoftIap ---------------------------------------------------------

nx::vector<store::StoreProduct> MicrosoftIap::products() const {
  const std::lock_guard lock(m_mutex);
  return m_products;
}

bool MicrosoftIap::purchase_pending() const {
  const std::lock_guard lock(m_mutex);
  return m_purchase_pending;
}

// The returned view is only valid at the moment of the call - a concurrent
// WinRT-thread-pool completion could reassign the string it points into
// right after this returns, unlike every other backend in this family
// (all single-threaded via their own pump). Read it promptly; don't hold
// onto it across another call into this class.
nx::string_view MicrosoftIap::purchase_error() const {
  const std::lock_guard lock(m_mutex);
  return m_purchase_error.view();
}

bool MicrosoftIap::purchase(const nx::string_view product_id) {
  if (!m_platform.ready())
    return false;
  const winrt::hstring store_id = hstring_from_utf8(product_id);
  {
    const std::lock_guard lock(m_mutex);
    m_purchase_pending = true;
    m_purchase_error = nx::string{};
  }
  m_platform.context().RequestPurchaseAsync(store_id).Completed(
      [this](const winrt::Windows::Foundation::IAsyncOperation<
                 winrt::Windows::Services::Store::StorePurchaseResult> &op,
             const winrt::Windows::Foundation::AsyncStatus status) {
        const std::lock_guard lock(m_mutex);
        m_purchase_pending = false;
        if (status != winrt::Windows::Foundation::AsyncStatus::Completed) {
          m_purchase_error = nx::string("purchase request did not complete");
          return;
        }
        try {
          using winrt::Windows::Services::Store::StorePurchaseStatus;
          const winrt::Windows::Services::Store::StorePurchaseResult result =
              op.GetResults();
          switch (result.Status()) {
          case StorePurchaseStatus::Succeeded:
          case StorePurchaseStatus::AlreadyPurchased:
            break;
          default:
            m_purchase_error =
                nx::format("Microsoft Store error {}",
                           static_cast<i32>(result.Status()));
          }
        } catch (const winrt::hresult_error &) {
          m_purchase_error = nx::string("purchase request threw");
        }
      });
  return true;
}

void MicrosoftIap::refresh_products() {
  if (!m_platform.ready())
    return;
  const auto product_kinds = winrt::single_threaded_vector<winrt::hstring>(
      {L"Durable", L"Consumable", L"UnmanagedConsumable"});
  m_platform.context()
      .GetAssociatedStoreProductsAsync(product_kinds)
      .Completed([this](const winrt::Windows::Foundation::IAsyncOperation<
                            winrt::Windows::Services::Store::
                                StoreProductQueryResult> &op,
                        const winrt::Windows::Foundation::AsyncStatus status) {
        if (status != winrt::Windows::Foundation::AsyncStatus::Completed)
          return;
        try {
          const winrt::Windows::Services::Store::StoreProductQueryResult
              result = op.GetResults();
          nx::vector<store::StoreProduct> products;
          for (const auto &entry : result.Products()) {
            const winrt::Windows::Services::Store::StoreProduct &item =
                entry.Value();
            store::StoreProduct product;
            product.id = utf8_from_hstring(item.StoreId());
            product.title = utf8_from_hstring(item.Title());
            product.price_display =
                utf8_from_hstring(item.Price().FormattedPrice());
            products.push_back(std::move(product));
          }
          const std::lock_guard lock(m_mutex);
          m_products = std::move(products);
        } catch (const winrt::hresult_error &) {
        }
      });
}

}
