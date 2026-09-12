#pragma once

#include "core/foundation/core/foundation.h"
#include "core/foundation/strings/utf8_string.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Services.Store.h>

namespace nxm::store_microsoft {

/// Converts between UTF-8 (nx::string/nx::string_view) and UTF-16
/// (winrt::hstring), the string type every Windows.Services.Store API
/// takes/returns.
[[nodiscard]] winrt::hstring hstring_from_utf8(nx::string_view text);
[[nodiscard]] nx::string utf8_from_hstring(const winrt::hstring &text);

/// Owns the one real precondition this backend has: **package identity**.
/// Windows.Services.Store's `StoreContext` only works inside a process
/// running with an MSIX (or "sparse") package identity - an ordinary
/// unpackaged Win32 exe (exactly what `nx2d.exe` is, confirmed: this
/// project has no MSIX/packaging step anywhere) gets
/// `APPMODEL_ERROR_NO_PACKAGE` the moment it tries to use it. That's a
/// structural, environment-level gate, not a missing-live-client situation
/// like Steam/EOS/GOG/Stove each have - so `has_package_identity()` (a
/// cheap, synchronous, side-effect-free `GetCurrentPackageFullName()`
/// probe, no WinRT call at all) is checked before ever touching
/// `StoreContext`, the same guard-path role `GogPlatform::logged_on()`/
/// `StovePlatform::*_ready()` already play for their own backends.
///
/// Also unlike every other backend in this family: **there is no per-frame
/// pump**. Steam/EOS/GOG/Stove all resolve their async callbacks from
/// inside an explicit `RunCallbacks()`/`ProcessData()`/`Base_RunCallback()`
/// call this module's own `on_attach()` would otherwise schedule every
/// frame - WinRT's `IAsyncOperation<T>::Completed()` callback fires on its
/// own thread-pool thread independently of any pump, so
/// `StoreMicrosoftModule` never registers a `.pump` system at all. Every
/// service class below therefore guards its own cached state with a mutex
/// instead of relying on "only ever touched from the game's own tick".
class MicrosoftPlatform {
public:
  ~MicrosoftPlatform();

  bool initialize();
  void shutdown();

  [[nodiscard]] bool ready() const noexcept { return m_ready; }
  [[nodiscard]] winrt::Windows::Services::Store::StoreContext context() const {
    return m_context;
  }

private:
  bool m_apartment_initialized = false;
  bool m_ready = false;
  winrt::Windows::Services::Store::StoreContext m_context{nullptr};
};

}
