# iFit-Wolf3 - Autoincline control of treadmill via ADB and OCR
# Author: Al Udell
# Revised: November 23, 2023

# zwift-incline.py - take Zwift screenshot, crop incline, OCR incline

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

# Crop image to incline area
screenwidth, screenheight = screenshot.size

# Values for Zwift regular incline
col1 = int(screenwidth/3000 * 2800)
row1 = int(screenheight/2000 * 90)
col2 = int(screenwidth/3000 * 2975)
row2 = int(screenheight/2000 * 195)

cropped = screenshot.crop((col1, row1, col2, row2))

# Convert image to np array
cropped_np = np.array(cropped)

# Convert np array to PIL
cropped_pil = Image.fromarray(cropped_np)

# Convert PIL image to cv2 RGB
cropped_cv2 = cv2.cvtColor(np.array(cropped_pil), cv2.COLOR_RGB2BGR)

# Convert cv2 RGB to HSV
result = cropped_cv2.copy()
image = cv2.cvtColor(cropped_cv2, cv2.COLOR_BGR2HSV)

# Isolate white mask
lower = np.array([0,0,159])
upper = np.array([0,0,255])
mask0 = cv2.inRange(image, lower, upper)
result0 = cv2.bitwise_and(result, result, mask=mask0)

# Isolate yellow mask
lower = np.array([24,239,241])
upper = np.array([24,253,255])
mask1 = cv2.inRange(image, lower, upper)
result1 = cv2.bitwise_and(result, result, mask=mask1)

# Isolate orange mask
lower = np.array([8,191,243])
upper = np.array([8,192,243])
mask2 = cv2.inRange(image, lower, upper)
result2 = cv2.bitwise_and(result, result, mask=mask2)

# Isolate red mask
lower = np.array([0,255,255])
upper = np.array([10,255,255])
mask3 = cv2.inRange(image, lower, upper)
result3 = cv2.bitwise_and(result, result, mask=mask3)

# Join colour masks
mask = mask0+mask1+mask2+mask3

# Set output image to zero everywhere except mask
merge = image.copy()
merge[np.where(mask==0)] = 0

# Convert to grayscale
gray = cv2.cvtColor(merge, cv2.COLOR_BGR2GRAY)

# Convert to black/white by threshold
ret,bin = cv2.threshold(gray, 70, 255, cv2.THRESH_BINARY_INV)

# Apply gaussian blur
gaussianBlur = cv2.GaussianBlur(bin,(3,3),0)

# OCR image
ocr = PaddleOCR(lang='en', use_gpu=False, show_log=False, det_db_unclip_ratio=2.0, det_db_box_thresh=0.40, drop_score=0.40, rec_algorithm='CRNN', cls_model_dir='paddleocr/ch_ppocr_mobile_v2.0_cls_infer', det_model_dir='paddleocr/en_PP-OCRv3_det_infer', rec_model_dir='paddleocr/en_PP-OCRv3_rec_infer')
result = ocr.ocr(gaussianBlur, cls=False, det=True, rec=True)

# Extract OCR text
ocr_text = ''
for line in result:
    for word in line:
        ocr_text += f"{word[1][0]}"

# Remove all characters that are not "-" and integers from OCR text
pattern = r"[^-\d]+"
ocr_text = re.sub(pattern, "", ocr_text)
if ocr_text:
    incline = ocr_text
else:
    incline = 'None'

print(incline)
