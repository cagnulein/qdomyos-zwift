# iFit-Workout - Auto-incline and auto-speed control of treadmill via ADB and OCR for Zwift workouts
# Author: Al Udell
# Revised: November 25, 2023

# zwift-workout.py - take Zwift screenshot, crop speed/incline instruction, OCR speed/incline

# imports
import cv2
import numpy as np
import re
from PIL import Image, ImageGrab
import requests
import win32gui

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

# Write zwift image
cv2.imwrite('zwift.png', cropped_np, [cv2.IMWRITE_PNG_COMPRESSION, 0])

# OCR image
image_data = open("zwift.png","rb").read()
ocr = requests.post("http://localhost:32168/v1/image/ocr", files={"image":image_data}).json()

# Extract label values from the 'predictions' list and merge into a single string
labels = [prediction['label'] for prediction in ocr.get('predictions', [])]
result = ' '.join(labels)

# Find the speed number
if "kph" in result.lower():
    pattern = r'-?\d+(?:\.\d+)?'
    numbers = re.findall(pattern, result)
    speed = str(float(numbers[1]))
else:
    speed = 'None'

# Find the incline number
if "incline" in result.lower():
    pattern = r'-?\d+(?:\.\d+)?'
    numbers = re.findall(pattern, result)
    incline = str(float(numbers[0]))
else:
    incline = 'None'

print(speed + ";" + incline)
