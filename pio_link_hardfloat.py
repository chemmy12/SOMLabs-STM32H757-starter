# pio_link_hardfloat.py
from SCons.Script import Import
Import("env")

# Ensure the final link uses hard-float & M7 settings
env.Append(
    LINKFLAGS=[
        "-mcpu=cortex-m7",
        "-mthumb",
        "-mfpu=fpv5-d16",
        "-mfloat-abi=hard",
    ]
)
