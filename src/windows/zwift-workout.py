# iFit-Workout - Auto-incline and auto-speed control of treadmill via ADB and OCR for Zwift workouts
# Author: Al Udell
# Revised: January 26, 2024

# zwift-workout.py - take Zwift screenshot, crop speed/incline instruction, OCR speed/incline

# imports
import cv2
import numpy as np
import re
import win32gui
from datetime import datetime
from paddleocr import PaddleOCR
from PIL import Image, ImageGrab

# Enable DPI aware on Windows
from ctypes import windll
user32 = windll.user32
user32.SetProcessDPIAware()

# Take Zwift screenshot - windowed mode only
hwnd = win32gui.FindWindow(None, 'Zwift')
if not hwnd:
    print("Zwift is not running")
    exit()
x, y, x1, y1 = win32gui.GetClientRect(hwnd)
x, y = win32gui.ClientToScreen(hwnd, (x, y))
x1, y1 = win32gui.ClientToScreen(hwnd, (x1, y1))
screenshot = ImageGrab.grab((x, y, x1, y1))

# Scale image to 3000 x 2000
screenshot = screenshot.resize((3000, 2000))

# Crop image to workout instruction area
screenwidth, screenheight = screenshot.size

# Values for Zwift workout instructions
col1 = int(screenwidth/3000 * 1010)
row1 = int(screenheight/2000 * 260)
col2 = int(screenwidth/3000 * 1285)
row2 = int(screenheight/2000 * 480)

cropped = screenshot.crop((col1, row1, col2, row2))

# Convert image to np array
cropped_np = np.array(cropped)

# OCR image
ocr = PaddleOCR(lang='en', use_gpu=False, show_log=False, det_db_unclip_ratio=2.0, det_db_box_thresh=0.40, drop_score=0.40, rec_algorithm='CRNN', cls_model_dir='paddleocr/ch_ppocr_mobile_v2.0_cls_infer', det_model_dir='paddleocr/en_PP-OCRv3_det_infer', rec_model_dir='paddleocr/en_PP-OCRv3_rec_infer')
result = ocr.ocr(cropped_np, cls=False, det=True, rec=True)

# Extract OCR text
ocr_text = ''
for line in result:
    for word in line:
        ocr_text += f"{word[1][0]} "

# Find the incline number
if "incline" in ocr_text.lower():
    pattern = r'-?\d+(?:\.\d+)?'
    numbers = re.findall(pattern, ocr_text)
    incline = str(float(numbers[0]))
    speedindex = 1
else:
    incline = 'None'
    speedindex = 0

# Find the speed number
if "kph" in ocr_text.lower():
    pattern = r'-?\d+(?:\.\d+)?'
    numbers = re.findall(pattern, ocr_text)
    speed = str(float(numbers[speedindex]))
else:
    speed = 'None'

print(speed + ";" + incline)

