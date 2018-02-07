
# coding: utf-8

# In[195]:


import pyrealsense2 as rs
import numpy as np
import cv2

import time
import math


# # detect device

# In[196]:


deviceDetect = False
ctx = rs.context()
ds5_dev = rs.device()
devices = ctx.query_devices()

for dev in devices:
    productName = str(dev.get_info(rs.camera_info.product_id))
    #print(productName)
  
    # DS 435 config
    if productName in "0B07":
        EnableColorStream = True
        EnableInfraredStream = False
        EnableDepthStream = True
        deviceDetect = True
        break
    
    # DS 415 config
    elif productName in "0AD3":
        EnableColorStream = True
        EnableInfraredStream = False
        EnableDepthStream = True
        deviceDetect = True
        break
        
    # DS 410 config
    elif productName in "0AD2":
        EnableColorStream = False
        EnableInfraredStream = True
        EnableDepthStream = True
        deviceDetect = True
        break

if deviceDetect is not True:
    raise Exception("No supported device was found")


# # application config

# In[197]:


ColorStreamResWidth = 1280
ColorStreamResHeight = 720
ColorStreamFPS = 30

InfraredStreamResWidth = 1280
InfraredStreamResHeight = 720
InfraredStreamFPS = 30

DepthStreamResWidth = 424
DepthStreamResHeight = 240
DepthStreamFPS = 30
DepthEqualization = True
DepthMin = 495
DepthMax = 505 # 1000 = 1 meter


font = cv2.FONT_HERSHEY_TRIPLEX
fontA = cv2.FONT_HERSHEY_DUPLEX
fontB = cv2.FONT_HERSHEY_SIMPLEX

textString = ''
sizeLine = 3
sizeLineP = 1
colorBlack = (0, 0, 0)
colorBlue = (255, 0, 0)
colorGreen = (0, 255, 0)
colorRed = (0, 0, 255)

colorYellow = (0, 255,255)
colorPurple = (255, 0,255)
colorTeal = (255,255,0)

colorWhite = (255,255,255)


# # RealSense camera config

# In[198]:


# Configure depth and color streams
pipeline = rs.pipeline()
config = rs.config()

if EnableColorStream is True:
    config.enable_stream(
        rs.stream.color, ColorStreamResWidth, ColorStreamResHeight, rs.format.bgr8, ColorStreamFPS)

if EnableInfraredStream is True:
    config.enable_stream(
        rs.stream.infrared, InfraredStreamResWidth, InfraredStreamResHeight, rs.format.bgr8, InfraredStreamFPS)
    
if EnableDepthStream is True:
    config.enable_stream(
        rs.stream.depth, DepthStreamResWidth, DepthStreamResHeight, rs.format.z16, DepthStreamFPS)

# Start streaming
cfg = pipeline.start(config)

# Alignment
if EnableColorStream is True:
    align_to = rs.stream.color
elif EnableInfraredStream is True:
    align_to = rs.stream.infrared

align = rs.align(align_to)
    
# Advanced settings
dev = cfg.get_device()
depth_sensor = dev.first_depth_sensor()
depth_sensor.set_option(rs.option.visual_preset, 4)
depth_sensor.set_option(rs.option.enable_auto_exposure, 1)
#depth_sensor.set_option(rs.option.min_distance, DepthMin)
#depth_sensor.set_option(rs.option.max_distance, DepthMax)

# Get depth scale
scale = depth_sensor.get_depth_scale()
#print("depth scale:" + str(scale))

# PointCloud settings
#pc = rs.pointcloud()

# Get intrinsics
if EnableColorStream is True:
    stream = cfg.get_stream(rs.stream.color )
    profile = stream.as_video_stream_profile()
elif EnableInfraredStream is True:
    stream = cfg.get_stream(rs.stream.infrared)
    profile = stream.as_video_stream_profile()
    
intrin = profile.get_intrinsics()
#print("Intrinsics:\n  width:" , intrin.width, "\n  height:" , intrin.height, "\n  ppx:" , intrin.ppx, "\n  ppy:" , intrin.ppy, "\n  fx:" , intrin.fx, "\n  fy:" , intrin.fy, "\n  model:" , intrin.model)

# # color map function

# In[199]:


def procColorMap(input_image):
    inp = input_image.copy()
    
    
    if DepthEqualization:
        scaleAlpha = 255 / (DepthMax - DepthMin)
        inp = cv2.convertScaleAbs(inp, None, scaleAlpha, -1*DepthMin*scaleAlpha)
        mask = np.where((input_image == 0), 0, 1).astype('uint8')
        inp = inp * mask[:,:]
        
        '''
        height, width = inp.shape
        for i in range(0, width):
            for j in range(0, height):
                if inp[j,i] is not 0:
                    inp[j,i] = int((inp[j,i] - DepthMin) * scaleAlpha)
        '''
    else:
        scaleAlpha = 255 / DepthMax
        inp = cv2.convertScaleAbs(inp, None, scaleAlpha, 0)
        
    depth_colormap = cv2.applyColorMap(inp, cv2.COLORMAP_JET)
    
    return depth_colormap


# # screen capture function

# In[200]:


# scrren capture global parameters
flagCapture = False


# In[201]:


def procScreenCapture():  
    global flagCapture, flagDispMeasure, flagDispGrab
    global color_image, infrared_image, depth_colormap
    global measureImg, grabImg, grabImg3, imgSec
    
    if flagCapture is True:
        flagCapture = False
        if EnableColorStream is True:
            fileName = "../imageCapture/color_" + time.strftime("%Y-%m-%d_%H%M%S-", time.localtime()) + '.png'
            cv2.imwrite(fileName, color_image, [int(cv2.IMWRITE_PNG_COMPRESSION), 0])
            if flagDispMeasure is True:
                fileName = "../imageCapture/colorMeasure_" + time.strftime("%Y-%m-%d_%H%M%S-", time.localtime()) + '.png'
                cv2.imwrite(fileName, measureImg, [int(cv2.IMWRITE_PNG_COMPRESSION), 0])
            

        if EnableInfraredStream is True:
            fileName = "../imageCapture/infrared_" + time.strftime("%Y-%m-%d_%H%M%S-", time.localtime()) + '.png'
            cv2.imwrite(fileName, infrared_image, [int(cv2.IMWRITE_PNG_COMPRESSION), 0])
            if flagDispMeasure is True:
                fileName = "../imageCapture/infrared_Measure_" + time.strftime("%Y-%m-%d_%H%M%S-", time.localtime()) + '.png'
                cv2.imwrite(fileName, measureImg, [int(cv2.IMWRITE_PNG_COMPRESSION), 0])

        if EnableDepthStream is True:
            fileName = "../imageCapture/depth_" + time.strftime("%Y-%m-%d_%H%M%S-", time.localtime()) + '.png'
            cv2.imwrite(fileName, depth_colormap, [int(cv2.IMWRITE_PNG_COMPRESSION), 0])
            
        if flagDispMeasure is True:
            fileName = "../imageCapture/sect_" + time.strftime("%Y-%m-%d_%H%M%S-", time.localtime()) + '.png'
            cv2.imwrite(fileName, imgSec, [int(cv2.IMWRITE_PNG_COMPRESSION), 0])
        
        if flagDispGrab is True:
            fileName = "../imageCapture/Grab1_" + time.strftime("%Y-%m-%d_%H%M%S-", time.localtime()) + '.png'
            cv2.imwrite(fileName, grabImg, [int(cv2.IMWRITE_PNG_COMPRESSION), 0])
            fileName = "../imageCapture/Grab2_" + time.strftime("%Y-%m-%d_%H%M%S-", time.localtime()) + '.png'
            cv2.imwrite(fileName, grabImg3, [int(cv2.IMWRITE_PNG_COMPRESSION), 0])


# # projection and deprojection function

# In[202]:


def pixelToPoint(pixel, intrin, depth):
    point = []
        
    x = float((pixel[0] - intrin.ppx) / intrin.fx)
    y = float((pixel[1] - intrin.ppy) / intrin.fy)
    
    point.append(float(depth * x))
    point.append(float(depth * y))
    point.append(float(depth))
   
    return point


# # measure distace and sectional drawing function

# In[203]:


# measure distance global parameters
flagMeasure = False
flagDispMeasure= False
flagClickMeasure = False

flagSect = True

measureCordLTX = -1
measureCordLTY = -1
measureCordRDX = -1
measureCordRDY = -1

measureImg = np.zeros((512,512,3), np.uint8)
measureImg2 = np.zeros((512,512,3), np.uint8)
imgSec = np.zeros((720, 720, 3), np.uint8)
measureDepth = np.zeros((512,512,1), np.uint8)


# In[204]:


# mouse click function
def procMouseMeasure(event,x,y,flags,param):
    global measureCordLTX, measureCordLTY, measureCordRDX, measureCordRDY
    global flagClickMeasure
    global measureImg, measureImg2
    
    if event is cv2.EVENT_LBUTTONDOWN:
        measureCordLTX = x
        measureCordLTY = y
        flagClickMeasure = True
        measureImg2 = measureImg.copy()
        
    elif event is cv2.EVENT_MOUSEMOVE:
        if flagClickMeasure is True:
            measureCordRDX = x
            measureCordRDY = y

    elif event is cv2.EVENT_LBUTTONUP:
        measureCordRDX = x
        measureCordRDY = y
        flagClickMeasure = False


# In[205]:


# sectional drawing function
def drawSect(secLTX, secLTY, secRDX, secRDY):
    global measureDepth, imgSec
    output = []
    
    xdiff = secRDX - secLTX
    ydiff = secLTY - secRDY
    
    if xdiff < ydiff:
        for i in range(0, ydiff):
            posX = math.floor(secLTX + i * xdiff / ydiff)
            posY = math.floor(secRDY + i)
            data = measureDepth[posY, posX]
            output.append(data)
            #print('x:' + str(posX) + ' y:' + str(posY) + ' z:' + str(data))
    else:
        for i in range(0, xdiff):
            posX = math.floor(secLTX + i)
            posY = math.floor(secRDY + i * ydiff / xdiff)
            data = measureDepth[posY, posX]
            output.append(data)
            #print('x:' + str(posX) + ' y:' + str(posY) + ' z:' + str(data))
    
    cv2.namedWindow('Sectional drawing')
    #print(len(output))
    
    minOut = None
    maxOut = None
    for value in output:
        if not minOut:
            minOut = value
        elif value < minOut:
            minOut = value
        
        if not maxOut:
            maxOut = value
        elif value > maxOut:
            maxOut = value
      
    #print('min:' + str(minOut) + ' max:' + str(maxOut))
    
    dist = maxOut - minOut + 100
    parm = 720 / dist
    
    imgSec = np.zeros((720, len(output), 3), np.uint8)
    
    for i in range(0, len(output)):
        cv2.line(imgSec, (i, 720), (i,  620 - int((maxOut - output[i])*parm) ), colorYellow, 1)
    
    imgSec = cv2.resize(imgSec, (1280, 720), interpolation = cv2.INTER_CUBIC)
    cv2.imshow('Sectional drawing', imgSec)


# In[206]:


# measure distance function
def procMeasureDistance(pixelA, pixelB, intrin):
    global measureDepth
    
    if measureDepth[pixelA[1], pixelA[0]] is 0 or measureDepth[pixelB[1], pixelB[0]] is 0:
        return 0

    pointA = pixelToPoint(pixelA, intrin, measureDepth[pixelA[1], pixelA[0]])
    pointB = pixelToPoint(pixelB, intrin, measureDepth[pixelB[1], pixelB[0]])
    
    delta = 0
    for i in range(0,3):
        diff = math.floor(pointA[i]) - math.floor(pointB[i])
        delta += diff * diff
    
    distance = math.floor(math.sqrt(delta))
    
    return distance


# In[207]:


# mouse flagMeasure function
def procMeasure():
    global flagMeasure, flagDispMeasure, flagClickMeasure
    global measureCordLTX, measureCordLTY, measureCordRDX, measureCordRDY
    global measureImg, measureImg2, depth_image, measureDepth
    global intrin
    
    if flagMeasure is True:
        if EnableColorStream is True:
            measureImg = color_image.copy()
        elif EnableInfraredStream is True:
            measureImg = infrared_image.copy()
        
        measureDepth = depth_image.copy()
        cv2.namedWindow('Measure Distance')
        cv2.setMouseCallback('Measure Distance', procMouseMeasure)
        cv2.imshow('Measure Distance', measureImg)
        flagDispMeasure= True
        flagMeasure = False
    
    if flagDispMeasure is True:
        if flagClickMeasure is True:
            measureImg2 = measureImg.copy()
            if measureCordLTX != -1 and measureCordRDX != -1:
                cv2.line(measureImg2, (measureCordLTX,measureCordLTY), (measureCordRDX, measureCordRDY), colorYellow, sizeLineP)
            cv2.imshow('Measure Distance', measureImg2)
        else:
            if measureCordLTX != -1 and measureCordRDX != -1:
                cv2.line(measureImg, (measureCordLTX,measureCordLTY), (measureCordRDX, measureCordRDY), colorGreen, sizeLine)
                distance = procMeasureDistance([measureCordLTX, measureCordLTY], [measureCordRDX, measureCordRDY], intrin)
                distance = math.floor(distance) / 1000
                textString = str(distance) + 'm'
                textX = int((measureCordLTX+measureCordRDX)/2)
                textY = int((measureCordLTY+measureCordRDY)/2)
                cv2.putText(measureImg, textString, ( textX-100, textY), fontA, 1, colorBlack, 1, cv2.LINE_AA)
                cv2.putText(measureImg, textString, ( textX-100, textY), fontB, 1, colorYellow, 1, cv2.LINE_AA)
                
                if flagSect is True:
                    drawSect(measureCordLTX, measureCordLTY, measureCordRDX, measureCordRDY)

                measureCordLTX = -1
                measureCordLTY = -1
                measureCordRDX = -1
                measureCordRDY = -1


            cv2.imshow('Measure Distance', measureImg)


# # measure area funcion

# In[208]:


def procAreaTriangle(pixelA, pixelB, pixelC, depthA, depthB, depthC):
    global intrin
    
    if depthA is 0 or depthB is 0 or depthC is 0:
        return 0
    
    pointA = pixelToPoint(pixelA, intrin, depthA)
    pointB = pixelToPoint(pixelB, intrin, depthB)
    pointC = pixelToPoint(pixelC, intrin, depthC)
        
    vectorAB = np.array((pointB[0] - pointA[0], pointB[1] - pointA[1], pointB[2] - pointA[2]))
    vectorAC = np.array((pointC[0] - pointA[0], pointC[1] - pointA[1], pointC[2] - pointA[2]))
    
    cross = np.cross(vectorAB, vectorAC)
    crossVal = math.floor(math.sqrt(cross[0] * cross[0] + cross[1] * cross[1] + cross[2] * cross[2]))

    return 0.5 * abs(crossVal)


# In[209]:


def procMeasureArea(inputDepth):
    
    area = 0
    height, width = inputDepth.shape
    
    for i in range(0, width-1):
        for j in range(0, height-1):
            area += procAreaTriangle([i,j], [i+1, j], [i+1, j+1], inputDepth[j,i], inputDepth[j,i+1], inputDepth[j+1,i+1])
            area += procAreaTriangle([i,j], [i, j+1], [i+1, j+1], inputDepth[j,i], inputDepth[j+1,i], inputDepth[j+1,i+1])
    
    return math.floor(area)


# # grabcut function

# In[210]:


# grabcut global parameters
flagGrab = False
flagDispGrab = False
flagClickGrab = False

flagPaintYellow = False
flagPaintRed = False
flagRect = False

grabCordLTX = -1
grabCordLTY = -1
grabCordRDX = -1
grabCordRDY = -1

grabImg = np.zeros((512,512,3), np.uint8)
grabImg2 = np.zeros((512,512,3), np.uint8)
grabDepth = np.zeros((512,512,1), np.uint8)

grabPaintX = -1
grabPaintY = -1

# mask = np.zeros(img.shape[:2], np.uint8)
mask = np.zeros((512,512,3), np.uint8)

rect = (-1, -1, -1, -1)
thickness = 25 

flagGrabCut3D = False
parmDist = 300
parmDist2 = 400


#GCD_BGD（=0），背景；
#GCD_FGD（=1），前景；
#GCD_PR_BGD（=2），可能的背景；
#GCD_PR_FGD（=3），可能的前景。


# In[211]:


# mouse click function
def procMouseGrab(event,x,y,flags,param):
    global grabCordLTX, grabCordLTY, grabCordRDX, grabCordRDY
    global grabPaintX, grabPaintY
    global flagClickGrab, flagPaintYellow, flagPaintRed, flagRect
    global grabImg, grabImg2, grabImg3
    
    if event is cv2.EVENT_LBUTTONDOWN:
        if flagRect is True:
            if flags & cv2.EVENT_FLAG_CTRLKEY:
                grabPaintX = x
                grabPaintY = y
                flagPaintYellow = True
            elif flags & cv2.EVENT_FLAG_SHIFTKEY:
                grabPaintX = x
                grabPaintY = y
                flagPaintRed = True
        else:      
            if grabCordLTY != -1 and grabCordRDY != -1:
                grabImg = infrared_image.copy()
                grabImg3 = infrared_image.copy()

            grabCordLTX = x
            grabCordLTY = y
            flagClickGrab = True
            grabImg2 = grabImg.copy()
        
    elif event is cv2.EVENT_MOUSEMOVE:
        if flagClickGrab is True:
            grabCordRDX = x
            grabCordRDY = y
        elif flagPaintYellow is True or flagPaintRed is True:
            grabPaintX = x
            grabPaintY = y

    elif event is cv2.EVENT_LBUTTONUP:
        if flagPaintYellow is True:
            grabPaintX = x
            grabPaintY = y
            flagPaintYellow = False
        elif flagPaintRed is True:
            grabPaintX = x
            grabPaintY = y
            flagPaintRed = False
        else:    
            grabCordRDX = x
            grabCordRDY = y
            flagClickGrab = False


# In[212]:


def procGrabUI():
    global flagGrab, flagDispGrab, flagClickGrab, flagPaintYellow, flagPaintRed, flagRect
    global grabCordLTX, grabCordLTY, grabCordRDX, grabCordRDY
    global grabPaintX, grabPaintY
    global grabImg, grabImg2, grabImg3, depth_image, grabDepth
    global mask, bgdModel, fgdModel, rect
    global flagGrabCut3D
    
    if flagGrab is True:
        if EnableColorStream is True:
            grabImg = color_image.copy()
            grabImg3 = color_image.copy()
        elif EnableInfraredStream is True:
            grabImg = infrared_image.copy()
            grabImg3 = infrared_image.copy()
        
        grabDepth = depth_image.copy()
        cv2.namedWindow('GrabCutUI')
        cv2.setMouseCallback('GrabCutUI', procMouseGrab)
        cv2.imshow('GrabCutUI', grabImg)
        mask = np.zeros(grabImg.shape[:2], np.uint8)
        flagDispGrab= True
        flagGrab = False
    
    if flagDispGrab is True:
        
        if flagPaintYellow is True and flagRect is True:
            cv2.circle(grabImg, (grabPaintX,grabPaintY), thickness, (0, 255, 255), -1)
            cv2.circle(mask, (grabPaintX,grabPaintY), thickness, 1, -1)
            cv2.imshow('GrabCutUI', grabImg)
        
        if flagPaintRed is True and flagRect is True:
            cv2.circle(grabImg, (grabPaintX,grabPaintY), thickness, (0, 0, 255), -1)
            cv2.circle(mask, (grabPaintX,grabPaintY), thickness, 0, -1)
            cv2.imshow('GrabCutUI', grabImg)

        if flagClickGrab is True:
            grabImg2 = grabImg.copy()
            if grabCordLTX != -1 and grabCordRDX != -1:
                cv2.rectangle(grabImg2, (grabCordLTX,grabCordLTY), (grabCordRDX, grabCordRDY), colorYellow, sizeLineP)
            cv2.imshow('GrabCutUI', grabImg2)
        else:
            if grabCordLTX != -1 and grabCordRDX != -1:
                cv2.rectangle(grabImg, (grabCordLTX,grabCordLTY), (grabCordRDX, grabCordRDY), colorGreen, sizeLine)
                
                xmin = min(grabCordLTX,grabCordRDX)
                ymin = min(grabCordLTY,grabCordRDY)
                xnum = abs(grabCordLTX-grabCordRDX)
                ynum = abs(grabCordLTY-grabCordRDY)
                
                rect = (xmin, ymin, xnum, ynum)
                
                bgdModel = np.zeros((1,65),np.float64)
                fgdModel = np.zeros((1,65),np.float64)
                cv2.grabCut(grabImg3, mask, rect, bgdModel, fgdModel, 1, cv2.GC_INIT_WITH_RECT)
                
                if flagGrabCut3D is True:
                    grabMask = grabDepth.copy()
                    kernel = np.ones((5,5),np.uint8)

                    grabMask[grabMask == 0] = 65535
                    _, grabMask = cv2.threshold(grabMask, parmDist, 65535, cv2.THRESH_BINARY_INV)
                    grabMask = cv2.dilate(grabMask, kernel, iterations = 3)
                    grabMask = cv2.erode(grabMask, kernel, iterations = 6)
                    mask[grabMask == 0] = 2
                    mask[grabMask == 65535] = 1
                    #cv2.imshow('grabMask', grabMask)
                    
                    grabMask2 = grabDepth.copy()
                    _, grabMask2 = cv2.threshold(grabMask2, parmDist2, 65535, cv2.THRESH_BINARY)
                    grabMask2 = cv2.dilate(grabMask2, kernel, iterations = 3)
                    grabMask2 = cv2.erode(grabMask2, kernel, iterations = 10)
                    mask[grabMask2 == 65535] = 0
                    #cv2.imshow('grabMask2', grabMask2)
                    
                    mask2 = np.where((mask == 2) | (mask == 0), 0, 1).astype('uint8')
                    grabImg = grabImg * mask2[:,:,np.newaxis]
                    cv2.rectangle(grabImg, (grabCordLTX,grabCordLTY), (grabCordRDX, grabCordRDY), colorGreen, sizeLine)
                    cv2.imshow('GrabCutUI', grabImg)

                grabCordLTX = -1
                grabCordRDX = -1
                flagRect = True

            cv2.imshow('GrabCutUI', grabImg)


# In[213]:


def procGrabCut():
    global grabImg3, grabDepth
    global mask, bgdModel, fgdModel, rect
   
    cv2.grabCut(grabImg3, mask, rect, bgdModel, fgdModel, 1, cv2.GC_INIT_WITH_MASK)
    mask2 = np.where((mask == 2) | (mask == 0), 0, 1).astype('uint8')
    grabImg3 = grabImg3 * mask2[:,:,np.newaxis]
    
    grabDepth2 = grabDepth.copy()
    grabDepth2 = grabDepth2 * mask2[:,:]
    #cv2.imshow('grabDepth2', grabDepth2)
    area = procMeasureArea(grabDepth2)
    
    grabImg3 = cv2.copyMakeBorder(grabImg3, 0, 40, 0, 0, cv2.BORDER_CONSTANT, value=(0, 0, 0))
    textString = 'area: ' + str(area)
    cv2.putText(grabImg3, textString, (10, InfraredStreamResHeight+30), font, 1, colorTeal, 1, cv2.LINE_AA)
    cv2.imshow('GRABCUT', grabImg3)


# # utility function

# In[214]:


# utility global parameters
utilityCoordX = -1
utilityCoordY = -1

flagCoord = True
flagLine = True
ColorAlign = True

if EnableColorStream is True:
    CoordBorderW = ColorStreamResWidth
    CoordBorderH = ColorStreamResHeight
elif EnableInfraredStream is True:
    CoordBorderW = InfraredStreamResWidth
    CoordBorderH = InfraredStreamResHeight


# In[215]:


# proc for development
def procDepthValue(event,x,y,flags,param):
    global utilityCoordX, utilityCoordY
    
    if event is cv2.EVENT_MOUSEMOVE:
        if x != 0 and y != 0:
            utilityCoordX = x
            utilityCoordY = y


# In[216]:


if EnableColorStream is True:
    cv2.namedWindow('Color image')
    cv2.setMouseCallback('Color image', procDepthValue)

if EnableInfraredStream is True:
    cv2.namedWindow('Infrared image')
    cv2.setMouseCallback('Infrared image', procDepthValue) 
    
if EnableDepthStream is True:
    cv2.namedWindow('Depth image')
    cv2.setMouseCallback('Depth image', procDepthValue)


# In[217]:


# draw line function
def procDrawLine():
    if flagLine is True:
        global color_image, infrared_image, depth_colormap

        if EnableColorStream is True:
            cv2.line(color_image, (0, int(ColorStreamResHeight/2)), (ColorStreamResWidth, int(ColorStreamResHeight/2)), colorGreen, 1)
            cv2.line(color_image, (int(ColorStreamResWidth/2), 0), (int(ColorStreamResWidth/2), ColorStreamResHeight), colorGreen, 1)

        if EnableInfraredStream is True:
            cv2.line(infrared_image, (0, int(ColorStreamResHeight/2)), (ColorStreamResWidth, int(ColorStreamResHeight/2)), colorGreen, 1)
            cv2.line(infrared_image, (int(ColorStreamResWidth/2), 0), (int(ColorStreamResWidth/2), ColorStreamResHeight), colorGreen, 1)

        if EnableDepthStream is True:
            cv2.line(depth_colormap, (0, int(ColorStreamResHeight/2)), (ColorStreamResWidth, int(ColorStreamResHeight/2)), colorGreen, 1)
            cv2.line(depth_colormap, (int(ColorStreamResWidth/2), 0), (int(ColorStreamResWidth/2), ColorStreamResHeight), colorGreen, 1)


# # main loop

# In[218]:


try:
    while True:
        tick1 = cv2.getTickCount()
        # Wait for a coherent pair of frames: depth and color
        frames = pipeline.wait_for_frames()

        # Aligh frames
        aligned_frames = align.proccess(frames)

        if EnableColorStream is True:
            color_frame = aligned_frames.get_color_frame()
            if not color_frame:
                continue

        if EnableInfraredStream is True:
            infrared_frame = aligned_frames.first(rs.stream.infrared)
            if not infrared_frame:
                continue

        if EnableDepthStream is True:
            depth_frame = aligned_frames.get_depth_frame()
            if not depth_frame:
                continue

        # Convert images to numpy arrays
        if EnableColorStream is True:
            color_image = np.asanyarray(color_frame.get_data())

        if EnableInfraredStream is True:
            infrared_image = np.asanyarray(infrared_frame.get_data())

        if EnableDepthStream is True:
            depth_image = np.asanyarray(depth_frame.get_data())

        # colorize depth image
        depth_colormap = procColorMap(depth_image)
          

        # Painit function
        procMeasure()

        # Grabcut function
        procGrabUI()
        
        # draw line function
        procDrawLine()
        
        # FPS calculate
        tick2 = cv2.getTickCount()
        tick = math.floor( ((tick2 - tick1) * 1000) / cv2.getTickFrequency())

        # Show images
        if EnableColorStream is True:
            temp = color_image.copy()
            
            if ColorAlign is True:
                mask = np.where((depth_image == 0), 0, 1).astype('uint8')
                temp = temp * mask[:,:,np.newaxis]

            if utilityCoordX <= CoordBorderW and utilityCoordY <= CoordBorderH and flagCoord is True:
                cv2.circle(temp, (utilityCoordX,utilityCoordY), 5, colorWhite, -1)
                #textString = str(depth_image[utilityCoordY, utilityCoordX])
                #cv2.putText(temp, textString, (utilityCoordX-25, utilityCoordY), fontA, 1, colorBlack, 1, cv2.LINE_AA)
                #cv2.putText(temp, textString, (utilityCoordX-25, utilityCoordY), fontB, 1, colorYellow, 1, cv2.LINE_AA)
                
                point = pixelToPoint([utilityCoordX, utilityCoordY], intrin, depth_image[utilityCoordY, utilityCoordX])
                textString = str(math.floor(point[1])) + " " + str(math.floor(point[0])) + " " + str(math.floor(point[2]))
                cv2.putText(temp, textString, (utilityCoordX-100, utilityCoordY+40), fontA, 1, colorBlack, 1, cv2.LINE_AA)
                cv2.putText(temp, textString, (utilityCoordX-100, utilityCoordY+40), fontB, 1, colorYellow, 1, cv2.LINE_AA)

            temp = cv2.copyMakeBorder(temp, 0, 40, 0, 0, cv2.BORDER_CONSTANT, value=(0, 0, 0))
            textString = str(tick) + " ms"
            cv2.putText(temp, textString, (10, ColorStreamResHeight+30), font, 1, colorTeal, 1, cv2.LINE_AA)
            
            cv2.imshow('Color image', temp)
            
            if flagCapture is True and flagCoord is True:
                fileName = "../imageCapture/colorText_" + time.strftime("%Y-%m-%d_%H%M%S-", time.localtime()) + '.png'
                cv2.imwrite(fileName, temp, [int(cv2.IMWRITE_PNG_COMPRESSION), 0])

        if EnableInfraredStream is True:
            temp = infrared_image.copy()

            if utilityCoordX <= CoordBorderW and utilityCoordY <= CoordBorderH and flagCoord is True:
                cv2.circle(temp, (utilityCoordX,utilityCoordY), 5, colorWhite, -1)
                #textString = str(depth_image[utilityCoordY, utilityCoordX])
                #cv2.putText(temp, textString, (utilityCoordX-25, utilityCoordY), fontA, 1, colorBlack, 1, cv2.LINE_AA)
                #cv2.putText(temp, textString, (utilityCoordX-25, utilityCoordY), fontB, 1, colorYellow, 1, cv2.LINE_AA)
                
                point = pixelToPoint([utilityCoordX, utilityCoordY], intrin, depth_image[utilityCoordY, utilityCoordX])
                textString = str(math.floor(point[1])) + " " + str(math.floor(point[0])) + " " + str(math.floor(point[2]))
                cv2.putText(temp, textString, (utilityCoordX-100, utilityCoordY+40), fontA, 1, colorBlack, 1, cv2.LINE_AA)
                cv2.putText(temp, textString, (utilityCoordX-100, utilityCoordY+40), fontB, 1, colorYellow, 1, cv2.LINE_AA)

            temp = cv2.copyMakeBorder(temp, 0, 40, 0, 0, cv2.BORDER_CONSTANT, value=(0, 0, 0))
            textString = str(tick) + " ms"
            cv2.putText(temp, textString, (10, InfraredStreamResHeight+30), font, 1, colorTeal, 1, cv2.LINE_AA)

            cv2.imshow('Infrared image', temp)
            
            if flagCapture is True and flagCoord is True:
                fileName = "../imageCapture/infraredText_" + time.strftime("%Y-%m-%d_%H%M%S-", time.localtime()) + '.png'
                cv2.imwrite(fileName, temp, [int(cv2.IMWRITE_PNG_COMPRESSION), 0])

        if EnableDepthStream is True:
            temp = depth_colormap.copy()

            if utilityCoordX <= CoordBorderW and utilityCoordY <= CoordBorderH and flagCoord is True:
                cv2.circle(temp, (utilityCoordX,utilityCoordY), 5, colorWhite, -1)
                #textString = str(depth_image[utilityCoordY, utilityCoordX])
                #cv2.putText(temp, textString, (utilityCoordX-25, utilityCoordY), fontA, 1, colorBlack, 1, cv2.LINE_AA)
                #cv2.putText(temp, textString, (utilityCoordX-25, utilityCoordY), fontB, 1, colorYellow, 1, cv2.LINE_AA)
                
                point = pixelToPoint([utilityCoordX, utilityCoordY], intrin, depth_image[utilityCoordY, utilityCoordX])
                textString = str(math.floor(point[1])) + " " + str(math.floor(point[0])) + " " + str(math.floor(point[2]))
                cv2.putText(temp, textString, (utilityCoordX-100, utilityCoordY+40), fontA, 1, colorBlack, 1, cv2.LINE_AA)
                cv2.putText(temp, textString, (utilityCoordX-100, utilityCoordY+40), fontB, 1, colorYellow, 1, cv2.LINE_AA)

            cv2.imshow('Depth image', temp)
            if flagCapture is True and flagCoord is True:
                fileName = "../imageCapture/depthText_" + time.strftime("%Y-%m-%d_%H%M%S-", time.localtime()) + '.png'
                cv2.imwrite(fileName, temp, [int(cv2.IMWRITE_PNG_COMPRESSION), 0])

        # Screen capture
        procScreenCapture()
        
        # Read device temperature
        '''
        temp = depth_sensor.get_option(rs.option.projector_temperature)
        print("proj: " + str(temp))
        temp = depth_sensor.get_option(rs.option.asic_temperature)
        print("asic: " + str(temp))
        '''

        # Generate point cloud
        #procPointCloud()


        # Keyboard command
        getKey = cv2.waitKey(10) & 0xFF
        if getKey is ord('c') or getKey is ord('C'):
            flagCapture = True
        elif getKey is ord('q') or getKey is ord('Q'):
            break
        elif getKey is ord('f') or getKey is ord('F'):
            if flagCoord is True:
                flagCoord = False
            else:
                flagCoord = True
        elif getKey is ord('l') or getKey is ord('L'):
            if flagLine is True:
                flagLine = False
            else:
                flagLine = True
        elif getKey is ord('a') or getKey is ord('A'):
            flagMeasure = True
        elif getKey is ord('g') or getKey is ord('G'):
            flagGrab = True
        elif getKey is ord('h') or getKey is ord('H'):
            if flagRect is True:
                procGrabCut()
        elif getKey is ord('x') or getKey is ord('X'):
            cv2.destroyAllWindows()
            flagDispMeasure = False
            flagDispGrab = False
            flagRect = False

            if EnableColorStream is True:
                cv2.namedWindow('Color image')
                cv2.setMouseCallback('Color image', procDepthValue)

            if EnableInfraredStream is True:
                cv2.namedWindow('Infrared image')
                cv2.setMouseCallback('Infrared image', procDepthValue) 

            if EnableDepthStream is True:
                cv2.namedWindow('Depth image')
                cv2.setMouseCallback('Depth image', procDepthValue)

except Exception as e:
    print(e)
    pass

finally:
    # Stop streaming
    cv2.destroyAllWindows()
    pipeline.stop()

