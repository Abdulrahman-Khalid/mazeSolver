PIO='${HOME}/.local/bin/pio'
PLATFORM='uno'

compile: src/* lib/* include/*
	${PIO} run -e ${PLATFORM}

upload:
	sudo ${PIO} run -e ${PLATFORM} -t upload

installPIO:
	sudo python -c "`curl -fsSL https://raw.githubusercontent.com/platformio/platformio/develop/scripts/get-platformio.py`"