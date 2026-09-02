import serial
import time

# ==============================================================================
#                       SENSOR CONFIGURATION PARAMETERS
#  (Change the values on the right side of '=' according to the comments)
# ==============================================================================

# 1. CONNECTION
# ------------------------------------------------------------------------------
PORT = '/dev/ttyUSB0'  # Serial port (Windows: 'COM3', 'COM4' | Linux: '/dev/ttyUSB0')
BAUD = 115200          # Communication speed (default: 115200)


# 2. MAIN DETECTION PARAMETERS
# ------------------------------------------------------------------------------
# MAXIMUM DETECTION DISTANCE (in meters):
# • Allowed values: from 0.2 to 10.5 m (0.1 m step)
# • Effect: the sensor completely ignores movement and presence beyond this distance.
# • Examples: 0.4 (for a 40 cm sink), 3.0 (workplace), 7.5 (large room).
MAX_DISTANCE_METERS = 0.4

# ABSENCE DETECTION DELAY (in seconds):
# • Allowed values: from 0 to 65535.
# • Effect: how many seconds the "presence detected" signal is maintained
#   after a person leaves the detection zone.
# • Examples: 0 (turn off immediately / faucet), 5 (corridor lighting),
#   30-60 (presence detection in a room).
ABSENCE_DELAY_SECONDS = 0

# MOTION DETECTION SENSITIVITY (Motion coefficient):
# • Allowed values: from 10 to 500 (default: 60).
# • Effect: REVERSE LOGIC — the higher the value, the lower the motion sensitivity.
#   If the sensor reacts to drafts or vibrations, increase this value.
MOTION_COEFFICIENT = 60

# MOUNTING TYPE (Sensor installation method):
# • Allowed values:
#     1 — Wall-mounted (sensor installed vertically on a wall)
#     0 — Ceiling-mounted (sensor installed on the ceiling and facing downward)
# • Effect: the radar optimizes its scanning algorithms according to the
#   geometry of the installation area.
MOUNT_TYPE = 1


# 3. STATIC / BREATHING SENSITIVITY THRESHOLDS (GATES 0..15)
# ------------------------------------------------------------------------------
# Each Gate corresponds approximately to a distance zone of ~0.5–0.75 m.
# • Measurement unit: dB (from 0.0 to 96.3 dB).
# • Effect: energy threshold used for detecting micro-movements
#   (breathing, sitting, etc.).
# • HIGHER dB value = HARDER to trigger (lower sensitivity).
# • LOWER dB value = HIGHER sensitivity (may react to vibrations).
# • Gate 00: the zone closest to the sensor (approximately 0–40/70 cm).
GATE_THRESHOLDS_DB = [
    48.0,  # Gate 00: 0 - 0.5 m
    44.0,  # Gate 01: 0.5 - 1.0 m
    35.0,  # Gate 02: 1.0 - 1.5 m
    30.0,  # Gate 03: 1.5 - 2.0 m
    27.0,  # Gate 04: 2.0 - 2.5 m
    27.0,  # Gate 05: 2.5 - 3.0 m
    27.0,  # Gate 06: 3.0 - 3.5 m
    27.0,  # Gate 07: 3.5 - 4.0 m
    27.0,  # Gate 08: 4.0 - 4.5 m
    27.0,  # Gate 09: 4.5 - 5.0 m
    26.0,  # Gate 10: 5.0 - 5.5 m
    26.0,  # Gate 11: 5.5 - 6.0 m
    26.0,  # Gate 12: 6.0 - 6.5 m
    26.0,  # Gate 13: 6.5 - 7.0 m
    26.0,  # Gate 14: 7.0 - 7.5 m
    26.0   # Gate 15: 7.5 - 8.0 m
]

# ==============================================================================
#                       SYSTEM CODE (DO NOT MODIFY BELOW)
# ==============================================================================

def send_packet(ser, payload):
    header = bytes([0xFD, 0xFC, 0xFB, 0xFA])
    footer = bytes([0x04, 0x03, 0x02, 0x01])
    length = len(payload).to_bytes(2, byteorder='little')
    
    packet = header + length + payload + footer
    ser.write(packet)
    time.sleep(0.15)
    return ser.read_all()

def main():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        print(f"Connected to port {PORT}")

        # 1. Enable configuration mode
        send_packet(ser, bytes([0xFF, 0x00, 0x01, 0x00]))
        print("[1/3] Configuration mode activated.")

        # 2. Build the 0xA5 configuration command using the specified parameters
        cmd = bytearray()
        cmd.extend([0xA5, 0x00])  # Configuration command code

        # Convert distance to 0.1 m units
        raw_dist = int(round(MAX_DISTANCE_METERS * 10))
        cmd.extend(raw_dist.to_bytes(2, byteorder='little'))

        # Absence delay in seconds
        cmd.extend(int(ABSENCE_DELAY_SECONDS).to_bytes(2, byteorder='little'))

        # Motion detection coefficient
        cmd.extend(int(MOTION_COEFFICIENT).to_bytes(2, byteorder='little'))

        # Mounting type (0 or 1)
        cmd.extend(int(MOUNT_TYPE).to_bytes(2, byteorder='little'))

        # Convert all 16 dB thresholds to 0.1 dB units
        if len(GATE_THRESHOLDS_DB) != 16:
            raise ValueError("GATE_THRESHOLDS_DB must contain exactly 16 values!")
            
        for db_val in GATE_THRESHOLDS_DB:
            raw_db = int(round(db_val * 10))
            cmd.extend(raw_db.to_bytes(2, byteorder='little'))

        # Send the configuration packet
        send_packet(ser, cmd)
        print(f"[2/3] Parameters sent:")
        print(f"      • Max. distance : {MAX_DISTANCE_METERS} m")
        print(f"      • Absence delay : {ABSENCE_DELAY_SECONDS} sec")
        print(f"      • Motion coeff. : {MOTION_COEFFICIENT}")
        print(f"      • Mounting      : {'Wall-mounted' if MOUNT_TYPE == 1 else 'Ceiling-mounted'}")

        # 3. Exit configuration mode (save settings to Flash memory)
        send_packet(ser, bytes([0xFE, 0x00]))
        print("[3/3] Parameters saved to radar Flash memory!")

        ser.close()
        print("\nDone. The sensor has been switched to normal operating mode.")

    except Exception as e:
        print(f"\n[ERROR]: {e}")

if __name__ == '__main__':
    main()
