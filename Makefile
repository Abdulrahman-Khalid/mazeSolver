PIO='${HOME}/.local/bin/pio'
ENV='uno_dev'
BAUD=9600

compile: src/* lib/* include/*
	${PIO} run -e ${ENV}

upload:
	sudo ${PIO} run -e ${ENV} -t upload

monitor:
	sudo ${PIO} device monitor -b ${BAUD} -e ${ENV}

testing:
	${PIO} run -e ${ENV} -t test

installPIO:
	sudo python -c "`curl -fsSL https://raw.githubusercontent.com/platformio/platformio/develop/scripts/get-platformio.py`"