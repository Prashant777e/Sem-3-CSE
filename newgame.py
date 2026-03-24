import pygame
import random
import sys
from game_config import colors
pygame.init()
screen_width = 800
screen_height = 600
screen = pygame.display.set_mode((screen_width, screen_height))
pygame.display.set_caption("Hand Control Dodger")
clock = pygame.time.Clock()


player_img = pygame.image.load('assets/aircraft.png').convert_alpha()
obstacle_img = pygame.image.load('assets/asteroid.png').convert_alpha()
background_img = pygame.image.load('assets/space_bg.png').convert_alpha()
background_img = pygame.transform.scale(background_img, (screen_width, screen_height))


player_img = pygame.transform.scale(player_img, (60, 60)) 
obstacle_img = pygame.transform.scale(obstacle_img, (50, 50))  
try:
    from hand_controller import HandController
    hand_controller = HandController()
    print(" Hand Controller working")
except ImportError:
    print(" Hand Controller not working.")
    pygame.quit()
    sys.exit()

def draw_rectangle(color, x, y, width, height):
    pygame.draw.rect(screen, color, (x, y, width, height))

def show_text(text, x, y, color=colors["WHITE"]):
    font = pygame.font.Font(None, 36)
    text_surface = font.render(text, True, color)
    screen.blit(text_surface, (x, y))

def game_loop():
    player_x = 400
    player_y = 500
    player_width = 60
    player_height = 60
    player_speed = 12
    
    # Game variables
    obstacles = []
    score = 0
    game_over = False
    
    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            
            if game_over and event.type == pygame.KEYDOWN:
                if event.key == pygame.K_r:
                    return True  
                if event.key == pygame.K_q:
                    return False  
        
        if not game_over:
            gesture = hand_controller.get_gesture()
            
            if gesture == "quit":
                running = False
            elif gesture == "left":
                player_x -= player_speed
            elif gesture == "right":
                player_x += player_speed
            
            if player_x < 0:
                player_x = 0
            if player_x > screen_width - player_width:
                player_x = screen_width - player_width
            
            if random.random() < 0.02:
                obstacle_width = random.randint(30, 70)
                obstacle_height = random.randint(30, 70)
                obstacle_x = random.randint(0, screen_width - obstacle_width)
                
                new_obstacle = {
                    'x': obstacle_x,
                    'y': -50,
                    'width': obstacle_width,
                    'height': obstacle_height
                }
                obstacles.append(new_obstacle)
            
            for obstacle in obstacles[:]:
                obstacle['y'] += 4
                
                if obstacle['y'] > screen_height:
                    obstacles.remove(obstacle)
                    score += 1
                
                player_rect = pygame.Rect(player_x, player_y, player_width, player_height)
                obstacle_rect = pygame.Rect(obstacle['x'], obstacle['y'], obstacle['width'], obstacle['height'])
                
                if player_rect.colliderect(obstacle_rect):
                    game_over = True
        
        # Draw background image instead of blue fill
        screen.blit(background_img, (0, 0))
        
        # Draw player image instead of green rectangle
        screen.blit(player_img, (player_x, player_y))
        
        # Draw obstacle images instead of red rectangles
        for obstacle in obstacles:
            screen.blit(obstacle_img, (obstacle['x'], obstacle['y']))
        
        show_text(f"Score: {score}", 20, 20)
        show_text("Control: Hand Gestures", 20, 60)
        
        if game_over:
            overlay = pygame.Surface((screen_width, screen_height))
            overlay.set_alpha(180)
            overlay.fill(colors["BLACK"])
            screen.blit(overlay, (0, 0))
            show_text("GAME OVER", 300, 250, colors["YELLOW"])
            show_text(f"Final Score: {score}", 300, 300)
            show_text("Press R to Restart", 280, 350, colors["GREEN"])
            show_text("Press Q to Quit", 300, 400, colors["RED"])
        
        pygame.display.flip()
        clock.tick(60)
    
    return False

def main():
    restart = True
    while restart:
        restart = game_loop()
    
    hand_controller.cleanup()
    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()
