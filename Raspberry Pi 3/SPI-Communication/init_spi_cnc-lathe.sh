#!/bin/sh
### BEGIN INIT INFO
# Provides:          spi_cnc-lathe
# Required-Start:    
# Required-Stop:     
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Starts & Stops CNC-Lathe Webinterface Backend
# Description:       Starts & Stops CNC-Lathe Webinterface Backend for SPI-Commuication with Arduino
### END INIT INFO

#File must have linux-file-endings

#Installation
#sudo cp init_spi_cnc-lathe.sh /etc/init.d/
#sudo chmod 755 /etc/init.d/init_spi_cnc-lathe.sh
#sudo update-rc.d init_spi_cnc-lathe.sh defaults

#Deinstallation
#sudo update-rc.d init_spi_cnc-lathe.sh remove
#sudo rm /etc/init.d/init_spi_cnc-lathe.sh

#Manual commands
#sudo /etc/init.d/init_spi_cnc-lathe.sh start
#sudo /etc/init.d/init_spi_cnc-lathe.sh stop
#sudo /etc/init.d/init_spi_cnc-lathe.sh restart
 
#Switch case for first Parameter
case "$1" in
    start)
		#action for start
		#DISPLAY=:0.0
		echo "Start CNC-Lathe Webinterface Backend"
		su -c "/home/pi/spi_com/spi_cnc-lathe" - pi
		;;
 
    stop)
		#action for stop
		echo "Stop CNC-Lathe Webinterface Backend"
		killall -e spi_cnc-lathe
		;;
 
    restart)
		#action for restart
		#DISPLAY=:0.0
		echo "Restart CNC-Lathe Webinterface Backend"
		killall -e -w spi_cnc-lathe
		su -c "/home/pi/spi_com/spi_cnc-lathe" - pi
		;;
		
	*)
		#default action
		echo "(start|stop|restart)"
		;;
esac
 
exit 0