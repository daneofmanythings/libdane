setup:
	@ echo === SETUP BEGIN ===
	@ meson setup builddir
	@ echo === SETUP END ===

build: setup
	@ echo === BUILD BEGIN ===
	@ meson compile -C builddir
	@ echo === BUILD END ===

install: build
	@ echo === INSTALL BEGIN ===
	@ meson install -C builddir
	@ sudo ldconfig
	@ echo === INSTALL END ===

test: build
	@ echo === TEST BEGIN ===
	@ meson test -C builddir
	@ echo === TEST END ===

debug:
	@ gdb -q ./builddir/platform/test_platform \
		-ex="layout src target remote localhost:1234"
