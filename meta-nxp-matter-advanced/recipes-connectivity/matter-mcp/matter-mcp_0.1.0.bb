SUMMARY = "MCP server exposing Matter device tools to a local LLM"
DESCRIPTION = "A Python MCP server that talks to chip-tool over WebSocket and \
exposes Matter tools, bridged to an Ollama LLM via ollama-mcp-bridge."
HOMEPAGE = "https://github.com/nxp-imx/imx-matter-utils"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=d3800cfcc18f27ff8823d542831950f7"

SRCBRANCH = "master"
SRC_URI = "git://github.com/nxp-imx/imx-matter-utils.git;protocol=https;branch=${SRCBRANCH}"
SRCREV = "48f837fbc77371e3f77669ec0788665da8ac249b"

inherit python_setuptools_build_meta

RDEPENDS:${PN} += " \
    python3-mcp \
    python3-websocket-client \
    python3-ollama-mcp-bridge \
    python3-core \
"

do_install:append() {
    # Editable configuration under /etc/matter-mcp
    install -d ${D}${sysconfdir}/matter-mcp
    install -m 0644 ${S}/config/mcp-config.json.target ${D}${sysconfdir}/matter-mcp/mcp-config.json
    install -m 0644 ${S}/config/device_registry.json ${D}${sysconfdir}/matter-mcp/device_registry.json
    install -m 0644 ${S}/config/system-prompt.txt ${D}${sysconfdir}/matter-mcp/system-prompt.txt

    # Helper scripts into /usr/bin with renamed names
    install -d ${D}${bindir}
    install -m 0755 ${S}/scripts/run_bridge.sh ${D}${bindir}/matter-mcp-bridge
    install -m 0755 ${S}/scripts/chat_cli.py ${D}${bindir}/matter-mcp-chat
    install -m 0755 ${S}/scripts/start_device_stack.sh ${D}${bindir}/matter-mcp-start-stack
    install -m 0755 ${S}/scripts/stop_device_stack.sh ${D}${bindir}/matter-mcp-stop-stack
}

FILES:${PN} += " \
    ${sysconfdir}/matter-mcp \
    ${bindir}/matter-mcp-bridge \
    ${bindir}/matter-mcp-chat \
    ${bindir}/matter-mcp-start-stack \
    ${bindir}/matter-mcp-stop-stack \
"

CONFFILES:${PN} += " \
    ${sysconfdir}/matter-mcp/mcp-config.json \
    ${sysconfdir}/matter-mcp/device_registry.json \
    ${sysconfdir}/matter-mcp/system-prompt.txt \
"

# start/stop stack scripts need tmux + chip-tool at runtime (optional helpers)
RRECOMMENDS:${PN} += "tmux"
