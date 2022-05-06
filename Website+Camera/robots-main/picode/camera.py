import cv2
import numpy as np
  

def motion_detect(debug = False, threshold = 5):
    frame_count = 0
    motions_detected = 0
    previous_frame = None  
    # define a video capture object
    vid = cv2.VideoCapture(0)

    while(True):
        frame_count += 1
        if(frame_count > 300):
            vid.release()
            cv2.destroyAllWindows()
            return False
        ret, frame = vid.read()

        # 2. Prepare image; grayscale and blur
        prepared_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        prepared_frame = cv2.GaussianBlur(src=prepared_frame, ksize=(5, 5), sigmaX=0)

        # 2. Calculate the difference
        if (previous_frame is None):
            # First frame; there is no previous one yet
            previous_frame = prepared_frame
            continue

        # 3. Set previous frame and continue if there is None
        if (previous_frame is None):
            # First frame; there is no previous one yet
            previous_frame = prepared_frame
            continue

        # calculate difference and update previous frame
        diff_frame = cv2.absdiff(src1=previous_frame, src2=prepared_frame)
        previous_frame = prepared_frame

        # 4. Dilute the image a bit to make differences more seeable; more suitable for contour detection
        kernel = np.ones((5, 5))
        diff_frame = cv2.dilate(diff_frame, kernel, 1)

        # 5. Only take different areas that are different enough (>20 / 255)
        thresh_frame = cv2.threshold(src=diff_frame, thresh=20, maxval=255, type=cv2.THRESH_BINARY)[1]

        # 6. Find and optionally draw contours
        contours, _ = cv2.findContours(image=thresh_frame, mode=cv2.RETR_EXTERNAL, method=cv2.CHAIN_APPROX_SIMPLE)
        # Comment below to stop drawing contours
        if len(contours) > 5:
            motions_detected += 1
            if motions_detected >= threshold:
                vid.release()
                cv2.destroyAllWindows()
                return True


        cv2.drawContours(image=frame, contours=contours, contourIdx=-1, color=(0, 255, 0), thickness=2, lineType=cv2.LINE_AA)
        # Uncomment 6 lines below to stop drawing rectangles
        # for contour in contours:
        #   if cv2.contourArea(contour) < 50:
        #     # too small: skip!
        #       continue
        #   (x, y, w, h) = cv2.boundingRect(contour)
        #   cv2.rectangle(img=img_rgb, pt1=(x, y), pt2=(x + w, y + h), color=(0, 255, 0), thickness=2)
        if debug:
            cv2.imshow('frame', frame)            
            # the 'q' button is set as the
            # quitting button you may use any
            # desired button of your choice
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
        
    # After the loop release the cap object
    vid.release()
    # Destroy all the windows
    cv2.destroyAllWindows()

if __name__ == '__main__':
    is_detected = motion_detect()
    print(is_detected)