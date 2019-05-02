PIO='${HOME}/.local/bin/pio'
ENV='uno_dev'

compile: src/* lib/* include/*
	${PIO} run -e ${ENV}

upload:
	sudo ${PIO} run -e ${ENV} -t upload

installPIO:
	sudo python -c "`curl -fsSL https://raw.githubusercontent.com/platformio/platformio/develop/scripts/get-platformio.py`"