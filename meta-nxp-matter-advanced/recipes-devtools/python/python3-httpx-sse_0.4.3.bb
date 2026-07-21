SUMMARY = "Consume Server-Sent Events (SSE) with HTTPX"
HOMEPAGE = "https://github.com/florimondmanca/httpx-sse"
SECTION = "devel/python"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=847662516af24b720ff6168b0dfcc6b1"

inherit pypi setuptools3

PYPI_PACKAGE = "httpx_sse"

SRC_URI[sha256sum] = "9b1ed0127459a66014aec3c56bebd93da3c1bc8bb6618c8082039a44889a755d"

RDEPENDS:${PN} += "\
    python3-httpx \
    python3-core \
"
