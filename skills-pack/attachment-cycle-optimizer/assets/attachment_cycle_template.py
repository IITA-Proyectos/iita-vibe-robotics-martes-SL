from pybricks.pupdevices import Motor
from pybricks.parameters import Port, Direction, Stop

attachment = Motor(Port.A, Direction.CLOCKWISE)

HOME_ANGLE = 0
ACQUIRE_ANGLE = 35
DELIVER_ANGLE = 120
RESET_SPEED = 400
WORK_SPEED = 700


def home_attachment():
    attachment.run_target(RESET_SPEED, HOME_ANGLE, then=Stop.HOLD, wait=True)


def acquire():
    attachment.run_target(WORK_SPEED, ACQUIRE_ANGLE, then=Stop.HOLD, wait=True)


def deliver():
    attachment.run_target(WORK_SPEED, DELIVER_ANGLE, then=Stop.HOLD, wait=True)


def reset_cycle():
    home_attachment()
