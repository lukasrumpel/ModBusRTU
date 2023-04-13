import serial;
from tkinter import *
from threading import*
import crc

fenster = Tk();
fenster.title("Modbus Testprogramm") 
labelSlaveAdr = Label(master=fenster);
labelFuncCode = Label(master=fenster);
labelNumOfBytes = Label(master=fenster); 
textData = Text(master=fenster, width=30, height=5);
ser = serial.Serial('/dev/ttyUSB0', 19200, timeout=0.3);
MBadr = StringVar();
MBcmd = StringVar();
MBreg1 = StringVar();
MBreg2 = StringVar();
MBnum1 = StringVar();
MBnum2 = StringVar();
MBadr.set("01");
MBcmd.set("03");
MBreg1.set("00");
MBreg2.set("00");
MBnum1.set("00");
MBnum2.set("01");
data = [];

def crc16(data : bytearray, offset , length):
    if data is None or offset < 0 or offset > len(data)- 1 and offset+length > len(data):
        return 0
    crc = 0xFFFF
    for i in range(0, length):
        crc ^= data[offset + i] << 8
        for j in range(0,8):
            if (crc & 0x8000) > 0:
                crc =(crc << 1) ^ 0x1021
            else:
                crc = crc << 1
    return crc & 0xFFFF

def sendCMD():
    labelSlaveAdr.config(text="");
    labelFuncCode.config(text="");
    labelNumOfBytes.config(text="");
    textData.delete("1.0" , "end")
    cmd = [int(MBadr.get(), base=16), int(MBcmd.get(), base=16), int(MBreg1.get(), base=16), int(MBreg2.get(), base=16), int(MBnum1.get(), base=16), int(MBnum2.get(), base=16)];
    crcSum =crc16(cmd, 0, len(cmd));
    crcLB = crcSum & 0xFF;
    crcHB = crcSum>>8 & 0xFF;
    cmd = [int(MBadr.get(), base=16), int(MBcmd.get(), base=16), int(MBreg1.get(), base=16), int(MBreg2.get(), base=16), int(MBnum1.get(), base=16), int(MBnum2.get(), base=16), int(hex(crcHB), base=16), int(hex(crcLB), base=16)];
    cmd = bytearray(cmd)
    cmd = cmd + b"\n";
    ser.write(cmd);
    adrOfSlave = ser.read();
    funcCode = ser.read();
    numOfBytes = int.from_bytes(ser.read(), 'big');
    for i in range(numOfBytes-2):
        data.append(int.from_bytes(ser.read(), 'big'));
    labelSlaveAdr.config(text="Adr:" + str(adrOfSlave));
    labelFuncCode.config(text="Funccode: " + str(funcCode));
    labelNumOfBytes.config(text="Bytes: " + str(numOfBytes));
    data.pop();
    textData.insert(END, data);
    data.clear();
    

def initWindow():
    sendenButt = Button(master=fenster, text="senden", command=sendCMD);
    labelAdr = Label(master=fenster, text="Slave Adresse");
    labelCMD = Label(master=fenster, text="Befehl");
    labelReg1 = Label(master=fenster, text="RegAdr HB");
    labelReg2 = Label(master=fenster, text="RegAdr LB");
    labelNumReg1 = Label(master=fenster, text="NumRegs HB");
    labelNumReg2 = Label(master=fenster, text="NumRegs LB");
    labelRespond = Label(master=fenster, text="Respond:");
    labelRespond.grid(column=0, row=3);
    labelSlaveAdr.grid(column=1, row=3);
    labelFuncCode.grid(column=2, row=3);
    labelNumOfBytes.grid(column=3, row=3);
    textData.grid(column=4, row=3, columnspan=2);
    entAdr = Entry(master=fenster, textvariable=MBadr);
    entCMD = Entry(master=fenster, textvariable=MBcmd);
    entReg1 = Entry(master= fenster, textvariable=MBreg1);
    entReg2 = Entry(master=fenster, textvariable=MBreg2);
    entNum1 = Entry(master=fenster, textvariable=MBnum1);
    entNum2 = Entry(master=fenster, textvariable=MBnum2);
    labelAdr.grid(column=0, row=0);
    entAdr.grid(column=0, row=1);
    labelCMD.grid(column=1, row=0);
    entCMD.grid(column=1, row=1);
    labelReg1.grid(column=2, row=0);
    entReg1.grid(column=2, row= 1);
    labelReg2.grid(column=3, row=0);
    entReg2.grid(column=3, row=1);
    labelNumReg1.grid(column=4, row=0);
    entNum1.grid(column=4, row=1);
    labelNumReg2.grid(column=5, row=0);
    entNum2.grid(column=5, row=1);
    sendenButt.grid(column=5, row=2);




initWindow();
fenster.mainloop();

