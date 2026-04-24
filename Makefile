CXX = clang++
CXXFLAGS = -O3 -fPIC $(shell pkg-config --cflags Qt6Widgets)
LDFLAGS = $(shell pkg-config --libs Qt6Widgets)
PREFIX = /usr

all:
	$(CXX) $(CXXFLAGS) portager.cpp -o portager $(LDFLAGS)

install:
	install -D -m 755 portager $(DESTDIR)$(PREFIX)/bin/portager
	install -D -m 644 portager.png $(DESTDIR)$(PREFIX)/share/pixmaps/portager.png
	install -D -m 644 portager.desktop $(DESTDIR)$(PREFIX)/share/applications/portager.desktop
	install -D -m 644 background.png $(DESTDIR)$(PREFIX)/share/portager/background.png
