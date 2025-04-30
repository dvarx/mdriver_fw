import socket
import struct
from bitstring import BitArray

#create TNB_MNS_STRUCT
class tnb_mns_struct:
    def __init__(self,desCurrents_,desDuties_,stpFlags_,enbuckFlags_,enregFlags_,enresFlags_):
        self.desCurrents=[0,0,0,0,0,0]
        self.desDuties=[0,0,0,0,0,0]
        for i in range(0,6):
            #set 16bit current in mA
            self.desCurrents[i]=int(desCurrents_[i]*1000)
            #set 16bit duty
            self.desDuties[i]=int(2**16*desDuties_[i])
        #generate the control flags
        stpFlags_byte=BitArray("0x00")
        enbuckFlags_byte=BitArray("0x00")
        enregFlags_byte=BitArray("0x00")
        enresFlags_byte=BitArray("0x00")
        for i in range(0,6):
            if(stpFlags_[i]):
                stpFlags_byte[i]=True
            if(enregFlags_[i]):
                enregFlags_byte[i]=True
            if(enresFlags_[i]):
                enresFlags_byte[i]=True
            if(enbuckFlags_[i]):
                enbuckFlags_byte[i]=True
        #generate the message structure
        # H : unsigned int16
        # h : signed int 16
        # s : bytes
        #self.msg=struct.pack("h",self.desCurrents[0])
        self.msg=struct.pack("hhhhhhHHHHHHssss",self.desCurrents[0],self.desCurrents[1],self.desCurrents[2], 
                              self.desCurrents[3],self.desCurrents[4],self.desCurrents[5],self.desDuties[0],self.desDuties[1], 
                              self.desDuties[2],self.desDuties[3],self.desDuties[4],self.desDuties[5],
                              stpFlags_byte.tobytes(),enbuckFlags_byte.tobytes(),enregFlags_byte.tobytes(),enresFlags_byte.tobytes())
    def getmsg(self):
        return self.msg

# IP address of ccard is configured as 0xC0A80004 = 192.168.0.4;
# netmask of ccard is configured as 0xFFFFFF00;
# ccard listens on port 30

ip_ccard='192.168.0.4'
ip_rospc='192.168.0.1'
tcp_port_ccard=30
test_data=b'AABBCCDD'
buffer_size=1024

tcpsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    tcpsocket.connect((ip_ccard,tcp_port_ccard))
except TimeoutError:
    print("unable to connect to socket, is connection valid?")
    quit()

#define data to be sent to card
currents=[0,0,0,0,0,0]
duties=[0,0,0,0,0,0]
stpflgs=[False,False,False,False,False,False]
buckflgs=[True,False,False,False,False,False]
regenflags=[False,False,False,False,False,False]
resenflags=[False,False,False,False,False,False]
msg=tnb_mns_struct(currents,duties,stpflgs,buckflgs,regenflags,resenflags)
msg_bytes=msg.getmsg()
print("Data that will be sent to card:")
print(msg_bytes)

while(True):
    #send_data=input("type data to send: ")
    #print("Sending data: %s\n"%(send_data))
    try:
        tcpsocket.send(msg_bytes)
        #tcpsocket.send(send_data.encode())
    except Exception as ex:
        print("Error sending data via TCP: %s"%(str(ex)))
        tcpsocket.close()
        quit()
    try:
        datarcv=tcpsocket.recv(buffer_size)
    except TimeoutError:
        print("timeouterror: did not receive any data back fromcard")
        tcpsocket.close()
        quit()
    print("Received data: %s\n"%(str(datarcv)))