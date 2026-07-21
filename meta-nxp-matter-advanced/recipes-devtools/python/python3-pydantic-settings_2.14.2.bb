SUMMARY = "Settings management using pydantic"
HOMEPAGE = "https://github.com/pydantic/pydantic-settings"
SECTION = "devel/python"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=9adde1a30a7e74a03e57e456551c19ae"

inherit pypi python_hatchling

PYPI_PACKAGE = "pydantic_settings"

SRC_URI[sha256sum] = "c19dd64b19097f1de80184f0cc7b0272a13ae6e170cbf240a3e27e381ed14a5f"

RDEPENDS:${PN} += "\
    python3-pydantic \
    python3-python-dotenv \
    python3-typing-inspection \
    python3-core \
"
