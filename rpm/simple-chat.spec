Name:           simple-chat
Version:        VERSION
Release:        1%{?dist}
Summary:        Here be a simple chat server!

License:        GPLv3+
URL:            https://benjamintoll.com
Source0:        https://github.com/btoll/simple-chat/releases/download/VERSION/simple-chat_VERSION.tar.gz

BuildRequires:  gcc
Requires:       make

%description
Here be a simple chat server!

%prep
%setup -q

%build
make %{?_smp_mflags}

%install
%make_install

%files
%license LICENSE
%{_bindir}/%{name}

%changelog
