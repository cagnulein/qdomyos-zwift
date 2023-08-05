# iFit-Workout - Auto-incline and auto-speed control of treadmill via ADB and OCR for Zwift workouts
# Author: Al Udell
# Revised: August 4, 2023

# process-image.py - take Zwift screenshot, crop speed/incline instruction, OCR speed/incline

# imports
import cv2
import numpy as np
import re
from datetime import datetime
from paddleocr import PaddleOCR
from PIL import Image, ImageGrab

# Take Zwift screenshot
screenshot = ImageGrab.grab()

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

# Scale image to correct size for borderless window mode
width, height = cropped.size
cropped = cropped.resize((int(width * 1.04), int(height * 1.04)))

# Convert image to np array
cropped_np = np.array(cropped)

# OCR image
ocr = PaddleOCR(lang='en', use_gpu=False, enable_mkldnn=True, use_angle_cls=False, table=False, layout=False, show_log=False)
result = ocr.ocr(cropped_np, cls=False, det=True, rec=True)

# Extract OCR text
ocr_text = ''
for line in result:
    for word in line:
        ocr_text += f"{word[1][0]} "

# Find the speed number
num_pattern = r'\d+(\.\d+)?'  # Regular expression pattern to match numbers with optional decimal places
unit_pattern = r'\s+(kph|mph)'  # Regular expression pattern to match "kph" or "mph" units
speed_match = re.search(num_pattern + unit_pattern, ocr_text)
if speed_match:
    speed = speed_match.group(0)
    pattern = r'\d+\.\d+'
    speed = re.findall(pattern, speed)[0]
else:
    speed = 'None'

# Find the incline number
incline_pattern = r'\d+\s*%'  # Regular expression pattern to match numbers with "%"
incline_match = re.search(incline_pattern, ocr_text)
if incline_match:
    incline = incline_match.group(0)
    pattern = r'\d+'
    incline = re.findall(pattern, incline)[0]
else:
    incline = 'None'

print(f"{speed};{incline}")

