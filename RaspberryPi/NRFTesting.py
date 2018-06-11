import serial
import datetime

StartTime = datetime.datetime.now()															# 시작 시간
StopTime = datetime.datetime.now()															# 끝 시간
SpendTime = 0																				# 소요시간

ser = serial.Serial("COM3", 9600)															# 시리얼 포트 <= 라즈베리파이 포팅시 리눅스 포트방식으로 전환

nowTime = ""																				# 스테이션으로 전송 할 시간 문자열

if ser.isOpen():																			# 시리얼 포트를 열고
	interval = int(input())																	# 측정 할 인터벌을 입력받는다		

	while 1:																				# 인터벌 설정 루프
		ser.write(b"START : " + bytearray('{:04d}'.format(interval), 'ascii') + b"\r\n")	# 스테이션으로 명령어 START : [Interval] 전송
		
		if ser.in_waiting:																	# 응답이 있을 때 까지 기다림
			result = ser.readline().decode('ascii')											# 받은 응답을 문자열로 전환
			print(result)
			
			if not(result.find('OK')):														# 응답으로 OK가 오면 ( 첫번째 문자로 OK가 오면 .find()는 0 리턴)
				break																		# 루프 탈출

	while 1:																				# 시간 세팅 루프
		s = datetime.datetime.now()															# 현재시간 측정
		nowTime = s.strftime('%Y-%m-%d %H:%M:%S.%f')										# 스테이션으로 전송할 시간을 문자열로 전환
		ser.write(bytearray(nowTime, 'ascii'))												# 문자열을 스테이션으로 전환

		if ser.in_waiting:																	# 응답이 있을 때 까지 기다림
			result = ser.readline().decode('ascii')											# 받은 응답을 문자열로 전환
			print(result)																	
			if not(result.find('OK')):														# 응답으로 OK가 오면 ( 첫번째 문자로 OK가 오면 .find()는 0 리턴)
				result = result[5:]															# 스테이션에서 저장한 시간부분만 나누기
				StartTime = datetime.datetime.strptime(result,'%Y-%m-%d %H:%M:%S.%f')		# 시작 시간 저장
				break																		# 루프 탈툴
	
	while 1:
		if ser.in_waiting:
			result = ser.readline().decode('ascii')
			print(result)
			if not(result.find('STOP')):
				StopTime = datetime.datetime.now()
				SpendTime = StopTime - StartTime
				print(SpendTime)


ser.close()
