import pygame
import serial
import math
import time


# ============================================================
# ARDUINO SETTINGS
# ============================================================

SERIAL_PORT = "COM6"
BAUD_RATE = 9600


# ============================================================
# SCREEN SETTINGS
# ============================================================

WIDTH = 1280
HEIGHT = 720

pygame.init()

screen = pygame.display.set_mode(
    (WIDTH, HEIGHT)
)

pygame.display.set_caption(
    "Radar System Using Arduino"
)

clock = pygame.time.Clock()


# ============================================================
# COLORS
# ============================================================

BLACK = (2, 7, 3)

GREEN = (60, 255, 45)

YELLOW = (255, 220, 30)

RED = (255, 50, 35)


# ============================================================
# FONTS
# ============================================================

radar_font = pygame.font.Font(
    None,
    32
)

angle_font = pygame.font.Font(
    None,
    30
)

distance_font = pygame.font.Font(
    None,
    28
)

status_font = pygame.font.Font(
    None,
    32
)

target_font = pygame.font.Font(
    None,
    27
)


# ============================================================
# RADAR SETTINGS
# ============================================================

RADAR_CENTER_X = 630

RADAR_CENTER_Y = 665

RADAR_RADIUS = 515

MAX_DISTANCE = 40


# ============================================================
# CURRENT SENSOR DATA
# ============================================================

current_angle = 90

current_distance = 400

current_status = "SAFE"


# ============================================================
# PERSISTENT DANGER TARGETS
# ============================================================

targets = {}


# ============================================================
# CURRENT RADAR DETECTIONS
# ============================================================

radar_detections = []


# ============================================================
# SERIAL CONNECTION
# ============================================================

try:

    arduino = serial.Serial(
        SERIAL_PORT,
        BAUD_RATE,
        timeout=0.05
    )

    time.sleep(2)

    connected = True

    print(
        "Arduino connected on",
        SERIAL_PORT
    )

except Exception as error:

    print(
        "Arduino connection failed:"
    )

    print(error)

    connected = False

    arduino = None


# ============================================================
# RADAR POSITION FUNCTION
# ============================================================

def radar_position(
    angle,
    distance
):

    distance = max(
        0,
        min(
            distance,
            MAX_DISTANCE
        )
    )

    radius = (
        distance
        / MAX_DISTANCE
    ) * RADAR_RADIUS

    angle_rad = math.radians(
        angle
    )

    x = (
        RADAR_CENTER_X
        + radius
        * math.cos(angle_rad)
    )

    y = (
        RADAR_CENTER_Y
        - radius
        * math.sin(angle_rad)
    )

    return int(x), int(y)


# ============================================================
# DRAW RADAR
# ============================================================

def draw_radar():

    # ========================================================
    # DISTANCE SEMICIRCLES
    # ========================================================

    for distance in [
        10,
        20,
        30,
        40
    ]:

        radius = int(
            RADAR_RADIUS
            * distance
            / MAX_DISTANCE
        )

        pygame.draw.arc(

            screen,

            GREEN,

            (
                RADAR_CENTER_X - radius,
                RADAR_CENTER_Y - radius,
                radius * 2,
                radius * 2
            ),

            0,

            math.pi,

            3
        )


    # ========================================================
    # ANGLE LINES
    # ========================================================

    for angle in range(
        0,
        181,
        30
    ):

        x, y = radar_position(
            angle,
            MAX_DISTANCE
        )

        pygame.draw.line(

            screen,

            GREEN,

            (
                RADAR_CENTER_X,
                RADAR_CENTER_Y
            ),

            (
                x,
                y
            ),

            2
        )


    # ========================================================
    # OUTER SEMICIRCLE
    # ========================================================

    pygame.draw.arc(

        screen,

        GREEN,

        (
            RADAR_CENTER_X - RADAR_RADIUS,
            RADAR_CENTER_Y - RADAR_RADIUS,
            RADAR_RADIUS * 2,
            RADAR_RADIUS * 2
        ),

        0,

        math.pi,

        3
    )


    # ========================================================
    # HORIZONTAL BASELINE
    # ========================================================

    pygame.draw.line(

        screen,

        GREEN,

        (
            RADAR_CENTER_X - RADAR_RADIUS,
            RADAR_CENTER_Y
        ),

        (
            RADAR_CENTER_X + RADAR_RADIUS,
            RADAR_CENTER_Y
        ),

        3
    )


    # ========================================================
    # ANGLE LABELS
    # ========================================================

    for angle in [
        0,
        30,
        60,
        90,
        120,
        150,
        180
    ]:

        x, y = radar_position(
            angle,
            MAX_DISTANCE
        )

        text = radar_font.render(
            f"{angle}°",
            True,
            GREEN
        )


        if angle == 0:

            tx = x + 5
            ty = y - 28


        elif angle == 180:

            tx = (
                x
                - text.get_width()
                - 5
            )

            ty = y - 28


        elif angle == 90:

            tx = (
                x
                - text.get_width()
                // 2
            )

            ty = y - 42


        else:

            tx = (
                x
                - text.get_width()
                // 2
            )

            ty = y - 32


        screen.blit(
            text,
            (tx, ty)
        )


    # ========================================================
    # DISTANCE LABELS
    # ========================================================

    for distance in [
        10,
        20,
        30,
        40
    ]:

        radius = int(
            RADAR_RADIUS
            * distance
            / MAX_DISTANCE
        )

        text = distance_font.render(
            f"{distance}cm",
            True,
            GREEN
        )

        tx = (
            RADAR_CENTER_X
            + radius
            - text.get_width()
            // 2
        )

        ty = (
            RADAR_CENTER_Y
            - 34
        )

        screen.blit(
            text,
            (tx, ty)
        )


# ============================================================
# DRAW SWEEP
# ============================================================

def draw_sweep(angle):

    trail = 35


    # ========================================================
    # SWEEP TRAIL
    # ========================================================

    for offset in range(
        trail,
        0,
        -1
    ):

        sweep_angle = (
            angle
            + offset
        )

        if sweep_angle > 180:

            continue

        x, y = radar_position(
            sweep_angle,
            MAX_DISTANCE
        )

        strength = (
            1.0
            - offset / trail
        )

        color = (

            int(
                5
                + 40 * strength
            ),

            int(
                55
                + 200 * strength
            ),

            int(
                8
                + 35 * strength
            )
        )

        pygame.draw.line(

            screen,

            color,

            (
                RADAR_CENTER_X,
                RADAR_CENTER_Y
            ),

            (
                x,
                y
            ),

            2
        )


    # ========================================================
    # CURRENT SWEEP LINE
    # ========================================================

    x, y = radar_position(
        angle,
        MAX_DISTANCE
    )

    pygame.draw.line(

        screen,

        GREEN,

        (
            RADAR_CENTER_X,
            RADAR_CENTER_Y
        ),

        (
            x,
            y
        ),

        4
    )


# ============================================================
# DRAW CURRENT RADAR DETECTIONS
# ============================================================

def draw_radar_detections():

    current_time = time.time()


    # ========================================================
    # REMOVE OLD DETECTIONS
    # ========================================================

    radar_detections[:] = [

        detection

        for detection in radar_detections

        if (
            current_time
            - detection["time"]
        ) < 0.8

    ]


    # ========================================================
    # DRAW DETECTIONS
    # ========================================================

    for detection in radar_detections:

        angle = detection["angle"]

        distance = detection["distance"]

        age = (
            current_time
            - detection["time"]
        )


        if distance <= 0:

            continue

        if distance > MAX_DISTANCE:

            continue


        x, y = radar_position(
            angle,
            distance
        )


        # ====================================================
        # COLOR BASED ON DISTANCE
        # ====================================================

        if distance <= 10:

            color = RED

        elif distance <= 30:

            color = YELLOW

        else:

            color = GREEN


        # ====================================================
        # FADE EFFECT
        # ====================================================

        strength = (
            1.0
            - age / 0.8
        )

        strength = max(
            0.2,
            strength
        )


        draw_color = (

            int(
                color[0]
                * strength
            ),

            int(
                color[1]
                * strength
            ),

            int(
                color[2]
                * strength
            )
        )


        # ====================================================
        # OUTER DETECTION RING
        # ====================================================

        pygame.draw.circle(

            screen,

            draw_color,

            (
                x,
                y
            ),

            9,

            2
        )


        # ====================================================
        # DETECTION CENTER DOT
        # ====================================================

        pygame.draw.circle(

            screen,

            draw_color,

            (
                x,
                y
            ),

            4
        )


# ============================================================
# DRAW PERSISTENT DANGER TARGETS
# ============================================================

def draw_targets():

    for target_id in targets:

        target = targets[
            target_id
        ]

        angle = target[
            "angle"
        ]

        distance = target[
            "distance"
        ]


        if distance <= 0:

            continue

        if distance > MAX_DISTANCE:

            continue


        x, y = radar_position(
            angle,
            distance
        )


        # ====================================================
        # DANGER COLOR
        # ====================================================

        color = RED


        # ====================================================
        # LARGE OUTER RING
        # ====================================================

        pygame.draw.circle(

            screen,

            color,

            (
                x,
                y
            ),

            14,

            2
        )


        # ====================================================
        # CENTER
        # ====================================================

        pygame.draw.circle(

            screen,

            color,

            (
                x,
                y
            ),

            5
        )


        # ====================================================
        # TARGET NUMBER
        # ====================================================

        target_text = target_font.render(

            f"T{target_id}",

            True,

            RED
        )

        screen.blit(

            target_text,

            (
                x + 12,
                y - 12
            )
        )


# ============================================================
# UPDATE STATUS
# ============================================================

def update_status(
    distance
):

    global current_status


    if distance <= 10:

        current_status = "DANGER"

    elif distance <= 30:

        current_status = "CAUTION"

    else:

        current_status = "SAFE"


# ============================================================
# DRAW TOP-LEFT STATUS BOX
# ============================================================

def draw_status_box():

    box_x = 25

    box_y = 25

    box_width = 300

    box_height = 135


    # ========================================================
    # STATUS COLOR
    # ========================================================

    if current_status == "DANGER":

        border_color = RED

    elif current_status == "CAUTION":

        border_color = YELLOW

    else:

        border_color = GREEN


    # ========================================================
    # BOX
    # ========================================================

    pygame.draw.rect(

        screen,

        BLACK,

        (
            box_x,
            box_y,
            box_width,
            box_height
        )
    )


    pygame.draw.rect(

        screen,

        border_color,

        (
            box_x,
            box_y,
            box_width,
            box_height
        ),

        3
    )


    # ========================================================
    # TITLE
    # ========================================================

    title = radar_font.render(

        "RADAR STATUS",

        True,

        GREEN
    )

    screen.blit(

        title,

        (
            box_x + 18,
            box_y + 12
        )
    )


    # ========================================================
    # OBJECT MESSAGE
    # ========================================================

    if current_distance <= 30:

        object_message = (
            "OBJECT DETECTED"
        )

    else:

        object_message = (
            "NO OBJECT"
        )


    object_text = status_font.render(

        object_message,

        True,

        border_color
    )

    screen.blit(

        object_text,

        (
            box_x + 18,
            box_y + 52
        )
    )


    # ========================================================
    # STATUS
    # ========================================================

    status_text = status_font.render(

        f"Status: {current_status}",

        True,

        border_color
    )

    screen.blit(

        status_text,

        (
            box_x + 18,
            box_y + 91
        )
    )


# ============================================================
# DRAW TOP-RIGHT DANGER TARGET BOX
# ============================================================

def draw_target_box():

    box_width = 310

    box_height = 190

    box_x = (
        WIDTH
        - box_width
        - 25
    )

    box_y = 25


    # ========================================================
    # BOX
    # ========================================================

    pygame.draw.rect(

        screen,

        BLACK,

        (
            box_x,
            box_y,
            box_width,
            box_height
        )
    )


    pygame.draw.rect(

        screen,

        RED,

        (
            box_x,
            box_y,
            box_width,
            box_height
        ),

        3
    )


    # ========================================================
    # TITLE
    # ========================================================

    title = radar_font.render(

        "DANGER TARGETS",

        True,

        RED
    )

    screen.blit(

        title,

        (
            box_x + 18,
            box_y + 12
        )
    )


    # ========================================================
    # NO ACTIVE TARGET
    # ========================================================

    if len(targets) == 0:

        no_target = target_font.render(

            "No active targets",

            True,

            GREEN
        )

        screen.blit(

            no_target,

            (
                box_x + 18,
                box_y + 60
            )
        )

        return


    # ========================================================
    # TARGET LIST
    # ========================================================

    y_position = (
        box_y + 55
    )


    for target_id in sorted(
        targets.keys()
    ):

        target = targets[
            target_id
        ]

        angle = target[
            "angle"
        ]

        distance = target[
            "distance"
        ]


        text = target_font.render(

            f"T{target_id}   "
            f"Angle: {angle}°   "
            f"Dist: {distance}cm",

            True,

            RED
        )


        screen.blit(

            text,

            (
                box_x + 18,
                y_position
            )
        )


        y_position += 30


        if y_position > (
            box_y
            + box_height
            - 25
        ):

            break


# ============================================================
# DRAW BOTTOM INFORMATION BAR
# ============================================================

def draw_bottom_information():

    bar_left = 80

    bar_right = 1180

    bar_top = (
        RADAR_CENTER_Y
        + 2
    )

    bar_bottom = 705


    # ========================================================
    # BLACK INFORMATION BAR
    # ========================================================

    pygame.draw.rect(

        screen,

        BLACK,

        (
            bar_left,
            bar_top,
            bar_right - bar_left,
            bar_bottom - bar_top
        )
    )


    # ========================================================
    # GREEN TOP BORDER
    # ========================================================

    pygame.draw.line(

        screen,

        GREEN,

        (
            bar_left,
            bar_top
        ),

        (
            bar_right,
            bar_top
        ),

        3
    )


    # ========================================================
    # RADAR LABEL
    # ========================================================

    radar_text = radar_font.render(

        "RADAR",

        True,

        GREEN
    )

    screen.blit(

        radar_text,

        (
            180,
            bar_top + 12
        )
    )


    # ========================================================
    # LIVE ANGLE
    # ========================================================

    angle_text = angle_font.render(

        f"Angle: {current_angle}°",

        True,

        GREEN
    )

    screen.blit(

        angle_text,

        (
            520,
            bar_top + 12
        )
    )


    # ========================================================
    # LIVE DISTANCE
    # ========================================================

    if current_distance >= 400:

        display_distance = "---"

    else:

        display_distance = (
            f"{current_distance} cm"
        )


    distance_text = distance_font.render(

        f"Distance: {display_distance}",

        True,

        GREEN
    )

    screen.blit(

        distance_text,

        (
            780,
            bar_top + 14
        )
    )


# ============================================================
# READ SERIAL DATA
# ============================================================

def read_serial():

    global current_angle

    global current_distance

    global radar_detections


    if not connected:

        return


    try:

        while arduino.in_waiting:

            data = (

                arduino
                .readline()
                .decode(
                    "utf-8",
                    errors="ignore"
                )
                .strip()
            )


            # =================================================
            # SCAN DATA
            # =================================================
            #
            # Arduino format:
            #
            # SCAN,90,25
            # =================================================

            if data.startswith(
                "SCAN,"
            ):

                values = data.split(",")


                if len(values) != 3:

                    continue


                try:

                    angle = int(
                        values[1]
                    )

                    distance = int(
                        values[2]
                    )

                except ValueError:

                    continue


                # =============================================
                # LIMIT ANGLE
                # =============================================

                current_angle = max(

                    0,

                    min(
                        angle,
                        180
                    )
                )


                # =============================================
                # UPDATE DISTANCE
                # =============================================

                current_distance = (
                    distance
                )


                # =============================================
                # UPDATE STATUS
                # =============================================

                update_status(
                    distance
                )


                # =============================================
                # STORE CURRENT RADAR DETECTION
                # =============================================

                if (

                    distance > 0

                    and

                    distance <= MAX_DISTANCE

                ):

                    radar_detections.append({

                        "angle": angle,

                        "distance": distance,

                        "time": time.time()

                    })


            # =================================================
            # TARGET DATA
            # =================================================
            #
            # TARGET,1,64,8,NEW
            #
            # TARGET,1,64,8,ACTIVE
            #
            # TARGET,1,64,8,LOST
            # =================================================

            elif data.startswith(
                "TARGET,"
            ):

                values = data.split(",")


                if len(values) != 5:

                    continue


                try:

                    target_id = int(
                        values[1]
                    )

                    angle = int(
                        values[2]
                    )

                    distance = int(
                        values[3]
                    )

                    state = values[4]


                except ValueError:

                    continue


                # =============================================
                # NEW TARGET
                # =============================================

                if state == "NEW":

                    targets[
                        target_id
                    ] = {

                        "angle": angle,

                        "distance": distance

                    }


                # =============================================
                # ACTIVE TARGET
                # =============================================

                elif state == "ACTIVE":

                    targets[
                        target_id
                    ] = {

                        "angle": angle,

                        "distance": distance

                    }


                # =============================================
                # LOST TARGET
                # =============================================

                elif state == "LOST":

                    if target_id in targets:

                        del targets[
                            target_id
                        ]


    except Exception:

        pass


# ============================================================
# MAIN LOOP
# ============================================================

running = True


while running:


    # ========================================================
    # HANDLE EVENTS
    # ========================================================

    for event in pygame.event.get():

        if event.type == pygame.QUIT:

            running = False


    # ========================================================
    # READ ARDUINO DATA
    # ========================================================

    read_serial()


    # ========================================================
    # CLEAR SCREEN
    # ========================================================

    screen.fill(
        BLACK
    )


    # ========================================================
    # DRAW RADAR
    # ========================================================

    draw_radar()


    # ========================================================
    # DRAW SWEEP
    # ========================================================

    draw_sweep(
        current_angle
    )


    # ========================================================
    # DRAW NORMAL DETECTION DOTS
    # ========================================================

    draw_radar_detections()


    # ========================================================
    # DRAW PERSISTENT DANGER TARGETS
    # ========================================================

    draw_targets()


    # ========================================================
    # DRAW TOP-LEFT STATUS
    # ========================================================

    draw_status_box()


    # ========================================================
    # DRAW TOP-RIGHT DANGER TARGETS
    # ========================================================

    draw_target_box()


    # ========================================================
    # DRAW BOTTOM INFORMATION
    # ========================================================

    draw_bottom_information()


    # ========================================================
    # UPDATE DISPLAY
    # ========================================================

    pygame.display.flip()

    clock.tick(60)


# ============================================================
# CLOSE EVERYTHING
# ============================================================

if arduino:

    arduino.close()

pygame.quit()