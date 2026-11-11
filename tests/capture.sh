#!/bin/bash
# Capture the ctags inputs arduino-cli generates for the test sketches.
#
# Installs a pinned arduino-cli and core into WORK, replaces the builtin
# ctags with a wrapper that saves the file arduino-cli passes to it, then
# runs `arduino-cli compile --preprocess` on every sketch in sketches/.
# Each captured input is written to cases/<sketch>.cpp.
#
# Each sketch is a <name>/<name>.ino directory, as arduino-cli requires. The
# Edge* sketches are ours; the rest are copied from arduino-cli's
# internal/integrationtest/compile_4/testdata at CLI_COMMIT, without the
# expected-output files, the symlink and invalid sketch tests, and the
# sketches that need AVR-only libraries or macros to preprocess
# (SketchThatChecksIfSPIHasTransactions*, SketchUsingRobotIRRemote,
# SketchWithEscapedDoubleQuote, SketchWithFastledsLibrary, SketchWithUsbcon;
# EdgeCommentsAndStrings covers the escaped quotes). Their
# testdata/libraries is fetched here, since some sketches include them.
#
# The expected outputs, cases/<sketch>.ctags, are generated afterwards from
# the shipped ctags this installs (5.8-arduino11, aarch64-linux-gnu):
#
#   python3 -I run.py --update \
#       --ctags /tmp/ctags-capture/data/packages/builtin/tools/ctags/5.8-arduino11/ctags.real
#
# Runs on Linux arm64. The sketch paths end up in the captured files, so WORK
# is fixed.

set -euo pipefail

CLI_VERSION=1.6.0
CLI_COMMIT=50a0e80daf5d5a4e6cdf57b51fbfd71f07a43462
CLI_SHA256=2ed4e1436bb3be1265966af8eba95dd25e28f3a9f92d0a58d371ed14dbaf550f
CORE=arduino:zephyr_main@1.1.0
FQBN=arduino:zephyr_main:giga
CTAGS_VERSION=5.8-arduino11

TESTS=$(cd "$(dirname "$0")" && pwd)
WORK=/tmp/ctags-capture
TESTDATA=${WORK}/arduino-cli/internal/integrationtest/compile_4/testdata

if [ "$(uname -sm)" != "Linux aarch64" ]; then
    echo "Error: expected Linux aarch64, got $(uname -sm)"
    exit 1
fi

rm -rf "${WORK}"
mkdir -p "${WORK}/bin" "${WORK}/sketches" "${WORK}/logs"

export ARDUINO_DIRECTORIES_DATA=${WORK}/data
export ARDUINO_DIRECTORIES_USER=${WORK}/user
export ARDUINO_DIRECTORIES_DOWNLOADS=${WORK}/staging

# arduino-cli
NAME="arduino-cli_${CLI_VERSION}_Linux_ARM64.tar.gz"
curl -fsSL -o "${WORK}/${NAME}" \
    "https://github.com/arduino/arduino-cli/releases/download/v${CLI_VERSION}/${NAME}"
echo "${CLI_SHA256}  ${WORK}/${NAME}" | sha256sum -c -
tar xzf "${WORK}/${NAME}" -C "${WORK}/bin" arduino-cli
CLI=${WORK}/bin/arduino-cli
"${CLI}" version

# Libraries used by arduino-cli's test sketches.
git clone -q --filter=blob:none --no-checkout --depth 1 --branch "v${CLI_VERSION}" \
    https://github.com/arduino/arduino-cli "${WORK}/arduino-cli"
git -C "${WORK}/arduino-cli" sparse-checkout set internal/integrationtest/compile_4/testdata
git -C "${WORK}/arduino-cli" checkout -q
test "$(git -C "${WORK}/arduino-cli" rev-parse HEAD)" = "${CLI_COMMIT}"

cp -a "${TESTS}"/sketches/. "${WORK}/sketches/"

# Core, and the libraries arduino-cli's test installs.
"${CLI}" core update-index
"${CLI}" core install "${CORE}"
"${CLI}" lib install Bridge@1.6.1 CapacitiveSensor@0.5 FastLED@3.1.0

# Replace ctags with a wrapper that saves its input (the last argument).
CTAGS_DIR=${ARDUINO_DIRECTORIES_DATA}/packages/builtin/tools/ctags/${CTAGS_VERSION}
if [ ! -x "${CTAGS_DIR}/ctags" ]; then
    echo "Error: ${CTAGS_DIR}/ctags not installed"
    exit 1
fi
mv "${CTAGS_DIR}/ctags" "${CTAGS_DIR}/ctags.real"
cat > "${CTAGS_DIR}/ctags" <<EOF
#!/bin/sh
for last; do :; done
cp "\${last}" "${WORK}/input.cpp"
exec "${CTAGS_DIR}/ctags.real" "\$@"
EOF
chmod +x "${CTAGS_DIR}/ctags"

mkdir -p "${TESTS}/cases"
rm -f "${TESTS}"/cases/*.cpp
captured=0
missing=()
for dir in "${WORK}"/sketches/*/; do
    name=$(basename "${dir}")
    rm -f "${WORK}/input.cpp"
    if "${CLI}" compile -b "${FQBN}" --preprocess --libraries "${TESTDATA}/libraries" \
            "${dir}" > "${WORK}/logs/${name}.log" 2>&1; then
        status=ok
    else
        status=failed
    fi
    if [ -f "${WORK}/input.cpp" ]; then
        cp "${WORK}/input.cpp" "${TESTS}/cases/${name}.cpp"
        captured=$((captured + 1))
        echo "captured ${name} (preprocess ${status})"
    else
        missing+=("${name}")
        echo "no ctags input for ${name} (preprocess ${status}, see ${WORK}/logs/${name}.log)"
    fi
done

echo "Captured ${captured}, no input for ${#missing[@]}: ${missing[*]:-}"
