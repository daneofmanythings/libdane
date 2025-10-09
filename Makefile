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
	@ echo === INSTALL END ===
