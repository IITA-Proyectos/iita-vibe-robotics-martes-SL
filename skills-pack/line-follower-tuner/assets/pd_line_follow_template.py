from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor
from pybricks.parameters import Port, Direction
from pybricks.robotics import DriveBase
from pybricks.tools import wait

hub = PrimeHub()
left_motor = Motor(Port.C, Direction.COUNTERCLOCKWISE)
right_motor = Motor(Port.E, Direction.CLOCKWISE)
color = ColorSensor(Port.F)
drive = DriveBase(left_motor, right_motor, 56.0, 112.0)

BLACK = 10
WHITE = 84
THRESHOLD = (BLACK + WHITE) / 2
BASE_SPEED = 220
KP = 2.4
KD = 7.0
EDGE_SIGN = 1  # use -1 if tracking the opposite edge


def follow_line_for_ms(duration_ms):
    elapsed = 0
    last_error = 0
    while elapsed < duration_ms:
        reflection = color.reflection()
        error = EDGE_SIGN * (THRESHOLD - reflection)
        derivative = error - last_error
        turn_rate = KP * error + KD * derivative
        drive.drive(BASE_SPEED, turn_rate)
        last_error = error
        wait(10)
        elapsed += 10
    drive.stop()


follow_line_for_ms(3000)
