from spike import PrimeHub, LightMatrix, Button, StatusLight, ForceSensor, MotionSensor, Speaker, ColorSensor, App, DistanceSensor, Motor
from spike.control import wait_for_seconds, wait_until, Timer
from spike import MotorPair 
from utime import sleep_ms, ticks_ms

import hub 
import time
import math
import random
import sys, os


Primehub = PrimeHub()

timer = Timer()
contador = 1
# engine
right_engine= Motor('A')
left_engine= Motor('C')
motor_pair = MotorPair('C', 'A')
motor_pair.set_motor_rotation(3.2 * math.pi,'cm')

# color sensor
left_color = ColorSensor("F")
forward_color = ColorSensor("D")
right_color = ColorSensor("B")

#luces y sonido
proof= False
#valores
basic_blak_color=26
side_black_color=29
black_color_front=20
obstacule_distace=40
double_blck=32
recue_distance=180
global s


########################################################################################################################################################################################
#################################################################################### Husky Lens ########################################################################################
########################################################################################################################################################################################


commandHeaderAndAddress = "55AA11"

algorithmsByteID = {
    "ALGORITHM_OBJECT_TRACKING": "0100",
    "ALGORITHM_FACE_RECOGNITION": "0000",
    "ALGORITHM_OBJECT_RECOGNITION": "0200",
    "ALGORITHM_LINE_TRACKING": "0300",
    "ALGORITHM_COLOR_RECOGNITION": "0400",
    "ALGORITHM_TAG_RECOGNITION": "0500",
    "ALGORITHM_OBJECT_CLASSIFICATION": "0600",
    "ALGORITHM_QR_CODE_RECOGNITION" : "0700",
    "ALGORITHM_BARCODE_RECOGNITION":"0800",
}


class Arrow:
    def __init__(self, xTail, yTail , xHead , yHead, ID):
        self.xTail=xTail
        self.yTail=yTail
        self.xHead=xHead
        self.yHead=yHead
        self.ID=ID
        self.learned= True if ID > 0 else False
        self.type="ARROW"


class Block:
    def __init__(self, x, y , width , height, ID):
        self.x = x
        self.y=y
        self.width=width
        self.height=height
        self.ID=ID
        self.learned= True if ID > 0 else False
        self.type="BLOCK"

class HuskyLensCamera:
    def __init__(self, port = 0, baudrate = 9600, debug=True):
        self.checkOnceAgain=True
        self.DEBUG=debug
        self.unprocessed_data=b''
        self.baudrate = baudrate
        self.reads_per_ms = 10

        if type(port) == str:
            self.uart = eval("primeHub.port."+port)
        else:
            self.uart = port
        self.uart.mode(1)
        sleep_ms(300)
        self.uart.baud(baudrate)
        sleep_ms(200)
        self.lastCmdSent = ""
        self.flush()
        self.knock()

    def force_read(self, size=1, timeout=50):
        data = b''
        r = self.uart.read(1)
        for i in range(timeout*self.reads_per_ms):
            if r==None:
                r=b''
            data += r
            if len(data) == size:
                return data
            else:
                r=self.uart.read(1)
            #if i > 3 and self.DEBUG:
            #    print("Waiting for data in force read...")
        return data

    def available(self):
        self.unprocessed_data=self.force_read(1, timeout=1)
        if self.unprocessed_data==None:
            self.unprocessed_data=b''
        return len(self.unprocessed_data)

    def read_all(self):
        # Read full receive buffer
        available = self.available()
        data = self.unprocessed_data
        self.unprocessed_data = b''
        while True:
            r=self.uart.read(1)
            if r==b'': break
            data += r
        return data

    def read(self, n = 1):
        if n==0:
            return b''
        data = self.force_read(n, timeout=2)
        if data==None:
            data=b''
        return data

    def flush(self):
        _ = self.read_all()
        if self.DEBUG: print("Flushed: %r" % _)

    def writeToHuskyLens(self, msg):
        window = 32
        if self.DEBUG: print("Sending %r" % msg)
        while len(msg) > window:
            self.uart.write(msg[:window])
            sleep_ms(5)
            msg = msg[window:]
        self.uart.write(msg)
        #self.flush()

    def calculateChecksum(self, hexStr):
        total = 0
        for i in range(0, len(hexStr), 2):
            total += int(hexStr[i:i+2], 16)
        hexStr = hex(total)[-2:]
        return hexStr

    def convert_to_class_object(self,data,isBlock):
        tmp=[]
        for i in data:
            if(isBlock):
                obj = Block(i[0],i[1],i[2],i[3],i[4])
            else:
                obj = Arrow(i[0],i[1],i[2],i[3],i[4])
            tmp.append(obj)
        return tmp

    def knock(self):
        cmd = self.cmdToBytes(commandHeaderAndAddress+"002c3c")
        self.writeToHuskyLens(cmd)
        sleep_ms(5)
        return self.processReturnData()

    def blocks(self):
        cmd = self.cmdToBytes(commandHeaderAndAddress+"002131")
        self.writeToHuskyLens(cmd)
        return self.processReturnData()

    def algorithm(self, alg):
        if alg in algorithmsByteID:
            cmd = commandHeaderAndAddress+"022d"+algorithmsByteID[alg]
            cmd += self.calculateChecksum(cmd)
            cmd = self.cmdToBytes(cmd)
            self.writeToHuskyLens(cmd)
            sleep_ms(200)
            return self.processReturnData()
        else:
            print("INCORRECT ALGORITHM NAME")

    def learn(self,x):
        data = "{:04x}".format(x)
        part1=data[2:]
        part2=data[0:2]
        #reverse to correct endiness
        data=part1+part2
        dataLen = "{:02x}".format(len(data)//2)
        cmd = commandHeaderAndAddress+dataLen+"36"+data
        cmd += self.calculateChecksum(cmd)
        cmd = self.cmdToBytes(cmd)
        self.writeToHuskyLens(cmd)
        return self.processReturnData()

    def forget(self):
        cmd = self.cmdToBytes(commandHeaderAndAddress+"003747")
        self.writeToHuskyLens(cmd)
        return self.processReturnData()

    def setCustomName(self,name,idV):
        nameDataSize = "{:02x}".format(len(name)+1)
        #name = name.encode("utf-8").hex()+"00"
        name = ''.join(self.bytesToHex(name.encode('utf-8')))+'00'
        localId = "{:02x}".format(idV)
        data = localId+nameDataSize+name
        dataLen = "{:02x}".format(len(data)//2)
        cmd = commandHeaderAndAddress+dataLen+"2f"+data
        cmd += self.calculateChecksum(cmd)
        cmd = self.cmdToBytes(cmd)
        self.writeToHuskyLens(cmd)
        return self.processReturnData()

    def customText(self,nameV,xV,yV):
        #name=nameV.encode("utf-8").hex()
        name = ''.join(self.bytesToHex(nameV.encode('utf-8')))
        nameDataSize = "{:02x}".format(len(name)//2)
        if(xV>255):
            x="ff"+"{:02x}".format(xV%255)
        else:
            x="00"+"{:02x}".format(xV)
        y="{:02x}".format(yV)

        data = nameDataSize+x+y+name
        dataLen = "{:02x}".format(len(data)//2)

        cmd = commandHeaderAndAddress+dataLen+"34"+data
        cmd += self.calculateChecksum(cmd)
        cmd = self.cmdToBytes(cmd)
        self.writeToHuskyLens(cmd)
        return self.processReturnData()

    def clearText(self):
        cmd = self.cmdToBytes(commandHeaderAndAddress+"003545")
        self.writeToHuskyLens(cmd)
        return self.processReturnData()

    def requestAll(self):
        cmd = self.cmdToBytes(commandHeaderAndAddress+"002030")
        self.writeToHuskyLens(cmd)
        return self.processReturnData()

    def saveModelToSDCard(self,idVal):
        idVal = "{:04x}".format(idVal)
        idVal = idVal[2:]+idVal[0:2]
        cmd = commandHeaderAndAddress+"0232"+idVal
        cmd += self.calculateChecksum(cmd)
        cmd = self.cmdToBytes(cmd)
        self.writeToHuskyLens(cmd)
        return self.processReturnData()

    def loadModelFromSDCard(self,idVal):
        idVal = "{:04x}".format(idVal)
        idVal = idVal[2:]+idVal[0:2]
        cmd = commandHeaderAndAddress+"0233"+idVal
        cmd += self.calculateChecksum(cmd)
        cmd = self.cmdToBytes(cmd)
        self.writeToHuskyLens(cmd)
        return self.processReturnData()

    def savePictureToSDCard(self):
        self.uart.timeout=5
        cmd = self.cmdToBytes(commandHeaderAndAddress+"003040")
        self.writeToHuskyLens(cmd)
        return self.processReturnData()

    def saveScreenshotToSDCard(selxf):
        cmd = self.cmdToBytes(commandHeaderAndAddress+"003949")
        self.writeToHuskyLens(cmd)
        return self.processReturnData()

    def arrows(self):
        cmd = self.cmdToBytes(commandHeaderAndAddress+"002232")
        self.writeToHuskyLens(cmd)
        return self.processReturnData()[0]

    def learned(self):
        cmd = self.cmdToBytes(commandHeaderAndAddress+"002333")
        self.writeToHuskyLens(cmd)
        return self.processReturnData()[0]

    def learnedBlocks(self):
        cmd = self.cmdToBytes(commandHeaderAndAddress+"002434")
        self.writeToHuskyLens(cmd)
        return self.processReturnData()[0]

    def learnedArrows(self):
        cmd = self.cmdToBytes(commandHeaderAndAddress+"002535")
        self.writeToHuskyLens(cmd)
        return self.processReturnData()[0]

    def getObjectByID(self, idVal):
        idVal = "{:04x}".format(idVal)
        idVal = idVal[2:]+idVal[0:2]
        cmd = commandHeaderAndAddress+"0226"+idVal
        cmd += self.calculateChecksum(cmd)
        cmd = self.cmdToBytes(cmd)
        self.writeToHuskyLens(cmd)
        return self.processReturnData()[0]

    def getBlocksByID(self, idVal):
        idVal = "{:04x}".format(idVal)
        idVal = idVal[2:]+idVal[0:2]
        cmd = commandHeaderAndAddress+"0227"+idVal
        cmd += self.calculateChecksum(cmd)
        cmd = self.cmdToBytes(cmd)
        self.writeToHuskyLens(cmd)
        return self.processReturnData()

    def getArrowsByID(self, idVal):
        idVal = "{:04x}".format(idVal)
        idVal = idVal[2:]+idVal[0:2]
        cmd = commandHeaderAndAddress+"0228"+idVal
        cmd += self.calculateChecksum(cmd)
        cmd = self.cmdToBytes(cmd)
        self.writeToHuskyLens(cmd)
        return self.processReturnData()[0]

    def count(self):
        cmd = self.cmdToBytes(commandHeaderAndAddress+"002030")
        self.writeToHuskyLens(cmd)
        return len(self.processReturnData())

    def learnedObjCount(self):
        cmd = self.cmdToBytes(commandHeaderAndAddress+"002030")
        self.writeToHuskyLens(cmd)
        return self.processReturnData(numIdLearnFlag=True)[-1]

    def frameNumber(self):
        cmd = self.cmdToBytes(commandHeaderAndAddress+"002030")
        self.writeToHuskyLens(cmd)
        return self.processReturnData(frameFlag=True)[-1]


    def cmdToBytes(self, cmd):
        # https://stackoverflow.com/questions/47727170/difference-between-bytes-fromhex-and-encode
        #return bytes.fromhex(cmd)
        return bytes(int(cmd[i:i + 2], 16) for i in range(0, len(cmd), 2))

    def bytesToHex(self, buf):
        return list('{0:02x}'.format(int(buf[i]), 16) for i in range(0, len(buf)))

    def splitCommandToParts(self, cmd):
        #print("cmd: %s" % cmd)
        headers = cmd[0]+cmd[1]
        address = cmd[2]
        data_length = int(cmd[3], 16) # decode base 16 int
        command = cmd[4]
        if(data_length > 0):
            data = ''.join(cmd[5:5+data_length])
        else:
            data= []
        checkSum = cmd[-1] # bug

        return [headers, address, data_length, command, data, checkSum]

    def getBlockOrArrowCommand(self):
        byteString = self.force_read(5)
        byteString += self.force_read(byteString[3])
        byteString += self.uart.read(1)
        byteStringHex = self.bytesToHex(byteString)
        if self.DEBUG: print ("get block returned: %r" % byteStringHex)
        commandSplit = self.splitCommandToParts(byteStringHex)
        isBlock = True if commandSplit[3] == "2a" else False
        return (commandSplit[4],isBlock)

    def processReturnData(self, numIdLearnFlag=False, frameFlag=False):
        byteString = b''
        try:
            byteString = self.force_read(5)
            byteString += self.force_read(byteString[3])
            byteString += self.force_read(1)

            if self.DEBUG: print ("Returned: %r" % byteString)
            byteStringHex = self.bytesToHex(byteString)
            if self.DEBUG: print ("as hex list: %r" % byteStringHex)
            commandSplit = self.splitCommandToParts(byteStringHex)
            if self.DEBUG: print("command split: %r" % commandSplit)
            chk = self.calculateChecksum(''.join(byteStringHex[:-1]))
            if self.DEBUG: print("calc checksum:", chk)
            if (chk != commandSplit[-1]):
                print("checksum error")
                return []

            cmdType = commandSplit[3]
            if(cmdType == "2e"):
                # self.checkOnceAgain=True
                if self.DEBUG: print("COMMAND_RETURN_OK")
                self.flush()
                return "OK"
            else:
                if (cmdType == "29") and self.DEBUG:
                    print ("COMMAND_RETURN_INFO")
                returnData = []
                numberOfBlocksOrArrow = int( commandSplit[4][2:4]+commandSplit[4][0:2], 16 )
                numberOfIDLearned = int( commandSplit[4][6:8]+commandSplit[4][4:6], 16 )
                frameNumber = int( commandSplit[4][10:12]+commandSplit[4][8:10], 16 )
                if self.DEBUG:print("qty:",numberOfBlocksOrArrow)
                if self.DEBUG:print("nID:", numberOfIDLearned)
                if self.DEBUG: print("frm:", frameNumber)

                isBlock=True
                for i in range(numberOfBlocksOrArrow):
                    #if self.DEBUG: print("getting block ", i)
                    tmpObj=self.getBlockOrArrowCommand()
                    isBlock=tmpObj[1]
                    returnData.append(tmpObj[0])

                finalData = []
                tmp = []
                # print(returnData)
                for i in returnData:
                    tmp = []
                    for q in range(0, len(i), 4):
                        low=int(i[q:q+2], 16)
                        high=int(i[q+2:q+4], 16)
                        if(high>0):
                            val=low+255+high
                        else:
                            val=low
                        tmp.append(val)
                    finalData.append(tmp)
                    tmp = []
                self.checkOnceAgain=True
                ret=self.convert_to_class_object(finalData,isBlock)
                if(numIdLearnFlag):
                    ret.append(numberOfIDLearned)
                if(frameFlag):
                    ret.append(frameNumber)
                return ret

        except Exception as exc:
            print("EXCEPTION : "+str(exc))
            if(self.checkOnceAgain):
                #self.uart.timeout=5
                self.checkOnceAgain=False
                #self.uart.timeout=.5
                return self.processReturnData()
            print("Read response error, please try again")
            self.flush()
            return []

######################################################################################################################################################################################
#################################################################### Definiciones Robot ##############################################################################################
######################################################################################################################################################################################


def update():
    global color_l
    global color_r
    color_l = left_color.get_color()
    color_r = right_color.get_color()
    global light_l
    global ligth_f
    global ligth_r
    light_l = left_color.get_reflected_light()
    ligth_f = forward_color.get_reflected_light()
    ligth_r = right_color.get_reflected_light()

def doubleblack():
    if right_color.get_reflected_light() < double_blck and left_color.get_reflected_light() < double_blck :
        motor_pair.stop()
        if proof == True:
            Primehub.light_matrix.write('N')
            wait_for_seconds(0.5)
            Primehub.light_matrix.off()
        motor_pair.move_tank(3,'degrees',left_speed=60, right_speed=60)
        if right_color.get_reflected_light() < double_blck and left_color.get_reflected_light() < double_blck:
            if proof == True :
                Primehub.light_matrix.write('n')
                wait_for_seconds(0.5)
                Primehub.light_matrix.off()
            motor_pair.move_tank(100,'degrees',left_speed=60, right_speed=60)
        else:
            motor_pair.move_tank(-16,'degrees',left_speed=60, right_speed=60)
            
def green():
    if right_color.get_color() =='green':
        motor_pair.stop()
        right_engine.run_for_degrees(4)
        if left_color.get_color()== 'green':
            if proof == True :
                PrimeHub.light_matrix.show_image('ARROW_N')
                wait_for_seconds(0.5)
                PrimeHub.light_matrix.off()
            turn_180_rigth()
        else:
            if proof == True :
                Primehub.light_matrix.show_image('ARROW_W')
                wait_for_seconds(0.5)
                Primehub.light_matrix.off()
            motor_pair.move_tank(180,'degrees',left_speed=60, right_speed=60)
            turn_90_right()
            motor_pair.move_tank(80,'degrees',left_speed=60, right_speed=60)
            look_rigth_line()
    if left_color.get_color() =='green':
        motor_pair.stop()
        left_engine.run_for_degrees(3)
        if right_color.get_color() == 'green':
            if proof == True :
                Primehub.light_matrix.show_image('ARROW_S')
                wait_for_seconds(0.5)
                Primehub.light_matrix.off()
            turn_180_rigth()
        else:
            if proof == True :
                Primehub.light_matrix.show_image('ARROW_E')
                wait_for_seconds(0.5)
                Primehub.light_matrix.off()
            motor_pair.move_tank(180,'degrees',left_speed=60, right_speed=60)
            turn_90_left()
            motor_pair.move_tank(80,'degrees',left_speed=60, right_speed=60)
            look_left_line()
def right_intersection():
    if right_color.get_reflected_light() < side_black_color and forward_color.get_reflected_light() < black_color_front:
        motor_pair.start_tank(0,0)
        if proof == True :
            Primehub.light_matrix.write('Td')
            wait_for_seconds(1)
            Primehub.light_matrix.off()
        motor_pair.move_tank(3,'degrees',left_speed=30, right_speed=30)
        if right_color.get_reflected_light() < side_black_color and forward_color.get_reflected_light() < black_color_front:
            if proof == True :
                if proof == True :
                    Primehub.light_matrix.show_image('YES')
                    wait_for_seconds(1)
                    Primehub.light_matrix.off()
            motor_pair.move_tank(100,'degrees',left_speed=60, right_speed=60)
        else:
            if proof == True :
                Primehub.light_matrix.write('no')
                wait_for_seconds(1)
                Primehub.light_matrix.off()
            motor_pair.move_tank(-15,'degrees',left_speed=60, right_speed=60)
def left_intersection():
    if left_color.get_reflected_light() < 32 and forward_color.get_reflected_light() < 32:
        motor_pair.start_tank(0,0)
        if proof == True :
            Primehub.light_matrix.write('Ti')
            wait_for_seconds(0.5)
            Primehub.light_matrix.off()
        motor_pair.move_tank(3,'degrees',left_speed=30, right_speed=30)
        if left_color.get_reflected_light() < 32 and forward_color.get_reflected_light() < 32:
            if proof == True :
                Primehub.light_matrix.show_image('YES')
                wait_for_seconds(1)
                Primehub.light_matrix.off()
            motor_pair.move_tank(100,'degrees',left_speed=60, right_speed=60)
        else:
            if proof == True :
                Primehub.light_matrix.write('no')
                wait_for_seconds(1)
                Primehub.light_matrix.off()
            motor_pair.move_tank(-15,'degrees',left_speed=60, right_speed=60)
def look_rigth_line():
    motor_pair.stop()
    wait_for_seconds(1)
    Primehub.motion_sensor.reset_yaw_angle()
    if proof == True :
        Primehub.light_matrix.write('Cde')
    motor_pair.start_tank(-30,30)
    while (Primehub.motion_sensor.get_yaw_angle() < -10):
        print(Primehub.motion_sensor.get_yaw_angle())
    motor_pair.stop()
    motor_pair.start_tank(30,-30)
    while(right_color.get_reflected_light() > 30):
        print('sensor derecho')
    motor_pair.stop()
    angle = Primehub.motion_sensor.get_yaw_angle()

def look_left_line():
    motor_pair.stop()
    wait_for_seconds(1)
    Primehub.motion_sensor.reset_yaw_angle()
    if proof == True :
        Primehub.light_matrix.write('Ciz')
    motor_pair.start_tank(30,-30)
    while (Primehub.motion_sensor.get_yaw_angle() <10):
        print(Primehub.motion_sensor.get_yaw_angle())
    motor_pair.stop()
    motor_pair.start_tank(-30,30)
    while(left_color.get_reflected_light() > 30):
        print('sensor izquierdo')
    motor_pair.stop()
    angle = Primehub.motion_sensor.get_yaw_angle()

def turn_90_left():
    motor_pair.start_tank(0,0)
    wait_for_seconds(1)
    Primehub.motion_sensor.reset_yaw_angle()
    motor_pair.start_tank(30,-30)
    while (Primehub.motion_sensor.get_yaw_angle() <85):
        print(Primehub.motion_sensor.get_yaw_angle())
    motor_pair.start_tank(0,0)
    if proof == True :
        Primehub.speaker.beep(60, 1)
    #wait_for_seconds(1)
    angle = Primehub.motion_sensor.get_yaw_angle()
    # print('Angulo:', angle)

def turn_90_right():
    motor_pair.start_tank(0,0)
    wait_for_seconds(1)
    Primehub.motion_sensor.reset_yaw_angle()
    motor_pair.start_tank(-30,30)
    while (Primehub.motion_sensor.get_yaw_angle() >-85):
        print(Primehub.motion_sensor.get_yaw_angle())
    motor_pair.start_tank(0,0)
    if proof == True :
        Primehub.speaker.beep(60, 1)
    # wait_for_seconds(1)
    angle = Primehub.motion_sensor.get_yaw_angle()
    # print('Angulo:', angle)

def turn_180_rigth():
    turn_90_left()
    turn_90_left()
    motor_pair.move_tank(0.7,'cm',30,-30)

def turn_180_left():
    turn_90_right()
    turn_90_right()
    motor_pair.move_tank(0.7,'cm',-30,30)

def turn_90_rigth_rescue(): #90 degree turns but for the rescue zone
    motor_pair.start_tank(0,0)
    wait_for_seconds(1)
    Primehub.motion_sensor.reset_yaw_angle()
    motor_pair.start_tank(40,-20)
    while (Primehub.motion_sensor.get_yaw_angle() < 83):
        print(Primehub.motion_sensor.get_yaw_angle())
    motor_pair.start_tank(0,0)
    # wait_for_seconds(1)
    angle = Primehub.motion_sensor.get_yaw_angle()
    # print('Angulo:', angle)


def Aproximity():
    ball = huskyLens.getBlocksByID(1)

    if len(ball) == 1:
        FollowVictim()
        motor_pair.stop()
        Primehub.light_matrix.write('!')
        wait_for_seconds(0.5)
        Primehub.light_matrix.off()
        wait_for_seconds(1)

def cuadrado():
    if forward_color.get_green() > 630 and forward_color.get_blue() > 700 and forward_color.get_red() > 620:
        motor_pair.stop()
        wait_for_seconds(2)
        print("Zona de evacuacion.")
        #motor_pair.move_tank(3, 'seconds', left_speed=30, right_speed=30)
        motor_pair.start_tank(25,25)
        start = time.ticks_ms()
        while time.ticks_diff(time.ticks_ms(), start) <6000:
            Aproximity()
        turn_90_rigth_rescue()
        motor_pair.start_tank(25,25)
        start = time.ticks_ms()
        while time.ticks_diff(time.ticks_ms(), start) <6000:
            Aproximity()
        turn_90_right_rescue()
        motor_pair.start_tank(25,25)
        start = time.ticks_ms()
        while time.ticks_diff(time.ticks_ms(), start) <5000:
            Aproximity()
        turn_90_right_rescue()
        motor_pair.start_tank(25,25)
        start = time.ticks_ms()
        while time.ticks_diff(time.ticks_ms(), start) <6000:
            Aproximity()
        turn_90_right_rescue()



def FollowVictim():
    while True:
        vivo = huskyLens.getBlocksByID(1)


        if len(vivo) == 1:
            display.show_image("ANGRY")

            x_position = vivo[0].x
            h_height = vivo[0].height

            print("X: ", x_position, " H: ", h_height)

            if x_position < 150:# Si el objeto está a la izquierda del eje X

                motor_pair.move_tank(0.5, "cm", left_speed=0, right_speed=15)

            elif x_position > 180:# Si el objeto está a la derecha del eje X
                # motor_pair.move(0.5, "cm", steering=-100)

                motor_pair.move_tank(0.5, "cm", left_speed=15, right_speed=0)
            elif h_height > 28 and h_height < 30:
                print("X: ", x_position, " H: ", h_height)
                motor_pair.move_tank(3 , "cm", left_speed = 15, right_speed= 15)
                motor_pair.stop()
                Primehub.light_matrix.write('!')
                wait_for_seconds(10)
                Primehub.light_matrix.off()
                wait_for_seconds(5)
            else:
                print("X: ", x_position, " H: ", h_height)
                if x_position > 140 and x_position < 180:
                    if x_position > 170:
                        print("X: ", x_position, " H: ", h_height)
                        motor_pair.start(10, 5)
                    else:
                        if x_position < 150:

                            print("X: ", x_position, " H: ", h_height)
                            motor_pair.start(5, 10)
                        else:
                            motor_pair.start(10, 10)
                    print("X: ", x_position, " H: ", h_height)
        else:
            print("X: ", x_position, " H: ", h_height)
            display.show_image("GHOST")
            motor_pair.move_tank(1, "cm", left_speed=-15, right_speed= -15)
            ## motor_pair.start_tank_at_power(-10, -10)

#########################################################################################################################################################################################
#################################################################################### MAIN CODE ##########################################################################################
#########################################################################################################################################################################################

controlPID = False
tanterior = 0.01
tiempo = 0.01
integral = 0.0
derivada = 0.0
error_previo = 0.0
cuenta = -1


huskyLens = HuskyLensCamera(hub.port.E, baudrate = 9600, debug= False)
display = LightMatrix()

if (huskyLens.knock()=="OK"):
    display.show_image("HAPPY")
else:
    display.show_image("SAD")



while True:
    #global cuenta
    global color_l
    global color_r
    global light_l
    global ligth_f
    global ligth_r
    cuenta = cuenta + 1
    color_l = left_color.get_color()
    color_r = right_color.get_color()
    light_l = left_color.get_reflected_light()
    ligth_f = forward_color.get_reflected_light()
    ligth_r = right_color.get_reflected_light()
    #if cuenta%20 == 0:
    #pidedistancia()

    if controlPID == True:
        velocidad = 10
        kp = 1.2    # 3.95 antes
        ki = 0.0    # 0.02 antes
        kd = 0.15# 0.4antes
        tanterior = tiempo
        tiempo = time.ticks_ms()
        dt = tiempo - tanterior
        error = ligth_r - light_l
        proporcional = error
        integral = integral+ error * 0.005
        derivada = (error - error_previo)
        #    salida = int(kp * proporcional + ki * integral + kd * derivada)
        error_previo = error
        salida = int(kp * proporcional + ki * integral + kd * derivada)
        #print(error, derivada, integral, salida, velocidad + salida, velocidad - salida, dt)
        motor_pair.start_tank(velocidad + salida,velocidad - salida)
    else:
        if left_color.get_reflected_light() > basic_blak_color:
            right_engine.start_at_power(30)
        else:
            right_engine.start_at_power(-50)
        if right_color.get_reflected_light() > basic_blak_color:
            left_engine.start_at_power(-30)
        else:
            left_engine.start_at_power(50)

    if light_l < 30 or ligth_r < 30 or ligth_f < 40 :
        timer.reset()

    if timer.now() > 2.5:
        motor_pair.stop()
        motor_pair.start_tank(-30,-30)
        while (left_color.get_reflected_light() > 30 and right_color.get_reflected_light() > 30 and forward_color.get_reflected_light() > 40):
            print('sin linea')
        motor_pair.stop()
        if left_color.get_reflected_light() < 30:
            right_engine.run_for_degrees(100,-45)
            motor_pair.stop()
        else:
            if right_color.get_reflected_light() < 30:
                left_engine.run_for_degrees(100, 45)# bien
                motor_pair.stop()

    green()
    doubleblack()
    right_intersection()
    left_intersection()
    cuadrado()
