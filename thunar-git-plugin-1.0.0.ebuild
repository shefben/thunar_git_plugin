# Copyright 2025 MiniMax Agent
# Distributed under the terms of the GNU General Public License v2

EAPI=8

inherit meson xdg-utils

DESCRIPTION="Native Git integration plugin for Thunar file manager"
HOMEPAGE="https://github.com/minimax-agent/thunar-git-plugin"
SRC_URI=""

LICENSE="GPL-2+"
SLOT="0"
KEYWORDS="amd64 x86 arm arm64"
IUSE=""

DEPEND="
    >=dev-libs/glib-2.50.0:0
    >=x11-libs/gtk+-3.22.0:3
    >=dev-libs/libgit2-1.0.0
    >=xfce-base/thunar-1.8.0
"
RDEPEND="${DEPEND}"
BDEPEND="
    >=dev-util/meson-0.50.0
    virtual/pkgconfig
"

src_configure() {
    meson_src_configure
}

src_compile() {
    meson_src_compile
}

src_install() {
    meson_src_install
}

pkg_postinst() {
    xdg_icon_cache_update
    
    elog "Thunar Git Plugin has been installed."
    elog ""
    elog "To activate the plugin:"
    elog "1. Restart Thunar or log out and log back in"
    elog "2. Right-click on files/folders in Git repositories"
    elog "3. The 'Git' submenu should now be available"
    elog ""
    elog "For emblem overlays to work, you may need to configure"
    elog "your GTK icon theme to include the installed emblems."
}

pkg_postrm() {
    xdg_icon_cache_update
}
