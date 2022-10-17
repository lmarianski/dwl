# Maintainer: Devin J. Pohly <djpohly+arch@gmail.com>
pkgname=dwl-git
pkgver=0.3.1.r139.e9826de
pkgrel=1
pkgdesc="Simple, hackable dynamic tiling Wayland compositor (dwm for Wayland)"
arch=('x86_64')
url="https://github.com/djpohly/dwl"
license=('GPL')
depends=('wlroots>=0.15')
makedepends=('git' 'wayland-protocols')
optdepends=('xorg-xwayland: for XWayland support')
provides=("${pkgname%-git}")
conflicts=("${pkgname%-git}")
# append #branch=wlroots-next to build against wlvroots-git
source=('git+https://github.com/djpohly/dwl'
	'shiftview.patch::https://github.com/djpohly/dwl/compare/main...guidocella:shiftview.patch'
	'swaycompat.patch::https://raw.githubusercontent.com/StratusFearMe21/dwl/main/0001-For-patching.patch'
	'https://github.com/djpohly/dwl/compare/main...guidocella:output-power-management.patch'
	'dwl-scratchpad.patch'
)
sha256sums=('SKIP'
            '9d2b6e8ae8f172ab6e910856c05a051e2bec871708bcf36fbce62014854b53fb'
            '7759a53f011f8ca8f4ec210d1b2aacf5cd6b6531ec45d7280e2f2f1c30ca6e87'
            '020e5fb360281aee3a63915049814dc6cd457b79e743f902afca775611722b6a'
            'd566b2a8a531a0cc826c7e8b2407db22cd45f4b9ee903530371f947c5813775e')

prepare() {
	cd "$srcdir/${pkgname%-git}"

	# Uncomment to compile with XWayland support
	sed -i -e '/-DXWAYLAND/s/^#//' config.mk
	sed -i -e '/xcb/s/^#//' config.mk

	patch -p1 -t --input "$srcdir/shiftview.patch"
	# patch -p1 --input "$srcdir/swaycompat.patch" --merge
	# patch -p1 --input "$srcdir/dwl-scratchpad.patch"

	# This package provides a mechanism to provide a custom config.h. Multiple
	# configuration states are determined by the presence of two files in
	# $BUILDDIR:
	#
	# config.h  config.def.h  state
	# ========  ============  =====
	# absent    absent        Initial state. The user receives a message on how
	#                         to configure this package.
	# absent    present       The user was previously made aware of the
	#                         configuration options and has not made any
	#                         configuration changes. The package is built using
	#                         default values.
	# present                 The user has supplied his or her configuration. The
	#                         file will be copied to $srcdir and used during
	#                         build.
	#
	# After this test, config.def.h is copied from $srcdir to $BUILDDIR to
	# provide an up to date template for the user.
	if [ -e "$BUILDDIR/config.h" ]; then
		cp "$BUILDDIR/config.h" .
	elif [ ! -e "$BUILDDIR/config.def.h" ]; then
		msg='This package can be configured in config.h. Copy the config.def.h '
		msg+='that was just placed into the package directory to config.h and '
		msg+='modify it to change the configuration. Or just leave it alone to '
		msg+='continue to use default values.'
		echo "$msg"
	fi
	cp config.def.h "$BUILDDIR"
}

pkgver() {
	cd "$srcdir/${pkgname%-git}"
	printf "%s" "$(git describe --long | sed 's/^v//;s/\([^-]*-\)g/r\1/;s/-/./g')"
}

build() {
	cd "$srcdir/${pkgname%-git}"
	make
}

package() {
	cd "$srcdir/${pkgname%-git}"
	make PREFIX="$pkgdir/usr/" install
}
