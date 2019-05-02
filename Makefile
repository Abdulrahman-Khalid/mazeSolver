PIO='${HOME}/.local/bin/pio'
PLATFORM='uno'

compile: src/* lib/* include/*
	${PIO} run -e ${PLATFORM}

upload:
	sudo ${PIO} run -e ${PLATFORM} -t upload