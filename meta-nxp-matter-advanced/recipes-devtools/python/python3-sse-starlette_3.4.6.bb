SUMMARY = "Server Sent Events for Starlette and FastAPI"
HOMEPAGE = "https://github.com/sysid/sse-starlette"
SECTION = "devel/python"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=512780230e4edd77125d98c5f52abafe"

inherit pypi python_setuptools_build_meta

PYPI_PACKAGE = "sse_starlette"

SRC_URI[sha256sum] = "725f8a1bd6d26ae1b2c9610c0ef5065dfdd496f3988d28adcf8c4b49dc25c627"

RDEPENDS:${PN} += "\
    python3-starlette \
    python3-anyio \
    python3-core \
"
