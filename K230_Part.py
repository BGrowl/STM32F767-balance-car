import time, os
from media.sensor import *
from media.display import *
from media.media import *
from machine import UART, FPIOA

# Forward first, then find and kick the red ball.
# Wiring:
# K230 UART1_TXD(GPIO40) -> STM32 PA10 / USART1_RX
# K230 UART1_RXD(GPIO41) -> STM32 PA9  / USART1_TX
# K230 GND              -> STM32 GND

fpioa = FPIOA()
fpioa.set_function(40, FPIOA.UART1_TXD)
fpioa.set_function(41, FPIOA.UART1_RXD)
uart1 = UART(UART.UART1, 9600)

CMD_STOP = "$0,0,0,0,0,0,0,0,0,0#"
CMD_FORWARD = "$1,0,0,0,0,0,0,0,0,0#"
CMD_LEFT = "$3,0,0,0,0,0,0,0,0,0#"
CMD_RIGHT = "$4,0,0,0,0,0,0,0,0,0#"
CMD_SPIN_LEFT = "$0,1,0,0,0,0,0,0,0,0#"
CMD_SPIN_RIGHT = "$0,2,0,0,0,0,0,0,0,0#"
CMD_BALL_MID_ANGLE = "$5,0,0,0,0,0,0,0,0,0#"

DISP_W = 640
DISP_H = 480
DET_W = 320
DET_H = 240
SCALE_X = DISP_W // DET_W
SCALE_Y = DISP_H // DET_H
CENTER_X = DET_W // 2

START_DELAY = 0.5
FORWARD_TIME = 4.5
LANDING_STOP_TIME = 1.0
MID_ANGLE_SET_TIME = 0.5
BALL_MODE_WAIT_TIME = 0.25
SEND_INTERVAL = 0.25

# Red ball detection. LAB threshold: (L_min, L_max, A_min, A_max, B_min, B_max)
BALL_THRESHOLD = [(20, 85, 20, 80, 0, 75)]
MIN_BALL_PIXELS = 120
MIN_BALL_AREA = 100
BALL_CONFIRM_FRAMES = 6

DEADZONE = 60
LOST_HOLD_TIME = 0.35
LOST_SEARCH_TIME = 1.2

last_send_time = 0
last_ball_time = 0
last_ball_x = CENTER_X
ball_seen_count = 0


def send_cmd(cmd, force=False):
    global last_send_time
    now = time.time()
    if force or (now - last_send_time) > SEND_INTERVAL:
        uart1.write(cmd.encode())
        last_send_time = now


def draw_status(img, status, cmd, elapsed):
    img.draw_string(10, 10, "FORWARD + RED", scale=2, color=(255, 255, 0))
    img.draw_string(10, 45, status, scale=2, color=(0, 255, 0))
    img.draw_string(10, 80, cmd[:2], scale=2, color=(255, 255, 255))
    img.draw_string(10, 115, str(int(elapsed)), scale=2, color=(255, 255, 255))


def make_detect_img(img):
    return img.copy(x_scale=0.5, y_scale=0.5)


def scale_rect(rect):
    x, y, w, h = rect
    return (x * SCALE_X, y * SCALE_Y, w * SCALE_X, h * SCALE_Y)


def scale_point(x, y):
    return x * SCALE_X, y * SCALE_Y


def largest_blob(blobs):
    if blobs:
        return max(blobs, key=lambda b: b.pixels())
    return None


def find_ball(det_img):
    blobs = det_img.find_blobs(BALL_THRESHOLD,
                               pixels_threshold=MIN_BALL_PIXELS,
                               area_threshold=MIN_BALL_AREA,
                               merge=True)
    return largest_blob(blobs)


def draw_blob(img, blob, color):
    img.draw_rectangle(scale_rect(blob.rect()), color=color)
    cx, cy = scale_point(blob.cx(), blob.cy())
    img.draw_cross(cx, cy, color=color)


sensor = Sensor(width=DISP_W, height=DISP_H)
sensor.reset()
sensor.set_framesize(Sensor.VGA)
sensor.set_pixformat(Sensor.RGB565)

Display.init(Display.ST7701, width=DISP_W, height=DISP_H, to_ide=True)
MediaManager.init()
sensor.run()

start_time = time.time()
send_cmd(CMD_STOP, force=True)

try:
    while True:
        os.exitpoint()
        img = sensor.snapshot()
        now = time.time()
        elapsed = now - start_time

        if elapsed < START_DELAY:
            status = "WAIT"
            cmd = CMD_STOP
        elif elapsed < START_DELAY + FORWARD_TIME:
            status = "FORWARD"
            cmd = CMD_FORWARD
        elif elapsed < START_DELAY + FORWARD_TIME + LANDING_STOP_TIME:
            status = "LANDING STOP"
            cmd = CMD_STOP
        elif elapsed < START_DELAY + FORWARD_TIME + LANDING_STOP_TIME + MID_ANGLE_SET_TIME:
            status = "SET MID 0.2"
            cmd = CMD_BALL_MID_ANGLE
        elif elapsed < START_DELAY + FORWARD_TIME + LANDING_STOP_TIME + MID_ANGLE_SET_TIME + BALL_MODE_WAIT_TIME:
            status = "BALL WAIT"
            cmd = CMD_STOP
        else:
            det_img = make_detect_img(img)
            ball = find_ball(det_img)

            if ball:
                draw_blob(img, ball, (255, 0, 0))
                ball_seen_count += 1
                last_ball_time = now
                last_ball_x = ball.cx()

                x_off = ball.cx() - CENTER_X
                centered = abs(x_off) < DEADZONE
                ball_confirmed = ball_seen_count >= BALL_CONFIRM_FRAMES

                if not ball_confirmed:
                    status = "CONFIRM"
                    cmd = CMD_STOP
                elif centered:
                    status = "PUSH"
                    cmd = CMD_FORWARD
                elif x_off < 0:
                    status = "AIM LEFT"
                    cmd = CMD_LEFT
                else:
                    status = "AIM RIGHT"
                    cmd = CMD_RIGHT
            else:
                ball_seen_count = 0
                lost_time = now - last_ball_time
                if lost_time < LOST_HOLD_TIME:
                    status = "LOST PUSH"
                    cmd = CMD_FORWARD
                elif lost_time < LOST_SEARCH_TIME:
                    if last_ball_x < CENTER_X:
                        status = "SEARCH L"
                        cmd = CMD_SPIN_LEFT
                    else:
                        status = "SEARCH R"
                        cmd = CMD_SPIN_RIGHT
                else:
                    status = "SEARCH"
                    cmd = CMD_SPIN_LEFT

        draw_status(img, status, cmd, elapsed)
        send_cmd(cmd, force=(status == "PUSH" or status == "AIM LEFT" or status == "AIM RIGHT"))
        Display.show_image(img)

except Exception as e:
    print("Error:", e)
finally:
    send_cmd(CMD_STOP, force=True)
    sensor.stop()
    Display.deinit()
    MediaManager.deinit()
