import serial
import datetime
import sys
from PyQt5 import QtWidgets
from PyQt5 import uic
from PyQt5 import QtGui
from PyQt5 import QtCore
from PyQt5.QtCore import pyqtSlot
from PyQt5.QtCore import pyqtSignal
import threading

# StartTime = datetime.datetime.now()															# 시작 시간
# StopTime = datetime.datetime.now()															# 끝 시간
# SpendTime = 0																				# 소요시간

# ser = serial.Serial("COM3", 9600)															# 시리얼 포트 <= 라즈베리파이 포팅시 리눅스 포트방식으로 전환

# nowTime = ""																				# 스테이션으로 전송 할 시간 문자열


class Form(QtWidgets.QMainWindow):
		upt_sig = pyqtSignal(str,name = "update")
		def __init__(self,parent=None):
				QtWidgets.QMainWindow.__init__(self, parent)
				self.ui = uic.loadUi("TestTool.ui", self)
				self.ui.show()
				self.upt_sig.connect(w.update_PlainText)
		def update_PlainText(self,value):
				self.ui.plainTextEdit.appendPlainText(value + "\n")

class WindowTh(threading.Thread):
		app = QtWidgets.QApplication(sys.argv)
		w = Form()
		def __init__(self):
				threading.Thread.__init__(self)
		def run(self):
				sys.exit(self.app.exec())

Thr = WindowTh()
Thr.start()
# Thr.w.upt_sig.emit("wow")

# if ser.isOpen():																			# 시리얼 포트를 열고
# 	interval = int(input())																	# 측정 할 인터벌을 입력받는다		

# 	while 1:																				# 인터벌 설정 루프
# 		ser.write(b"START : " + bytearray('{:04d}'.format(interval), 'ascii') + b"\r\n")	# 스테이션으로 명령어 START : [Interval] 전송
		
# 		if ser.in_waiting:																	# 응답이 있을 때 까지 기다림
# 			result = ser.readline().decode('ascii')											# 받은 응답을 문자열로 전환
# 			print(result)
			
# 			if not(result.find('OK')):														# 응답으로 OK가 오면 ( 첫번째 문자로 OK가 오면 .find()는 0 리턴)
# 				break																		# 루프 탈출

# 	while 1:																				# 시간 세팅 루프
# 		s = datetime.datetime.now()															# 현재시간 측정
# 		nowTime = s.strftime('%Y-%m-%d %H:%M:%S.%f')										# 스테이션으로 전송할 시간을 문자열로 전환
# 		ser.write(bytearray(nowTime, 'ascii'))												# 문자열을 스테이션으로 전환

# 		if ser.in_waiting:																	# 응답이 있을 때 까지 기다림
# 			result = ser.readline().decode('ascii')											# 받은 응답을 문자열로 전환
# 			print(result)																	
# 			if not(result.find('OK')):														# 응답으로 OK가 오면 ( 첫번째 문자로 OK가 오면 .find()는 0 리턴)
# 				result = result[5:]															# 스테이션에서 저장한 시간부분만 나누기
# 				StartTime = datetime.datetime.strptime(result,'%Y-%m-%d %H:%M:%S.%f')		# 시작 시간 저장
# 				break																		# 루프 탈툴
	
# 	while 1:																				# 데이터 수신 루프 
# 		if ser.in_waiting:																	# 수신 데이터가 있을 때 까지 기다림 
# 			result = ser.readline().decode('ascii')											# 수신 데이터를 문자열로 전환
# 			print(result)																	# 수신 데이터를 출력
# 			if not(result.find('STOP')):													# STOP이란 응답을 받으면 
# 				StopTime = datetime.datetime.now()											# 멈춘 시간을 저장
# 				SpendTime = StopTime - StartTime											# 소요된 시간을 구하고 저장
# 				print(SpendTime)															# 소요된 시간 출력


# ser.close()
