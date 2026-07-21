SUMMARY = "The official Python SDK for Model Context Protocol servers and clients"
HOMEPAGE = "https://github.com/modelcontextprotocol/python-sdk"
SECTION = "devel/python"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=7ae711d8a91d3871696f50e34ad3c2d7"

inherit pypi python_hatchling

PYPI_PACKAGE = "mcp"

SRC_URI += "file://0001-use-static-version.patch"
SRC_URI[sha256sum] = "d51e36a5f5644faea4f85ea649bfffa6bc6c26770d42798ad6a3de3d2ba69683"

# Upstream builds its version from git tags via uv-dynamic-versioning, which is
# not packaged for Yocto. The patch pins a static version so hatchling can build
# from the released sdist without VCS metadata.

RDEPENDS:${PN} += "\
    python3-anyio \
    python3-httpx \
    python3-httpx-sse \
    python3-jsonschema \
    python3-pydantic \
    python3-pydantic-settings \
    python3-pyjwt \
    python3-cryptography \
    python3-python-multipart \
    python3-sse-starlette \
    python3-starlette \
    python3-typing-extensions \
    python3-typing-inspection \
    python3-uvicorn \
    python3-core \
"
