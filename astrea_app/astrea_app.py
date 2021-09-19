import sys
from ainterface import Ui_MainWindow
from PyQt5 import QtWidgets , QtCore 
import serial.tools.list_ports
import csv
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas



class myApp(QtWidgets.QMainWindow): #interface

    def __init__(self):

        super(myApp,self).__init__()
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        #combobox içerikleri
        self.ports = serial.tools.list_ports.comports()
        for i in self.ports:
            self.ui.comboBox.addItem(str(i))
        

        boud = ["300", "1200", "2400", "4800", "9600", "19200", "38400", "57600", "74880", "115200", "230400", "250000",
                "500000", "1000000", "2000000"]
        for i in boud:
            self.ui.comboBox_2.addItem(i)
        self.ui.comboBox_2.setCurrentText(boud[4])

        #seriport connection

        self.mySerial = serialThreadClass()
        self.mySerial.message.connect(self.savingData)   #def
        self.mySerial.start()

        #button

        self.ui.pushButton.clicked.connect(self.serialConnect)  #def
        self.ui.pushButton_2.clicked.connect(self.serialDisconnect)  #def

        #graf
        self.figurex = plt.figure()
        self.figurey = plt.figure()
        self.figurealt = plt.figure()
        self.canvasx = FigureCanvas(self.figurex)
        self.canvasy = FigureCanvas(self.figurey)
        self.canvasalt = FigureCanvas(self.figurealt)
        self.ui.verticalLayout.addWidget(self.canvasx)
        self.ui.verticalLayout.addWidget(self.canvasy)
        self.ui.verticalLayout_2.addWidget(self.canvasalt)


        #set csv file
        fieldnames = ["x_value","y_value","temp","pressure","altitude","accZ","control1","control2"]
        with open("data.csv","w") as csv_file:
            csv_writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
            csv_writer.writeheader()
        self.show()





    #animation
    def animation(self):
        self.timer = QtCore.QTimer()
        self.timer.setInterval(100)
        self.timer.timeout.connect(self.plot)
        self.timer.start()

    #grafics
    def plot(self):
        #data
        data = pd.read_csv("data.csv")
        self.x = data["x_value"]
        self.y = data["y_value"]
        self.alt = data["altitude"]

        self.figurex.clear()
        self.figurey.clear()
        self.figurealt.clear()

        xplot = self.figurex.add_subplot(111)
        yplot = self.figurey.add_subplot(111)
        altplot = self.figurealt.add_subplot(111)

        #plot data
        xplot.plot(self.x , "red")
        yplot.plot(self.y,"blue")
        altplot.plot(self.alt,"black")

        #refresh canvas
        self.canvasx.draw()
        self.canvasy.draw()
        self.canvasalt.draw()


    #connect
    def serialConnect(self):
        self.porttext = self.ui.comboBox.currentText()
        self.port = self.porttext.split()
        self.boudrate = self.ui.comboBox_2.currentText()
        self.mySerial.serialPort.boudrate = int(self.boudrate)
        self.mySerial.serialPort.port = self.port[0]

        try:
            self.mySerial.serialPort.open()
        except:
            print("bağlantı hatası")
        
        if(self.mySerial.serialPort.isOpen()):
            self.ui.pushButton.setEnabled(False)
            self.ui.comboBox.setEnabled(False)
            self.ui.comboBox_2.setEnabled(False)
            self.animation()  #def
    

    #disconnect
    def serialDisconnect(self):
        if self.mySerial.serialPort.isOpen():
            self.mySerial.serialPort.close()
            if self.mySerial.serialPort.isOpen == False:
                self.ui.pushButton.setEnabled(True)
                self.ui.comboBox.setEnabled(True)
                self.ui.comboBox_2.setEnabled(True)
        else:
            self.ui.textEdit.append("Serial port already close")
    

    #saving data 
    def savingData (self):
        self.incomingMessage = str(self.mySerial.data.decode())
        values = self.incomingMessage.split()
        x = int(values[0])
        y = int(values[1])
        temp = int(values[2])
        pressure = int(values[3])
        altitude = int(values[4])
        accZ = int(values[5])
        control1 = int(values[6])
        control2 = int(values[7])


        fieldnames = ["x_value","y_value","temp","pressure","altitude","accZ","control1","control2"]

        with open("data.csv","a") as csv_file:
            csv_writer = csv.DictWriter(csv_file,fieldnames=fieldnames)

            info = {
                "x_value" : x ,
                "y_value" : y ,
                "temp" : temp ,
                "pressure" : pressure ,
                "altitude" : altitude ,
                "accZ" : accZ ,
                "control1" : control1 ,
                "control2" : control2

            }
            csv_writer.writerow(info)
        
        self.writedata()
    

    def writedata(self):
        data = pd.read_csv("data.csv")
        np_alt = np.array()
        alt = int(data["altitude"])
        np_alt = np.append(np_alt,alt)

        self.ui.textEdit_2.append("----CURRENT VALUES----")
        self.ui.textEdit_2.append(f'Tempature : {data["temp"]}')
        self.ui.textEdit_2.append(f'Pressure: {data["pressure"]}')
        self.ui.textEdit_2.append(f'Acceleration : {data["accZ"]}')
        self.ui.textEdit_2.append(f"Max altitude: {np_alt.max()}")

        if (int(data["control1"]) == 1) :
            self.ui.textEdit.append("ACENDING START")
        if (int(data["control2"]) == 1):
            self.ui.textEdit.append("DESCENDING START")
            QtCore.QTimer.singleShot(2000, self.ui.textEdit.append("PARACHUTE DEPLOYMENT"))
        



class serialThreadClass(QtCore.QThread):
    message = QtCore.pyqtSignal(str)

    def __init__(self, parent = None):
        super(serialThreadClass,self).__init__(parent)
        self.serialPort = serial.Serial()
        self.stopflag = False 
    
    def stop(self):
        self.stopflag = True

    def run(self):
        while True:
            if (self.stopflag):
                self.stopflag = False
                break
            elif(self.serialPort.isOpen()):
                try:
                    self.data = self.serialPort.readline()
                except:
                    print("Hata\n")
                self.message.emit(str(self.data.decode()))



def app():
    app = QtWidgets.QApplication(sys.argv)
    win = myApp()
    win.show()
    sys.exit(app.exec())

app()
