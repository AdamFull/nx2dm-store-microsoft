#pragma once

#include "store_microsoft/store_microsoft_platform.h"

#include "store/store_service.h"

#include <mutex>

namespace nxm::store_microsoft {

/// Two of the five neutral services (store_service.h), backed by
/// Windows.Services.Store - store.achievements/store.cloud_saves/
/// store.presence are never registered: that namespace genuinely has none
/// of those subsystems (Xbox achievements/stats/social live in the much
/// larger, separately-registered Xbox Live/GDK SDK, out of scope here), the
/// same "simply doesn't provide it" shape used elsewhere in this module
/// family for a backend whose SDK lacks a subsystem.
///
/// Both classes below guard their cached state with a mutex, not just a
/// platform-ready check - see store_microsoft_platform.h's class comment
/// for why (WinRT's Completed() callback fires on its own thread-pool
/// thread, not from a per-frame pump this module owns).

class MicrosoftCore final : public store::StoreCore {
public:
  explicit MicrosoftCore(MicrosoftPlatform &platform) noexcept
      : m_platform(platform) {}

  [[nodiscard]] bool is_owned(nx::string_view dlc_id = {}) const override;
  [[nodiscard]] nx::vector<nx::string> owned_dlc_ids() const override;
  [[nodiscard]] nx::string_view store_name() const noexcept override {
    return "microsoft";
  }

  /// Fires StoreContext::GetAppLicenseAsync(), refreshing the cached
  /// license is_owned()/owned_dlc_ids() read from - StoreAppLicense's own
  /// IsActive() (base game) and AddOnLicenses() (a map of DLC Store ID ->
  /// StoreLicense, each with its own IsActive()) map directly onto this
  /// interface, unlike GOG's own is_owned() which has no equivalent
  /// full-game license flag to read and has to proxy through sign-in
  /// success instead.
  void refresh_license();

  /// Windows.Services.Store always queries the full license (base game +
  /// every add-on) in one call - @p dlc_id is ignored, same as
  /// store::StoreCore::refresh_ownership() documents for any bulk-capable
  /// backend.
  void refresh_ownership(nx::string_view = {}) override { refresh_license(); }

private:
  MicrosoftPlatform &m_platform;
  mutable std::mutex m_mutex;
  bool m_base_active = false;
  nx::vector<nx::string> m_owned_dlc_ids;
};

class MicrosoftIap final : public store::StoreIap {
public:
  explicit MicrosoftIap(MicrosoftPlatform &platform) noexcept
      : m_platform(platform) {}

  [[nodiscard]] nx::vector<store::StoreProduct> products() const override;
  bool purchase(nx::string_view product_id) override;
  [[nodiscard]] bool purchase_pending() const override;
  [[nodiscard]] nx::string_view purchase_error() const override;

  /// Fires StoreContext::GetAssociatedStoreProductsAsync() for every
  /// purchasable add-on kind ("Durable", "Consumable",
  /// "UnmanagedConsumable"), refreshing products(). Unlike Stove,
  /// purchase() doesn't need this cache's data to complete a purchase (the
  /// Store ID alone is enough for RequestPurchaseAsync()) - it only backs
  /// the neutral listing. @p product_ids is ignored, same as
  /// store::StoreIap::refresh_products() documents for any bulk-capable
  /// backend.
  void refresh_products(const nx::vector<nx::string> &product_ids = {}) override;

private:
  MicrosoftPlatform &m_platform;
  mutable std::mutex m_mutex;
  nx::vector<store::StoreProduct> m_products;
  bool m_purchase_pending = false;
  nx::string m_purchase_error;
};

}
