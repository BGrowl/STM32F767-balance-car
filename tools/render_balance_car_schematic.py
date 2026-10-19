from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "双轮平衡车完整电气连接原理图.png"

W, H = 2600, 1750
BG = "#F7FAFC"
NAVY = "#183B56"
BLUE = "#2F6F8F"
TEAL = "#2A9D8F"
RED = "#D64545"
AMBER = "#E6A93D"
INK = "#243746"
MUTED = "#667985"
GRID = "#C8D8E1"


def font(size, bold=False):
    path = Path(r"C:\Windows\Fonts\simsun.ttc")
    return ImageFont.truetype(str(path), size, index=1 if bold else 0)


F_TITLE = font(58, True)
F_H = font(34, True)
F = font(25)
F_S = font(20)
F_PIN = font(22)


img = Image.new("RGB", (W, H), BG)
d = ImageDraw.Draw(img)


def box(rect, title, fill="#FFFFFF", outline=GRID):
    d.rounded_rectangle(rect, radius=18, fill=fill, outline=outline, width=3)
    x1, y1, x2, _ = rect
    d.rectangle((x1, y1, x2, y1 + 58), fill=NAVY)
    d.text(((x1 + x2) / 2, y1 + 29), title, font=F_H, fill="white", anchor="mm")


def label(x, y, text, color=INK, anchor="la", f=F_PIN):
    d.text((x, y), text, font=f, fill=color, anchor=anchor)


def wire(points, color=BLUE, width=5):
    d.line(points, fill=color, width=width, joint="curve")


def pin_left(rect, y, text):
    x = rect[0]
    d.ellipse((x - 7, y - 7, x + 7, y + 7), fill=BLUE)
    label(x + 18, y, text, anchor="lm")
    return x, y


def pin_right(rect, y, text):
    x = rect[2]
    d.ellipse((x - 7, y - 7, x + 7, y + 7), fill=BLUE)
    label(x - 18, y, text, anchor="rm")
    return x, y


def power_pin(rect, xoff, text, color=RED):
    x = rect[0] + xoff
    y = rect[3]
    d.ellipse((x - 7, y - 7, x + 7, y + 7), fill=color)
    label(x, y - 18, text, color=color, anchor="ms", f=F_S)
    return x, y


d.rectangle((0, 0, W, 105), fill=NAVY)
d.text((W / 2, 53), "双轮平衡车完整电气连接原理图", font=F_TITLE, fill="white", anchor="mm")
d.text((W - 65, 72), "模块级原理连接图", font=F_S, fill="#D9E8EF", anchor="rm")

# Module geometry
battery = (60, 180, 400, 470)
dc = (60, 570, 400, 900)
imu = (520, 150, 1050, 440)
mcu = (840, 520, 1720, 1220)
k230 = (1910, 155, 2525, 500)
bt = (1910, 585, 2525, 910)
driver = (780, 1360, 1770, 1690)
enc_l = (60, 1110, 470, 1430)
enc_r = (2130, 1110, 2540, 1430)
motor_l = (1830, 1385, 2100, 1515)
motor_r = (1830, 1550, 2100, 1680)

box(battery, "动力电池与输入保护", "#FFF7E6")
box(dc, "DC-DC 5 V稳压模块", "#FFF7E6")
box(imu, "MPU6050 姿态传感器", "#EAF6F3")
box(mcu, "STM32F767 主控制器", "#EDF4F8")
box(k230, "K230 视觉模块", "#EAF6F3")
box(bt, "蓝牙模块（任务一）", "#FFF7E6")
box(driver, "双路直流电机驱动器", "#F1EFF8")
box(enc_l, "左轮编码器", "#EAF6F3")
box(enc_r, "右轮编码器", "#EAF6F3")
box(motor_l, "左电机", "#FFFFFF")
box(motor_r, "右电机", "#FFFFFF")

# Battery and DC-DC
label(90, 275, "电池正极 → 总开关 → 保险丝 → 防反接", f=F)
label(90, 340, "VBAT：仅供电机功率端与DC-DC输入", color=AMBER, f=F_S)
label(90, 405, "电池负极 → 公共GND", f=F)
label(95, 670, "输入：VBAT", color=AMBER)
label(95, 735, "输出：5 V（建议≥5 A）", color=RED)
label(95, 800, "输出：GND", color=INK)
label(95, 855, "输出端：100 uF + 0.1 uF", color=MUTED, f=F_S)

# MCU pins
left_pins = [
    (625, "PB8  I2C1_SCL"),
    (685, "PB7  I2C1_SDA"),
    (745, "PH4  EXTI (MPU INT)"),
    (835, "PA6  TIM3_CH1 / PWMA"),
    (895, "PA7  TIM3_CH2 / PWMB"),
    (955, "PI5  AIN1"),
    (1015, "PI6  AIN2"),
    (1075, "PG13 BIN1"),
    (1135, "PG14 BIN2"),
]
right_pins = [
    (625, "PA10 USART1_RX"),
    (685, "PA9  USART1_TX"),
    (790, "PD13 E1A"),
    (850, "PD12 E1B"),
    (940, "PH10 E2A"),
    (1000, "PH11 E2B"),
    (1100, "USART3_RX（蓝牙可选）"),
    (1160, "USART3_TX（蓝牙可选）"),
]
lp = {text.split()[0]: pin_left(mcu, y, text) for y, text in left_pins}
rp = {text.split()[0]: pin_right(mcu, y, text) for y, text in right_pins}
power_pin(mcu, 180, "5V_IN")
power_pin(mcu, 430, "3.3V")
power_pin(mcu, 680, "GND", INK)

# IMU pins and wires
imu_pins = {
    "SCL": pin_right(imu, 245, "SCL"),
    "SDA": pin_right(imu, 300, "SDA"),
    "INT": pin_right(imu, 355, "INT"),
}
wire([imu_pins["SCL"], (1160, 245), (1160, 625), lp["PB8"]], BLUE)
wire([imu_pins["SDA"], (1195, 300), (1195, 685), lp["PB7"]], BLUE)
wire([imu_pins["INT"], (1230, 355), (1230, 745), lp["PH4"]], TEAL)
label(550, 400, "VCC=3.3 V，GND；近端0.1 uF去耦", color=MUTED, f=F_S)

# K230 serial
k_tx = pin_left(k230, 275, "GPIO40 UART1_TXD")
k_rx = pin_left(k230, 345, "GPIO41 UART1_RXD")
wire([k_tx, (1810, 275), (1810, 625), rp["PA10"]], BLUE)
wire([rp["PA9"], (1845, 685), (1845, 345), k_rx], TEAL)
label(1950, 420, "5 V稳定供电；100 uF + 0.1 uF", color=RED, f=F_S)
label(1950, 458, "GND必须与STM32共地", color=INK, f=F_S)

# Bluetooth serial
bt_tx = pin_left(bt, 705, "TXD")
bt_rx = pin_left(bt, 780, "RXD")
wire([bt_tx, (1830, 705), (1830, 1100), rp["USART3_RX（蓝牙可选）"]], BLUE)
wire([rp["USART3_TX（蓝牙可选）"], (1870, 1160), (1870, 780), bt_rx], TEAL)
label(1950, 845, "VCC按模块额定电压；GND共地", color=MUTED, f=F_S)

# Encoders
el_a = pin_right(enc_l, 1220, "E1A")
el_b = pin_right(enc_l, 1300, "E1B")
wire([el_a, (650, 1220), (650, 790), rp["PD13"]], BLUE)
wire([el_b, (700, 1300), (700, 850), rp["PD12"]], TEAL)
label(95, 1380, "VCC=5 V；GND共地", color=MUTED, f=F_S)

er_a = pin_left(enc_r, 1220, "E2A")
er_b = pin_left(enc_r, 1300, "E2B")
wire([rp["PH10"], (1900, 940), (1900, 1220), er_a], BLUE)
wire([rp["PH11"], (1950, 1000), (1950, 1300), er_b], TEAL)
label(2170, 1380, "VCC=5 V；GND共地", color=MUTED, f=F_S)

# Motor driver control: use explicit fan-out routes
drv_inputs = {
    "PWMA": pin_left(driver, 1440, "PWMA"),
    "PWMB": pin_left(driver, 1490, "PWMB"),
    "AIN1": pin_left(driver, 1540, "AIN1"),
    "AIN2": pin_left(driver, 1590, "AIN2"),
    "BIN1": pin_left(driver, 1640, "BIN1"),
    "BIN2": pin_left(driver, 1680, "BIN2"),
}
for idx, (src, key) in enumerate([
    (lp["PA6"], "PWMA"), (lp["PA7"], "PWMB"), (lp["PI5"], "AIN1"),
    (lp["PI6"], "AIN2"), (lp["PG13"], "BIN1"), (lp["PG14"], "BIN2"),
]):
    x = 540 + idx * 35
    wire([src, (x, src[1]), (x, drv_inputs[key][1]), drv_inputs[key]], BLUE if idx < 2 else TEAL, 4)

label(1020, 1435, "VM/VBAT：动力电池", color=AMBER, f=F_S)
label(1020, 1485, "VCC：5 V逻辑", color=RED, f=F_S)
label(1020, 1535, "GND：功率地与逻辑地共地", f=F_S)
label(1020, 1585, "驱动近端：100 uF + 0.1 uF", color=MUTED, f=F_S)
label(1020, 1635, "OUT_A± → 左电机；OUT_B± → 右电机", color=AMBER, f=F_S)

wire([(driver[2], 1450), (motor_l[0], 1450)], AMBER, 6)
wire([(driver[2], 1608), (motor_r[0], 1608)], AMBER, 6)

# Power buses
wire([(400, 315), (500, 315), (500, 620), (400, 620)], AMBER, 8)  # battery to DC
wire([(400, 315), (610, 315), (610, 1320), (1270, 1320), (1270, 1360)], AMBER, 8)
label(635, 1295, "VBAT / VM", color=AMBER, f=F_S)

power_y = 1265
wire([(400, 735), (760, 735), (760, power_y), (2350, power_y)], RED, 7)
label(470, 715, "5 V BUS", color=RED, f=F_S)
wire([(1020, power_y), (1020, 1220)], RED, 5)
wire([(2200, power_y), (2200, 500)], RED, 5)
wire([(2280, power_y), (2280, 910)], RED, 5)

gnd_y = 1715
wire([(80, gnd_y), (2520, gnd_y)], INK, 8)
for x, y in [(260, 470), (280, 900), (1520, 1220), (2250, 500), (2350, 910), (270, 1430), (2350, 1430), (1280, 1690)]:
    wire([(x, y), (x, gnd_y)], INK, 4)
label(1300, 1730, "公共GND：电池、DC-DC、STM32、K230、蓝牙、传感器、编码器与电机驱动全部相连", color=INK, anchor="ms", f=F_S)

# 3.3 V from MCU to IMU
wire([(1270, 1220), (1270, 480), (790, 480), (790, 440)], RED, 4)
label(825, 468, "3.3 V", color=RED, f=F_S)

# UART caution
d.rounded_rectangle((1760, 955, 2525, 1060), radius=12, fill="#FFF3CD", outline=AMBER, width=2)
label(1790, 985, "注意：K230与蓝牙若共用PA9/PA10，不可同时主动发送；", color=INK, f=F_S)
label(1790, 1025, "建议使用不同USART，或用跳线/模拟开关进行选择。", color=INK, f=F_S)

# Legend
d.rounded_rectangle((60, 1475, 600, 1665), radius=12, fill="white", outline=GRID, width=2)
label(90, 1510, "图例", color=NAVY, f=F_H)
for y, c, txt in [
    (1560, RED, "5 V / 3.3 V电源"),
    (1600, AMBER, "VBAT / 电机功率"),
    (1640, BLUE, "数字信号 / UART / I2C"),
]:
    wire([(95, y), (165, y)], c, 6)
    label(185, y, txt, anchor="lm", f=F_S)

img.save(OUT, "PNG", optimize=True)
print(OUT)
