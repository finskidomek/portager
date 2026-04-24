CXX = clang++
CXXFLAGS = -O3 -fPIC $(shell pkg-config --cflags Qt6Widgets)
LDFLAGS = $(shell pkg-config --libs Qt6Widgets)
PREFIX = /usr

all:
	$(CXX) $(CXXFLAGS) portager.cpp -o portager $(LDFLAGS)

install:
	# Kopiowanie programu do /usr/bin
	install -D -m 755 portager $(DESTDIR)$(PREFIX)/bin/portager
	
	# Kopiowanie ikony do /usr/share/pixmaps (stąd systemy zawsze ją odczytają)
	install -D -m 644 portager.png $(DESTDIR)$(PREFIX)/share/pixmaps/portager.png
	
	# Kopiowanie pliku menu do /usr/share/applications
	install -D -m 644 portager.desktop $(DESTDIR)$(PREFIX)/share/applications/portager.desktop
