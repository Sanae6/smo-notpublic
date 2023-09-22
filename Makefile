LOGGER_IP ?= "" # put log server IP in string
FTP_IP ?= 192.168.1.136 # 172.22.88.125 # put console IP here
.PHONY: all clean

all:
	cmake --toolchain=cmake/toolchain.cmake -DFTP_IP=$(FTP_IP) -DLOGGER_IP="64.201.219.20" -S . -B build && $(MAKE) -C build subsdk9_meta

clean:
	rm -r build || true

log: all
	python3.8 scripts/tcpServer.py 0.0.0.0
