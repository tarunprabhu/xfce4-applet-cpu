plugindir = $(libdir)/xfce4/panel/plugins
plugin_LTLIBRARIES = libappletcpu.la

libappletcpu_la_CFLAGS =					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"			\
	@LIBXFCE4UI_CFLAGS@					\
	@LIBXFCE4PANEL_CFLAGS@

libappletcpu_la_LIBADD =					\
	@LIBXFCE4UI_LIBS@					\
	@LIBXFCE4PANEL_LIBS@

libappletcpu_la_SOURCES =                                 \
	cpu.c                                                   \
	cpu.h                                                   \
	os.c                                                    \
	os.h                                                    \
	properties.c                                            \
	properties.h                                            \
	settings.c                                              \
	settings.h

libappletcpu_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined \
	-export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

# .desktop file
#
desktop_in_files = applet-cpu.desktop.in

desktopdir = $(datadir)/xfce4/panel/plugins
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

EXTRA_DIST = $(desktop_in_files)

DISTCLEANFILES = $(desktop_DATA)

%desktop: %desktop.in
	cp $< $@
