Summary: Network Monitoring Utility
Name: netled
Version: 1.0
Release: 1
Copyright: GPL
Group: Network/Deamons
Source: https://github.com/amcgregor/netled/

%description
NetLED monitors the RD and SD of interfaces using the LEDs on your keyboard.

%prep
%setup

%build
make clean all

%install
make install
