#!/bin/sh
# Rebuilds PULP on macOS from a clean checkout. The counterpart of build.bat.
#   ./build.sh              release build of everything, runs the tests, fills dist/PULP (PULP.app)
#   ./build.sh test         engine + tests only, in Debug, and runs them
#   ./build.sh asan         the same with AddressSanitizer and UndefinedBehaviorSanitizer
#   ./build.sh quick        release build of engine, render and tests only (no JUCE), runs the tests
#   ./build.sh app          release build of the program only, fills dist/PULP, skips the tests
#   ./build.sh universal    as the plain build, but one program for Apple silicon and Intel Macs
#   ./build.sh testplugins  the two test plugins (VST3 and AU) that scripts/m10_vst3.txt hosts
#   ./build.sh installer    packs dist/PULP into dist/PULP.dmg (run ./build.sh first)
#   ./build.sh library      fetches the recordings (tools/fetch-samples.py) and rebuilds the factory instruments
#
# Needs the Xcode command line tools (xcode-select --install) and CMake 3.24 or later
# (brew install cmake). Ninja is used when it is installed, Makefiles otherwise. Python 3 is needed
# only for the recorded part of the factory library.
#
# Signing: the bundle is signed ad hoc, which is enough to run it on this Mac. To give it to others,
# set PULP_SIGN_IDENTITY to a "Developer ID Application: ..." identity; it is then signed with the
# hardened runtime and tools/macos/PULP.entitlements, ready for notarization.
set -e
cd "$(dirname "$0")"

command -v cmake >/dev/null 2>&1 || { echo "CMake not found: brew install cmake (or get it from cmake.org)"; exit 1; }
xcrun --find clang++ >/dev/null 2>&1 || { echo "No compiler: run xcode-select --install"; exit 1; }
if command -v ninja >/dev/null 2>&1; then GEN="Ninja"; else GEN="Unix Makefiles"; fi
JOBS="$(sysctl -n hw.ncpu)"
export CMAKE_BUILD_PARALLEL_LEVEL="$JOBS"

configure() {   # configure <dir> <type> <extra cmake args...>
  dir="$1"; type="$2"; shift 2
  cmake -S . -B "$dir" -G "$GEN" -DCMAKE_BUILD_TYPE="$type" "$@"
}

# Fetches the recordings listed in factory/sources.json (checksums verified) and builds the factory
# instruments: from those recordings where there are any, by synthesis for the rest. Without Python
# or a network the synthesized instruments are still built.
makelibrary() {
  "$1/pulp_render" --make-factory factory
  if command -v python3 >/dev/null 2>&1; then python3 -I tools/fetch-samples.py || true; fi
  "$1/pulp_render" --build-library factory/sources.json factory-cache factory
  if command -v python3 >/dev/null 2>&1; then python3 -I tools/make_credits.py || true; fi
}

signapp() {
  app="dist/PULP/PULP.app"
  [ -d "$app" ] || return 0
  if [ -n "$PULP_SIGN_IDENTITY" ]; then
    codesign --force --deep --timestamp --options runtime --entitlements tools/macos/PULP.entitlements -s "$PULP_SIGN_IDENTITY" "$app"
    codesign --force --timestamp --options runtime -s "$PULP_SIGN_IDENTITY" dist/PULP/pulp_render
  else
    # ad hoc: runs on this Mac (and others after "Open" in the Finder's context menu)
    codesign --force --deep --entitlements tools/macos/PULP.entitlements -s - "$app"
  fi
}

release() {   # release <dir> <extra cmake args...>
  dir="$1"; shift
  configure "$dir" Release -DPULP_BUILD_APP=ON "$@"
  cmake --build "$dir"
  "$dir/pulp_tests"
  # the factory instruments are built once; "./build.sh library" rebuilds them
  [ -d factory/Instruments ] || makelibrary "$dir"
  cmake --build "$dir" --target dist
  signapp
  echo
  echo "Built dist/PULP/PULP.app"
}

case "$1" in
  quick)
    configure build/quick Release -DPULP_BUILD_APP=OFF
    cmake --build build/quick
    shift; build/quick/pulp_tests "$@"
    ;;
  test)
    configure build/debug Debug -DPULP_BUILD_APP=OFF -DPULP_ASAN=OFF
    cmake --build build/debug
    shift; build/debug/pulp_tests "$@"
    ;;
  asan)
    configure build/asan Debug -DPULP_BUILD_APP=OFF -DPULP_ASAN=ON
    cmake --build build/asan
    shift; ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 build/asan/pulp_tests "$@"
    ;;
  library)
    configure build/quick Release -DPULP_BUILD_APP=OFF
    cmake --build build/quick --target pulp_render
    makelibrary build/quick
    ;;
  app)
    configure build/release Release -DPULP_BUILD_APP=ON
    cmake --build build/release --target dist
    signapp
    ;;
  testplugins)
    configure build/release Release -DPULP_BUILD_APP=ON
    cmake --build build/release --target testplugins
    ;;
  installer)
    sh tools/macos/make-dmg.sh
    ;;
  universal)
    release build/universal -DPULP_UNIVERSAL=ON
    ;;
  "")
    release build/release
    ;;
  *)
    echo "unknown command: $1 (see the top of build.sh)"; exit 1
    ;;
esac
