# Constants
RPM = 9
STEPS_PER_REV = 2048

# Shared
steps_per_min = RPM * STEPS_PER_REV  # 18,432
steps_per_sec = steps_per_min / 60  # 307.2

# Rail
travel_cm = 10  # Measured with calipers
steps_per_cm = 313  # Averaged over long travel
steps = travel_cm * steps_per_cm  # 3130
rail_secs = steps / steps_per_sec  # 10.19

# Actuator
travel_cm = 9.3  # Measured with calipers
steps_per_cm = 376  # Averaged over long travel
steps = travel_cm * steps_per_cm  # 3496.8
act_steps = steps / steps_per_sec  # 11.38
act_steps *= 2  # 22.76 - Advance & Retract == *2

print(f"Exp Rail: {rail_secs} secs")
print(f"Exp Actuator: {act_steps} secs")
print(f"Exp Total: {rail_secs + act_steps} secs")

# Exp Rail: 10.188802083333334 secs
# Exp Actuator: 22.765625000000004 secs
# Exp Total: 32.954427083333336 secs
