import pygame
import random
import os
from hand_controller import HandController
from game_config import colors  # NEW: Import colors from config


pygame.init()


SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600
screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
pygame.display.set_caption("RUN AWAY RUSH")

WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
RED = (255, 0, 0)
GREEN = (0, 255, 0)
BLUE = (0, 120, 255)
GRAY = (100, 100, 100)
YELLOW = (255, 255, 0)
TRACK_COLOR = (60, 60, 60)

FPS = 60
clock = pygame.time.Clock()

RUNNER_WIDTH = 60
RUNNER_HEIGHT = 100
GRAVITY = 1.0
JUMP_STRENGTH = -18


LANES = [200, 400, 600]


obstacles = []
BASE_OBSTACLE_SPEED = 8
obstacle_speed = BASE_OBSTACLE_SPEED
obstacle_timer = 0
OBSTACLE_SPAWN_INTERVAL = 50


track_offset = 0
BASE_TRACK_SPEED = 8
track_speed = BASE_TRACK_SPEED


score = 0
game_state = "menu"
current_gesture = "none"
last_speed_increase_score = 0


elapsed_time = 0  


font = pygame.font.SysFont("Arial", 28)
big_font = pygame.font.SysFont("Arial", 48)


player_x = LANES[1]
player_y = SCREEN_HEIGHT - RUNNER_HEIGHT - 20
player_vel_y = 0
player_jumping = False
player_crouching = False
player_lane = 1


def load_image(filename, width, height):
    try:
        image = pygame.image.load(f"assets/{filename}")
        image = pygame.transform.scale(image, (width, height))
        return image
    except:
        print(f"Could not load image: {filename}")
        return None


player_normal_img = load_image("player_normal.png", RUNNER_WIDTH, RUNNER_HEIGHT)
player_jump_img = load_image("player_jumping.png", RUNNER_WIDTH, RUNNER_HEIGHT)
player_crouch_img = load_image("player_crouching.png", RUNNER_WIDTH, RUNNER_HEIGHT//2)

box_img = load_image("box.png", 60, 60)
barrier_img = load_image("barrier.png", 80, 80)
hole_img = load_image("hole.png", 100, 40)
train_img = load_image("train.png", 140, 120)
low_barrier_img = load_image("low_barrier.png", 70, 35)


track_img = load_image("track.png", 160, 40)
use_track_image = track_img is not None


try:
    hand_controller = HandController()
    print("Hand controller started successfully!")
except Exception as e:
    print(f"Error starting hand controller: {e}")
    hand_controller = None

def update_player():
    global player_y, player_vel_y, player_jumping, player_x
    
    # Only update player if game is playing
    if game_state != "playing":
        return
    
    if player_jumping:
        player_vel_y += GRAVITY
        player_y += player_vel_y
    
   
    if player_jumping and player_y >= SCREEN_HEIGHT - RUNNER_HEIGHT - 20:
        player_y = SCREEN_HEIGHT - RUNNER_HEIGHT - 20
        player_vel_y = 0
        player_jumping = False
    
    
    target_x = LANES[player_lane]
    player_x += (target_x - player_x) * 0.2

def draw_player():
    if player_crouching and player_crouch_img:
        screen.blit(player_crouch_img, (player_x - RUNNER_WIDTH//2, player_y + RUNNER_HEIGHT//2))
    elif player_jumping and player_jump_img:
        screen.blit(player_jump_img, (player_x - RUNNER_WIDTH//2, player_y))
    elif player_normal_img:
        screen.blit(player_normal_img, (player_x - RUNNER_WIDTH//2, player_y))
    else:
        
        color = BLUE if not player_jumping else (100, 200, 255)
        height = RUNNER_HEIGHT if not player_crouching else RUNNER_HEIGHT//2
        y_pos = player_y if not player_crouching else player_y + RUNNER_HEIGHT//2
        pygame.draw.rect(screen, color, (player_x - RUNNER_WIDTH//2, y_pos, RUNNER_WIDTH, height))
        pygame.draw.rect(screen, WHITE, (player_x - RUNNER_WIDTH//2, y_pos, RUNNER_WIDTH, height), 2)

def create_obstacle():
    # Only create obstacles if game is playing
    if game_state != "playing":
        return
        
    lane = random.randint(0, 2)
    obstacle_type = random.choice(['box', 'barrier', 'hole', 'train', 'low_barrier'])
    
    if obstacle_type == 'box':
        width, height = 60, 60
        image = box_img
    elif obstacle_type == 'barrier':
        width, height = 80, 80
        image = barrier_img
    elif obstacle_type == 'hole':
        width, height = 100, 40
        image = hole_img
    elif obstacle_type == 'train':
        width, height = 140, 120
        image = train_img
    else:
        width, height = 70, 35
        image = low_barrier_img
    
    obstacles.append({
        'x': LANES[lane],
        'y': -height,
        'width': width,
        'height': height,
        'type': obstacle_type,
        'image': image,
        'lane': lane,
        'speed': obstacle_speed
    })

def draw_obstacles():
    for obstacle in obstacles:
        if obstacle['image']:
            screen.blit(obstacle['image'], 
                       (obstacle['x'] - obstacle['width']//2, obstacle['y']))
        else:

            if obstacle['type'] == 'box':
                pygame.draw.rect(screen, RED, (obstacle['x'] - obstacle['width']//2, obstacle['y'], obstacle['width'], obstacle['height']))
            elif obstacle['type'] == 'barrier':
                pygame.draw.rect(screen, (200, 100, 0), (obstacle['x'] - obstacle['width']//2, obstacle['y'], obstacle['width'], obstacle['height']))
            elif obstacle['type'] == 'hole':
                pygame.draw.rect(screen, BLACK, (obstacle['x'] - obstacle['width']//2, obstacle['y'], obstacle['width'], obstacle['height']))
            elif obstacle['type'] == 'train':
                pygame.draw.rect(screen, (60, 60, 60), (obstacle['x'] - obstacle['width']//2, obstacle['y'], obstacle['width'], obstacle['height']))
            else:  
                pygame.draw.rect(screen, (255, 165, 0), (obstacle['x'] - obstacle['width']//2, obstacle['y'], obstacle['width'], obstacle['height']))

def update_obstacles():
    global obstacles
    
    # Only update obstacles if game is playing
    if game_state != "playing":
        return
    
    for obstacle in obstacles[:]:
        obstacle['y'] += obstacle['speed']
        if obstacle['y'] > SCREEN_HEIGHT + 50:
            obstacles.remove(obstacle)
            

def check_collisions():
    # Only check collisions if game is playing
    if game_state != "playing":
        return False
        
    player_height_temp = RUNNER_HEIGHT if not player_crouching else RUNNER_HEIGHT//2
    player_y_temp = player_y if not player_crouching else player_y + RUNNER_HEIGHT//2
    
    player_rect = pygame.Rect(player_x - RUNNER_WIDTH//2, player_y_temp, RUNNER_WIDTH, player_height_temp)
    
    for obstacle in obstacles:
        obstacle_rect = pygame.Rect(obstacle['x'] - obstacle['width']//2, 
                                  obstacle['y'], 
                                  obstacle['width'], 
                                  obstacle['height'])
        
        if player_rect.colliderect(obstacle_rect):
            if obstacle['type'] == 'hole':
                if not player_jumping:  
                    return True
            elif obstacle['type'] == 'low_barrier':
                if not player_crouching:  
                    return True
            else:
                return True  
    return False

def draw_tracks():
    global track_offset
    
    # Only update track offset if game is playing
    if game_state == "playing":
        track_offset = (track_offset + track_speed) % 40
    
    for lane in LANES:
        if use_track_image:
            for y in range(-40, SCREEN_HEIGHT + 40, 40):
                track_y = y + track_offset
                track_rect = track_img.get_rect(topleft=(lane - 80, track_y))
                screen.blit(track_img, track_rect)
        else:
            pygame.draw.rect(screen, TRACK_COLOR, (lane - 80, 0, 160, SCREEN_HEIGHT))
            
            for y in range(-40, SCREEN_HEIGHT + 40, 40):
                line_y = y + track_offset
                pygame.draw.rect(screen, YELLOW, (lane - 60, line_y, 120, 10))
        
        pygame.draw.rect(screen, (150, 150, 150), (lane - 80, 0, 10, SCREEN_HEIGHT))
        pygame.draw.rect(screen, (150, 150, 150), (lane + 70, 0, 10, SCREEN_HEIGHT))

def draw_score():
    score_text = font.render(f"Score: {score}", True, WHITE)
    gesture_text = font.render(f"Gesture: {current_gesture}", True, YELLOW)
    speed_text = font.render(f"Speed: {obstacle_speed:.1f}", True, GREEN)
    
    screen.blit(score_text, (20, 20))
    screen.blit(gesture_text, (SCREEN_WIDTH - 200, 20))
    screen.blit(speed_text, (20, 60))

def draw_menu():
    overlay = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT), pygame.SRCALPHA)
    overlay.fill((0, 0, 0, 200))  # Semi-transparent black
    screen.blit(overlay, (0, 0))
    
    title = big_font.render("SUBWAY RUNNER", True, YELLOW)
    controls = font.render("Hand Controls:", True, WHITE)
    control1 = font.render("Move Palm UP = Jump", True, GREEN)
    control2 = font.render("Move Palm DOWN = Crouch", True, GREEN)
    control3 = font.render("Move Palm LEFT = Move Left", True, GREEN)
    control4 = font.render("Move Palm RIGHT = Move Right", True, GREEN)
    start = font.render("Press SPACE to Start", True, YELLOW)
    
    screen.blit(title, (SCREEN_WIDTH//2 - title.get_width()//2, 100))
    screen.blit(controls, (SCREEN_WIDTH//2 - controls.get_width()//2, 200))
    screen.blit(control1, (SCREEN_WIDTH//2 - control1.get_width()//2, 250))
    screen.blit(control2, (SCREEN_WIDTH//2 - control2.get_width()//2, 290))
    screen.blit(control3, (SCREEN_WIDTH//2 - control3.get_width()//2, 330))
    screen.blit(control4, (SCREEN_WIDTH//2 - control4.get_width()//2, 370))
    screen.blit(start, (SCREEN_WIDTH//2 - start.get_width()//2, 450))

def draw_game_over():
    overlay = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT))
    overlay.fill(BLACK)  # NEW: Solid black instead of transparent
    screen.blit(overlay, (0, 0))
    
    game_over = big_font.render("GAME OVER", True, RED)
    score_text = font.render(f"Final Score: {score}", True, WHITE)
    speed_text = font.render(f"Final Speed: {obstacle_speed:.1f}", True, GREEN)
    restart = font.render("Press R to Restart or Q to Quit", True, GREEN)
    
    screen.blit(game_over, (SCREEN_WIDTH//2 - game_over.get_width()//2, 150))
    screen.blit(score_text, (SCREEN_WIDTH//2 - score_text.get_width()//2, 220))
    screen.blit(speed_text, (SCREEN_WIDTH//2 - speed_text.get_width()//2, 260))
    screen.blit(restart, (SCREEN_WIDTH//2 - restart.get_width()//2, 320))

def reset_game():
    global player_x, player_y, player_vel_y, player_jumping, player_crouching, player_lane
    global obstacles, score, obstacle_speed, obstacle_timer, track_offset, track_speed
    global last_speed_increase_score, elapsed_time  # NEW: Added elapsed_time
    
    player_x = LANES[1]
    player_y = SCREEN_HEIGHT - RUNNER_HEIGHT - 20
    player_vel_y = 0
    player_jumping = False
    player_crouching = False
    player_lane = 1
    obstacles = []
    score = 0
    obstacle_speed = BASE_OBSTACLE_SPEED
    track_speed = BASE_TRACK_SPEED
    obstacle_timer = 0
    track_offset = 0
    last_speed_increase_score = 0
    elapsed_time = 0  # NEW: Reset scoring timer

def handle_gesture(gesture):
    global current_gesture, player_jumping, player_crouching, player_lane, player_vel_y
    
    current_gesture = gesture
    
    if game_state != "playing":
        return
    
    if gesture == "jump" and not player_jumping and not player_crouching:
        player_vel_y = JUMP_STRENGTH
        player_jumping = True
        player_crouching = False
        print("JUMP!")
    elif gesture == "crouch" and not player_jumping:
        player_crouching = True
    elif gesture == "left" and player_lane > 0:
        player_lane -= 1
    elif gesture == "right" and player_lane < 2:
        player_lane += 1
    elif gesture == "none":
        player_crouching = False


def update_score():
    global score, elapsed_time
    
    # Only update score if game is playing
    if game_state != "playing":
        return
        
    elapsed_time += 1/FPS  
    if elapsed_time >= 0.1:  
        points_to_add = int(obstacle_speed * 2)  
        score += points_to_add
        elapsed_time = 0  

def increase_speed():
    global obstacle_speed, track_speed, last_speed_increase_score
    
    # Only increase speed if game is playing
    if game_state != "playing":
        return
        
    if score > 0 and score % 50 == 0 and score != last_speed_increase_score:
        obstacle_speed += 1.0
        track_speed += 1.0
        last_speed_increase_score = score
        print(f" Speed increased to: {obstacle_speed}")

# Main game loop
running = True
print("Game started! Press SPACE to begin...")

while running:
    clock.tick(FPS)
    
    # Get hand gesture from  controller
    if hand_controller:
        gesture = hand_controller.get_gesture()
        if gesture == "quit":
            running = False
    else:
        gesture = "none"
    
    
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_SPACE and game_state == "menu":
                game_state = "playing"
                reset_game()
                print("Game started! Use hand gestures to control.")
            elif event.key == pygame.K_r and game_state == "game_over":
                game_state = "playing"
                reset_game()
            elif event.key == pygame.K_q:
                running = False
    
    
    handle_gesture(gesture)
    
    if game_state == "playing":
        
        update_player()
        
        
        update_score()
        
   
        obstacle_timer += 1
        if obstacle_timer >= OBSTACLE_SPAWN_INTERVAL:
            create_obstacle()
            obstacle_timer = 0
        
       
        update_obstacles()
        
        # Increase speed
        increase_speed()
        
        
        if check_collisions():
            game_state = "game_over"
            print("Game Over! Collision detected.")
    
    
    # Draw everything
    screen.fill(GRAY)
    draw_tracks()
    draw_obstacles()
    draw_player()
    draw_score()
    
    if game_state == "menu":
        draw_menu()
    elif game_state == "game_over":
        draw_game_over()  # This will now completely cover the game
    
    pygame.display.flip()


if hand_controller:
    hand_controller.cleanup()
pygame.quit()
