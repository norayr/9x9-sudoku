%define ver      0.6
%define RELEASE  1
%define rel      %RELEASE
%define prefix   /usr
%define PACKAGE_VERSION  0

Summary: Thumb Sudoku
Name: thumb_sudoku
Version: %ver
Release: %rel
License: GPL-v2
Group: Games
Source: http://www.iki.fi/too/sw/thumb_sudoku
BuildRoot: /var/tmp/tsu-%{PACKAGE_VERSION}-root
URL: http://www.iki.fi/too/sw/

%description
 A Sudoku Game playable with fingers (ymmv).

%prep
%setup

%build
make

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install_maemo # XXX

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644, root, root, 755)

%doc README
opt/*
usr/*

%changelog
thumb-sudoku (0.6) unstable; urgency=low

  * Fixed bug with using hints (those small numbers) (one off-by-one offset).

 -- Tomi Ollila <tomi.ollila@iki.fi>  Mon, 01 Mar 2010 16:18:37 +0200

thumb-sudoku (0.5) unstable; urgency=low

  * Fixed bug when loading (small number) reference value from saved state

 -- Tomi Ollila <tomi.ollila@iki.fi>  Fri, 26 Feb 2010 08:29:44 +0200

thumb-sudoku (0.4) unstable; urgency=low

  * Save game state on exit which is loaded at startup

  * Hildon application menu with 'New Game' item

 -- Tomi Ollila <tomi.ollila@iki.fi>  Mon, 22 Feb 2010 20:23:06 +0000

thumb-sudoku (0.3) unstable; urgency=low

  * Initial release

 -- Tomi Ollila <tomi.ollila@iki.fi>  Tue, 02 Feb 2010 22:04:25 +0000
