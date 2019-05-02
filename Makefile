PIO='${HOME}/.local/bin/pio'
ENV='uno_dev'
BAUD=9600

compile: src/* lib/* include/*
	${PIO} run -e ${ENV}

upload:
	sudo ${PIO} run -e ${ENV} -t upload

monitor:
	sudo ${PIO} device monitor -b ${BAUD} -e ${ENV} -p /dev/ttyACM0

installPIO:
	sudo python -c "`curl -fsSL https://raw.githubusercontent.com/platformio/platformio/develop/scripts/get-platformio.py`"