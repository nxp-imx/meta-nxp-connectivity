SUMMARY = "Bridge API service connecting Ollama with Model Context Protocol (MCP) servers"
HOMEPAGE = "https://github.com/jonigl/ollama-mcp-bridge"
SECTION = "devel/python"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=4b4b68072bc246cb89db90eafbbc26bf"

inherit pypi python_hatchling

PYPI_PACKAGE = "ollama_mcp_bridge"

SRC_URI += "file://0001-use-static-version.patch"
SRC_URI[sha256sum] = "c2215f54e3e38ccec0437c720b2cac062e27826aa0ff67a50ac3e78ebc61d6c4"

# Upstream derives its version from git via hatch-vcs. The released sdist already
# ships a baked src/ollama_mcp_bridge/_version.py, so the patch drops the VCS
# build hook and pins a static version for a reproducible Yocto build.

RDEPENDS:${PN} += "\
    python3-fastapi \
    python3-httpx \
    python3-loguru \
    python3-mcp \
    python3-packaging \
    python3-typer \
    python3-uvicorn \
    python3-core \
"
