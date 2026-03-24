import cv2
import mediapipe as mp
import numpy as np

class HandController:
    def __init__(self):
        self.cap = cv2.VideoCapture(0)
        self.mp_hands = mp.solutions.hands
        self.hands = self.mp_hands.Hands(
            static_image_mode=False,
            max_num_hands=1,
            min_detection_confidence=0.7,
            min_tracking_confidence=0.5
        )
        self.mp_draw = mp.solutions.drawing_utils
        
    def get_gesture(self):
        success, img = self.cap.read()
        if not success:
            print("❌ Cannot access camera")
            return "none"
            
        img = cv2.flip(img, 1)
        img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        results = self.hands.process(img_rgb)
        
        gesture = "none"
        
        # Get camera center
        h, w, c = img.shape
        center_x, center_y = w // 2, h // 2
        
        # Draw center point and crosshair
        cv2.circle(img, (center_x, center_y), 8, (255, 255, 255), -1)
        cv2.line(img, (center_x - 20, center_y), (center_x + 20, center_y), (255, 255, 255), 2)
        cv2.line(img, (center_x, center_y - 20), (center_x, center_y + 20), (255, 255, 255), 2)
        
        # Draw zones
        cv2.putText(img, "JUMP", (center_x - 30, center_y - 40), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        cv2.putText(img, "CROUCH", (center_x - 35, center_y + 60), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        cv2.putText(img, "LEFT", (center_x - 80, center_y + 10), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        cv2.putText(img, "RIGHT", (center_x + 40, center_y + 10), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        
        if results.multi_hand_landmarks:
            for hand_landmarks in results.multi_hand_landmarks:
                # Draw all hand landmarks
                self.mp_draw.draw_landmarks(img, hand_landmarks, self.mp_hands.HAND_CONNECTIONS)
                
                landmarks = hand_landmarks.landmark
                
                # Get palm center (average of wrist and palm base landmarks)
                wrist = landmarks[0]
                middle_mcp = landmarks[9]  # Middle finger base (palm center)
                
                # Calculate palm center position
                palm_x = (wrist.x + middle_mcp.x) / 2
                palm_y = (wrist.y + middle_mcp.y) / 2
                
                # Convert to pixel coordinates
                palm_pixel_x = int(palm_x * w)
                palm_pixel_y = int(palm_y * h)
                
                # Draw palm center point
                cv2.circle(img, (palm_pixel_x, palm_pixel_y), 12, (0, 255, 255), -1)
                cv2.circle(img, (palm_pixel_x, palm_pixel_y), 12, (255, 0, 0), 2)
                
                # Draw line from center to palm
                cv2.line(img, (center_x, center_y), (palm_pixel_x, palm_pixel_y), (0, 255, 255), 2)
                
                # Calculate distance from center
                dx = palm_pixel_x - center_x
                dy = palm_pixel_y - center_y
                
                # Define gesture zones (adjust these values as needed)
                zone_threshold = 80  # Distance threshold from center
                
                # Determine gesture based on palm position relative to center
                if abs(dx) > abs(dy):  # Horizontal movement dominant
                    if dx > zone_threshold:
                        gesture = "right"
                    elif dx < -zone_threshold:
                        gesture = "left"
                else:  # Vertical movement dominant
                    if dy < -zone_threshold:
                        gesture = "jump"
                    elif dy > zone_threshold:
                        gesture = "crouch"
                
                # Display current gesture
                cv2.putText(img, f"GESTURE: {gesture.upper()}", (50, 50), 
                          cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)
                
                # Display palm position info
                cv2.putText(img, f"Palm X: {dx:+04d}", (50, 90), 
                          cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 1)
                cv2.putText(img, f"Palm Y: {dy:+04d}", (50, 120), 
                          cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 1)
        
        # Display instructions
        cv2.putText(img, "Move PALM relative to center:", (10, h - 100), 
                  cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
        cv2.putText(img, "UP = Jump, DOWN = Crouch", (10, h - 70), 
                  cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
        cv2.putText(img, "LEFT/RIGHT = Move", (10, h - 40), 
                  cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
        
        cv2.imshow("Hand Controller - Move PALM around center point", img)
        
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            return "quit"
        
        return gesture
    
    def cleanup(self):
        self.cap.release()
        cv2.destroyAllWindows()
