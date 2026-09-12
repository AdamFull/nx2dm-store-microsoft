#include "framework/nxtest.h"

#include "store_microsoft/store_microsoft_platform.h"

using namespace nxm::store_microsoft;

// Unlike Steam/EOS/GOG/Stove's own initialize() (each does real I/O - a
// local Steamworks call, a network round trip, or a launcher check), this
// one is cheap and side-effect-free: GetCurrentPackageFullName() is a
// synchronous Win32 API with no I/O, no network, and nothing to fake. It's
// therefore safe (and worth doing) to call it for real here rather than
// only asserting the guard path never touches it - this test doubles as a
// real regression check that the guard condition genuinely holds in
// exactly the kind of build this project produces (nx_store_microsoft_tests
// is an ordinary unpackaged Win32 exe, with no MSIX identity).
TEST_CASE("store_microsoft: an unpackaged test binary has no package "
          "identity") {
  MicrosoftPlatform platform;
  CHECK_FALSE(platform.initialize());
  CHECK_FALSE(platform.ready());
}

TEST_CASE("store_microsoft: utf8/hstring round-trips") {
  const winrt::hstring wide = hstring_from_utf8("hello store");
  CHECK(utf8_from_hstring(wide).view() == "hello store");
  CHECK(hstring_from_utf8("").empty());
  CHECK(utf8_from_hstring(winrt::hstring{}).empty());
}
