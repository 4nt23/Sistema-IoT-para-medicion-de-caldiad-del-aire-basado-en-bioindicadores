import serial
from time import sleep as sleep
#Constants
DELIMITER_START = b'<Pic>'
DELIMITER_END =  b'</Pic>'
PACKET_SIZE = 32

#Funciones
def send_OK(arg=-1):
	'''Send and OK to notify a packet suscessful reception'''
	arg = arg if type(arg) is type(0) else -1
	if arg >= 0:
		a = b'OK'+("{:05d}".format(arg)).encode("utf-8")+b'\n'
	else:
		a = b'OK\n'
	ser.reset_output_buffer()  
	ser.write(a) #reply
	print(b'Reply: '+a)

def wait_for_command():
	'''Search a command from the list of command in the data received. If any command in the list match with the data receives, then function return empty byte'''
	commands = [b'Img',b'len',b'Rdy',b'Err',b'']
	arg = b''
	cmd = ser.readline()
	for c in commands:
		if cmd.find(c) >= 0:
			arg = cmd[cmd.find(c)+3:cmd.find(c)+8] #get argument from command
			break;
	print(f"Recibido{cmd} --> Comando retornado: {c} : Argumento: {arg}")

	return [c,arg];

def get_pic (img_size):
	PACKET_SIZE =32
	packet_repeats = 0
	data = [b'EMPTY',-1];
	buff1 = b'NADA'
	img_size = int(img_size.decode("utf-8"))
	send_OK()
	total_size = 0
	num_packet = 0
	total_packets = (img_size//PACKET_SIZE) if (img_size%PACKET_SIZE==0) else (img_size//PACKET_SIZE)+1
	last_packet_size = PACKET_SIZE if img_size%PACKET_SIZE == 0 else img_size%PACKET_SIZE
	print(f"Paquetes totales: {total_packets}\n")
	while num_packet < total_packets:

		if num_packet == total_packets-1:
			PACKET_SIZE = last_packet_size

		message_length = PACKET_SIZE + len(DELIMITER_START+DELIMITER_END)

		#buff = ser.read(message_length)
		buff = ser.read_until(DELIMITER_END,100)
		ser.reset_input_buffer()
		packet_OK = (buff.find(DELIMITER_END)>=0)
		print(f"Paquete {num_packet} : {buff}")
		if packet_OK:
			data = get_img_data (buff)
			print(f"paquete esperado: {num_packet}, paquete recibido: {data[1]}")
			buff = data[0]
			print(f"Paquete {num_packet} recibido")
			if buff != b'EMPTY':
				if data[1]==num_packet:
					write_file("Imagen0", buff)
					total_size+=len(buff);
					print(f"bytes en paquete {len(buff)}. Total escritos: {total_size}")
					send_OK(num_packet)
					print(f"Paquete {num_packet} recibido correctamente\n")
					buff1 = buff
					num_packet += 1
					packet_repeats=0
				else:
					print(f"Paquete recibido es igual que el paquete anterior num {num_packet-1}")
					
					if packet_repeats >=5:
						send_OK(num_packet-1)
						print("Reply AGAIN!")
						packet_repeats=0
					else:
						ser.reset_input_buffer()
						sleep(1)
					packet_repeats+=1
					print(f"Paquete recibido es igual que el paquete anterior num {num_packet-1}")
			else:
				print(f"Paquete {num_packet} erróneo, no se encontraron delimitadores\n")

		else:
			print(f"Paquete {num_packet} incompleto ")			
	print(f"Recibidos todos los paquetes: {num_packet} de {total_packets}")

def get_img_data (packet=b''):

	start = packet.find(DELIMITER_START) + len (DELIMITER_START) -1 if packet.find(DELIMITER_START)>=0 else -1
	end = packet.find(DELIMITER_END) - 1  if packet.find(DELIMITER_END)>=0 else -1
	#print(f"Packet_Start in: {start}\nPacket_End in:{end}\n")
	
	data = packet[start+6:end+1]
	num = (packet[start+1:start+6]).decode("utf-8")
	print(f"Packet número {num} recibido: {data}")
	return ([data,int(num)]  if ((start > 0) and (end > 0)) else [b'EMPTY',-1])

def write_file (filename, data):

	with open(filename,"ab") as pic:
		pic.write(data)
	pic.close();

def error (err):
		if err == 1: #usar lista o diccionario
			print("Error en envío de comando")
		elif err == 2:
			print("Cámra no pudo tomar foto.")
		elif err == 3:
			print("Error enviando imagen.")
		else:
			print("Error desconocido")




#Serial takes two parameters: serial device and baudrate
ser = serial.Serial(port='COM10', baudrate=19200, bytesize=8, parity='N', stopbits=1, timeout=10000, xonxoff=0, rtscts=0)
i = 0
count = 0

acciones = {b'Img':send_OK, b'len':get_pic, b'Rdy':send_OK, b'Err': error}
input("Press enter to take picture") #take picture
ser.reset_output_buffer()
ser.write(b'take\n')
#while True:
while True:

	order = wait_for_command()
	print(f"comando {order[0]}, argumento: {order[1]}")
	accion = acciones.get(order[0], lambda x:print(f"accion no contemplada. Argumento: {x}"))
	accion(order[1])	
				








