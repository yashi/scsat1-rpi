SRC_URI += "file://mcp2517fd-overlay.dts;subdir=git/arch/${ARCH}/boot/dts/overlays"

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

PACKAGE_ARCH = "${MACHINE_ARCH}"

