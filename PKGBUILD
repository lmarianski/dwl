# Maintainer: Devin J. Pohly <djpohly+arch@gmail.com>
pkgname=dwl-git
pkgver=0.3.1.r0.b86fcf6
pkgrel=1
pkgdesc="Simple, hackable dynamic tiling Wayland compositor (dwm for Wayland)"
arch=('x86_64')
url="https://github.com/djpohly/dwl"
license=('GPL')
depends=('wlroots>=0.13')
makedepends=('git' 'wayland-protocols')
optdepends=('xorg-xwayland: for XWayland support')
provides=("${pkgname%-git}")
conflicts=("${pkgname%-git}")
# append #branch=wlroots-next to build against wlroots-git
source=('git+https://github.com/djpohly/dwl'
		'dwl-scratchpad.patch'
)
sha256sums=('SKIP'
            'SKIP')

prepare() {
	cd "$srcdir/${pkgname%-git}"
	cp "$srcdir/config.h" config.h
	# Uncomment to compile with XWayland support
	sed -i -e '/-DXWAYLAND/s/^#//' config.mk

	patch -p1 --input "$srcdir/dwl-scratchpad.patch"

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
	echo $srcdir

	echo 

	if [ -e "$BUILDDIR/config.h" ]
	then
		cp "$BUILDDIR/config.h" .
	elif [ ! -e "$BUILDDIR/config.def.h" ]
	then
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
