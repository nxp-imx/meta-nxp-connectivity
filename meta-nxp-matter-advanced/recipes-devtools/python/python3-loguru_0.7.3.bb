SUMMARY = "Python logging made (stupidly) simple"
HOMEPAGE = "https://github.com/Delgan/loguru"
SECTION = "devel/python"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${UNPACKDIR}/LICENSE;md5=4c50b6bd2e1e6481b0c7ed7691425651"

inherit pypi python_flit_core

SRC_URI[sha256sum] = "19480589e77d47b8d85b2c827ad95d49bf31b0dcde16593892eb51dd18706eb6"

# The sdist ships no LICENSE file; provide it from the upstream repo.
SRC_URI += "file://LICENSE"

RDEPENDS:${PN} += "\
    python3-logging \
    python3-core \
"
