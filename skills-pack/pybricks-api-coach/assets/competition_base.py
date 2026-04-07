from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor
from pybricks.parameters import Port, Direction
from pybricks.robotics import DriveBase

hub = PrimeHub()

LEFT_DRIVE_PORT = Port.C
RIGHT_DRIVE_PORT = Port.E
LEFT_DRIVE_DIR = Direction.COUNTERCLOCKWISE
RIGHT_DRIVE_DIR = Direction.CLOCKWISE

WHEEL_DIAMETER_MM = 56.0
AXLE_TRACK_MM = 112.0

STRAIGHT_SPEED = 400
STRAIGHT_ACCEL = 900
TURN_RATE = 300
TURN_ACCEL = 700

left_motor = Motor(LEFT_DRIVE_PORT, LEFT_DRIVE_DIR)
right_motor = Motor(RIGHT_DRIVE_PORT, RIGHT_DRIVE_DIR)
drive = DriveBase(left_motor, right_motor, WHEEL_DIAMETER_MM, AXLE_TRACK_MM)
drive.settings(STRAIGHT_SPEED, STRAIGHT_ACCEL, TURN_RATE, TURN_ACCEL)


def reset_robot():
    left_motor.reset_angle(0)
    right_motor.reset_angle(0)
    drive.stop()


def leave_base():
    drive.straight(250)
    drive.turn(90)


def main():
    reset_robot()
    leave_base()


main()
