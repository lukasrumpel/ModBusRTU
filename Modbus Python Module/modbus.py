import serial;
from enum import Enum

class CfuncCode(Enum):
    readAnalogHold = 0x03;
    writeAnalogHold = 0x06;
    changeModbusAdr = 0x69;

class CmodbusConnecton:

    def __init__(self, serialPort:serial, slaveAdr) -> None:
        self.serialPort = serialPort;
        self.serialPort.Timeout = 0.3;
        self.slaveAdr = slaveAdr;
        self.respone = [];
        pass

    def calcCRC(self,data):
        crc = 0xFFFF;
        for i in range(len(data)):
            crc ^= data[i];
            for b in range(8):
                if (crc & 1) != 0:
                    crc = (crc>>1) ^ 0xA001;
                else:
                    crc >>= 1;
        return crc;
    
    def sendRequest(self, funcCode:CfuncCode, register, payload, loraFlag=False, loraPayload="")->None:
        buffer = [];
        if(loraFlag):
            registerLB = register & 0x00FF;
            registerHB = register>>8;
            payloadLB = payload & 0x00FF;
            payloadHB = payload >> 8;
            command = [self.slaveAdr, funcCode.value, registerHB, registerLB, payloadHB, payloadLB];
            crcVal = self.calcCRC(command);
            crcLB = crcVal & 0x00FF;
            crcHB = (crcVal>>8);
            command = [self.slaveAdr, funcCode.value, registerHB, registerLB, payloadHB, payloadLB, crcHB, crcLB];
            command = bytearray(command);
            self.serialPort.write(command);
            buff = self.serialPort.readline();
            buff = buff[1:len(buff)-1];
            for i in buff:
                self.respone.append(int(str(i), base=10)); 
            return;
        else:
            buff = loraPayload;
            buff = buff[1:len(buff)-1];
            for i in loraPayload:
                self.respone.append(int(str(i), base=10)); 
            return;
    
    def getAnalogHoldRegister(self, registerAdr=0, numOfRegs=1)->list:
        if numOfRegs != 0:
            
            self.respone.clear();
            self.sendRequest(funcCode=CfuncCode.readAnalogHold, register=registerAdr, payload=numOfRegs);
            regValsRaw = self.respone[3:len(self.respone)-2];
            regVals = [0 for i in range(int(len(regValsRaw)/2))];
            for i in range(int(len(regValsRaw)/2)):
                regVals[i] = regValsRaw[i*2]<<8;
                regVals[i] |= regValsRaw[i*2+1];
            return regVals;
        return [];

    def writeAnalogHoldRegister(self, registerAdr=0, value2Write=0)->None:
        self.sendRequest(funcCode=CfuncCode.writeAnalogHold, register=registerAdr, payload=value2Write);
        return;

    def setSlaveAdr(self, newSlaveAdr)->None:
        if ((newSlaveAdr >= 0) and (newSlaveAdr <= 255)):
            self.sendRequest(funcCode=CfuncCode.changeModbusAdr, register=0, payload=newSlaveAdr);
            self.slaveAdr = newSlaveAdr;
        else:
            return;
        return;
