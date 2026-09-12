#include "store_microsoft/store_microsoft_platform.h"

#include "core/foundation/diagnostics/log.h"

#include <Windows.h>
#include <appmodel.h>

#pragma comment(lib, "windowsapp")

namespace nxm::store_microsoft {
namespace {

const nx::log::Category log_store_microsoft = nx::log::category("store_microsoft");

[[nodiscard]] bool has_package_identity() {
  UINT32 length = 0;
  const LONG result = ::GetCurrentPackageFullName(&length, nullptr);
  return result != APPMODEL_ERROR_NO_PACKAGE;
}

} // namespace

winrt::hstring hstring_from_utf8(const nx::string_view text) {
  if (text.empty())
    return {};
  const int need =
      ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
                             static_cast<int>(text.size()), nullptr, 0);
  if (need <= 0)
    return {};
  nx::vector<wchar_t> wide(static_cast<usize>(need));
  ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
                        static_cast<int>(text.size()), wide.data(), need);
  return winrt::hstring(wide.data(), static_cast<uint32_t>(wide.size()));
}

nx::string utf8_from_hstring(const winrt::hstring &text) {
  if (text.empty())
    return {};
  const int wide_len = static_cast<int>(text.size());
  const int need = ::WideCharToMultiByte(CP_UTF8, 0, text.data(), wide_len,
                                         nullptr, 0, nullptr, nullptr);
  if (need <= 0)
    return {};
  nx::string out;
  out.resize(static_cast<usize>(need));
  ::WideCharToMultiByte(CP_UTF8, 0, text.data(), wide_len, out.data(), need,
                        nullptr, nullptr);
  return out;
}

MicrosoftPlatform::~MicrosoftPlatform() { shutdown(); }

bool MicrosoftPlatform::initialize() {
  if (!has_package_identity()) {
    // Expected for an ordinary unpackaged build (exactly what nx2d.exe is
    // today) - not an error, the same honest-failure shape store_steam's
    // SteamAPI_Init() already has when no Steam client is running.
    nx::logi(log_store_microsoft,
              "no package identity (APPMODEL_ERROR_NO_PACKAGE) - staying idle");
    return false;
  }

  try {
    winrt::init_apartment();
    m_apartment_initialized = true;
    m_context = winrt::Windows::Services::Store::StoreContext::GetDefault();
    m_ready = m_context != nullptr;
  } catch (const winrt::hresult_error &error) {
    nx::logw(log_store_microsoft, "StoreContext::GetDefault() failed: {}",
              utf8_from_hstring(error.message()).view());
    m_ready = false;
  }
  return m_ready;
}

void MicrosoftPlatform::shutdown() {
  m_context = nullptr;
  m_ready = false;
  if (m_apartment_initialized) {
    winrt::uninit_apartment();
    m_apartment_initialized = false;
  }
}

}
